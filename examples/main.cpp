#include <Arduino.h>
#include "BasicOTA.h"
#include "ESPAsyncWebServer.h"

// Replace with your network credentials

const char *ssid = "";
const char *password = "";

AsyncWebServer webserver(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  webserver.begin();
  BasicOTA.begin(&webserver);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("\nConnecting to WiFi..");
  }
Serial.println("You can access here:");
Serial.print(WiFi.localIP());
Serial.println("/update");

}

void loop() {
  // put your main code here, to run repeatedly:
}