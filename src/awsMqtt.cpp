#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <mbed_mktime.h>
#include "arduino_secrets.h"
#include "rddlSDKAPI.h"


// Enter your sensitive data in arduino_secrets.h
char SECRET_CERTIFICATE[2048];
constexpr char broker[] { SECRET_BROKER };
constexpr unsigned port { SECRET_PORT };
const char* certificate { TEST_CERTIFICATE };


#include <WiFi.h>
#include <WiFiUdp.h>

WiFiClient tcpClient;
WiFiUDP NTPUdp;

NTPClient timeClient(NTPUdp);
BearSSLClient sslClient(tcpClient);
MqttClient mqttClient(sslClient);


void setNtpTime()
{
    timeClient.forceUpdate();
    const auto epoch = timeClient.getEpochTime();
    set_time(epoch);
}

unsigned long getTime()
{
    const auto now = time(nullptr);
    return now;
}

void onMessageReceived(int messageSize)
{
    // we received a message, print out the topic and contents
    Serial.println();
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    /*
    // Message from AWS MQTT Test Client
    {
      "message": "Hello from AWS IoT console"
    }
    */

    char bytes[messageSize] {};
    for (int i = 0; i < messageSize; i++)
        bytes[i] = mqttClient.read();

    JSONVar jsonMessage = JSON.parse(bytes);
    auto text = jsonMessage["message"];

    Serial.print("[");
    Serial.print(time(nullptr));
    Serial.print("] ");
    Serial.print("Message: ");
    Serial.println(text);

    Serial.println();
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the received signal strength:
    Serial.print("signal strength (RSSI):");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println();

    // print your board's IP address:
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Local GW: ");
    Serial.println(WiFi.gatewayIP());
    Serial.println();
}


bool checkNetwork(){
    int status  = WiFi.status();

    if(status != WL_CONNECTED) {
        return false;
    }

    return true;
}


bool connectWifi(){
    int status  = WL_IDLE_STATUS;
    int cnt{0};
    deviceInfo devInfo;

    sdkReadFile("devInfo", (uint8_t*)&devInfo.ssid[0], sizeof(deviceInfo));  
    while (status != WL_CONNECTED && cnt < 5) {
        Serial.print("- Attempting to connect to WPA SSID: ");
        Serial.println(devInfo.ssid);
        status = WiFi.begin(devInfo.ssid, devInfo.pass);
        delay(500);
        cnt++;
    }

    if(cnt >= 5){
        Serial.print("Coulndt Connect WIFI!");
        return false;
    }

    printWifiStatus();
    return true;
}

void wifi_setup(){
    // Wait for Serial Monitor or start after 2.5s
    for (const auto startNow = millis() + 2500; !Serial && millis() < startNow; delay(250));
    // Attempt Wi-Fi connection
    if(!connectWifi())
        return;

    timeClient.begin();
    setNtpTime();

    if (!ECCX08.begin()) {
        Serial.println("No ECCX08 present!");
        while (1)
            ;
    }

    // Configure TLS to use HSM and the key/certificate pair
    ArduinoBearSSL.onGetTime(getTime);
}


void awsMqttSetup()
{
    // Check for HSM
    if (!ECCX08.begin()) {
        Serial.println("No ECCX08 present!");
        while (1)
            ;
    }

    // Configure TLS to use HSM and the key/certificate pair
    ArduinoBearSSL.onGetTime(getTime);
    sslClient.setEccSlot(0, SECRET_CERTIFICATE);

    deviceInfo devInfo;
    sdkReadFile("devInfo", (uint8_t*)&devInfo.ssid[0], sizeof(deviceInfo)); 
    mqttClient.setId(devInfo.devName);
    mqttClient.onMessage(onMessageReceived);
}


bool awsCheckConnection(){
    return mqttClient.connected();
}


void connectMQTT()
{
    Serial.print("Attempting to MQTT broker: ");
    Serial.print(broker);
    Serial.print(":");
    Serial.print(port);
    Serial.println();

    int status;
    int cnt{0};
    while ((status = mqttClient.connect(broker, port)) == 0) {
        // failed, retry
        Serial.println(status);
        delay(1000);
        cnt++;
        if(cnt >= 5){ 
            Serial.print("Fail to connect MQTT broker ");
            return;
        }
    }
    Serial.println();

    Serial.println("You're connected to the MQTT broker");
    Serial.println();

    // subscribe to a topic with QoS 1
    constexpr char incomingTopic[] { "arduino/incoming" };
    constexpr int incomingQoS { 1 };
    Serial.print("Subscribing to topic: ");
    Serial.print(incomingTopic);
    Serial.print(" with QoS ");
    Serial.println(incomingQoS);
    mqttClient.subscribe(incomingTopic, incomingQoS);   
}


void publishMessage(String msg)
{
    Serial.println("Publishing message");

    JSONVar payload;
    payload["data"] = msg;
    payload["rssi"] = WiFi.RSSI();

    JSONVar message;
    message["ts"] = static_cast<unsigned long>(time(nullptr));
    message["payload"] = payload;

    String messageString = JSON.stringify(message);
    Serial.println(messageString);

    // send message, the Print interface can be used to set the message contents
    constexpr char outgoingTopic[] { "arduino/outgoing" };

    mqttClient.beginMessage(outgoingTopic);
    mqttClient.print(messageString);
    mqttClient.endMessage();
}


