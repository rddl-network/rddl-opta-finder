#include <iostream>
#include <sstream>
#include <vector>
#include <Arduino.h>
#include "rddlSDKAPI.h"
#include "arduino_secrets.h"
#include "cmdInterpreter.h"
#include "awsMqtt.h"

using namespace std;

void setup() {
  Serial.begin(115200);

  wifi_setup();
}


void loop() {
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
 
    cmdInterpreter(v);
  }
}
