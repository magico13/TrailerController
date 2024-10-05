/*
 * Provides a base for an ESP32 LIN sniffer with OTA updates.
 * Based on the ElegantOTA example for OTA updates.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>

#include "lin.h"
#include "wifi_creds.h"

const char* AP_SSID     = "TCU-Access-Point";
const char* AP_PASSWORD = "123456789";

const char* OTA_USERNAME = "ota";
const char* OTA_PASSWORD = "123456789";

bool led_state = false;
WebServer httpServer(80);
HTTPUpdateServer httpUpdater;

unsigned long ota_progress_millis = 0;

bool pin2_state = false;
unsigned long last_pin2_toggle = 0;

// LIN Variables
// const byte ident = 1; // Identification Byte
// const byte data_size=8; // length of byte array
// const byte frame_size = data_size+3; // length of LIN frame
// byte data[frame_size]; // byte array for received data

// lin_stack_esp32 LIN1(2,ident); // 2 - channel (GPIO 16 & 17), ident - Identification Byte

lin linStack;
byte data[lin::MAX_BYTES];

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
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state);
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
  WiFi.softAP(AP_SSID, AP_PASSWORD);

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
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state);
    delay(500);
    Serial.print(".");
  }
  led_state = true;
  digitalWrite(LED_BUILTIN, led_state);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT); 
  led_state = true;
  digitalWrite(LED_BUILTIN, led_state);
  Serial.begin(115200);
  Serial.println("Booting");

  // set GP2 as an output
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  // Setup WiFi
  // If WiFi credentials are not provided, start Access Point
  if (strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0) {
    setupClient();
  } else {
    setupAccessPoint();
  }

  // Setup LIN
  linStack.setupSerial();

  httpUpdater.setup(&httpServer, OTA_USERNAME, OTA_PASSWORD);
  httpServer.begin();

  Serial.println("HTTP server started");
  led_state = false;
  digitalWrite(LED_BUILTIN, led_state);
}

void loop(void) {
  httpServer.handleClient();

  // every 5 second, flip the status of pin 2
  if (millis() - last_pin2_toggle > 5000) {
    pin2_state = !pin2_state;
    digitalWrite(2, pin2_state);
    last_pin2_toggle = millis();
    digitalWrite(LED_BUILTIN, pin2_state);
  }

  // int bytesRead = linStack.readFrame(data, 0xCF);
  // if (bytesRead > 2) {
  //   byte calculatedChecksum = linStack.calculateChecksum(data, bytesRead - 1);
  //   for (int i = 0; i < bytesRead; i++) {
  //     Serial.print(data[i], HEX);
  //     Serial.print(" ");
  //   }
  //   byte receivedChecksum = data[bytesRead - 1];
  //   if (calculatedChecksum == receivedChecksum) {
  //     Serial.print("OK");
  //   } else {
  //     Serial.print("ERR ");
  //     Serial.print(calculatedChecksum, HEX);
  //   }
  //   Serial.println();
  // }

  // TODO: Use webserver to view data instead of serial monitor
}