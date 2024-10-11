/*
 * Provides a base for an ESP32 LIN sniffer with OTA updates.
 * Based on the ElegantOTA example for OTA updates.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>

#include "lin.h"
#include "wifi_creds.h"

#define LIN_FRAME_PID 0xCF
#define LEFT_PIN 2
#define RIGHT_PIN 3
#define TAIL_PIN 4

bool left_state = false;
bool right_state = false;
bool tail_state = false;

const char* AP_SSID     = "TCU-Access-Point";
const char* AP_PASSWORD = "123456789";

const char* OTA_USERNAME = "ota";
const char* OTA_PASSWORD = "123456789";

bool led_state = false;
WebServer httpServer(80);
HTTPUpdateServer httpUpdater;

String latestFrameString = "";

// LIN Variables
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

void processLightLINFrame(byte dataByte) {
  // First bit is left light, second bit is right light, third bit is tail light
  // This is a four pin trailer connector, so brakes and reverse do not matter, but are present in the LIN frame
  // See docs if you need to add support for those
  left_state = dataByte & 0x01;
  right_state = dataByte & 0x02;
  tail_state = dataByte & 0x04;
  digitalWrite(LEFT_PIN, left_state);
  digitalWrite(RIGHT_PIN, right_state);
  digitalWrite(TAIL_PIN, tail_state);
}

void runTestSequence() {
  flashLED(3, 100);
  digitalWrite(LEFT_PIN, true);
  delay(1000);
  digitalWrite(LEFT_PIN, false);
  digitalWrite(RIGHT_PIN, true);
  delay(1000);
  digitalWrite(RIGHT_PIN, false);
  digitalWrite(TAIL_PIN, true);
  delay(1000);
  digitalWrite(TAIL_PIN, false);
  flashLED(4, 100);
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Latest Data Frame: " + latestFrameString + "</h1>";
  html += "<button onclick=\"location.href='/runTest'\">Run Test Sequence</button>";
  html += "<button onclick=\"location.href='/update'\">Firmware Update</button>";
  html += "</body></html>";
  httpServer.send(200, "text/html", html);
}

void handleRunTest() {
  runTestSequence();
  httpServer.send(200, "text/html", "Test sequence executed.");
}

void setup(void) {
  // set control pins as an output and set them to LOW
  pinMode(LEFT_PIN, OUTPUT);
  digitalWrite(LEFT_PIN, LOW);
  pinMode(RIGHT_PIN, OUTPUT);
  digitalWrite(RIGHT_PIN, LOW);
  pinMode(TAIL_PIN, OUTPUT);
  digitalWrite(TAIL_PIN, LOW);

  pinMode(LED_BUILTIN, OUTPUT); 
  led_state = true;
  digitalWrite(LED_BUILTIN, led_state);
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
  linStack.setupSerial();

  httpUpdater.setup(&httpServer, OTA_USERNAME, OTA_PASSWORD);
  httpServer.on("/", handleRoot);
  httpServer.on("/runTest", handleRunTest);
  httpServer.begin();

  Serial.println("HTTP server started");
  led_state = false;
  digitalWrite(LED_BUILTIN, led_state);
}

void loop(void) {
  httpServer.handleClient();

  int bytesRead = linStack.readFrame(data, LIN_FRAME_PID);
  if (bytesRead > 2) {
    byte calculatedChecksum = linStack.calculateChecksum(data, bytesRead - 1);
    for (int i = 0; i < bytesRead; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    byte receivedChecksum = data[bytesRead - 1];
    if (calculatedChecksum == receivedChecksum) {
      Serial.print("OK");
      processLightLINFrame(data[2]);
    } else {
      Serial.print("ERR ");
      Serial.print(calculatedChecksum, HEX);
    }
    Serial.println();
    latestFrameString = "";
    for (int i = 0; i < bytesRead; i++) {
      latestFrameString += "0x" + String(data[i], HEX) + " ";
    }
  }
}