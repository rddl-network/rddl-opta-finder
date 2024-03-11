#include <iostream>
#include <sstream>
#include <vector>
#include <Arduino.h>
#include "rddlSDKAPI.h"
#include "arduino_secrets.h"
#include "cmdInterpreter.h"
#include "awsMqtt.h"

using namespace std;

char TEST_SEED[] = "12345d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f11";

void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}


command_code getUserCommand (std::string const& cmd) {
  if (cmd.find("SetWifi")       != std::string::npos) return eSetWifi;
  if (cmd.find("Mnemonic")      != std::string::npos) return eMnemonic;
  if (cmd.find("GetPublickey")  != std::string::npos) return eGetPublicKey;
  if (cmd.find("AttestMachine") != std::string::npos) return eAttestMachine;
  if (cmd.find("NotarizeData")  != std::string::npos) return eNotarizeData;
  if (cmd.find("CreateCsr")     != std::string::npos) return eCreateCsr;
  if (cmd.find("StoreCert")     != std::string::npos) return eStoreCert;
  if (cmd.find("ReadCert")      != std::string::npos) return eReadCert;
  if (cmd.find("TestAWSMQTTMsg")!= std::string::npos) return eTestAWSMQTTMsg;
  return NumOfUserCommand;
}


void portentaDeleteFile(const char * filename);

/* TO DO: IT COULDNT WRITE WIFI CREDENTIAL TO FLASH */
void CmndSetWifi(vector<string> &cmd){
  wifiCredentials wifiInfo;
  portentaDeleteFile("Wifi");

  Serial.println("Enter Wifi SSID: ");
  while (Serial.available() == 0) {}
  auto ssid = Serial.readString();
  Serial.println(ssid);
  strcpy(wifiInfo.ssid, ssid.c_str());

  Serial.println("Enter Wifi PASSWORD: ");
  while (Serial.available() == 0) {}
  auto pass = Serial.readString();
  Serial.println(pass);
  strcpy(wifiInfo.pass, pass.c_str());

  sdkWriteFile("Wifi", (uint8_t*)&wifiInfo.ssid[0], 128);  

  memset(&wifiInfo.ssid[0], 0, sizeof(wifiCredentials));

  sdkReadFile("Wifi", (uint8_t*)&wifiInfo.ssid[0], 128);  

  Serial.println(wifiInfo.ssid);
  Serial.println(wifiInfo.pass);
}


void CmndMnemonic(vector<string> &cmd){
  char* mnemonic = NULL;

  mnemonic = sdkSetSeed((char*)cmd[1].c_str(), cmd[1].size());
  Serial.print("Mnemonic: "); 
  Serial.println(mnemonic);
}


void CmndPublicKeys(){
  sdkStoreSeed(TEST_SEED);

  if( !sdkGetPlntmntKeys() ){
    Serial.println("Initialize Keys first (Mnemonic)");
  }else{
    Serial.print("Address "); Serial.println(sdkGetRDDLAddress());
    Serial.print("Liquid "); Serial.println(sdkGetExtPubKeyLiquid());
    Serial.print("Planetmint "); Serial.println(sdkGetExtPubKeyPlanetmint());
    Serial.print("Machine ID "); Serial.println(sdkGetMachinePublicKey());
  }
}


void CmndAttestMachine(vector<string> &cmd){
  char machinecid_buffer[64] = {0};
  memset((void*)machinecid_buffer,0, 58+1);

  runRDDLSDKMachineAttestation("otherserial", "RDDL", machinecid_buffer);
}


void CmndNottarizeData(vector<string> &cmd){

  runRDDLSDKNotarizationWorkflow(cmd[1].c_str(), cmd[1].length()-1);
}


void CmndCreateCsr(){
  certCreateSetup();
}


void CmndStoreCert(){
  int index = 0;
  portentaDeleteFile("cert");
  Serial.println("Enter ur Certificate: ");
  SECRET_CERTIFICATE[index++] = '\n';
  while (true) {
    while (Serial.available() == 0) {}
    auto line = Serial.readString();

    line.replace("\r", "");
    
    memcpy(&SECRET_CERTIFICATE[index], line.c_str(), line.length());
    index += line.length();

    if(index >= sizeof(SECRET_CERTIFICATE)){
      Serial.println("Certificate buffer overflow");
      break;
    }
    
    if (line.indexOf("END CERTIFICATE") != -1) 
      break;
  }
  portentaDeleteFile("cert");
  SECRET_CERTIFICATE[index++] = '\n';
  Serial.println("Writing Cert file...");

  sdkWriteFile( "cert", (uint8_t *)SECRET_CERTIFICATE, index);
}


void CmndReadCert(){
  Serial.println("Reading the cert file...");
  sdkReadFile( "cert", (uint8_t *)SECRET_CERTIFICATE, sizeof(SECRET_CERTIFICATE));
  Serial.println("DONE");
  Serial.println(SECRET_CERTIFICATE);
  Serial.println(" ");
  Serial.println(" ");
}


void CmndAWSTest(){
  if(strlen(SECRET_CERTIFICATE) == 0){
    Serial.println("Reading the cert file...");
    sdkReadFile( "cert", (uint8_t *)SECRET_CERTIFICATE, sizeof(SECRET_CERTIFICATE));
  }

  aws_mqtt_setup();
  connectMQTT();

  for(int i=0; i<5; ++i){
    publishMessage();
    delay(4000);
  }
}


void cmdInterpreter(std::vector<std::string> &cmd){
  switch(getUserCommand(cmd[0])){
    case eSetWifi:
      CmndSetWifi(cmd);
      break;

    case eMnemonic:
      CmndMnemonic(cmd);
      break;

    case eGetPublicKey:
      CmndPublicKeys();
      break;

    case eAttestMachine:
      CmndAttestMachine(cmd);
      break;

    case eNotarizeData:
      CmndNottarizeData(cmd);
      break;

    case eCreateCsr:
      CmndCreateCsr();
      break;

    case eStoreCert:
      CmndStoreCert();
      break;

    case eReadCert:
      CmndReadCert();
      break;

    case eTestAWSMQTTMsg:
      CmndAWSTest();
      break;

    default:
      Serial.println("Wrong Command!");
      break;
  } 
}
