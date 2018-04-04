/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  char ok[2+1] = "OK";  
  int address = 0;
  
  EEPROM.begin(512);
  
  EEPROM.get(address, wifi_ssid);
  address += sizeof(wifi_ssid);
  EEPROM.get(address, wifi_pass);
  address += sizeof(wifi_pass);
  EEPROM.get(address, mqtt_server);
  address += sizeof(mqtt_server);
  EEPROM.get(address, mqtt_port);
  address += sizeof(mqtt_port);
  EEPROM.get(address, mqtt_secure);
  address += sizeof(mqtt_secure);      
  EEPROM.get(address, mqtt_user);
  address += sizeof(mqtt_user);  
  EEPROM.get(address, mqtt_pass);
  address += sizeof(mqtt_pass);  
  EEPROM.get(address, ok);
  address += sizeof(ok);  
       
  EEPROM.end();
  
  if (String(ok) != String("OK")) {
    wifi_ssid[0] = 0;
    wifi_pass[0] = 0;
    mqtt_server[0] = 0;
    mqtt_port = 1883;
    mqtt_secure = false;
    mqtt_user[0] = 0;
    mqtt_pass[0] = 0;
  }
/*
  Serial.println("Recovered data:");
  Serial.print("wifi_ssid:");
  Serial.println(wifi_ssid);
  Serial.print("wifi_pass:");
  Serial.println(wifi_pass);
  Serial.print("mqtt_server:");
  Serial.println(mqtt_server);
  Serial.print("mqtt_port:");
  Serial.println(mqtt_port);
  Serial.print("mqtt_secure:");
  Serial.println(mqtt_secure);    
  Serial.print("mqtt_user:");
  Serial.println(mqtt_user);   
  Serial.print("mqtt_pass:");
  Serial.println(mqtt_pass);     
*/
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  char ok[2+1] = "OK";  
  int address = 0;
  
  EEPROM.begin(512);

  EEPROM.put(address, wifi_ssid);
  address += sizeof(wifi_ssid);
  EEPROM.put(address, wifi_pass);
  address += sizeof(wifi_pass);
  EEPROM.put(address, mqtt_server);
  address += sizeof(mqtt_server);
  EEPROM.put(address, mqtt_port);
  address += sizeof(mqtt_port);
  EEPROM.put(address, mqtt_secure);
  address += sizeof(mqtt_secure);
  EEPROM.put(address, mqtt_user);
  address += sizeof(mqtt_user);
  EEPROM.put(address, mqtt_pass);
  address += sizeof(mqtt_pass);
  EEPROM.put(address, ok);
  address += sizeof(ok);
    
  EEPROM.commit();
  EEPROM.end();
  
}

void clearCredentials() {
  char ok[2+1] = "OK";  
  int address = 0;
  
  EEPROM.begin(512);

  EEPROM.put(address, "");
  address += sizeof(wifi_ssid);
  EEPROM.put(address, "");
  address += sizeof(wifi_pass);
  EEPROM.put(address, "");
  address += sizeof(mqtt_server);
  EEPROM.put(address, 1883);
  address += sizeof(mqtt_port);
  EEPROM.put(address, false);
  address += sizeof(mqtt_secure);
  EEPROM.put(address, "");
  address += sizeof(mqtt_user);
  EEPROM.put(address, "");
  address += sizeof(mqtt_pass);
  EEPROM.put(address, ok);
  address += sizeof(ok);
    
  EEPROM.commit();
  EEPROM.end();
}

