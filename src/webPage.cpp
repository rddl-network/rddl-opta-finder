#include <WiFi.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h

int status = WL_IDLE_STATUS;

WiFiServer server(80);


void webPageSetup(){
  // Start the server
  server.begin();
  Serial.println("Server started");
}


String webPageLoop(){
  String cmd{};
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    bool httpBody = false; // Flag to determine when the HTTP body begins
    String userInput = ""; // Variable to store user input

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (!httpBody) {
          // Check for end of HTTP header
          currentLine += c;
          if (c == '\n' && currentLine.endsWith("\r\n\r\n")) {
            httpBody = true; // HTTP body begins after this line
          }
        } else {
          // Process HTTP body
          if (c == '\n') {
            // Check if the HTTP request is a form submission
            if (currentLine.startsWith("GET /submit")) {
              // Extract user input
              int inputIndex = currentLine.indexOf("input=") + 6;
              if (inputIndex != -1) {
                userInput = currentLine.substring(inputIndex);
                userInput.trim();
              }
              cmd = userInput;
              // Send HTTP response
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.println("<html><head><title>Arduino Portenta Web Server</title></head><body>");
              client.println("<h1>String submitted successfully!</h1>");
              client.println("<p>You entered: " + userInput + "</p>");
              client.println("</body></html>");
              break; // Exit the loop after handling the submission
            }
            currentLine = ""; // Reset current line for next iteration
          }
        }
      }
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");

    cmd.replace("+", " ");
  }

  return cmd;
}