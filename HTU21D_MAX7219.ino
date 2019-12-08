/* Sensormodul für Holzwerkstatt.
   Verkabelung ist einfach:
   MAX7219
   VCC: 3.3V
   GND: GND
   DIN: D7 (GPIO13)
   CS:  D8 (GPIO15)
   CLK: D5 (GPIO14)

   HTU21D I2C Temperatur/Feuchte-Sensor
   VCC: 3.3V
   GND: GND
   SCK: D1 (GPIO5)
   SDA: D2 (GPIO4)

   Für Deep-Sleep:
   Brücke zwischen D0 (DPIO16) und Reset
   (Achtung: Zum Laden der Firmware die Brücke entfernen)

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <U8g2lib.h>

#include <SPI.h>
#include <Wire.h>
#include "SparkFunHTU21D.h"

#include "wificredentials.h"

HTU21D myHumidity;
float humd;
float temp;


U8G2_MAX7219_32X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ D5, /* data=*/ D7, /* cs=*/ D8, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);

/* Credentials look at wificredentials.h
  const char* ssid = "";
  const char* password = "";
*/

const char* mqtt_server = "192.168.142.1";
const int sleepTimeS = 300;

long lastMsg = 0;
char msg[50];
int value = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  char tmp[8];
  char hmd[8];
  int retries = 0;

  delay(2000);

  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  Serial.begin(115200);
  Serial.println("Hello!");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  myHumidity.begin();
  humd = myHumidity.readHumidity();
  temp = myHumidity.readTemperature();
  dtostrf(temp, 2, 1, tmp);
  dtostrf(humd, 2, 1, hmd);

  Serial.print("HTU21 Temp: "); Serial.print(temp);
  Serial.print("\t\tHum: "); Serial.println(humd);

  u8g2.begin();
  u8g2.setFont(u8g2_font_profont12_tn);
  //u8g2.setFontDirection(0);
  u8g2.setDisplayRotation(U8G2_R2);
  u8g2.clearBuffer();

  u8g2.setCursor(0, 8);
  u8g2.print(humd, 0);
  u8g2.setCursor(16, 8);
  u8g2.print(temp, 0);

  u8g2.sendBuffer();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && retries < 60) {
    delay(250);
    digitalWrite(D4, LOW);
    Serial.print(".");
    delay(250);
    digitalWrite(D4, HIGH);
    retries++ ;
  }
  
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(D4, LOW);
    Serial.println("");
    Serial.println("WiFi connected");
    delay(200);

    reconnect();
    delay(500);

    snprintf (msg, 75, "%s", tmp);
    Serial.print("Publish message: ");
    Serial.println(msg);
    if (!client.publish("glab/holzwerkstatt/sensHTU21Temp", msg)) {
      Serial.println("Publish failed ");
    }
    delay(500);

    snprintf (msg, 75, "%s", hmd);
    Serial.print("Publish message: ");
    Serial.println(msg);
    if (!client.publish("glab/holzwerkstatt/sensHTU21Humd", hmd)) {
      Serial.println("Publish failed ");
    }
    // put your main code here, to run repeatedly:

  }
  else
  {
        Serial.println("WiFi failes");
  }

  delay(4000);
  Serial.println("...deep sleep");
  client.disconnect();
  ESP.deepSleep(sleepTimeS * 1000000, WAKE_RF_DEFAULT);
  delay(100000);

}

void callback(char* topic, byte* payload, unsigned int length) {
  int i;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for ( i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicString = String(topic);
  Serial.print("topicString: ");
  Serial.println(topicString);
}

void reconnect() {
  // Loop until we're reconnected
  digitalWrite(D3, HIGH);
  while (!client.connected()) {
    Serial.print("State ");
    Serial.println(client.state());
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      delay(500);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  digitalWrite(D3, LOW);

}

void loop() {

}
