// constants
const int LED = 4;

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "utils.h"

const long interval = 15000;
const long wifiInterval = 30000;
const String URL = "https://url/to/your/server/";
const String deviceName = "uniesp-001";

// variables
unsigned long previousMillis = 0;
unsigned long previousWifiMillis = 0;
unsigned long currentMillis = 0;
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

// setup
void setup(void) {
  pinMode(LED, OUTPUT);

  // intial LED state
  digitalWrite(LED, HIGH);

  Serial.begin(115200);
  delay(1000);
  Serial.println("ready.");

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("SSID1", "PSK1");
  wifiMulti.addAP("SSID2", "PSK2");
  wifiMulti.addAP("SSID3", "PSK3");

  // wait for wifi connection
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // WiFi connected
  digitalWrite(LED, LOW);
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(deviceName)) {
    Serial.println("MDNS responder started.");
  }

  server.on("/", []() {
    server.send(200, "text/html", "uniesp,v1.0," + deviceName + ".local," + WiFi.macAddress() + "," + WiFi.localIP().toString() + "," + WiFi.SSID() + "," + WiFi.RSSI());
  });

  server.begin();
  Serial.println("webserver started.");

}


void loop(void) {

  currentMillis = millis();
  if (currentMillis - previousWifiMillis >= wifiInterval && currentMillis != 0) {
    //check if interval has passed
    previousWifiMillis = currentMillis;
    Serial.print("checking for wifi status: ");
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("FAIL");
      reboot();
    } else {
      Serial.println("PASS");
    }
  }

  if (currentMillis - previousMillis >= interval && currentMillis != 0) {
    //check if interval has passed
    previousMillis = currentMillis;
    Serial.println("timer is up");

    Serial.println("pinging the hub.");
    sendHTTPRequest(URL, ">PING");
  }

  server.handleClient();
  MDNS.update();

}
