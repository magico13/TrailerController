/*
 * Pico W-based Tesla trailer controller unit.
 * Mike Marvin (magico13)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <LittleFS.h>

#include "lin.h"

#define LIN_FRAME_PID 0xCF
#define TAIL_PIN 2
#define LEFT_PIN 3
#define RIGHT_PIN 4

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
bool lfsReady = false;

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

void setupAccessPoint(char* ssid, char* password) {
  if (strlen(ssid) == 0) {
    ssid = (char*)AP_SSID;
  }
  if (strlen(password) == 0) {
    password = (char*)AP_PASSWORD;
  }
  Serial.print("Starting AP with SSID: ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

bool setupClient(char* ssid, char* password, int timeout = 30) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection, with timeout
  unsigned long timeoutMs = millis() + (timeout * 1000);
  while (WiFi.status() != WL_CONNECTED) {
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state);
    delay(500);
    Serial.print(".");
    if (millis() > timeoutMs) {
      Serial.println("Failed to connect to WiFi");
      return false;
    }
  }
  led_state = true;
  digitalWrite(LED_BUILTIN, led_state);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
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
  // open index.html from /web folder
  File file = LittleFS.open("/web/index.html", "r");
  if (!file) {
    httpServer.send(404, "text/plain", "File not found");
    return;
  }
  // replace placeholders in the HTML file
  String html = file.readString();
  html.replace("{lin_frame}", latestFrameString);
  httpServer.send(200, "text/html", html);
}

void handleRunTest() {
  // open runTest.html from /web folder
  File file = LittleFS.open("/web/runTest.html", "r");
  if (!file) {
    httpServer.send(404, "text/plain", "File not found");
    return;
  }
  httpServer.streamFile(file, "text/html");

  runTestSequence();
}

void setup(void) {
  // set control pins as an output and set them to LOW
  pinMode(LEFT_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);
  pinMode(TAIL_PIN, OUTPUT);
  digitalWrite(LEFT_PIN, false);
  digitalWrite(RIGHT_PIN, false);
  digitalWrite(TAIL_PIN, false);

  pinMode(LED_BUILTIN, OUTPUT); 
  led_state = true;
  digitalWrite(LED_BUILTIN, led_state);
  Serial.begin(115200);
  Serial.println("Booting");

  // Setup LittleFS
  lfsReady = LittleFS.begin();
  if (!lfsReady) {
    Serial.println("An Error has occurred while mounting LittleFS");
  }

  // Load configuration
  char wifiSSID[32];
  char wifiPassword[32];
  char apSSID[32];
  char apPassword[32];
  String otaUsername;
  String otaPassword;
  if (lfsReady) {
    File wifiConfig = LittleFS.open("/config/wifi.txt", "r");
    if (wifiConfig) {
      int len = wifiConfig.readBytesUntil('\n', wifiSSID, 31);
      wifiSSID[len] = '\0'; // Null-terminate the string
      len = wifiConfig.readBytesUntil('\n', wifiPassword, 31);
      wifiPassword[len] = '\0'; // Null-terminate the string
      wifiConfig.close();
    }

    File apConfig = LittleFS.open("/config/ap.txt", "r");
    if (apConfig) {
      int len = apConfig.readBytesUntil('\n', apSSID, 31);
      apSSID[len] = '\0'; // Null-terminate the string
      len = apConfig.readBytesUntil('\n', apPassword, 31);
      apPassword[len] = '\0'; // Null-terminate the string
      apConfig.close();
    }

    File otaConfig = LittleFS.open("/config/ota.txt", "r");
    if (otaConfig) {
      otaConfig.readStringUntil('\n');
      otaConfig.readStringUntil('\n');
      otaConfig.close();
    }
  }

  // Setup WiFi
  bool wifiConnected = false;
  if (strlen(wifiSSID) > 0 && strlen(wifiPassword) > 0) {
    wifiConnected = setupClient(wifiSSID, wifiPassword);
  }

  if (!wifiConnected) {
    // Fallback to AP mode if WiFi fails/has not been configured
    setupAccessPoint(apSSID, apPassword);
  }

  // Setup LIN
  linStack.setupSerial();

  // Setup OTA
  if (otaUsername.length() == 0) {
    otaUsername = OTA_USERNAME;
  }
  if (otaPassword.length() == 0) {
    otaPassword = OTA_PASSWORD;
  }
  httpUpdater.setup(&httpServer, otaUsername, otaPassword);

  // Setup HTTP server
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