/*
         " Plug NB-IoT Shield on Arduino Mega "
    1. Jump Pin 8 (RX) NB to Pin 48 Mega
    2. Jump Pin 9 (TX) NB to Pin 46 Mega

    If you have any questions, please contact us at https://www.facebook.com/AISDEVIO
*/
#include "AIS_BC95_API.h"
AIS_BC95_API nb;

String       host        = "";               // Your IP address or mqtt server url
String       port        = "";               // Your server port
String       clientID    = "";               // Your client id < 120 characters
String       topic       = "";               // Your topic     < 128 characters
String       payload     = "HelloWorld!";    // Your payload   < 500 characters
String       username    = "";               // username for mqtt server, username <= 100 characters
String       password    = "";               // password for mqtt server, password <= 100 characters
unsigned int subQoS      = 0;                // subQoS      : unsigned int : 0, 1, 2
unsigned int pubQoS      = 0;                // pubQoS      : unsigned int : 0, 1, 2
unsigned int pubRetained = 0;                // pubRetained : unsigned int : 0, 1

const long    interval       = 20000;    // time in millisecond
unsigned long previousMillis = 0;
int           cnt            = 0;

void setup() {
  Serial.begin(9600);
  nb.debug = false;
  nb.begin();
  setupMQTT();
  nb.setCallback(callback);
  previousMillis = millis();
}
void loop() {
  nb.MQTTresponse();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    cnt++;
    connectStatus();
    nb.publish(topic, payload + String(cnt));
    //  nb.publish(topic, payload, pubQoS, pubRetained);
    previousMillis = currentMillis;
  }
}

//=========== MQTT Function ================
void setupMQTT() {
  if (nb.connectMQTT(host, port, clientID, username, password)) {
    nb.subscribe(topic, subQoS);
  }
  // nb.unsubscribe(topic);
}
void connectStatus() {
  if (!nb.isMQTTConnected()) {
    if (nb.debug)
      Serial.println("reconnectMQ ");
    setupMQTT();
  }
}
void callback(String &topic, String &payload) {
  Serial.println("-------------------------------");
  Serial.println("# Message from Topic \"" + topic + "\" : " + payload);
}
