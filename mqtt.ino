unsigned long     lastPost        = 0;
float             temp            = 0;
float             prevTemp        = 0; 
float             hum             = 0; 
float             prevHum         = 0;
char              buf[10];
  
void callback(char *topic, byte *payload, unsigned int length) {
//  Serial.print("Message arrived ");
//  Serial.println(topic);
  if (strcmp(topic, mqtt_topic)==0){
    char* message = (char*)malloc(length);
    memcpy(message,payload,length);
    sendIR(message);
    free(message);
  }
}

void reconnect() {
  while (!mqtt_client.connected()) {   
    if (mqtt_client.connect(ap_ssid)) {
      Serial.println("mqtt connected");
      mqtt_client.subscribe(mqtt_topic, 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());    
      delay(5000);
    }
  }
}
