#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>

#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

#define           THING_TYPE        "ir"
#define           PIN_IR            13  //an IR led is connected to GPIO12
#define           PIN_GRN           4

const char        hostname[]      = "esp8266";
const byte        DNS_PORT        = 53;

char              ap_ssid[16]     = {0};
IPAddress         apIP(192, 168, 1, 1);
IPAddress         netMsk(255, 255, 255, 0);

char              wifi_ssid[16]   = {0};
char              wifi_pass[16]   = {0};

char              mqtt_server[32] = {0};
int               mqtt_port       = 1883;
boolean           mqtt_secure     = false;
char              mqtt_user[16]   = {0};
char              mqtt_pass[16]   = {0};
char              mqtt_topic[32]  = {0};

unsigned long     lastConnect     = 0;

DNSServer         dnsServer;
ESP8266WebServer  server(80);
WiFiClient        espClient;
PubSubClient      mqtt_client(espClient);
IRsend            irsend(PIN_IR);
  
void setup() {      
  Serial.begin(115200);
  delay(100);
  Serial.println("Setup...");
 
  pinMode(PIN_IR, OUTPUT);
  pinMode(PIN_GRN, OUTPUT);

  digitalWrite(PIN_IR, LOW);
  digitalWrite(PIN_GRN, HIGH);

  sprintf(ap_ssid, "IoT-%06X", ESP.getChipId());
  loadCredentials();
  if (strlen(wifi_ssid) == 0){
    Serial.println("Start AP mode");
    WiFi.softAPConfig(apIP, apIP, netMsk);
    
    WiFi.softAP(ap_ssid);  
    delay(500);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
       
    server.on("/", handleRoot);
    server.on("/wifisave", handleWifiSave);
    server.onNotFound ( handleNotFound );

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);
    Serial.println("DNS Server started");
  }else{
    Serial.println("Start STA mode");
    WiFi.mode(WIFI_STA); 
    WiFi.begin (wifi_ssid, wifi_pass);
  
    server.on("/", handleRoot);
    irsend.begin();
    mqtt_client.setServer(mqtt_server, mqtt_port);
    mqtt_client.setCallback(callback);

    sprintf(mqtt_topic, "%s%c%06X", THING_TYPE, '/', ESP.getChipId());
    Serial.println(mqtt_topic);
    if (strlen(mqtt_user) != 0) { 
      strcat(mqtt_topic, mqtt_user);
      strcat(mqtt_topic, "/");    
    }
  }
  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  if (strlen(wifi_ssid) == 0){
    dnsServer.processNextRequest();
  }else{
    if (millis() < lastConnect) lastConnect = millis();  
    if (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      if (millis() > (lastConnect + 60000) ) {
        Serial.println("Reconnect Wi-Fi");
        WiFi.disconnect();
        delay(250);
        WiFi.begin (wifi_ssid, wifi_pass);
        lastConnect = millis();
      }
      //blink
      digitalWrite(PIN_GRN, LOW);
      delay(250);
      digitalWrite(PIN_GRN, HIGH);
      delay(250);
    } else {
      lastConnect = millis();

      //MQTT
      if (!mqtt_client.connected()) reconnect();
      else{
        mqtt_client.loop();  
      }
    }
  }
  
  //HTTP
  server.handleClient();
}

