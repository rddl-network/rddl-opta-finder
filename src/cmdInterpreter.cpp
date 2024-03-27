#include <iostream>
#include <sstream>
#include <vector>
#include <Arduino.h>
#include "rddlSDKAPI.h"
#include "arduino_secrets.h"
#include "cmdInterpreter.h"
#include "awsMqtt.h"
#include "rddl.h"
#include "janitza604.h"

using namespace std;

char TEST_SEED[] = "12345d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f11";

void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}


command_code getUserCommand (std::string const& cmd) {
  if (cmd.find("Help")          != std::string::npos) return eHelp;
  if (cmd.find("help")          != std::string::npos) return eHelp;
  if (cmd.find("SetWifi")       != std::string::npos) return eSetWifi;
  if (cmd.find("Mnemonic")      != std::string::npos) return eMnemonic;
  if (cmd.find("StoreSeed")     != std::string::npos) return eStoreSeed;
  if (cmd.find("Publickeys")    != std::string::npos) return eGetPublicKey;
  if (cmd.find("AttestMachine") != std::string::npos) return eAttestMachine;
  if (cmd.find("NotarizeData")  != std::string::npos) return eNotarizeData;
  if (cmd.find("CreateCsr")     != std::string::npos) return eCreateCsr;
  if (cmd.find("StoreCert")     != std::string::npos) return eStoreCert;
  if (cmd.find("ReadCert")      != std::string::npos) return eReadCert;
  if (cmd.find("TestAWSMQTTMsg")!= std::string::npos) return eTestAWSMQTTMsg;
  if (cmd.find("Balance")       != std::string::npos) return eBalance;
  if (cmd.find("Showfiles")     != std::string::npos) return eShowFiles;
  if (cmd.find("ReadSMDate")    != std::string::npos) return eReadSMDate;
  return NumOfUserCommand;
}


void CmndHelp(){
  Serial.println("Help");
  Serial.println("SetWifi");
  Serial.println("Mnemonic (optional)<mnemonic> ");
  Serial.println("StoreSeed <Seed>");
  Serial.println("Publickeys");
  Serial.println("AttestMachine");
  Serial.println("NotarizeData <data>");
  Serial.println("CreateCsr");
  Serial.println("StoreCert");
  Serial.println("ReadCert");
  Serial.println("TestAWSMQTTMsg");
  Serial.println("Balance");
  Serial.println("Showfiles");
  Serial.println("ReadSMDate");
}


void CmndSetWifi(vector<string> &cmd){
  deviceInfo devInfo;

  Serial.println("Enter Wifi SSID: ");
  while (Serial.available() == 0) {}
  auto ssid = Serial.readString();
  Serial.println(ssid);
  strcpy(devInfo.ssid, ssid.c_str());

  Serial.println("Enter Wifi PASSWORD: ");
  while (Serial.available() == 0) {}
  auto pass = Serial.readString();
  Serial.println(pass);
  strcpy(devInfo.pass, pass.c_str());

  sdkReadFile ("devInfo", (uint8_t*)&devInfo.ssid[0], sizeof(deviceInfo)); 
  sdkWriteFile("devInfo", (uint8_t*)&devInfo.ssid[0], sizeof(deviceInfo));  

  memset(&devInfo.ssid[0], 0, sizeof(deviceInfo));

  sdkReadFile("devInfo", (uint8_t*)&devInfo.ssid[0], sizeof(deviceInfo));  

  Serial.println(devInfo.ssid);
  Serial.println(devInfo.pass);
}


void CmndMnemonic(vector<string> &cmd){
  char* mnemonic = NULL;

  mnemonic = sdkSetSeed((char*)cmd[1].c_str(), cmd[1].size());
  Serial.print("Mnemonic: "); 
  Serial.println(mnemonic);
}


void CmndStoreSeed(vector<string> &cmd){
  if( cmd[1].size() == 128)
  {
    sdkStoreSeed((char*)fromHexString(cmd[1].c_str()));
    Serial.println("Seed Stored!");
  }
  else
  {
    Serial.println("FAIL! SEED SIZE IS NOT 64 BYTES");
  }
  
}


void CmndPublicKeys(){
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

  if(!awsCheckConnection()){
    awsMqttSetup();
    connectMQTT();
  }

  if(awsCheckConnection()){
    for(int i=0; i<3; ++i){
      publishMessage("Hello World");
      delay(4000);
    }
  }
}


std::pair<int, String> sendHttpsGetRequest(const char* domainUrl, const char* urlPath, const std::vector<std::pair<String, String>>& headers);
void CmndBalance(){
  String uri{};
  uri += "/cosmos/bank/v1beta1/balances/";
  uri += sdkGetRDDLAddress();

  std::vector<std::pair<String, String>> headers;
  headers.push_back(std::make_pair(String{"Content-Type"}, String{"application/json"}));

  auto response = sendHttpsGetRequest("testnet-api.rddl.io", uri.c_str(), headers);
  Serial.print("Balance: ");
  Serial.print(response.second.c_str());
}


extern void printAllFSArduino();
void CmndShowFiles(){
  printAllFSArduino();
}


void CmndReadSMDate()
{
  Janitza604 j;
  j.init(192, 168, 10, 29);
  std::vector<int> date = j.getDateTCP(25);
  Serial.print(date[0]);
  Serial.print("/");
  Serial.print(date[1]);
  Serial.print("/");
  Serial.println(date[2]);

  Serial.print(date[3]);
  Serial.print(":");
  Serial.print(date[4]);
  Serial.print(":");
  Serial.println(date[5]);
}

void cmdInterpreter(std::vector<std::string> &cmd){
  switch(getUserCommand(cmd[0])){
    case eHelp:
      CmndHelp();
      break;

    case eSetWifi:
      CmndSetWifi(cmd);
      break;

    case eMnemonic:
      CmndMnemonic(cmd);
      break;
    
    case eStoreSeed:
      CmndStoreSeed(cmd);
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

    case eBalance:
      CmndBalance();
      break;

    case eShowFiles:
      CmndShowFiles();
      break;

    case eReadSMDate:
      CmndReadSMDate();
      break;

    default:
      Serial.println("Wrong Command!");
      break;
  } 
}
