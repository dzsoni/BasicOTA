/**************************************************************
  BasicOTA:On The Air firmware and filesystem update via local
   http page.
   Author: dzsoni https://github.com/dzsoni/BasicOTA
   Licensed under MIT license
 **************************************************************/

#ifndef BasicOTA_h
#define BasicOTA_h

#include "Arduino.h"


#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#include "Update.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
#endif


#include "ESPAsyncWebServer.h"
#include "FS.h"

#include "OTAwebpage.h"

extern "C" uint32_t _FS_start;
extern "C" uint32_t _FS_end;

class BasicOtaClass
{

public:
    void begin(AsyncWebServer *server)
    {
        _server = server;

        _server->on("/update", HTTP_GET, [&](AsyncWebServerRequest *request)
                    {
                        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", OTA_HTML, OTA_HTML_SIZE);
                        response->addHeader("Content-Encoding", "gzip");
                        request->send(response);
                    });

        _server->on(
        "/update", HTTP_POST, [&, this](AsyncWebServerRequest *request)
        {
            AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 500 : 200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");

            request->send(response);
            yield();
            delay(1000);
            yield();
            ESP.restart();
        } ,
        [&, this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            if (!index)
            {
                Serial.printf("Update Start: %s\n", filename.c_str());

#if defined(ESP8266)

                String s = request->arg("firmwaretype");
                Serial.println(s);
                int cmd = (s == "filesystem") ? U_FS : U_FLASH;

                Update.runAsync(true);
                size_t fsSize = ((size_t)&_FS_end - (size_t)&_FS_start);
                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                if (!Update.begin((cmd == U_FS) ? fsSize : maxSketchSpace, cmd))
                {
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA could not begin");
                }
#else
                if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
                {
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA could not begin");
                }
#endif
            }
            if (len)
            {
                if (Update.write(data, len) != len)
                {
                    Update.printError(Serial);
                }
            }
            if (final)
            {
                if (Update.end(true))
                {
                    Serial.printf("Update Success: %uB\n", index + len);
                    //request->send(200, "text/plain", "OK.");
                }
                else
                {
                    Update.printError(Serial);
                }
            }
        }
    );
    }
private:
    AsyncWebServer *_server;
};
BasicOtaClass BasicOTA;
#endif
