#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>

#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

#define           PIN_IR            13  //an IR led is connected to GPIO12
#define           PIN_LED           4

const char        hostname[]      = "esp8266";
const byte        DNS_PORT        = 53;

char              ap_ssid[16]     = {0};
IPAddress         apIP(192, 168, 1, 1);
IPAddress         netMsk(255, 255, 255, 0);

char              wifi_ssid[16]   = {0};
char              wifi_pass[16]   = {0};
unsigned long     wifi_lastCon    = 0;

char              mqtt_server[32] = {0};
int               mqtt_port       = 1883;
boolean           mqtt_secure     = false;
char              mqtt_user[16]   = {0};
char              mqtt_pass[16]   = {0};
char              mqtt_topic[32]  = {0};
char              mqtt_cmd[32]   = {0};

int               mqtt_syncFreq   = 60;             // how often we have to send MQTT message
unsigned long     mqtt_lastPost   = 0;
char              buf[12];

DNSServer         dnsServer;
ESP8266WebServer  server(80);
WiFiClient        espClient;
PubSubClient      mqtt_client(espClient);
IRsend            irsend(PIN_IR);
  
void callback(char *topic, byte *payload, unsigned int length) 
{
  if (strcmp(topic, mqtt_cmd)==0)
  {   
    char* message = (char*)malloc(length);
    memcpy(message,payload,length);
    sendIR(message);
    free(message);

    digitalWrite(PIN_LED, HIGH);
    delay(250);
    digitalWrite(PIN_LED, LOW);
    delay(250);
  }
}

void setup() {  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_IR, OUTPUT);
    
  digitalWrite(PIN_LED, LOW);
     
  Serial.begin(115200);
  delay(100);
  Serial.println("Setup...");

  sprintf(ap_ssid, "IoT-%06X", ESP.getChipId());
  loadCredentials();
  if (strlen(wifi_ssid) == 0)
  {
    Serial.println("Start AP mode");
    digitalWrite(PIN_LED, HIGH);
      
    WiFi.softAPConfig(apIP, apIP, netMsk);
    
    WiFi.softAP(ap_ssid);  
    delay(500);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
       
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);
    Serial.println("DNS Server started");
  }
  else
  {
    Serial.println("Start STA mode");
    WiFi.mode(WIFI_STA); 
    WiFi.begin (wifi_ssid, wifi_pass);
  
    mqtt_client.setServer(mqtt_server, mqtt_port);
    mqtt_client.setCallback(callback);
    
    sprintf(mqtt_topic, "%06X", ESP.getChipId());
    sprintf(mqtt_cmd, "%s%s", mqtt_topic, "/command");
  }
    
  server.on("/", handleRoot);
  server.on("/wifisave", handleWifiSave);
  server.onNotFound ( handleNotFound );
        
  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  if (strlen(wifi_ssid) == 0)
  {
    dnsServer.processNextRequest();
  }
  else
  { 
    if (WiFi.status() != WL_CONNECTED)
    {
      if (millis() > (wifi_lastCon + 60000) || (wifi_lastCon == 0)) 
      {    
        //reconnect WiFi 
        Serial.println("");
        Serial.println("Connect Wi-Fi");
        WiFi.disconnect();
        delay(250);
        WiFi.begin (wifi_ssid, wifi_pass);
        wifi_lastCon = millis();
      }
      digitalWrite(PIN_LED, HIGH);
      delay(250);
      digitalWrite(PIN_LED, LOW);
      delay(250);
      Serial.print(".");
    } 
    else 
    {
      wifi_lastCon = millis();

      if (!mqtt_client.connected()) 
      { 
        Serial.println("");
        Serial.println("Connect MQTT");          
        if (mqtt_client.connect(ap_ssid)) 
        {
          Serial.println("MQTT connected");
          mqtt_client.subscribe(mqtt_cmd, 0);
        } 
        else 
        {
          Serial.print("MQTT failed, rc=");
          Serial.print(mqtt_client.state());    
        }
      }
      
      if ((millis() > (mqtt_lastPost + mqtt_syncFreq * 1000)) || (mqtt_lastPost == 0))
      {
        if (mqtt_client.connected()) 
        {
          mqtt_lastPost = millis(); 
          sprintf(buf, "%d", mqtt_lastPost);
          mqtt_client.publish(mqtt_topic, buf);
        }
      }
      mqtt_client.loop();
    }
  }

  //HTTP
  server.handleClient();
}

