bool checkNetwork();
bool connectWifi();
void wifi_setup();
bool checkNetwork();
void publishMessage(String msg);
void connectMQTT();
void awsMqttSetup();
void aws_mqtt_loop();
bool awsCheckConnection();
String certCreateSetup();

String webPageLoop();
void webPageSetup();