/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
//  Serial.println("captivePortal");
//  Serial.println("server.hostHeader(): ");
//  Serial.println(server.hostHeader());
//  Serial.println("isIp(server.hostHeader(): ");
//  Serial.println(isIp(server.hostHeader()));
  
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(hostname)+".local")) {
//    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Handle root or redirect to captive portal */
void handleRoot() {
//  Serial.println("handleRoot");
  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.

  server.sendContent("<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>" + String(ap_ssid) + "</title>"
    "<style type='text/css'>* { font-size: 16px; font-family: 'Verdana' } html { margin:0; height: 100%; background: linear-gradient(rgba(0, 0, 0, 0),rgba(0, 100, 200, 0.2)); text-align: center; } "
    "input, select { webkit-box-sizing: border-box; moz-box-sizing: border-box; box-sizing: border-box; padding: 10px; margin: 5px auto; border: 1px solid #ccc; border-radius: 10px; width: 90%; height: 40px; text-align: center; } "
    "</style></head><body><form method='POST' action='wifisave' class='content'><h1>" + String(ap_ssid) + " setup</h1><select id='wifi_ssid' name='wifi_ssid'>"
  );

//  Serial.println("scan start");
  int n = WiFi.scanNetworks();
//  Serial.println("scan done");
  for (int i = 0; i < n; i++) {
    server.sendContent("<option" + String((WiFi.SSID(i) == wifi_ssid) ? " selected" : "") + ">" + WiFi.SSID(i) + "</option>");
  }
  
  server.sendContent(
    "</select><br/><input type='password' name='wifi_password' placeholder='Wi-Fi Password' value='" + String(wifi_pass) + "'/><br/>"
    "<input type='text' name='mqtt_server' placeholder='MQTT server address' value='" + String(mqtt_server) + "'/><br/><input type='text' name='mqtt_port' placeholder='MQTT server port' value='" + String(mqtt_port) + "'/><br/>"
    "<select name='mqtt_secure'/><option>open connection</option><option" + String((mqtt_secure == true) ? " selected" : "") + ">secured</option></select><br/><input type='text' name='mqtt_user' placeholder='MQTT Username' value='" + String(mqtt_user) + "'/><br/>"
    "<input type='password' name='mqtt_pass' placeholder='MQTT Password' value='" + String(mqtt_pass) + "'/><br/>"
    "<input type='text' name='mqtt_syncFreq' placeholder='Sync frequency, pulses' value='" + String(mqtt_syncFreq) + "'/><br/>"
    "<input type='submit' value='SAVE'/></form>"
  );  
  if (strlen(wifi_ssid) != 0){
    server.sendContent(
      "<p>IP: " + WiFi.localIP().toString() + "<br/>"
      "WIFI: " + ((WiFi.status() == WL_CONNECTED) ? "connected" : "-") + "<br/>"
      "RSSI: " + String(WiFi.RSSI()) + " dBm<br/>" 
      "MQTT: " + ((mqtt_client.connected() == true) ? "connected" : "-") + "<br/>"    
      "</p>"    
    );
  }
  server.sendContent("</body></html>");
  server.client().stop(); // Stop is needed because we sent no content length  
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
//  Serial.println("handleWifiSave");
  
  server.arg("wifi_ssid").toCharArray(wifi_ssid, sizeof(wifi_ssid) - 1);
  server.arg("wifi_password").toCharArray(wifi_pass, sizeof(wifi_pass) - 1);
  server.arg("mqtt_server").toCharArray(mqtt_server, sizeof(mqtt_server) - 1);
  mqtt_port = server.arg("mqtt_port").toInt();
  if (server.arg("mqtt_secure") == "secured") mqtt_secure = true;
  server.arg("mqtt_user").toCharArray(mqtt_user, sizeof(mqtt_user) - 1);
  server.arg("mqtt_pass").toCharArray(mqtt_pass, sizeof(mqtt_pass) - 1);
  mqtt_syncFreq = server.arg("mqtt_syncFreq").toInt();

  saveCredentials();
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.

  server.sendContent(
    "<!doctype html><html><head><meta charset='utf-8'><title>" + String(ap_ssid) + "</title>"
    "<meta http-equiv='refresh' content='15;URL=http://atomigy.com' /><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<style type='text/css'> * { font-size: 16px; font-family: 'Verdana'; } html { margin:0; height: 100%; background: linear-gradient(rgba(0, 0, 0, 0),rgba(0, 100, 200, 0.2)); text-align: center; }</style></head>"
    "<body><h1>Congratulation!</h1><p>Configuration is complete</p><p>Your " + String(ap_ssid) + " will be rebooted and should functioning properly</p><h1 id='countdown'></h1>"
    "<script>var delay = 15; var el = document.getElementById('countdown'); el.innerHTML = 'in ' + delay + ' seconds ...'; var countdown = setInterval(function() { if (delay>0) { el.innerHTML = 'in ' + delay + ' seconds ...'; delay = delay-1;} }, 1000); var timer = setTimeout(function() { window.location='http://atomigy.com' }, delay * 1000);</script></body></html>"
  );
  server.client().stop();                 // Stop is needed because we sent no content length
  //Serial.println("RESTART");
  ESP.restart();
}

void handleNotFound() {
//  Serial.println("handleNotFound");
    
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 404, "text/plain", message );
}

