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

AT Command Dictionary for Quectel BC95 version 2.1.1
support Quectel BC95 & BC95-G
NB-IoT with AT command

Author: Device Innovation team
Create Date: 2 January 2020.
Modified: 24 February 2023.
*/
#ifndef AT_BC95_h
#define AT_BC95_h

#include <Arduino.h>
#include <Stream.h>

#define MAXTIME 60000

struct pingRESP {
  bool   status;
  String addr;
  String ttl;
  String rtt;
};

struct signal {
  String csq;
  String rssi;
  String ber;
};

struct radio {
  String pci  = "";
  String rsrq = "";
};

struct dateTime {
  String date = "";
  String time = "";
};

enum class ConnectionState { IDLE, CONNECTING, CONNECTED, ERROR };

typedef void (*MQTTClientCallback)(String &topic, String &payload);

class AT_BC95 {
public:
  AT_BC95();
  bool debug;
  bool bc95 = false;
  //==============Parameter==============
  const unsigned int msgLenMul = 1;
  //=========Initialization Module=======
  void     setupModule(String port = "", String address = "");
  void     reboot_module(bool printstate = false);
  void     powerSavingMode(unsigned int psm, String Requested_PeriodicTAU = "",
                           String Requested_Active_Time = "");
  void     check_module_ready();
  bool     checkNetworkConnection();
  bool     attachNetwork();
  bool     closeUDPSocket();
  pingRESP pingIP(String IP);
  //==========Get Parameter Value=========
  String   getFirmwareVersion();
  String   getIMEI();
  String   getICCID();
  String   getIMSI();
  String   getDeviceIP();
  String   getSignal();
  String   getAPN();
  String   getNetworkStatus();
  radio    getRadioStat();
  dateTime getClock(unsigned int timezone);
  bool     checkPSMmode();
  //==========Data send/rec.===============
  void waitResponse(String &retdata, String server);
  void sendCmd(String address, String port, unsigned int len);
  void sendCmd(String input);
  void sendCmd(unsigned int data);
  void sendCmd(char *);
  void endCmd();
  //===============Utility=================
  void   _serial_flush();
  String toHEX(const char *str);
  void   reset();
  int    waitForResponse(const String &success, const String &error, int timeout);
  //================MQTT===================
  bool isMQTTConnected();
  void disconnectMQTT();
  bool newMQTT(String server, String port, int keepalive, int version, int cleansession,
               int willflag, String willOption);
  bool sendMQTTconnectionPacket(String clientID, String username, String password);
  bool connectMQTT(String server, String port, String clientID, String username, String password,
                   int keepalive, int version, int cleansession, int willflag, String willOption);
  void publish(String topic, String payload, unsigned int qos, unsigned int retained);
  bool subscribe(String topic, unsigned int qos);
  void unsubscribe(String topic);
  bool MQTTresponse();

  //============ callback ==================
  MQTTClientCallback MQcallback_p;
  bool               setCallback(MQTTClientCallback callbackFunc);

private:
  //==============Buffer====================
  String data_input;
  String socket;
  //==============Flag======================
  bool hw_connected = false;
  bool end          = false;
  bool send_NSOMI   = false;
  //==============Parameter=================
  unsigned long       startTime    = 0;
  unsigned long       timePassed   = 0;
  const unsigned long timeout_ms   = 5000;
  int                 attemptCount = 0;
  //============Counter value===============
  byte k = 0;
  //==============Function==================
  bool setPhoneFunction();
  bool connectNetwork();
  bool createUDPSocket(String address, String port);
  void manageResponse(String &retdata, String server);
  void at_getBuffer(String socket, String nBuffer);
  void blankChk(String &val);
  bool isBC95();

protected:
  Stream *_Serial;
};

#endif
