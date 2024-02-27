#include <WiFi.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h

int keyIndex = 0;             // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

String parseCommand();
String parseParam(String paramName);
String handleCommand(String command, String param1, String param2);

void webPageSetup() {

  Serial.println("Access Point Web Server");

  pinMode(LEDR,OUTPUT);
  pinMode(LEDG,OUTPUT);
  pinMode(LEDB,OUTPUT); 

  // start the web server on port 80
  server.begin();
}

// void webPageLoop() {

//   WiFiClient client = server.available();   // listen for incoming clients

//   if (client) {                             // if you get a client,
//     Serial.println("new client");           // print a message out the serial port
//     String currentLine = "";                // make a String to hold incoming data from the client
  
//     while (client.connected()) {            // loop while the client's connected
     
//       if (client.available()) {             // if there's bytes to read from the client,
//         char c = client.read();             // read a byte, then
//         Serial.write(c);                    // print it out the serial monitor
//         if (c == '\n') {                    // if the byte is a newline character

//           // if the current line is blank, you got two newline characters in a row.
//           // that's the end of the client HTTP request, so send a response:
//           if (currentLine.length() == 0) {
//             // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//             // and a content-type so the client knows what's coming, then a blank line:
//             client.println("HTTP/1.1 200 OK");
//             client.println("Content-type:text/html");
//             client.println();

//             // the content of the HTTP response follows the header:
//             client.print("<html><head>");
//             client.print("<style>");
//             client.print("* { font-family: sans-serif;}");
//             client.print("body { padding: 2em; font-size: 2em; text-align: center;}");            
//             client.print("a { -webkit-appearance: button;-moz-appearance: button;appearance: button;text-decoration: none;color: initial; padding: 25px;} #red{color:red;} #green{color:green;} #blue{color:blue;}");
//             client.print("</style></head>");
//             client.print("<body><h1> RIDDLE & CODES </h1>");
//             client.print("<body><h1> ARDUINO WEB INTERFACE </h1>");
//             client.print("<h2><span id=\"red\">RED </span> LED </h2>");
//             client.print("<a href=\"/Hr\">ON</a> <a href=\"/Lr\">OFF</a>");
//             client.print("</body></html>");

//             // The HTTP response ends with another blank line:
//             client.println();
//             // break out of the while loop:
//             break;
//           } else {      // if you got a newline, then clear currentLine:
//             currentLine = "";
//           }
//         } else if (c != '\r') {    // if you got anything else but a carriage return character,
//           currentLine += c;      // add it to the end of the currentLine
//         }

//         // Check to see if the client request was "GET /H" or "GET /L":
//         if (currentLine.endsWith("GET /Hr")) {
//           digitalWrite(LEDR, LOW);               // GET /Hr turns the Red LED on
//         }
//         if (currentLine.endsWith("GET /Lr")) {
//           digitalWrite(LEDR, HIGH);                // GET /Lr turns the Red LED off
//         }
//       }
//     }
//     // close the connection:
//     client.stop();
//     Serial.println("client disconnected");
//   }
  
// }
void parseRequest(WiFiClient& client, String& command, String& param1, String& param2);

void webPageLoop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Parse command and parameters from the request
            String command;
            String param1;
            String param2;
            parseRequest(client, command, param1, param2);
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            // Handle command and parameters here
            String response = handleCommand(command, param1, param2);
            
            client.println(response);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    client.stop();
    Serial.println("Client disconnected");
  }
}

void parseRequest(WiFiClient& client, String& command, String& param1, String& param2) {
  String currentLine = "";
  String request = "";

  // Read the HTTP request headers
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n') {
        // Check if the current line contains the GET request line
        if (currentLine.startsWith("GET")) {
          // Extract the command and parameters from the request URL
          int commandIndex = currentLine.indexOf(' ');
          int queryIndex = currentLine.indexOf('?');
          int param1Index = currentLine.indexOf('=', queryIndex);
          int param1EndIndex = currentLine.indexOf('&', param1Index);
          int param2Index = currentLine.indexOf('=', param1EndIndex);

          if (queryIndex != -1) {
            if (param1Index != -1 && param1EndIndex != -1) {
              command = currentLine.substring(commandIndex + 1, queryIndex);
              param1 = currentLine.substring(param1Index + 1, param1EndIndex);
              if (param2Index != -1) {
                param2 = currentLine.substring(param2Index + 1, currentLine.indexOf(' ', param2Index));
              }
            }
          } else {
            command = currentLine.substring(commandIndex + 1, currentLine.indexOf(' ', commandIndex + 1));
          }
          break;
        }
        if (currentLine == "\r") {
          // End of HTTP request headers
          break;
        }
        currentLine = "";
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }
}


String handleCommand(String command, String param1, String param2) {
  // Implement your logic here to handle the command and parameters
  // Generate response based on the command and parameters
  // For demonstration, let's just return a simple response with the command and parameters
  return "Command: " + command + ", Param1: " + param1 + ", Param2: " + param2;
}