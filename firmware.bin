
/*
  SPARK INDUSTRIAL 360
  WiFi Configuration Portal
  ESP32 30 PIN
*/

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>

WebServer server(80);
WiFiManager wifiManager;

#define AP_NAME "SPARK_OTA_SETUP"
#define AP_PASSWORD "SPARK360"

String wifiStatus = "STARTING";

void handleRoot() {

  String page = "";

  page += "<html><head>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<title>SPARK INDUSTRIAL 360</title>";
  page += "</head><body>";

  page += "<h2>SPARK INDUSTRIAL 360</h2>";
  page += "<h3>ESP32 WiFi Status</h3>";

  page += "<p>Status: ";
  page += WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED";
  page += "</p>";

  page += "<p>IP Address: ";
  page += WiFi.localIP().toString();
  page += "</p>";

  page += "<p>WiFi Name: ";
  page += WiFi.SSID();
  page += "</p>";

  page += "<hr>";
  page += "<p>WiFi Configuration Portal Active</p>";

  page += "</body></html>";

  server.send(200, "text/html", page);
}

void handleReset() {

  server.send(200, "text/plain", "WiFi Resetting...");

  delay(1000);

  wifiManager.resetSettings();

  ESP.restart();
}

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("SPARK INDUSTRIAL 360");
  Serial.println("WIFI PORTAL STARTING");
  Serial.println("==============================");

  // WiFi Portal

  wifiManager.setConfigPortalTimeout(180);

  wifiManager.setConnectTimeout(30);

  bool connected = wifiManager.autoConnect(
    AP_NAME,
    AP_PASSWORD
  );

  if (!connected) {

    Serial.println("WiFi Connection Failed");

    ESP.restart();
  }

  Serial.println("WIFI CONNECTED");

  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP());

  // Web Server

  server.on("/", handleRoot);

  server.on("/reset", handleReset);

  server.begin();

  Serial.println("WEB SERVER STARTED");
}

void loop() {

  server.handleClient();

}
