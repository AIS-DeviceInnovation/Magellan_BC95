/*
         " Plug NB-IoT Shield on Arduino Mega "
    1. Jump Pin 8 (RX) NB to Pin 48 Mega
    2. Jump Pin 9 (TX) NB to Pin 46 Mega

    If you have any questions, please contact us at https://www.facebook.com/AISDEVIO
*/
#include "AIS_BC95_API.h"
AIS_BC95_API nb;

String       host         = "";               // Your IP address or mqtt server url
String       port         = "";               // Your server port
String       clientID     = "";               // Your client id < 120 characters
String       topic        = "";               // Your topic     < 128 characters
String       payload      = "HelloWorld!";    // Your payload   < 500 characters
String       username     = "";    // username for mqtt server, username <= 100 characters
String       password     = "";    // password for mqtt server, password <= 100 characters
int          keepalive    = 900;    // keepalive time (second)
int          version      = 3;     // MQTT veresion 3(3.1), 4(3.1.1)
int          cleansession = 0;     // cleanssion  : 0, 1
unsigned int subQoS       = 0;     // subQoS      : unsinged int : 0, 1, 2
unsigned int pubQoS       = 0;     // pubQoS      : unsinged int : 0, 1, 2
unsigned int pubRetained =
    0;    // pubRetained : unsigned int : 0  not retain the message, 1  retain the message

// The Will message defines the content of the message that is published to the will topic if the
// client is unexpectedly disconnected
int          willflag = 0;    // willflag    : 0, 1
unsigned int will_qos = 0;    // will_qos    : unsinged int : 0, 1, 2
unsigned int will_retain =
    0;    // will_retain : unsinged int : 0  not retain the message, 1  retain the message
String will_topic = "";    // Your Will topic string
String will_msg   = "";    // Your Will message. It can be a zero-length message.
String willOption = nb.willConfig(will_topic, will_qos, will_retain, will_msg);

const long    interval       = 20000;    // time in millisecond
unsigned long previousMillis = 0;

int cnt = 0;

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
    nb.publish(topic, payload + String(cnt), pubQoS, pubRetained);
    previousMillis = currentMillis;
  }
}

//=========== MQTT Function ================
void setupMQTT() {
  if (nb.connectMQTT(host, port, clientID, username, password, keepalive, version, cleansession,
                     willflag, willOption)) {
    nb.subscribe(topic, subQoS);
  }
  // nb.unsubscribe(topic);
}
void connectStatus() {
  if (!nb.isMQTTConnected()) {
    if (debug)
      Serial.println("reconnectMQ ");
    setupMQTT();
  }
}
void callback(String &topic, String &payload) {
  Serial.println("-------------------------------");
  Serial.println("# Message from Topic \"" + topic + "\" : " + payload);
}
