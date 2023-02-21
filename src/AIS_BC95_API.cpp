/*
Copyright (c) 2020, Advanced Wireless Network
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AIS_BC95_API v2.1.0 NB-IoT.
support Quectel BC95 & BC95-G
NB-IoT with AT command

Author: Device Innovation team
Create Date: 8 February 2021.
Modified: 17 February 2023.

Released for private usage.
*/
#include "AIS_BC95_API.h"

AT_BC95 at_BC95;
void    event_null(char *data) {}

/****************************************/
/**          Initialization            **/
/****************************************/

AIS_BC95_API::AIS_BC95_API() {
  Event_debug = event_null;
}

void AIS_BC95_API::begin(String addressI, String serverdesport) {
  Serial.println(F("\n----------------BEGIN----------------"));
  at_BC95.debug = debug;
  at_BC95.setupModule(addressI, serverdesport);
}

void AIS_BC95_API::pingIP(String IP) {
  at_BC95.pingIP(IP);
}

void AIS_BC95_API::powerSavingMode(unsigned int mode, String Requested_PeriodicTAU,
                                   String Requested_Active_Time) {
  at_BC95.powerSavingMode(mode, Requested_PeriodicTAU, Requested_Active_Time);
}

/****************************************/
/**          Send UDP Message          **/
/****************************************/

void AIS_BC95_API::sendMsgHEX(String address, String desport, String payload) {
  if (payload.length() > 1024) {
    Serial.println(F("Warning payload size exceed the limit. [Limit of HEX is 1024]"));
  } else
    send_msg(address, desport, payload.length() / 2, payload);
}

void AIS_BC95_API::sendMsgSTR(String address, String desport, String payload) {
  if (payload.length() > 512) {
    Serial.println(F("Warning payload size exceed the limit. [Limit of String is 512]"));
  } else {
    int  x_len = payload.length();
    char buf[x_len + 2];
    payload.toCharArray(buf, x_len + 1);
    send_msg(address, desport, payload.length(), at_BC95.toHEX(buf));
  }
}

void AIS_BC95_API::send_msg(String address, String desport, unsigned int len, String payload) {
  Serial.println(F("-------------------------------"));
  Serial.print(F("# Sending Data : "));
  Serial.println(payload);
  Serial.print(F("# IP : "));
  Serial.println(address);
  Serial.print(F("# Port : "));
  Serial.println(desport);

  at_BC95.sendCmd(address, desport, len);
  at_BC95.sendCmd(payload);
  at_BC95.endCmd();
}

/****************************************/
/**         Receive UDP Message        **/
/****************************************/
void AIS_BC95_API::waitResponse(String &retdata, String server) {
  at_BC95.waitResponse(retdata, server);
}

/****************************************/
/**          Get Parameter Value       **/
/****************************************/
/*
  - getSignal
        - Get NB-IoT signal
  - getDeviceIP
        - Get device ip after connected to network.
  - getIMSI
        - Get IMSI of eSIM
  - getICCID
        - Get eSIM serial number
  - getIMEI
        - Get Device IMEI
*/
String AIS_BC95_API::getSignal() {
  return at_BC95.getSignal();
}

String AIS_BC95_API::getDeviceIP() {
  return at_BC95.getDeviceIP();
}

String AIS_BC95_API::getIMSI() {
  return at_BC95.getIMSI();
}

String AIS_BC95_API::getICCID() {
  return at_BC95.getICCID();
}

String AIS_BC95_API::getIMEI() {
  return at_BC95.getIMEI();
}

radio AIS_BC95_API::getRadioStat() {
  return at_BC95.getRadioStat();
}

dateTime AIS_BC95_API::getClock(unsigned int timezone) {
  return at_BC95.getClock(timezone);
}

bool AIS_BC95_API::checkPSMmode() {
  return at_BC95.checkPSMmode();
}

bool AIS_BC95_API::isMQTTConnected() {
  return at_BC95.isMQTTConnected();
}

/****************************************/
/**                MQTT                **/
/****************************************/
/*
  - setupMQTT
      - setup module to use MQTT include serverIP, port, clientID, username, password, keep alive
  interval, will messege.
  - connectMQTT
      - setup module to use MQTT include serverIP, port, clientID, username, password, keep alive
  interval, will messege. This function doesn't have default value as 0.
  - newMQTT
      - connect device to MQTT server and port
  - sendMQTTconnectionPacket
      - connect device to MQTT with configuration value
  - willConfig
      - create payload for will messege
  - publish
      - publish payload within  1000 characters.
  - subscribe
      - subscribe to the topic to receive data
  - unsubscribe
      - unsubscribe to the topic
  - MQTTresponse
      - receive response from server
  - RegisMQCallback
      - receive response from server
*/
bool AIS_BC95_API::setupMQTT(String server, String port, String clientID, String username,
                             String password, int keepalive, int version, int cleansession,
                             int willflag, String willOption) {

  Serial.println(F(">>Connecting MQTT ..."));
  if (at_BC95.bc95) {
    Serial.println("Cannot use MQTT. Please use DEVIO NB-SHIELD I Plus (BC95-G)");
  } else {

    if (username.length() > 100 || password.length() > 100) {
      Serial.println(F("Username/Password is over 100."));
    } else if (username == "" && password != "") {
      Serial.println(F("Username is missing."));
    } else if (clientID.length() > 120 || clientID == "" && cleansession != 1) {
      Serial.println(F("ClientID is over 120 or ClientID is missing."));
    } else if (server == "" || port == "") {
      Serial.println(F("Address or port is missing."));
    } else if (version > 4 || version < 3) {
      Serial.println(F("Version must be 3 (MQTT 3.1) or 4 (MQTT 3.1.1)"));
    } else if (willflag == 1 && willOption == "") {
      Serial.println(F("Missing will option."));
    } else {
      if (isMQTTConnected()) {
        at_BC95.disconnectMQTT();
      }
      if (at_BC95.connectMQTT(server, port, clientID, username, password, keepalive, version,
                              cleansession, willflag, willOption)) {
        flag_mqtt_connect = true;
        isMQTTConnected();
        Serial.print(F("# ServerIP : "));
        Serial.println(server);
        Serial.print(F("# Port : "));
        Serial.println(port);
        Serial.print(F("# ClientID : "));
        Serial.println(clientID);
      }
    }
    at_BC95._serial_flush();
  }
  return flag_mqtt_connect;
}

bool AIS_BC95_API::connectMQTT(String server, String port, String clientID, String username,
                               String password) {
  return setupMQTT(server, port, clientID, username, password, 900, 3, 1, 0, "");
}

bool AIS_BC95_API::connectMQTT(String server, String port, String clientID, String username,
                               String password, int keepalive, int version, int cleansession,
                               int willflag, String willOption) {
  return setupMQTT(server, port, clientID, username, password, keepalive, version, cleansession,
                   willflag, willOption);
}

bool AIS_BC95_API::newMQTT(String server, String port, int keepalive, int version, int cleansession,
                           int willflag, String willOption) {
  return at_BC95.newMQTT(server, port, keepalive, version, cleansession, willflag, willOption);
}

bool AIS_BC95_API::sendMQTTconnectionPacket(String clientID, String username, String password) {
  return at_BC95.sendMQTTconnectionPacket(clientID, username, password);
}

String AIS_BC95_API::willConfig(String will_topic, unsigned int will_qos, unsigned int will_retain,
                                String will_msg) {
  return String(will_qos) + "," + String(will_retain) + ",\"" + will_topic + "\",\"" + will_msg +
         "\"";
}

void AIS_BC95_API::publish(String topic, String payload, unsigned int pubQoS,
                           unsigned int pubRetained) {
  if (flag_mqtt_connect) {
    if (topic != "" && (payload.length() * at_BC95.msgLenMul <= 1548)) {
      Serial.println(F("-------------------------------"));
      Serial.print(F("# Publish : "));
      Serial.println(payload);
      Serial.print(F("# Topic   : "));
      Serial.println(topic);
      at_BC95.publish(topic, payload, pubQoS, pubRetained);
    } else {
      if (topic == "") {
        Serial.println(F("Topic is missing."));
      }
      if (payload.length() * at_BC95.msgLenMul > 1548) {
        Serial.println(F("Payload hex string is over limit. (1548)"));
      }
    }
  }
}

bool AIS_BC95_API::subscribe(String topic, unsigned int subQoS) {
  if (flag_mqtt_connect) {
    if (topic == "") {
      Serial.println(F("Topic is missing."));
      return false;
    }
    Serial.println(F("-------------------------------"));
    Serial.println(F("# Subscribe "));
    Serial.print(F("# Topic : "));
    Serial.println(topic);
    at_BC95._serial_flush();
    startTime = millis();
    while (millis() - startTime < timeout_ms) {
      if (at_BC95.subscribe(topic, subQoS)) {
        return true;
      }
    }
    at_BC95._serial_flush();
  }
  return false;
}

void AIS_BC95_API::unsubscribe(String topic) {
  if (flag_mqtt_connect) {
    if (topic == "") {
      Serial.println(F("Topic is missing."));
    } else {
      at_BC95.unsubscribe(topic);
    }
  }
}

void AIS_BC95_API::MQTTresponse() {
  at_BC95.MQTTresponse();
}

bool AIS_BC95_API::setCallback(MQTTClientCallback callbackFunc) {
  return at_BC95.setCallback(callbackFunc);
}
