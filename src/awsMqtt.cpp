#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <Arduino_ConnectionHandler.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <mbed_mktime.h>
#include "rddlSDKAPI.h"
#include "arduino_secrets.h"

// Enter your sensitive data in arduino_secrets.h
constexpr char broker[] { SECRET_BROKER };
constexpr unsigned port { SECRET_PORT };
const char* certificate { TEST_CERTIFICATE };

#include <WiFi.h>
#include <WiFiUdp.h>
// Enter your sensitive data in arduino_secrets.h
WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);
WiFiClient tcpClient;
WiFiUDP NTPUdp;

NTPClient timeClient(NTPUdp);
BearSSLClient sslClient(tcpClient);
MqttClient mqttClient(sslClient);

unsigned long lastMillis { 0 };

void setNtpTime();
unsigned long getTime();
void connectMQTT();
void publishMessage();
void onMessageReceived(int messageSize);
void onNetworkConnect();
void onNetworkDisconnect();
void onNetworkError();
void printWifiStatus();
void sendHttpsGetRequestTest();

extern void webPageSetup();
extern void webPageLoop();


void wifi_setup(){
    // Wait for Serial Monitor or start after 2.5s
    for (const auto startNow = millis() + 2500; !Serial && millis() < startNow; delay(250));

    // WiFi.begin(SECRET_SSID, SECRET_PASS);
    
    // while(WiFi.status() != WL_CONNECTED){
    //     Serial.println(">>>> CONNECTING to Network...");
    //     delay(1000);
    // }

    // Serial.println(">>>> CONNECTED to network");

    // printWifiStatus();
    // setNtpTime();

    // Set the callbacks for connectivity management
    conMan.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
    conMan.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
    conMan.addCallback(NetworkConnectionEvent::ERROR, onNetworkError);

    timeClient.begin();
}

void aws_mqtt_setup()
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
    // String devName;
    // devName.reserve(128);
    //sdkReadFile("devName", (uint8_t*)&devName[0], 128);
    // if(devName.isEmpty()){
        // Serial.println("ERROR! Device name couldnt find!");
        // return;
    // }

    mqttClient.setId("Portenta03");
    mqttClient.onMessage(onMessageReceived);

    // Start the server
    webPageSetup();
}

void aws_mqtt_loop()
{
    // Automatically manage connectivity
    const auto conStatus = conMan.check();
    if (conStatus != NetworkConnectionState::CONNECTED)
        return;
    // auto wifiStatus = WiFi.status();
    // if(wifiStatus != WL_CONNECTED)
    //     return;

    if (!mqttClient.connected()) {
        // MQTT client is disconnected, connect
        connectMQTT();
    }

    // poll for new MQTT messages and send keep alives
    mqttClient.poll();

    // publish a message roughly every 30 seconds.
    if (millis() - lastMillis > 30000) {
        lastMillis = millis();

        publishMessage();
    }

    // Handle serving the webpage
    webPageLoop();

    //sendHttpsGetRequest();
}

bool checkNetwork(){
    const auto conStatus = conMan.check();

    if (conStatus != NetworkConnectionState::CONNECTED)
        return false;

    // auto wifiStatus = WiFi.status();
    // if(wifiStatus != WL_CONNECTED)
    //     return false;

    return true;
}


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

void publishMessage()
{
    Serial.println("Publishing message");

    JSONVar payload;
    String msg = "Hello, World! ";
    msg += millis();
    payload["message"] = msg;
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

void onNetworkConnect()
{
    Serial.println(">>>> CONNECTED to network");

    printWifiStatus();
    setNtpTime();
    // connectMQTT();
}

void onNetworkDisconnect()
{
    Serial.println(">>>> DISCONNECTED from network");
}

void onNetworkError()
{
    Serial.println(">>>> ERROR");
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
