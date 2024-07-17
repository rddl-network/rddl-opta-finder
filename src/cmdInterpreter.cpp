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
#include "atecc608_handler.h"

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
  if (cmd.find("TestATECC")     != std::string::npos) return eTestATECC;
  if (cmd.find("WriteCfgATECC") != std::string::npos) return eWriteConfigATECC;
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
  Serial.println("TestATECC");
  Serial.println("WriteCfgATECC");
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


void CmndReadSMDate(std::vector<std::string> &cmd)
{
  Janitza604 j;
  if(cmd.size() != 1){
    if(j.init(9600)){
      uint32_t data = j.modbusRead16(25, 6);
      Serial.print(data); Serial.print("/");
      data = j.modbusRead16(25, 7);
      Serial.print(data); Serial.print("/");
      data = j.modbusRead16(25, 8);
      Serial.println(data); 

      data = j.modbusRead16(25, 9);
      Serial.print(data); Serial.print(":");
      data = j.modbusRead16(25, 10);
      Serial.print(data); Serial.print(":");
      data = j.modbusRead16(25, 11);
      Serial.println(data); 

      float f = j.getVoltage(25, 1);
      Serial.println(f);
      String msg = String(f);
      if(awsCheckConnection()){
            publishMessage(msg);
      }
    }else{
      Serial.println("ERROR! Modbus Initialization");
    }
  }else{
    if(j.init(192, 168, 10, 29)){
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
  }
}


void printHexBuffer(std::vector<uint8_t> input) {
  for (int i = 0; i < input.size(); i++) {
    Serial.print(input[i] >> 4, HEX);
    Serial.print(input[i] & 0x0f, HEX);
  }
  Serial.println();
}


void CmndTestATECC(std::vector<std::string> &cmd)
{
  auto status = atecc_handler_init(0x60);
  if(status){
    Serial.print("atecc_handler_init Fail!");
    Serial.println(status);
    return;
  }

  int slotID = 1;
  std::vector<uint8_t> pub_key(64);
  status = atecc_handler_genkey(slotID, pub_key);
  if(status){
    Serial.print("atecc_handler_genkey Fail!");
    Serial.println(status);
    return;
  }
  Serial.println("Pub key generated: ");
  printHexBuffer(pub_key);
  
  std::vector<uint8_t> priv_key = {
    0x00, 0x00, 0x00, 0x00,
    0x39, 0xac, 0x9b, 0xf9, 0x17, 0x1d, 0xe8, 0x6f, 0xfa, 0x77, 0xe0, 0xb9, 0x05, 0x0b, 0xf6, 0xe0,
    0x6a, 0x2c, 0x1b, 0xc1, 0x76, 0x79, 0x36, 0xe6, 0xc7, 0x45, 0x79, 0xe4, 0x26, 0xa4, 0x47, 0x5f
  };

  status = atecc_handler_inject_priv_key(slotID, priv_key);
  if(status){
    Serial.print("atecc_handler_inject_priv_key Fail!");
    Serial.println(status);
    return;
  }

  
  status = atecc_handler_get_public_key(slotID, pub_key);
  if(status){
    Serial.print("atecc_handler_get_public_key Fail!");
    Serial.println(status);
    return;
  }

  Serial.println("Pub key: ");
  printHexBuffer(pub_key);

  std::vector<uint8_t> signature(64);
  std::vector<uint8_t> msg = {
    0xc8, 0x90, 0xf8, 0x65, 0xf3, 0xb0, 0x5f, 0x78, 0x27, 0x03, 0x4a, 0xae, 0x6a, 0xc2, 0x5c, 0xd5,
    0xcb, 0xca, 0x5d, 0x25, 0xeb, 0x0f, 0x0c, 0x35, 0xdf, 0x5d, 0x33, 0x90, 0x3e, 0x08, 0xfa, 0xbe
  };

  status = atecc_handler_sign(slotID, msg, signature);
  if(status){
    Serial.print("atecc_handler_sign Fail!");
    Serial.println(status);
    return;
  }

  Serial.println("Signature: ");
  printHexBuffer(signature);

  status = atecc_handler_verify(slotID, msg, signature, pub_key);
  if(status){
    Serial.print("atecc_handler_verify Fail!");
    Serial.println(status);
    return;
  }

  // status = atecc_handler_lock_slot(slotID);
  // if(status){
  //   Serial.print("atecc_handler_lock_slot Fail!");
  //   Serial.println(status);
  //   return;
  // }

  Serial.println("TEST ENDED SUCCESSFULLY!");
}


void CmndWriteConfigATECC(std::vector<std::string> &cmd)
{
  auto status = atecc_handler_init(0x60);
  if(status){
    Serial.print("atecc_handler_init Fail!");
    Serial.println(status);
    return;
  }

  status = atecc_handler_write_configuration(ECCX08_DEFAULT_CONFIGURATION_VALS, sizeof(ECCX08_DEFAULT_CONFIGURATION_VALS));
  if(status){
    Serial.print("atecc_handler_write_configuration Fail!");
    Serial.println(status);
    return;
  }

  status = atecc_handler_lock_zone(0);
  if(status){
    Serial.print("atecc_handler_lock_zone Fail!");
    Serial.println(status);
    return;
  }
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
      CmndReadSMDate(cmd);
      break;

    case eTestATECC:
      CmndTestATECC(cmd);
      break;

    case eWriteConfigATECC:
      CmndWriteConfigATECC(cmd);
      break;

    default:
      Serial.println("Wrong Command!");
      break;
  } 
}
