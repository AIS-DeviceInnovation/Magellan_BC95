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

AIS_BC95_API v1.1.1 NB-IoT.
support Quectel BC95
NB-IoT with AT command
 
Author: Device Innovation team     
Create Date: 3 February 2020. 
Modified: 18 February 2020.

Released for private usage.
*/
#include "AIS_BC95_API.h"

AT_BC95 at_bc95;
void event_null(char *data){}

/****************************************/
/**          Initialization            **/
/****************************************/

AIS_BC95_API::AIS_BC95_API(){
	Event_debug =  event_null;
}

void AIS_BC95_API:: begin(String addressI,String serverdesport){
	Serial.println(F("----------------BEGIN----------------"));
	at_bc95.debug = debug;
	at_bc95.setupModule(addressI,serverdesport);
}

void AIS_BC95_API::pingIP(String IP){
	at_bc95.pingIP(IP);
}

/****************************************/
/**          Send UDP Message          **/
/****************************************/

void AIS_BC95_API::sendMsgHEX(String address,String desport,String payload){
	if(payload.length()>1024){
		Serial.println(F("Warning payload size exceed the limit. [Limit of HEX is 1024]"));
	}
	else send_msg(address,desport,payload.length()/2,payload);	
}

void AIS_BC95_API::sendMsgSTR(String address,String desport,String payload){
	if(payload.length()>512){
		Serial.println(F("Warning payload size exceed the limit. [Limit of String is 512]"));
	}
	else {
		int x_len = payload.length();
		char buf[x_len+2];
		payload.toCharArray(buf,x_len+1);
		send_msg(address,desport,payload.length(),at_bc95.toHEX(buf));
	}
}

void AIS_BC95_API::send_msg(String address,String desport,unsigned int len,String payload){
	Serial.println(F("-------------------------------"));
    Serial.print(F("# Sending Data : "));
    Serial.println(payload);
    Serial.print(F("# IP : "));
    Serial.println(address);
    Serial.print(F("# Port : "));
    Serial.println(desport);

    at_bc95._Serial_print(address,desport,len);
    at_bc95._Serial_print(payload);
    at_bc95._Serial_println();
}

/****************************************/
/**         Receive UDP Message        **/
/****************************************/
void AIS_BC95_API::waitResponse(String &retdata,String server){
  	at_bc95.waitResponse(retdata,server);  	
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
String AIS_BC95_API::getSignal(){
	return at_bc95.getSignal();
}

String AIS_BC95_API::getDeviceIP(){
	return at_bc95.getDeviceIP();
}

String AIS_BC95_API::getIMSI(){
	return at_bc95.getIMSI();
}

String AIS_BC95_API::getICCID(){
	return at_bc95.getICCID();
}

String AIS_BC95_API::getIMEI(){
	return at_bc95.getIMEI();
}

radio AIS_BC95_API::getRadioStat(){
  return at_bc95.getRadioStat();
}

dateTime AIS_BC95_API::getClock(unsigned int timezone){
  return at_bc95.getClock(timezone);
}

// bool AIS_BC95_API::checkPSMmode(){
//   return at_bc95.checkPSMmode();
// }

