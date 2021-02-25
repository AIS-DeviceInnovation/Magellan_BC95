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

AT Command Dictionary for Quectel BC95 version 1.0.2
support Quectel BC95
NB-IoT with AT command

Author: Device Innovation team  
Create Date: 2 January 2020. 
Modified: 18 February 2021.
*/
#include "AT_BC95.h"  
#include "board.h"

/****************************************/
/**        Initialization Module       **/
/****************************************/
AT_BC95::AT_BC95(){}

void AT_BC95::setupModule(String address,String port){
  pinMode(4, OUTPUT);  
  serialPort.begin(buadrate);
  _Serial = &serialPort;

  #if defined(description)
     Serial.println(description);
  #endif 
  #if defined(stop)
    while(1);
  #endif 

  if(address==""||port=="") Serial.println(F("Warning, address or port is missing."));
  else{
    previous_check=millis();  
   
    check_module_ready();
    if (!hw_connected){
    }

    Serial.print(F(">>Reboot"));   
    reboot_module();

    check_module_ready();

    _Serial->println("AT+CMEE=1");                    // set report error
    _serial_flush();

    Serial.print(F(">>IMSI   : "));
    Serial.println(getIMSI());

    Serial.print(F(">>ICCID  : "));
    Serial.println(getICCID());

    Serial.print(F(">>IMEI   : "));
    Serial.println(getIMEI());

    if(debug)Serial.print(F(">>FW ver : "));
     if(debug)Serial.println(getFirmwareVersion());
  
    delay(500);
    Serial.print(F(">>Signal : "));
    Serial.print(getSignal()); 
    Serial.println(F(" dBm")); 

    delay(800);
    _serial_flush();
    Serial.print(F(">>Connecting "));

    if(attachNetwork()){  
      if(address!="" && port!=""){
        if(!createUDPSocket(address,port)){
          //Serial.println(">> Cannot create socket");
        }
      }   
      Serial.println(F("OK")); 
      Serial.println(F("---------- Connected ----------"));
    }
    else {
      //Serial.println(F("FAILED"));
      Serial.println(F("-------- Disconnected ---------"));
      //reset
      delay(200);
      __asm__ __volatile__ ("jmp 0x0000");
    }

    if(debug){
        Serial.print(F(">>APN   : "));
        Serial.println(getAPN());
    }

    previous=millis();
  }
}

void AT_BC95::check_module_ready(){
  int count = 0;
  _Serial->println(F("AT"));
  delay(100);
  _Serial->println(F("AT"));  
  while(1){
    if(_Serial->available()){
      data_input = _Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1){
        hw_connected=true;
        break;
      }
    }
    else{
      unsigned int current_check=millis();
      if (current_check-previous_check>5000){        
        previous_check=current_check;
        hw_connected=false;
        count++;  
        if(count > 5) {          
          Serial.print(F("\nError to connect NB Module, rebooting..."));
          delay(200);
          __asm__ __volatile__ ("jmp 0x0000");
        } 
           
      }
      else{
        delay(500);
        _Serial->println(F("AT"));
        delay(100);
      }
    }
  }
  delay(1000);
}

void AT_BC95::reboot_module(){
  _Serial->println(F("AT+NRB"));

  while(1){
    if(_Serial->available()){
      data_input = _Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1){
        Serial.println(F("OK"));
        break;
      }
      else{
        if (data_input.indexOf(F("REBOOT_"))!=-1){
        }
        else{
          Serial.print(F("."));
        }
      }
     }
  }
  delay(1000);
}

bool AT_BC95::attachNetwork(){
  bool status=false;
  if(!checkNetworkConnection()){
    for(int i=0;i<60;i++){
      setPhoneFunction();
      connectNetwork();
      delay(2000);
      if(checkNetworkConnection()){ 
        status=true;
        break;
      }
    }
  }
  else status=true;
    
  _serial_flush();
  _Serial->flush();
  return status;
}

// Check network connecting status : 1 connected, 0 not connected
bool AT_BC95::checkNetworkConnection(){
  bool status=false;
  _serial_flush();
  _Serial->println(F("AT+CGATT?"));
  delay(800);
  for(int i=0;i<60;i++){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("+CGATT:1"))!=-1){
        status=true;
      }
      else if(data_input.indexOf(F("+CGATT:0"))!=-1){
        status=false;
      }
      else if(data_input.indexOf(F("OK"))!=-1) {
        break;        
      }
      else if(data_input.indexOf(F("ERROR"))!=-1) {
        break;        
      }
    }
  }
  data_input="";
  return status;
}
// Set Phone Functionality : 1 Full functionality
bool AT_BC95::setPhoneFunction(){
  bool status=false;
  _Serial->println(F("AT+CFUN=1"));
   while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1){
        status=true;
        break;
      }
      else if(data_input.indexOf(F("ERROR"))!=-1){
        status=false;
        break;
      }
      Serial.print(F("."));
    }
  }
  return status;
}

// Attach network : 1 connected, 0 disconnected
void AT_BC95::connectNetwork(){  
  _Serial->println(F("AT+CGATT=1"));
  delay(1000);
  for(int i=0;i<30;i++){
    if(_Serial->available()){
      data_input =  _Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1) break;
      else if(data_input.indexOf(F("ERROR"))!=-1) break;
      Serial.print(F("."));
    }
  }
}

// Create a UDP socket and connect socket to remote address and port
bool AT_BC95::createUDPSocket(String address,String port){
  bool status=false;
  _Serial->print(F("AT+NSOCR=DGRAM,17,"));
  _Serial->println(port+",1");
  delay(500);
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1){
        status=true;
        break;
      }else if(data_input.indexOf(F("+CME ERROR: 4"))!=-1){
      	_Serial->print(F("AT+NSOCR=DGRAM,17,"));
  		_Serial->println(port+",1");
      }
      Serial.print(F("."));
    }
  }

  
  return status;
}

// Close a UDP socket 0
bool AT_BC95::closeUDPSocket(){
  _Serial->println(F("AT+NSOCL=0"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1){
        break;
      }
    }
  }
}

// Ping IP
pingRESP AT_BC95::pingIP(String IP){
  pingRESP pingr;
  String data = "";
  _Serial->println("AT+NPING="+IP);

  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("ERROR"))!=-1){
        break;
      }
      else if(data_input.indexOf(F("+NPING:"))!=-1){
        data=data_input;
        break;
      }
      else if(data_input.indexOf(F("+NPINGERR"))!=-1){
        break;
      }
    }
  }

  if(data!=""){  
    int index = data.indexOf(F(":"));
    int index2 = data.indexOf(F(","),index+1);
    int index3 = data.indexOf(F(","),index2+1);
    pingr.status = true;
    pingr.addr = data.substring(index+1,index2);
    pingr.ttl = data.substring(index2+1,index3);
    pingr.rtt = data.substring(index3+1,data.length()-1);
  }else { 
    Serial.println(">>Ping Failed");
  }
  blankChk(pingr.addr);
  blankChk(pingr.ttl);
  blankChk(pingr.rtt);
  if(data!="") Serial.println(">>Ping IP : "+pingr.addr + ", ttl= " + pingr.ttl + ", rtt= " + pingr.rtt);  
  _serial_flush();
  data_input="";
  
return pingr;
}

/****************************************/
/**          Get Parameter Value       **/
/****************************************/
String AT_BC95::getIMSI(){
  String imsi="";
  _Serial->println(F("AT+CIMI"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1 && imsi.indexOf(F("52003"))!=-1) break;
      else if(data_input.indexOf(F("ERROR"))!=-1) _Serial->println(F("AT+CIMI"));
      else imsi+=data_input;
    }
  }
  imsi.replace(F("OK"),"");  
  imsi.trim();
  blankChk(imsi); 
  return imsi;
}

String AT_BC95::getICCID(){
  String iccid="";
  _Serial->println(F("AT+NCCID"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1) break;
      else iccid+=data_input;
    }
  }
  iccid.replace(F("OK"),"");
  iccid.replace(F("+NCCID:"),"");
  iccid.trim();
  blankChk(iccid); 
  return iccid;
}

String AT_BC95::getIMEI(){
  String imei;
  _Serial->println(F("AT+CGSN=1"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("+CGSN:"))!=-1){
        data_input.replace(F("+CGSN:"),"");
        imei = data_input;
      }
      else if(data_input.indexOf(F("OK"))!=-1 && imei!="") break;
    }
  }
  blankChk(imei);
  return imei;
}

String AT_BC95::getDeviceIP(){
  _serial_flush();
  String deviceIP;
  _Serial->println(F("AT+CGPADDR=0"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("+CGPADDR"))!=-1){
        int index = data_input.indexOf(F(":"));
        int index2 = data_input.indexOf(F(","));
        deviceIP = data_input.substring(index2+1,data_input.length());
      }
      else if(data_input.indexOf(F("OK"))!=-1) break;
      else if(data_input.indexOf(F("ERROR"))!=-1) {
        deviceIP = "n/a";
        break;
      }
    }
  }
  deviceIP.replace(F("\""),"");
  deviceIP.trim();
  // Serial.print(F(">>Device IP : "));
  // Serial.println(deviceIP);
  return deviceIP;
}

String AT_BC95::getSignal(){
  _serial_flush();
  int rssi = 0;
  int count = 0;
  String data_csq = "";
  data_input = "";
  do
  {
    _Serial->println(F("AT+CSQ"));
    delay(500);
    while(1)  {    
      if(_Serial->available()){
        data_input = _Serial->readStringUntil('\n');
        if(data_input.indexOf(F("OK"))!=-1){
         break;
        }
        else{
          if(data_input.indexOf(F("+CSQ"))!=-1){
            int start_index = data_input.indexOf(F(":"));
            int stop_index  = data_input.indexOf(F(","));
            data_csq = data_input.substring(start_index+1,stop_index);

            rssi = data_csq.toInt();
            rssi = (2*rssi)-113;
            data_csq = String(rssi);
          }
        }
      }
    }
  if(rssi==-113 || rssi==85)count++;
  delay(1000);
  }while(rssi==-113&&count<=10 || rssi==85&&count<=10);
  if(rssi==-113 || rssi==85){
    data_csq = "-113";
    count= 0;
  }
  return data_csq;
}

String AT_BC95:: getAPN(){
  String out="";
  _Serial->println(F("AT+CGDCONT?"));

  while(1){ //+CGDCONT:0,"IP","NB.DEVELOPER",,0,0,,,,,0
    if(_Serial->available()){
      data_input = _Serial->readStringUntil('\n');
      if(data_input.indexOf(F("+CGDCONT:0"))!=-1){
        int index=0;
        int index2=0;
        index = data_input.indexOf(F(":"));
        index2 = data_input.indexOf(F(","));

        index = data_input.indexOf(F(","),index2+1);
        index2 = data_input.indexOf(F(","),index+1);
        out = data_input.substring(index+2,index2-1);
        if(out==",,") out="";
      }
      if(data_input.indexOf(F("OK"))!=-1){
        break;
      }
    }
  }
  _serial_flush();
  data_input="";
  blankChk(out);
  return out;
}

String AT_BC95::getFirmwareVersion(){
  String fw="";
  _Serial->println(F("AT+CGMR"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1) break;
      else fw+=data_input;
    }
  }
  fw.replace(F("OK"),"");
  fw.trim();
  blankChk(fw); 
  return fw;
}

String AT_BC95::getNetworkStatus(){
  String out = "";
  String data = "";
  int count=0;

  _Serial->println(F("AT+CEREG=2"));
  delay(500);
  _serial_flush();
  delay(1000);
  _Serial->println(F("AT+CEREG?"));
  while(1){
    if(_Serial->available()){
      data_input = _Serial->readStringUntil('\n');
      if(data_input.indexOf(F("+CEREG"))!=-1){
        count++;
        if(count<10 && data_input.indexOf(F(",2"))!=-1){
          _serial_flush();
          _Serial->println(F("AT+CEREG?"));
        }
        else {
          data=data_input;
          int index = data.indexOf(F(": "));
          int index2 = data.indexOf(F(","));
          int index3 = data.indexOf(F(","),index2+1);
          out = data.substring(index2+1,index2+2);
          if (out == F("1")) out = F("Registered");
          else if (out == "0") out = F("Not Registered");
          else if (out == "2") out = F("Trying");
        }
      }
      else if(data_input.indexOf(F("OK"))!=-1) break;

    }
  }
  return(out);
}

// Get radio stat.
radio AT_BC95::getRadioStat(){
  radio value;
  _Serial->println(F("AT+NUESTATS"));
  while(1){
    if(_Serial->available()){
      data_input = _Serial->readStringUntil('\n');
      if(data_input.indexOf(F("OK"))!=-1){
        break;
      }
      else{
        if(data_input.indexOf(F("PCI"))!=-1){
          int start_index = data_input.indexOf(F(":"));
          int stop_index  = data_input.indexOf(F("\n"));
          value.pci = data_input.substring(start_index+1,stop_index);
        }
        if(data_input.indexOf(F("RSRQ"))!=-1){
          int start_index = data_input.indexOf(F(":"));
          int stop_index  = data_input.indexOf(F("\n"));
          value.rsrq = data_input.substring(start_index+1,stop_index);
          value.rsrq = String(value.rsrq.toInt()/10);
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

void AT_BC95::blankChk(String& val){
  if(val==""){
    val = "N/A";
  }
}

// bool AT_BC95::checkPSMmode(){
//   bool status = false;
//   _Serial->println(F("AT+CPSMS?"));
//   while(1){
//     if(_Serial->available()){
//       data_input=_Serial->readStringUntil('\n');
//       if(data_input.indexOf(F("+CPSMS:"))!=-1){
//         if(data_input.indexOf(F("1"))!=-1) status = true;
//         else status = false;
//       }
//       if(data_input.indexOf(F("OK"))!=-1){
//         break;
//       }
//     }
//   }
//   return status;
// }

/****************************************/
/**          Send UDP Message          **/
/****************************************/
// Send AT command to send UDP message
void AT_BC95::_Serial_print(String address,String port,unsigned int len){
  _Serial->print(F("AT+NSOST=0"));
  _Serial->print(F(","));
  _Serial->print(address);
  _Serial->print(F(","));
  _Serial->print(port);
  _Serial->print(F(","));
  _Serial->print(len);
  _Serial->print(F(","));
}

// Send message type String
void AT_BC95::_Serial_print(String msg){
  _Serial->print(msg);
}

// Send message type unsigned int
void AT_BC95::_Serial_print(unsigned int msg){
  _Serial->print(msg);
}

// Send message type char *
void AT_BC95::_Serial_print(char *msg){
  _Serial->print(msg);
}

// Send '\r\n'
void AT_BC95::_Serial_println(){
  _Serial->println();
}

/****************************************/
/**        Receive UDP Message         **/
/****************************************/
// Receive incoming message
void AT_BC95:: waitResponse(String &retdata,String server){ 
  unsigned long current=millis();
  if((current-previous>=500) && !(_Serial->available())){
    _Serial->println(F("AT+NSORF=0,512"));
    previous=current;
  }

  if(_Serial->available()){
    char data=(char)_Serial->read();
    if(data=='\n' || data=='\r'){
      if(k>2){
        end=true;
        k=0;
      }
      k++;
    }
    else{
      data_input+=data;
    }
  }
  if (end){
    manageResponse(retdata,server);
    end=false;
    data_input=F("");
  }
  
}

// Split data from incoming message
void AT_BC95:: manageResponse(String &retdata,String server){ 

  if(end){
    if(data_input.indexOf(F("+NSONMI:"))!=-1){           
      at_getBuffer("0","512");
      data_input=F("");
      send_NSOMI=true;
      end=false;
    }
    else{
      end=false;
      if(data_input.indexOf(server)!=-1){  //serverIP
        String left_buffer="";
        //pack data to char array
        char buf[data_input.length()+1];
        memset(buf,'\0',data_input.length());
        data_input.toCharArray(buf, sizeof(buf));

        char *p = buf;
        char *str;
        int i=0;
        int j=0;
        while ((str = strtok_r(p, ",", &p)) != NULL){
          // delimiter is the comma
          if(data_input.indexOf(F("OK"))!=-1){
            j=5;
          }
          else{
            j=4;
          }
          if(i==j){
            retdata=str;
          }
          if(i==j+1){
            left_buffer=str;
          }
          i++;
        }

        if(left_buffer!="0"){
          at_getBuffer("0","512");          
        }
        send_NSOMI=false;
        data_input=F("");
      }       
    }
  }
}

// get incoming data after +NSONMI:
void AT_BC95::at_getBuffer(String socket,String nBuffer){
  _Serial->print(F("AT+NSORF="));
  _Serial->print(socket);
  _Serial->print(F(","));
  _Serial->println(nBuffer);
}

/****************************************/
/**          Utility                   **/
/****************************************/
// char * to hex
String AT_BC95::toHEX(char *str){
  String output="";
  char *hstr;
  hstr=str;
  char out[3];
  memset(out,'\0',2);
  int i=0;
  bool flag=false;
  while(*hstr){
    flag=itoa((int)*hstr,out,16);    
    if(flag){
      output+=out;
    }
    hstr++;
  }
  return output;
}

// Flush unwanted message from serial
void AT_BC95::_serial_flush(){
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
    }
    else{
      data_input="";
      break;
    }
  }
  _Serial->flush();
}

dateTime AT_BC95::getClock(unsigned int timezone){
  dateTime dateTime;
  _Serial->println(F("AT+CCLK?"));
  while(1){
    if(_Serial->available()){
      data_input=_Serial->readStringUntil('\n');
      if(data_input.indexOf(F("+CCLK:"))!=-1){
        byte index = data_input.indexOf(F(":"));
        byte index2 = data_input.indexOf(F(","),index+1);
        byte index3 = data_input.indexOf(F("+"),index2+1);
        dateTime.date = data_input.substring(index+1,index2);         //YY/MM/DD
        dateTime.time = data_input.substring(index2+1,index3);        //GMT time without adding timezone
      }
      if(data_input.indexOf(F("OK"))!=-1){
        break;
      }
    }
  }
  if(dateTime.time!="" && dateTime.date!=""){
    byte index = dateTime.date.indexOf(F("/"));
    byte index2 = dateTime.date.indexOf(F("/"),index+1);
    unsigned int yy = ("20"+dateTime.date.substring(0,index)).toInt();
    unsigned int mm = dateTime.date.substring(index+1,index2).toInt();
    unsigned int dd = dateTime.date.substring(index2+1,dateTime.date.length()).toInt();

    index = dateTime.time.indexOf(F(":"));
    unsigned int hr = dateTime.time.substring(0,index).toInt()+timezone;

    if(hr>=24){
      hr-=24;
      //date+1
      dd+=1;
      if(mm==2){
        if((yy % 4 == 0 && yy % 100 != 0 || yy % 400 == 0)){
          if (dd>29) {
            dd==1;
            mm+=1;
          }
        }
        else if(dd>28){ 
          dd==1;
          mm+=1;
        }
      }
      else if((mm==1||mm==3||mm==5||mm==7||mm==8||mm==10||mm==12)&&dd>31){
        dd==1;
        mm+=1;
      }
      else if(dd>30){
        dd==1;
        mm+=1;
      }
    }
    dateTime.time = String(hr)+dateTime.time.substring(index,dateTime.time.length());
    dateTime.date = String(dd)+"/"+String(mm)+"/"+String(yy);

    dateTime.time.trim();
    dateTime.date.trim();
  }

  blankChk(dateTime.time);
  blankChk(dateTime.date);

  return dateTime;
}
