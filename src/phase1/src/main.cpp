/*
 * Pico W-based Tesla trailer controller unit.
 * 2024-2025 Mike Marvin (magico13)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <LittleFS.h>
#include <LEAmDNS.h>

#include "lin.h"
#define VERSION "2025-05-05.1"

#define LIN_FRAME_PID 0xCF
#define TAIL_PIN 2
#define LEFT_PIN 3
#define RIGHT_PIN 4

const char* left_arrow_icon = "◄";
const char* right_arrow_icon = "►";
const char* headlight_icon = "💡";

bool output_enabled = false;
bool process_frames = true;
bool left_state = false;
bool right_state = false;
bool tail_state = false;

const char* AP_SSID     = "TCU-Access-Point";
const char* AP_PASSWORD = "123456789";

const char* OTA_USERNAME = "ota";
const char* OTA_PASSWORD = "123456789";

String wifiSSID;
String wifiPassword;
int wifiTimeout = 30;
String apSSID;
String apPassword;
String otaUsername;
String otaPassword;

bool led_state = false;
WebServer httpServer(80);
HTTPUpdateServer httpUpdater;

String latestFrameString = "";
bool lfsReady = false;
bool autoRefresh = false;

// LIN Variables
lin linStack;

// mDNS Responder
MDNSResponder mdns; // Declare mDNS responder

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

String populateActiveLights() {
  String activeLights = "";
  if (left_state) {
    activeLights += left_arrow_icon; // Left arrow
  }
  if (tail_state) {
    activeLights += headlight_icon; // Light bulb
  }
  if (right_state) {
    activeLights += right_arrow_icon; // Right arrow
  }
  return activeLights;
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

bool setupClient(char* ssid, char* password, int timeout) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  //if password provided, use it, otherwise assume open network
  if (strlen(password) > 0) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin(ssid);
  }
  Serial.println("");

  // Wait for connection, with timeout
  unsigned long timeoutMs = millis() + (timeout * 1000);
  while (WiFi.status() != WL_CONNECTED) {
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state);
    delay(500);
    Serial.print(".");
    if (millis() > timeoutMs) {
      Serial.print("Failed to connect to WiFi with SSID '");
      Serial.print(ssid);
      Serial.print("' pass '");
      Serial.print(password);
      Serial.println("'");
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

float getOnboardTemperature() {
  // Get the temperature in Celsius
  float temperature_celsius = analogReadTemp();
  
  // Convert to Fahrenheit
  return temperature_celsius * 9.0 / 5.0 + 32.0;
}

void setLightState(int pin, bool state) {
  if (output_enabled) {
    digitalWrite(pin, state);
  }
}

void toggleOutputEnabled() {
  output_enabled = !output_enabled;
  process_frames = true;
  // if filesystem is ready, set the state in a file so we can read it on boot
  if (lfsReady) {
    File file = LittleFS.open("/config/output_enabled.txt", "w");
    if (file) {
      file.println(output_enabled ? "1" : "0");
      file.close();
    }
  }

  if (!output_enabled) {
    // If we're disabling the output, turn off all the lights
    digitalWrite(LEFT_PIN, false);
    digitalWrite(RIGHT_PIN, false);
    digitalWrite(TAIL_PIN, false);
  }

  Serial.print("Output enabled: ");
  Serial.println(output_enabled);
}

void processLightLINFrame(byte dataByte) {
  // First bit is left light, second bit is right light, third bit is tail light
  // This is a four pin trailer connector, so brakes and reverse do not matter, but are present in the LIN frame
  // See docs if you need to add support for those
  left_state = dataByte & 0x01;
  right_state = dataByte & 0x02;
  tail_state = dataByte & 0x04;
  setLightState(LEFT_PIN, left_state);
  setLightState(RIGHT_PIN, right_state);
  setLightState(TAIL_PIN, tail_state);
}

void runTestSequence() {
  bool original_output_enabled = output_enabled;
  output_enabled = true;
  process_frames = false;

  // Set all lights to false
  setLightState(LEFT_PIN, false);
  setLightState(RIGHT_PIN, false);
  setLightState(TAIL_PIN, false);

  setLightState(LEFT_PIN, true);
  delay(1000);
  setLightState(LEFT_PIN, false);
  setLightState(RIGHT_PIN, true);
  delay(1000);
  setLightState(RIGHT_PIN, false);
  setLightState(TAIL_PIN, true);
  delay(1000);
  setLightState(TAIL_PIN, false);
  flashLED(4, 100);

  output_enabled = original_output_enabled;
  process_frames = true;
}

void handleRoot() {
  process_frames = true;
  // open index.html from /web folder
  File file = LittleFS.open("/web/index.html", "r");
  if (!file) {
    httpServer.send(404, "text/plain", "File not found");
    return;
  }
  // replace placeholders in the HTML file
  String html = file.readString();
  html.replace("{active_lights}", populateActiveLights());

  html.replace("{auto_refresh}", autoRefresh ? "<meta http-equiv=\"refresh\" content=\"1\">" : "");
  html.replace("{output_status}", output_enabled ? "Active" : "Disabled");
  html.replace("{lin_frame}", latestFrameString);
  html.replace("{tcu_temp}", String(getOnboardTemperature()));
  html.replace("{version}", VERSION);
  httpServer.send(200, "text/html", html);
}

void handleRunTestPage() {
  // open runTest.html from /web folder
  File file = LittleFS.open("/web/runTest.html", "r");
  if (!file) {
    httpServer.send(404, "text/plain", "File not found");
    return;
  }
  httpServer.streamFile(file, "text/html");

  runTestSequence();
}

void handleSettingsPage() {
  // open settings.html from /web folder
  File file = LittleFS.open("/web/settings.html", "r");
  if (!file) {
    httpServer.send(404, "text/plain", "File not found");
    return;
  }
  String html = file.readString();
  html.replace("{current_wifi_ssid}", wifiSSID);
  html.replace("{current_wifi_password}", wifiPassword);
  html.replace("{current_wifi_timeout}", String(wifiTimeout));
  html.replace("{current_ap_ssid}", apSSID);
  html.replace("{current_ap_password}", apPassword);
  html.replace("{current_ota_username}", otaUsername);
  httpServer.send(200, "text/html", html);
}

void handleUpdateSettings() {
  // check the ota password and verify it is the same as the current one, 403 if not
  // write to serial what values we got for the ota password
  if (!httpServer.hasArg("ota_password_current") || httpServer.arg("ota_password_current") != otaPassword) {
    Serial.println("Invalid OTA password");
    httpServer.send(403, "text/plain", "Invalid OTA password");
    return;
  }
  Serial.println("Valid OTA password, updating settings...");
  
  // We do allow nulling out the wifi settings, to always use AP mode
  String newWifiSSID = "";
  if (httpServer.hasArg("wifi_ssid")) 
    newWifiSSID = httpServer.arg("wifi_ssid");
  String newWifiPassword = "";
  if (httpServer.hasArg("wifi_password")) 
    newWifiPassword = httpServer.arg("wifi_password");
  newWifiSSID.trim();
  newWifiPassword.trim();
  wifiSSID = newWifiSSID;
  wifiPassword = newWifiPassword;

  if (httpServer.hasArg("wifi_timeout")) {
    wifiTimeout = httpServer.arg("wifi_timeout").toInt();
  }

  if (httpServer.hasArg("ap_ssid") && httpServer.hasArg("ap_password")) {
    String newApSSID = httpServer.arg("ap_ssid");
    String newApPassword = httpServer.arg("ap_password");
    newApSSID.trim();
    newApPassword.trim();
    if (newApSSID.length() > 0 && newApPassword.length() > 0) {
      apSSID = newApSSID;
      apPassword = newApPassword;
    }
  }

  if (httpServer.hasArg("ota_username") && httpServer.hasArg("ota_password")) {
    String newOtaUsername = httpServer.arg("ota_username");
    String newOtaPassword = httpServer.arg("ota_password");
    newOtaUsername.trim();
    newOtaPassword.trim();
    if (newOtaUsername.length() > 0 && newOtaPassword.length() > 0) {
      otaUsername = newOtaUsername;
      otaPassword = newOtaPassword;
    }
  }

  // Save the settings to the filesystem
  if (lfsReady) {
    File wifiConfig = LittleFS.open("/config/wifi.txt", "w");
    if (wifiConfig) {
      wifiConfig.println(wifiSSID);
      wifiConfig.println(wifiPassword);
      wifiConfig.println(wifiTimeout);
      wifiConfig.close();
    }

    File apConfig = LittleFS.open("/config/ap.txt", "w");
    if (apConfig) {
      apConfig.println(apSSID);
      apConfig.println(apPassword);
      apConfig.close();
    }

    File otaConfig = LittleFS.open("/config/ota.txt", "w");
    if (otaConfig) {
      otaConfig.println(otaUsername);
      otaConfig.println(otaPassword);
      otaConfig.close();
    }
  }

  // reboot to use new settings
  httpServer.send(200, "text/plain", "Settings updated, rebooting...");
  delay(1000);
  watchdog_reboot(0, 0, 0);
}

void handleToggleOutputPage() {
  toggleOutputEnabled();
  // redirect to the main page
  httpServer.sendHeader("Location", "/",true);
  httpServer.send(302, "text/plain", "");
}

void handleToggleAutoRefresh() {
  autoRefresh = !autoRefresh;
  // redirect to the main page
  httpServer.sendHeader("Location", "/",true);
  httpServer.send(302, "text/plain", "");
}

void handleControlPage() {
  // Turn on output but turn off lin processing
  output_enabled = true;
  process_frames = false;
  if (httpServer.hasArg("id")) {
    int id = httpServer.arg("id").toInt();
    switch (id) {
      case 0: // Left Signal
        left_state = !left_state;
        break;
      case 1: // Right Signal
        right_state = !right_state;
        break;
      case 2: // Tail Lights
        tail_state = !tail_state;
        break;
      default:
        // Unrecognized id;
        break;
    }
  } else {
    // If no id is provided, turn off all lights
    left_state = false;
    right_state = false;
    tail_state = false;
  }

  setLightState(LEFT_PIN, left_state);
  setLightState(RIGHT_PIN, right_state);
  setLightState(TAIL_PIN, tail_state);

  // open control.html from /web folder
  File file = LittleFS.open("/web/control.html", "r");
  if (!file) {
    httpServer.send(404, "text/plain", "File not found");
    return;
  }
  
  String html = file.readString();
  html.replace("{active_lights}", populateActiveLights());
  httpServer.send(200, "text/html", html);
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
  if (lfsReady) {
    File wifiConfig = LittleFS.open("/config/wifi.txt", "r");
    if (wifiConfig && wifiConfig.size() > 0) {
      wifiSSID = wifiConfig.readStringUntil('\n');
      wifiSSID.trim();
      wifiPassword = wifiConfig.readStringUntil('\n');
      wifiPassword.trim();
      wifiTimeout = wifiConfig.parseInt();
      wifiConfig.close();
    }

    File apConfig = LittleFS.open("/config/ap.txt", "r");
    if (apConfig && apConfig.size() > 0) {
      apSSID = apConfig.readStringUntil('\n');
      apSSID.trim();
      apPassword = apConfig.readStringUntil('\n');
      apPassword.trim();
      apConfig.close();
    }

    File otaConfig = LittleFS.open("/config/ota.txt", "r");
    if (otaConfig && otaConfig.size() > 0) {
      otaUsername = otaConfig.readStringUntil('\n');
      otaUsername.trim();
      otaPassword = otaConfig.readStringUntil('\n');
      otaPassword.trim();
      otaConfig.close();
    }

    // If we were active before, we should stay active until disabled
    // We might reboot just because the doors are all closed in park
    // So we should remember the state
    File outputConfig = LittleFS.open("/config/output_enabled.txt", "r");
    if (outputConfig && outputConfig.size() > 0) {
      output_enabled = outputConfig.parseInt();
      outputConfig.close();
    }
  }

  // Setup WiFi
  bool wifiConnected = false;
  if (wifiSSID.length() > 0 && wifiPassword.length() > 0) {
    char wifiSSIDArray[32];
    char wifiPasswordArray[32];
    wifiSSID.toCharArray(wifiSSIDArray, 32);
    wifiPassword.toCharArray(wifiPasswordArray, 32);
    wifiConnected = setupClient(wifiSSIDArray, wifiPasswordArray, wifiTimeout);
  }

  if (!wifiConnected) {
    // Fallback to AP mode if WiFi fails/has not been configured
    char apSSIDArray[32];
    char apPasswordArray[32];
    apSSID.toCharArray(apSSIDArray, 32);
    apPassword.toCharArray(apPasswordArray, 32);
    setupAccessPoint(apSSIDArray, apPasswordArray);
  }

  // Set hostname for the server
  const char* hostname = "trailercontroller";
  WiFi.setHostname(hostname);

  // Start mDNS service using LEAmDNS
  if (mdns.begin(hostname)) {
    Serial.println("mDNS responder started. Hostname: " + String(hostname) + ".local");
    mdns.addService("http", "tcp", 80); // Add HTTP service on port 80
  } else {
    Serial.println("Error setting up mDNS responder");
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
  httpServer.on("/runTest", handleRunTestPage);
  httpServer.on("/settings", handleSettingsPage);
  httpServer.on("/updateSettings", handleUpdateSettings);
  httpServer.on("/toggleOutput", handleToggleOutputPage);
  httpServer.on("/autoRefresh", handleToggleAutoRefresh);
  httpServer.on("/control", handleControlPage);
  httpServer.onNotFound([]() {
    httpServer.send(404, "text/plain", "File not found");
  });
  httpServer.begin();

  Serial.println("HTTP server started");
  led_state = false;
  digitalWrite(LED_BUILTIN, led_state);
}

void loop(void) {
  httpServer.handleClient();

  // Handle mDNS queries
  mdns.update();

  // Handle LIN frames
  if (process_frames) {
    short bytesRead = linStack.updateFrame(LIN_FRAME_PID);
    if (bytesRead > 0) {
      // Process the LIN frame
      latestFrameString = "";
      for (int i = 0; i < bytesRead; i++) {
        latestFrameString += "0x" + String(linStack.dataBuffer[i], HEX) + " ";
      }
      // Check if the checksum is valid
      byte calculatedChecksum = linStack.calculateChecksum(linStack.dataBuffer, bytesRead - 1);
      byte receivedChecksum = linStack.dataBuffer[bytesRead - 1];
      if (calculatedChecksum == receivedChecksum) {
        latestFrameString += "OK";
        // Process the LIN frame
        processLightLINFrame(linStack.dataBuffer[2]);
      } else {
        latestFrameString += "ERR 0x" + String(calculatedChecksum, HEX);
      }
      Serial.print(latestFrameString);
    }
  }
}