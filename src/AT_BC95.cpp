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

AT Command Dictionary for Quectel BC95 & BC95-G version 2.1.1
support Quectel BC95 & BC95-G
NB-IoT with AT command

Author: Device Innovation team
Create Date: 8 February 2021.
Modified: 24 February 2023.
*/
#include "AT_BC95.h"
#include "board.h"

/****************************************/
/**        Initialization Module       **/
/****************************************/
AT_BC95::AT_BC95() {}

void AT_BC95::setupModule(String address, String port) {
  pinMode(hwResetPin, OUTPUT);
  serialPort.begin(buadrate);
  _Serial = &serialPort;

#if defined(description)
  Serial.println(description);
#endif
#if defined(stop)
  while (true)
    ;
#endif

  reboot_module(true);
  startTime = millis();
  check_module_ready();

  _Serial->print(F("AT+CMEE=1"));    // set report error
  _Serial->println();
  _serial_flush();

  delay(700);

  Serial.print(F(">>IMSI   : "));
  Serial.println(getIMSI());

  Serial.print(F(">>ICCID  : "));
  Serial.println(getICCID());

  Serial.print(F(">>IMEI   : "));
  Serial.println(getIMEI());

  if (debug)
    Serial.print(F(">>FW ver : "));
  if (debug)
    Serial.println(getFirmwareVersion());

  if (debug)
    Serial.print(F(">>Module BC95 : "));
  bc95 = isBC95();
  if (debug)
    Serial.println(bc95);

  delay(800);
  _serial_flush();
  Serial.print(F(">>Connecting Network "));

  if (attachNetwork()) {
    if (address != "" && port != "") {
      if (!createUDPSocket(address, port)) {
        // Serial.println(">> Cannot create socket");
      }
    }
    Serial.println(F("OK"));

  } else {
    Serial.println(F("FAILED"));
    delay(200);
    reset();
  }

  delay(500);
  Serial.print(F(">>Signal : "));
  Serial.print(getSignal());
  Serial.println(F(" dBm"));
  Serial.println(F("--------------------"));

  timePassed = millis();
}

void AT_BC95::reboot_module(bool printstate) {
  digitalWrite(hwResetPin, HIGH);
  delay(500);
  digitalWrite(hwResetPin, LOW);
  delay(1000);
  Serial.println(F("Module is rebooting..."));
}

void AT_BC95::powerSavingMode(unsigned int psm, String Requested_PeriodicTAU,
                              String Requested_Active_Time) {
  // AT+CPSMS=<mode>[,<Requested_Periodic-RAU>[,<Requested_GPRSREADYtimer>[,<Requested_PeriodicTAU>
  // [,<Requested_Active-Time>]]]]
  _Serial->print("AT+CPSMS=");
  _Serial->print(psm);
  if (Requested_PeriodicTAU != "" || Requested_Active_Time != "") {
    _Serial->print(",,,");
    _Serial->print(Requested_PeriodicTAU);
    _Serial->print(",");
    _Serial->print(Requested_Active_Time);
  }
  _Serial->println();
  _serial_flush();
}

void AT_BC95::check_module_ready() {
  _Serial->println(F("AT"));
  while (true) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1) {
        hw_connected = true;
        powerSavingMode(2);
        break;
      }
    } else {

      if (millis() - startTime > MAXTIME) {
        _Serial->write(0x1a);
        startTime    = millis();
        hw_connected = false;
        attemptCount++;
        if (attemptCount > 5) {
          Serial.print(F("\nError to connect NB Module, rebooting..."));
          delay(200);
          reset();
        }

      } else {
        _Serial->println(F("AT"));
        delay(200);
      }
    }
  }
  delay(1000);
  attemptCount = 0;
}

// Check network connecting status : 1 connected, 0 not connected
bool AT_BC95::checkNetworkConnection() {
  bool networkStatus = false;
  _serial_flush();
  _Serial->println(F("AT+CGATT?"));
  delay(800);
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input += _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("+CGATT:1")) != -1 && data_input.indexOf(F("OK")) != -1) {
        networkStatus = true;
        break;
      } else if (data_input.indexOf(F("+CGATT:0")) != -1 || data_input.indexOf(F("ERROR")) != -1) {
        networkStatus = false;
        break;
      }
    }
  }
  data_input = "";
  return networkStatus;
}

bool AT_BC95::attachNetwork() {
  ConnectionState state = ConnectionState::IDLE;
  startTime             = millis();
  while (millis() - startTime < timeout_ms) {
    switch (state) {
    case ConnectionState::IDLE:
      connectNetwork();
      state = ConnectionState::CONNECTING;
      break;
    case ConnectionState::CONNECTING:
      if (checkNetworkConnection()) {
        state = ConnectionState::CONNECTED;
      }
      break;
    case ConnectionState::CONNECTED:
      _serial_flush();
      _Serial->flush();
      return true;
    case ConnectionState::ERROR:
      return false;
    }
  }
  return false;
}

// Set Phone Functionality : 1 Full functionality
bool AT_BC95::setPhoneFunction() {
  _Serial->println(F("AT+CFUN=1"));
  return waitForResponse("OK", "ERROR", timeout_ms) == 1;
}

// Attach network : 1 connected, 0 disconnected
bool AT_BC95::connectNetwork() {
  _Serial->println(F("AT+CGATT=1"));
  return waitForResponse("OK", "ERROR", timeout_ms) == 1;
}

// Create a UDP socket and connect socket to remote address and port
bool AT_BC95::createUDPSocket(String address, String port) {
  bool   status = false;
  String cmd;
  if (bc95) {
    cmd = "AT+NSOCR=DGRAM,17," + port + ",1";
  } else {
    cmd = "AT+NSOCR=DGRAM,17,5684,1";
  }
  _Serial->println(cmd);
  delay(500);
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input += _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1) {
        socket = data_input.substring(0, 2);
        socket.trim();
        status     = true;
        data_input = "";
        break;
      } else if (data_input.indexOf(F("+CME ERROR: 4")) != -1) {
        _Serial->println(cmd);
        data_input = "";
      }
    }
  }
  return status;
}

// Close a UDP socket 0
bool AT_BC95::closeUDPSocket() {
  _Serial->print(F("AT+NSOCL="));
  _Serial->print(socket);
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1) {
        break;
      }
    }
  }
}

// Ping IP
pingRESP AT_BC95::pingIP(String IP) {
  pingRESP pingr;
  String   data = "";
  _Serial->print("AT+NPING=" + IP);
  _Serial->println();

  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("ERROR")) != -1) {
        break;
      } else if (data_input.indexOf(F("+NPING:")) != -1) {
        data = data_input;
        break;
      } else if (data_input.indexOf(F("+NPINGERR")) != -1) {
        break;
      }
    }
  }

  if (data != "") {
    int index    = data.indexOf(F(":"));
    int index2   = data.indexOf(F(","), index + 1);
    int index3   = data.indexOf(F(","), index2 + 1);
    pingr.status = true;
    pingr.addr   = data.substring(index + 1, index2);
    pingr.ttl    = data.substring(index2 + 1, index3);
    pingr.rtt    = data.substring(index3 + 1, data.length() - 1);
  } else {
    Serial.println(">>Ping Failed");
  }

  blankChk(pingr.addr);
  blankChk(pingr.ttl);
  blankChk(pingr.rtt);
  if (data != "")
    Serial.println(">>Ping IP : " + pingr.addr + ", ttl= " + pingr.ttl + ", rtt= " + pingr.rtt);

  _serial_flush();
  data_input = "";
  return pingr;
}

/****************************************/
/**          Get Parameter Value       **/
/****************************************/
String AT_BC95::getIMSI() {
  String imsi = "";
  _Serial->print(F("AT+CIMI"));
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1 && imsi.indexOf(F("52003")) != -1) {
        imsi.replace(F("OK"), "");
        return imsi;
      } else if (data_input.indexOf(F("ERROR")) != -1) {
        setPhoneFunction();
        _Serial->println(F("AT+CIMI"));
      } else
        imsi += data_input;
    }
  }
  return "N/A";
}

String AT_BC95::getICCID() {
  String iccid = "";
  _Serial->print(F("AT+NCCID"));
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1)
        break;
      else
        iccid += data_input;
    }
  }
  iccid.replace(F("OK"), "");
  iccid.replace(F("+NCCID:"), "");
  iccid.trim();
  blankChk(iccid);
  return iccid;
}

String AT_BC95::getIMEI() {
  String imei;
  _Serial->print(F("AT+CGSN=1"));
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("+CGSN:")) != -1) {
        data_input.replace(F("+CGSN:"), "");
        imei = data_input;
      } else if (data_input.indexOf(F("OK")) != -1 && imei != "")
        break;
    }
  }
  blankChk(imei);
  return imei;
}

String AT_BC95::getDeviceIP() {
  _serial_flush();
  String deviceIP = "N/A";
  _Serial->print(F("AT+CGPADDR=0"));
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("+CGPADDR")) != -1) {
        int index  = data_input.indexOf(F(":"));
        int index2 = data_input.indexOf(F(","));
        deviceIP   = data_input.substring(index2 + 1, data_input.length());
      } else if (data_input.indexOf(F("OK")) != -1)
        break;
      else if (data_input.indexOf(F("ERROR")) != -1) {
        break;
      }
    }
  }
  deviceIP.replace(F("\""), "");
  deviceIP.trim();
  // Serial.print(F(">>Device IP : "));
  // Serial.println(deviceIP);
  return deviceIP;
}

String AT_BC95::getSignal() {
  _serial_flush();
  int    rssi     = 0;
  String data_csq = "";
  data_input      = "";
  do {
    _Serial->print(F("AT+CSQ"));
    _Serial->println();
    delay(500);
    startTime                  = millis();
    unsigned int current_check = millis();
    while (current_check - startTime < MAXTIME) {
      if (_Serial->available()) {
        data_input = _Serial->readStringUntil('\n');
        if (data_input.indexOf(F("OK")) != -1) {
          break;
        } else {
          if (data_input.indexOf(F("+CSQ")) != -1) {
            int start_index = data_input.indexOf(F(":"));
            int stop_index  = data_input.indexOf(F(","));
            data_csq        = data_input.substring(start_index + 1, stop_index);

            rssi     = data_csq.toInt();
            rssi     = (2 * rssi) - 113;
            data_csq = String(rssi);

          } else {
            startTime = current_check;
          }
        }
      }
    }
    if (rssi == -113 || rssi == 85)
      attemptCount++;
    delay(1000);
    data_input = "";
  } while (rssi == -113 && attemptCount <= 10 || rssi == 85 && attemptCount <= 10);
  if (rssi == -113 || rssi == 85) {
    data_csq     = "-113";
    attemptCount = 0;
  }
  return data_csq;
}

String AT_BC95::getAPN() {
  String out = "";
  _Serial->print(F("AT+CGDCONT?"));
  _Serial->println();

  while (true) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("+CGDCONT:0")) != -1) {
        int index  = 0;
        int index2 = 0;
        index      = data_input.indexOf(F(":"));
        index2     = data_input.indexOf(F(","));

        index  = data_input.indexOf(F(","), index2 + 1);
        index2 = data_input.indexOf(F(","), index + 1);
        out    = data_input.substring(index + 2, index2 - 1);
        if (out == ",,")
          out = "";
      }
      if (data_input.indexOf(F("OK")) != -1) {
        break;
      }
    }
  }
  _serial_flush();
  data_input = "";
  blankChk(out);
  return out;
}

String AT_BC95::getFirmwareVersion() {
  String fw = "";
  _Serial->print(F("AT+CGMR"));
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1)
        break;
      else
        fw += data_input;
    }
  }
  fw.replace(F("OK"), "");
  fw.trim();
  blankChk(fw);
  return fw;
}

String AT_BC95::getNetworkStatus() {
  String out  = "";
  String data = "";

  _Serial->print(F("AT+CEREG=2"));
  _Serial->println();
  delay(500);
  _serial_flush();
  delay(1000);
  _Serial->print(F("AT+CEREG?"));
  _Serial->println();
  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("+CEREG")) != -1) {
        attemptCount++;
        if (attemptCount < 10 && data_input.indexOf(F(",2")) != -1) {
          _serial_flush();
          _Serial->print(F("AT+CEREG?"));
          _Serial->println();
        } else {
          data       = data_input;
          int index  = data.indexOf(F(": "));
          int index2 = data.indexOf(F(","));
          int index3 = data.indexOf(F(","), index2 + 1);
          out        = data.substring(index2 + 1, index2 + 2);
          if (out == F("1"))
            out = F("Registered");
          else if (out == "0")
            out = F("Not Registered");
          else if (out == "2")
            out = F("Trying");
        }
      } else if (data_input.indexOf(F("OK")) != -1)
        break;
    }
  }
  attemptCount = 0;
  return (out);
}

// Get radio stat.
radio AT_BC95::getRadioStat() {
  radio value;
  _Serial->print(F("AT+NUESTATS"));
  _Serial->println();
  while (true) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("OK")) != -1) {
        break;
      } else {
        if (data_input.indexOf(F("PCI")) != -1) {
          int start_index = data_input.indexOf(F(":"));
          int stop_index  = data_input.indexOf(F("\n"));
          value.pci       = data_input.substring(start_index + 1, stop_index);
        }
        if (data_input.indexOf(F("RSRQ")) != -1) {
          int start_index = data_input.indexOf(F(":"));
          int stop_index  = data_input.indexOf(F("\n"));
          value.rsrq      = data_input.substring(start_index + 1, stop_index);
          value.rsrq      = String(value.rsrq.toInt() / 10);
        }
      }
    }
  }
  value.pci.trim();
  value.rsrq.trim();
  blankChk(value.pci);
  blankChk(value.rsrq);
  return value;
}

void AT_BC95::blankChk(String &val) {
  if (val == "") {
    val = "N/A";
  }
}

bool AT_BC95::checkPSMmode() {
  _Serial->println(F("AT+CPSMS?"));
  //+CPSMS:<mode>[,,,<Requested_Periodic-TAU>],[<Requested_Active-Time>]
  return waitForResponse("+CPSMS:1", "+CPSMS:0", timeout_ms);
}

/****************************************/
/**          Send UDP Message          **/
/****************************************/
// Send AT command to send UDP message
void AT_BC95::sendCmd(String address, String port, unsigned int len) {
  _Serial->print(F("AT+NSOST="));
  _Serial->print(socket);
  _Serial->print(F(","));
  _Serial->print(address);
  _Serial->print(F(","));
  _Serial->print(port);
  _Serial->print(F(","));
  _Serial->print(len);
  _Serial->print(F(","));
}

// Send message type String
void AT_BC95::sendCmd(String msg) {
  _Serial->print(msg);
}

// Send message type unsigned int
void AT_BC95::sendCmd(unsigned int msg) {
  _Serial->print(msg);
}

// Send message type char *
void AT_BC95::sendCmd(char *msg) {
  _Serial->print(msg);
}

// Send '\r\n'
void AT_BC95::endCmd() {
  _Serial->println();
}

/****************************************/
/**        Receive UDP Message         **/
/****************************************/
// Receive incoming message
void AT_BC95::waitResponse(String &retdata, String server) {
  startTime = millis();
  if ((startTime - timePassed >= 500) && !(_Serial->available())) {
    at_getBuffer(socket, "512");
    timePassed = startTime;
  }

  if (_Serial->available()) {
    char data = (char)_Serial->read();
    if (data == '\n' || data == '\r') {
      if (k > 2) {
        end = true;
        k   = 0;
      }
      k++;
    } else {

      data_input += data;
    }
  }
  if (end) {
    manageResponse(retdata, server);
    end = false;
  }
}

// Split data from incoming message
void AT_BC95::manageResponse(String &retdata, String server) {
  if (end) {

    if (data_input.indexOf(F("+NSONMI:")) != -1) {
      at_getBuffer(socket, "512");
      data_input = F("");
      send_NSOMI = true;
      end        = false;
    } else {
      end = false;
      if (data_input.indexOf(server) != -1) {    // serverIP
        String left_buffer = "";
        // pack data to char array
        char buf[data_input.length() + 1];
        memset(buf, '\0', data_input.length());
        data_input.toCharArray(buf, sizeof(buf));

        char *p = buf;
        char *str;
        int   i = 0;
        int   j = 0;
        while ((str = strtok_r(p, ",", &p)) != NULL) {
          // delimiter is the comma
          if (data_input.indexOf(F("OK")) != -1) {
            j = 5;
          } else {
            j = 4;
          }
          if (i == j) {
            retdata = str;
          }
          if (i == j + 1) {
            left_buffer = str;
          }
          i++;
        }

        if (left_buffer != "0") {
          at_getBuffer(socket, "512");
        }
        send_NSOMI = false;
        data_input = F("");
      }
    }
  }
}

// get incoming data after +NSONMI:
void AT_BC95::at_getBuffer(String _socket, String nBuffer) {
  _Serial->print(F("AT+NSORF="));
  _Serial->print(_socket);
  _Serial->print(F(","));
  _Serial->print(nBuffer);
  _Serial->println();
}

/****************************************/
/**          Utility                   **/
/****************************************/
// char * to hex
String AT_BC95::toHEX(const char *str) {
  String output = "";
  for (int i = 0; str[i]; i++) {
    output += String(str[i], HEX);
  }
  return output;
}

// Flush unwanted message from serial
void AT_BC95::_serial_flush() {
  while (_Serial->available()) {
    _Serial->readStringUntil('\n');
  }
  _Serial->flush();
  data_input = "";
}

dateTime AT_BC95::getClock(unsigned int timezone) {
  dateTime dateTime;
  _Serial->print(F("AT+CCLK?"));
  _Serial->println();

  unsigned long startTime = millis();
  while (millis() - startTime < timeout_ms) {
    data_input = _Serial->readStringUntil('\n');
    if (data_input.indexOf(F("+CCLK:")) != -1) {
      byte index    = data_input.indexOf(F(":"));
      byte index2   = data_input.indexOf(F(","), index + 1);
      byte index3   = data_input.indexOf(F("+"), index2 + 1);
      dateTime.date = data_input.substring(index + 1, index2);    // YY/MM/DD
      dateTime.time =
          data_input.substring(index2 + 1, index3);    // GMT time without adding timezone
    }
    if (data_input.indexOf(F("OK")) != -1) {
      break;
    }
  }
  if (dateTime.time != "" && dateTime.date != "") {
    byte         index  = dateTime.date.indexOf(F("/"));
    byte         index2 = dateTime.date.indexOf(F("/"), index + 1);
    unsigned int yy     = ("20" + dateTime.date.substring(0, index)).toInt();
    unsigned int mm     = dateTime.date.substring(index + 1, index2).toInt();
    unsigned int dd     = dateTime.date.substring(index2 + 1, dateTime.date.length()).toInt();

    index           = dateTime.time.indexOf(F(":"));
    unsigned int hr = dateTime.time.substring(0, index).toInt() + timezone;

    if (hr >= 24) {
      hr -= 24;
      dd += 1;
      if (mm == 2) {
        if (((yy % 4 == 0) && (yy % 100 != 0) || (yy % 400 == 0)) && (dd > 29)) {
          dd = 1;
          mm += 1;
        } else if (dd > 28) {
          dd = 1;
          mm += 1;
        }
      } else if ((mm == 1 || mm == 3 || mm == 5 || mm == 7 || mm == 8 || mm == 10 || mm == 12) &&
                 (dd > 31)) {
        dd = 1;
        mm += 1;
      } else if (dd > 30) {
        dd = 1;
        mm += 1;
      }
    }
    dateTime.time = String(hr) + dateTime.time.substring(index, dateTime.time.length());
    dateTime.date = String(dd) + "/" + String(mm) + "/" + String(yy);

    dateTime.time.trim();
    dateTime.date.trim();
  }
  blankChk(dateTime.time);
  blankChk(dateTime.date);
  return dateTime;
}

void AT_BC95::reset() {
  reboot_module();
  delay(500);
  __asm__ __volatile__("jmp 0x0000");
}

int AT_BC95::waitForResponse(const String &success, const String &error, int timeout) {
  int status = -1;
  startTime  = millis();
  while (millis() - startTime < timeout) {
    if (_Serial->available()) {
      data_input += _Serial->readStringUntil('\n');
      if (data_input.indexOf(success) != -1) {
        status = 1;
        break;
      } else if (data_input.indexOf(error) != -1) {
        status = 0;
        break;
      }
    }
  }
  _serial_flush();
  return status;
}

bool AT_BC95::isBC95() {
  _Serial->println(F("AT+CGMM"));
  return waitForResponse("BC95HB-02-STD_900", "BC95GJB-02-STD", 5000) == 1;
}

/****************************************/
/**                MQTT                **/
/****************************************/

bool AT_BC95::isMQTTConnected() {
  _Serial->println(F("AT+QMTCONN?"));
  startTime = millis();
  while (millis() - startTime < timeout_ms + 2000) {
    if (_Serial->available()) {
      data_input = _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("QMTCONN: 0,3")) != -1 ||
          data_input.indexOf(F("QMTCONN: 0,0,0")) != -1) {
        return true;
      } else if (data_input.indexOf(F("QMTCONN: 0,1")) != -1 ||
                 data_input.indexOf(F("QMTCONN: 0,2")) != -1 ||
                 data_input.indexOf(F("ERROR")) != -1) {
        return false;
      }
    }
  }
  return false;
}

// Config parameter and Open a Network for MQTT Client
bool AT_BC95::newMQTT(String server, String port, int keepalive, int version, int cleansession,
                      int willflag, String willOption) {
  // Configure the MQTT protocol version
  _Serial->print(F("AT+QMTCFG=\"version\",0,"));
  _Serial->print(version);
  _Serial->println();
  waitForResponse("OK", "ERROR", 2000);

  // Configure the keep-alive time
  _Serial->print(F("AT+QMTCFG=\"keepalive\",0,"));
  _Serial->print(keepalive);
  _Serial->println();
  waitForResponse("OK", "ERROR", 2000);

  // Configure the session type
  _Serial->print(F("AT+QMTCFG=\"session\",0,"));
  _Serial->print(cleansession);
  _Serial->println();
  waitForResponse("OK", "ERROR", 2000);

  // Configure timeout of message delivery
  // Timeout of the packet delivery. The range is 1-60. Unit: second. Retry times when packet
  // delivery times out. The range is 0-10.
  _Serial->println(F("AT+QMTCFG=\"timeout\",0,60,10,1"));
  waitForResponse("OK", "ERROR", 2000);

  // Configure Will Information
  //  AT+QMTCFG="will",<tcpconnectID>[,<will_fg>[,<will_qos>,<will_retain>,“<will_topic>”,“<will_msg>”]]
  if (willflag) {
    _Serial->print(F("AT+QMTCFG=\"will\",0,"));
    _Serial->print(willflag);
    _Serial->print(F(","));
    _Serial->print(willOption);
    _Serial->println();
    waitForResponse("OK", "ERROR", 2000);
  }
  _serial_flush();

  // Open a Network for MQTT Client
  _Serial->print(F("AT+QMTOPEN=0,\""));
  _Serial->print(server);
  _Serial->print(F("\","));
  _Serial->print(port);
  _Serial->println();

  startTime = millis();
  while (millis() - startTime < timeout_ms) {
    if (_Serial->available()) {
      data_input += _Serial->readStringUntil('\n');
      if (data_input.indexOf(F("QMTOPEN: 0,0")) != -1 && data_input.indexOf("OK") != -1) {
        return true;
      } else if (data_input.indexOf(F("QMTOPEN: 0,-1")) != -1 ||
                 data_input.indexOf(F("ERROR")) != -1) {
        disconnectMQTT();
        return false;
      }
    }
  }

  return false;
}

bool AT_BC95::sendMQTTconnectionPacket(String clientID, String username, String password) {
  _serial_flush();
  // AT+QMTCONN=<tcpconnectID>,"<clientID>"[,"<username>"[,"<password>"]]
  _Serial->print(F("AT+QMTCONN=0,"));    //<tcpconnectID>
  _Serial->print(F("\""));
  _Serial->print(clientID);    //<client_id> : should be unique.Max length is 120
  _Serial->print(F("\""));

  if (username.length() > 0) {
    _Serial->print(F(","));
    _Serial->print(F("\""));
    _Serial->print(username);    //<username> String, user name (option). Max length is 100
    _Serial->print(F("\""));
    _Serial->print(F(","));
    _Serial->print(F("\""));
    _Serial->print(password);    //<password> String, password (option). Max length is 100
    _Serial->print(F("\""));
  }
  _Serial->println();

  startTime = millis();
  while (millis() - startTime < timeout_ms) {    // timeout_ms = 5000ms
    data_input += _Serial->readStringUntil('\n');
    if (data_input.indexOf(F("OK")) != -1 && data_input.indexOf(F("+QMTCONN: 0,0,0")) != -1) {
      return true;
    } else if (data_input.indexOf(F("ERROR")) != -1 || data_input.indexOf(F("+CMQDISCON")) != -1) {
      Serial.println(F("Please check your parameter again."));
      return false;
    } else if (data_input.indexOf(F("+QMTSTAT")) != -1 ||
               data_input.indexOf(F("+QMTCONN: 0,2")) != -1) {

      return false;
    }
  }
  Serial.println(F("Timed out waiting for MQTT connection packet response"));
  return false;
}

void AT_BC95::disconnectMQTT() {
  _Serial->println("AT+QMTDISC=0");
  waitForResponse("OK", "ERROR", 2000);
}

bool AT_BC95::connectMQTT(String server, String port, String clientID, String username,
                          String password, int keepalive, int version, int cleansession,
                          int willflag, String willOption) {
  const int maxAttempts = 5;

  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    if (isMQTTConnected()) {
      disconnectMQTT();
    }

    if (newMQTT(server, port, keepalive, version, cleansession, willflag, willOption) &&
        sendMQTTconnectionPacket(clientID, username, password)) {
      return true;
    }

    if (debug)
      Serial.println("Attempt " + String(attempt + 1) + " of " + String(maxAttempts) + " failed");
    delay(500);
  }

  reset();

  return false;
}

bool AT_BC95::subscribe(String topic, unsigned int qos) {
  // AT+QMTSUB=<tcpconnectID>,<msgID>,"<topic1>",<qos1>[,"<topic2>",<qos2>…]
  _Serial->print(F("AT+QMTSUB=0,"));
  _Serial->print(random(1, 65535));
  _Serial->print(F(",\""));
  _Serial->print(topic);    //<topic> String, topic of subscribe message. Max length is 255.
  _Serial->print(F("\","));
  _Serial->print(qos);    //<Qos> Integer, message QoS, can be 0, 1 or 2.
  _Serial->println();
  return waitForResponse("OK", "ERROR", 2000);
}

void AT_BC95::unsubscribe(String topic) {
  _Serial->print(F("AT+QMTUNS=0,"));
  _Serial->print(random(1, 65535));
  _Serial->print(F(",\""));
  _Serial->print(topic);    //<topic> String, topic of subscribe message. Max length is 255.
  _Serial->print(F("\""));
  _Serial->println();

  if (waitForResponse("OK", "ERROR", 2000) && debug) {
    Serial.print("Unsubscribe topic :");
    Serial.println(topic);
  }
}

void AT_BC95::publish(String topic, String payload, unsigned int qos, unsigned int retained) {
  // AT+QMTPUB=<tcpconnectID>,<msgID>,<qos>,<retain>,"<topic>"
  data_input   = F("");
  String msgID = "0";    //<msgID> Message identifier of packet. The range is 0-65535.It will be 0
                         // only when<qos>=0.
  if (qos != 0)
    msgID = String(random(1, 65535));

  _Serial->print(F("AT+QMTPUB=0,"));
  _Serial->print(msgID);
  _Serial->print(F(","));
  _Serial->print(qos);    //<Qos> Integer, message QoS, can be 0, 1 or 2.
  _Serial->print(F(","));
  _Serial->print(retained);    //<retained> Integer, retained flag, can be 0 or 1.
  _Serial->print(F(",\""));
  _Serial->print(topic);    //<topic> String, topic of publish message. Max length is 255
  _Serial->print(F("\""));
  _Serial->println();

  if (waitForResponse(">", "ERROR", timeout_ms)) {
    _Serial->println(payload);    // Max length is 1024
    _Serial->write(0x1a);
    delay(500);
  }
}

bool AT_BC95::MQTTresponse() {
  bool publishSuccess = false;
  if (_Serial->available()) {
    char receivedData = (char)_Serial->read();
    if (receivedData == '\n' || receivedData == '\r') {
      end = true;
    } else {
      data_input += receivedData;
    }
  }
  if (end) {
    if (data_input.indexOf(F("OK+QMTPUB: 0")) != -1) {
      publishSuccess = true;
    } else if (data_input.indexOf(F("+QMTRECV:")) != -1) {
      //+QMTRECV: <tcpconnectID>,<msgID>,<topic>,<payload>
      byte index1 = data_input.indexOf(":");
      byte index2 = data_input.indexOf(",", index1 + 1);
      index1      = data_input.indexOf(",", index2 + 1);
      index2      = data_input.indexOf(",", index1 + 1);

      String retTopic = data_input.substring(index1 + 1, index2);
      retTopic.replace(F("\""), "");

      String retPayload = data_input.substring(index2 + 1, data_input.length());
      if (MQcallback_p != NULL) {
        MQcallback_p(retTopic, retPayload);
      }
    } else if (data_input.indexOf(F("ERROR")) != -1) {
      publishSuccess = false;
    }
    data_input = F("");
    end        = false;
  }
  return publishSuccess;
}

bool AT_BC95::setCallback(MQTTClientCallback callbackFunc) {
  if (MQcallback_p == NULL) {
    MQcallback_p = callbackFunc;
    return true;
  } else
    return false;
}
