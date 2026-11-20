/*
  ==========================================
       SPARK INDUSTRIAL 360
       ESP32 REMOTE OTA SYSTEM
       VERSION 1.2
  ==========================================
*/

#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <Update.h>

// ==========================================
// CURRENT FIRMWARE VERSION
// ==========================================

#define CURRENT_VERSION "1.2"

// ==========================================
// GITHUB OTA FILES
// ==========================================

const char* VERSION_URL =
"https://raw.githubusercontent.com/neo9586/SPARK-INDUSTRIAL-360-OTA/refs/heads/main/version.txt";

const char* FIRMWARE_URL =
"https://raw.githubusercontent.com/neo9586/SPARK-INDUSTRIAL-360-OTA/refs/heads/main/firmware.bin";

// ==========================================
// WIFI PORTAL SETTINGS
// ==========================================

#define AP_NAME "SPARK_OTA_SETUP"
#define AP_PASSWORD "SPARK360"

// ==========================================
// OBJECTS
// ==========================================

WiFiManager wifiManager;
WebServer server(80);

WiFiClientSecure secureClient;

// ==========================================
// OTA TIMER
// ==========================================

unsigned long lastCheck = 0;

const unsigned long CHECK_INTERVAL = 60000;

// ==========================================
// WEB PAGE
// ==========================================

void handleRoot() {

  String page = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>SPARK INDUSTRIAL 360</title>

    <style>
      body {
        background: #101820;
        color: white;
        font-family: Arial;
        text-align: center;
        padding: 20px;
      }

      .box {
        background: #1d2b36;
        padding: 25px;
        border-radius: 15px;
        max-width: 400px;
        margin: auto;
      }

      h1 {
        color: #00d4ff;
      }

      .status {
        color: #00ff88;
        font-size: 20px;
      }

      button {
        background: #ff4444;
        color: white;
        padding: 12px 25px;
        border: none;
        border-radius: 8px;
        font-size: 16px;
      }
    </style>
  </head>

  <body>

    <div class="box">

      <h1>SPARK INDUSTRIAL 360</h1>

      <h3>REMOTE OTA SYSTEM</h3>

      <p class="status">CONNECTED</p>

      <p>Firmware Version: 1.2</p>

      <p>WiFi Name: )rawliteral";

  page += WiFi.SSID();

  page += R"rawliteral(</p>

      <p>IP Address: )rawliteral";

  page += WiFi.localIP().toString();

  page += R"rawliteral(</p>

      <p>REMOTE OTA TEST 1.2 SUCCESS</p>

      <hr>

      <p>SPARK OTA SYSTEM ACTIVE</p>

      <br>

      <button onclick="location.href='/reset'">
        RESET WIFI
      </button>

    </div>

  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

// ==========================================
// RESET WIFI
// ==========================================

void handleReset() {

  server.send(
    200,
    "text/html",
    "<h2>WiFi Resetting...</h2>"
  );

  delay(1000);

  wifiManager.resetSettings();

  delay(500);

  ESP.restart();
}

// ==========================================
// CHECK GITHUB VERSION
// ==========================================

void checkForUpdate() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WIFI NOT CONNECTED");

    return;
  }

  Serial.println();
  Serial.println("==============================");
  Serial.println("CHECKING GITHUB VERSION");
  Serial.println("==============================");

  secureClient.setInsecure();

  HTTPClient http;

  http.setTimeout(15000);

  if (!http.begin(secureClient, VERSION_URL)) {

    Serial.println("VERSION URL CONNECTION FAILED");

    return;
  }

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {

    String remoteVersion = http.getString();

    remoteVersion.trim();

    Serial.print("CURRENT VERSION: ");
    Serial.println(CURRENT_VERSION);

    Serial.print("REMOTE VERSION: ");
    Serial.println(remoteVersion);

    float current = atof(CURRENT_VERSION);

    float remote = atof(remoteVersion.c_str());

    http.end();

    // ======================================
    // NEW VERSION AVAILABLE
    // ======================================

    if (remote > current) {

      Serial.println();
      Serial.println("NEW FIRMWARE AVAILABLE");
      Serial.println("STARTING REMOTE OTA UPDATE");

      secureClient.stop();

      delay(1000);

      secureClient.setInsecure();

      httpUpdate.rebootOnUpdate(true);

      httpUpdate.setFollowRedirects(
        HTTPC_STRICT_FOLLOW_REDIRECTS
      );

      t_httpUpdate_return result =
        httpUpdate.update(
          secureClient,
          FIRMWARE_URL
        );

      switch (result) {

        case HTTP_UPDATE_FAILED:

          Serial.println("OTA UPDATE FAILED");

          Serial.print("ERROR: ");

          Serial.println(
            httpUpdate.getLastErrorString()
          );

          break;

        case HTTP_UPDATE_NO_UPDATES:

          Serial.println("NO UPDATE FOUND");

          break;

        case HTTP_UPDATE_OK:

          Serial.println("OTA UPDATE SUCCESS");

          break;
      }

    }

    else {

      Serial.println("FIRMWARE ALREADY UPDATED");

    }

  }

  else {

    Serial.print("VERSION CHECK FAILED: ");

    Serial.println(httpCode);

    http.end();

  }

  Serial.println("==============================");
}

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();

  Serial.println("================================");
  Serial.println("SPARK INDUSTRIAL 360");
  Serial.println("REMOTE OTA SYSTEM");
  Serial.print("FIRMWARE VERSION: ");
  Serial.println(CURRENT_VERSION);
  Serial.println("================================");

  // WiFi Portal Timeout

  wifiManager.setConfigPortalTimeout(180);

  // WiFi Connection Timeout

  wifiManager.setConnectTimeout(30);

  Serial.println("CONNECTING WIFI...");

  bool connected = wifiManager.autoConnect(
    AP_NAME,
    AP_PASSWORD
  );

  if (!connected) {

    Serial.println("WIFI CONNECTION FAILED");

    delay(3000);

    ESP.restart();
  }

  Serial.println();

  Serial.println("WIFI CONNECTED");

  Serial.print("WIFI NAME: ");

  Serial.println(WiFi.SSID());

  Serial.print("IP ADDRESS: ");

  Serial.println(WiFi.localIP());

  // ======================================
  // START WEB SERVER
  // ======================================

  server.on("/", handleRoot);

  server.on("/reset", handleReset);

  server.begin();

  Serial.println("WEB SERVER STARTED");

  // ======================================
  // FIRST OTA VERSION CHECK
  // ======================================

  checkForUpdate();

  lastCheck = millis();
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  server.handleClient();

  // ======================================
  // CHECK OTA EVERY 60 SECONDS
  // ======================================

  if (millis() - lastCheck >= CHECK_INTERVAL) {

    lastCheck = millis();

    checkForUpdate();

  }

}