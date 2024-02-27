   
#include <iostream>
#include <sstream>
#include <vector>
#include <Arduino.h>
#include "rddlSDKAPI.h"

using namespace std;


char TEST_SEED[] = "12345d8b5ee5bcefd523ee4d4340a8956affbef5bb1978eb1e3f640318f87f11";

void aws_mqtt_setup();
void aws_mqtt_loop();
bool checkNetwork();
void callRddlCommand(vector<string> &cmd);


void setup() {
  Serial.begin(115200);

  aws_mqtt_setup();
}


void loop() {
  // aws_mqtt_loop();

  static bool tested = false;
  if(checkNetwork() && !tested){
    Serial.println();
    Serial.println();
    Serial.println("Enter Command: ");
    while (Serial.available() == 0) {}
    
    auto cmd = Serial.readString();

    Serial.println(cmd);
    stringstream ss(cmd.c_str());
    vector<string> v;
    string s;
    while (getline(ss, s, ' ')) {
 
        // store token string in the vector
        v.push_back(s);
    }
 
    callRddlCommand(v);
  }
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


enum command_code {
    eGetPublicKey,
    eAttestMachine,
    eNotarizeData,
    NumOfUserCommand
};


command_code getUserCommand (std::string const& cmd) {
  if (cmd.find("GetPublickey") != std::string::npos) return eGetPublicKey;
  if (cmd.find("AttestMachine") != std::string::npos) return eAttestMachine;
  if (cmd.find("NotarizeData") != std::string::npos) return eNotarizeData;
  return NumOfUserCommand;
}


void callRddlCommand(vector<string> &cmd){
  switch(getUserCommand(cmd[0])){
    case eGetPublicKey:
      CmndPublicKeys();
      break;

    case eAttestMachine:
      CmndAttestMachine(cmd);
      break;

    case eNotarizeData:
      CmndNottarizeData(cmd);
      break;

    default:
      Serial.println("Wrong Command!");
      break;
  } 
}
