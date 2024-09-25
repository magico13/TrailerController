/*
  -----------------------
  ElegantOTA - Async Demo Example
  -----------------------

  NOTE: Make sure you have enabled Async Mode in ElegantOTA before compiling this example!
  Guide: https://docs.elegantota.pro/async-mode/

  Skill Level: Beginner

  This example provides with a bare minimal app with ElegantOTA functionality which works
  with AsyncWebServer.

  Github: https://github.com/ayushsharma82/ElegantOTA
  WiKi: https://docs.elegantota.pro
*/

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <HardwareSerial.h>

#include <lin_stack_esp32.h>
#include "wifi_creds.h"

const char* ESP32_AP_SSID     = "ESP32-Access-Point";
const char* ESP32_AP_PASSWORD = "123456789";

bool state = false;
AsyncWebServer server(80);

unsigned long ota_progress_millis = 0;

// LIN Variables
const byte ident = 0; // Identification Byte
byte data_size=8; // length of byte array
byte data[8]; // byte array for received data

lin_stack_esp32 LIN1(2,ident); // 2 - channel (GPIO 16 & 17), ident - Identification Byte

void flashLED(int count, int delay_ms) {
  digitalWrite(LED_BUILTIN, false);
  delay(100);
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, true);
    delay(delay_ms);
    digitalWrite(LED_BUILTIN, false);
    delay(delay_ms);
  }
}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  flashLED(3, 100);
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    state = !state;
    digitalWrite(LED_BUILTIN, state);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
    flashLED(3, 250);
  } else {
    Serial.println("There was an error during OTA update!");
    flashLED(10, 250);
  }
}

void setupAccessPoint() {
  Serial.print("Setting AP...");
  WiFi.softAP(ESP32_AP_SSID, ESP32_AP_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void setupClient() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    state = !state;
    digitalWrite(LED_BUILTIN, state);
    delay(500);
    Serial.print(".");
  }
  state = true;
  digitalWrite(LED_BUILTIN, state);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT); 
  state = true;
  digitalWrite(LED_BUILTIN, state);
  Serial.begin(115200);
  Serial.println("Booting");

  // Setup WiFi
  // If WiFi credentials are not provided, start Access Point
  if (strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0) {
    setupClient();
  } else {
    setupAccessPoint();
  }

  // Setup LIN
  LIN1.setSerial();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo.");
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
  Serial.println("HTTP server started");
  state = false;
  digitalWrite(LED_BUILTIN, state);
}

void loop(void) {
  ElegantOTA.loop();

  // Check LIN bus and print data
  // Pulled from https://github.com/autobyt-es/ESP32_LIN/blob/master/examples/Sniffer/ESP32_Sniffer.ino
  byte a = LIN1.readStream(data, data_size);
  if(a == 1){ // If there was an event on LIN Bus, Traffic was detected. Print data to serial monitor
     Serial.println("Traffic detected!");
     Serial.print("Synch Byte: ");
     Serial.println(data[0], HEX);
     Serial.print("Ident Byte: ");
     Serial.println(data[1], HEX);
     Serial.print("Data Byte1: ");
     Serial.println(data[2]);
     Serial.print("Data Byte2: ");
     Serial.println(data[3]);
     Serial.print("Data Byte3: ");
     Serial.println(data[4]);
     Serial.print("Data Byte4: ");
     Serial.println(data[5]);
     Serial.print("Data Byte5: ");
     Serial.println(data[6]);
     Serial.print("Check Byte: ");
     Serial.println(data[7]);
     Serial.print("\n");
  }

  // TODO: Use webserver to view data instead of serial monitor
}