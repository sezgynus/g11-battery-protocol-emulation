#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* mySSID = "Desk Clock";
const char* myPASSWORD = "12345678";
#if defined(ESP32_RTOS) && defined(ESP32)
void ota_handle( void * parameter ) {
  for (;;) {
    ArduinoOTA.handle();
  }
}
#endif

void setupOTA(const char* nameprefix, const char* ssid, const char* password) {
  uint16_t maxlen = strlen(nameprefix) + 7;
  char *fullhostname = new char[maxlen];
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", nameprefix, mac[3], mac[4], mac[5]);
  ArduinoOTA.setHostname(fullhostname);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  ArduinoOTA.setPassword("openeye");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";
  });
  
  ArduinoOTA.onEnd([]() {

  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();

#if defined(ESP32_RTOS) && defined(ESP32)
  xTaskCreate(
    ota_handle,
    "OTA_HANDLE",
    10000,
    NULL,
    1,
    NULL);
#endif
}