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

const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

// const char* ssid     = "";
// const char* password = "";

bool state = false;
AsyncWebServer server(80);

unsigned long ota_progress_millis = 0;

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
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void setupClient() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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
  Serial.println(ssid);
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
  setupAccessPoint();
  // setupClient();

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
}