void reboot() {
  Serial.println("rebooting the device.");
  delay(1000);
  ESP.restart();
}

void update_started() {
  Serial.println("firmware update process started.");
}

void update_finished() {
  Serial.println("firmware update process finished.");
}

void update_progress(int cur, int total) {
  Serial.printf("firmware update process at %d of %d bytes\n", cur, total);
}

void update_error(int err) {
  Serial.printf("firmware update fatal error code: %d\n", err);
}

void performFirmwareUpdate(String updateURL) {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  Serial.println("updating firmware from: " + updateURL);

  WiFiClient client;

  ESPhttpUpdate.setLedPin(LED, HIGH);
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, updateURL);

}

void sendHTTPRequest(String httpURL, String data) {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  https.begin(*client, httpURL);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String ip = WiFi.localIP().toString();
  int httpResponseCode = https.POST("payload=" + data + "&MAC=" + WiFi.macAddress() + "&IP=" + ip + "&SSID=" + WiFi.SSID() + "&RSSI=" + WiFi.RSSI());
  digitalWrite(LED, HIGH);
  delay(25);
  digitalWrite(LED, LOW);

  // check for the commands
  if (httpResponseCode > 0) {
    Serial.println(httpResponseCode);
    Serial.println(https.getString());
    String response = https.getString();

    // process commands
    if (response.indexOf("UPDATE") != -1) {
      int delimiter;
      delimiter = response.indexOf(">");
      if (delimiter != -1) {    
        String updateURL = response.substring(delimiter + 1, response.length());
        performFirmwareUpdate(updateURL);
      }
    }

    if (response == "REBOOT") {
      reboot();
    }
  }

  https.end();
}
