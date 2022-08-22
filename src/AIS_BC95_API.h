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

AIS_BC95_API v2.0.0 NB-IoT.
support Quectel BC95-B8 & BC95-G
NB-IoT with AT command
 
Author: Device Innovation team     
Create Date: 8 February 2021. 
Modified: 31 May 2021.

Released for private usage.
*/

#ifndef AIS_BC95_API_h
#define AIS_BC95_API_h

#include <Arduino.h>
#include "AT_BC95.h"

class AIS_BC95_API{
public:
	AIS_BC95_API();
	bool debug;

	void (*Event_debug)(char *data);	

	void begin(String serverdesport, String addressI);

	String getDeviceIP();
	String getSignal();
	String getIMSI();
	String getIMEI();
	String getICCID();
	radio getRadioStat();
	dateTime getClock(unsigned int timezone=7);
	void pingIP(String IP);
	// bool checkPSMmode();

	void sendMsgHEX(String address,String desport,String payload);
	void sendMsgSTR(String address,String desport,String payload);	

	void waitResponse(String &retdata,String server);

private:
	void send_msg(String address,String desport,unsigned int len,String payload);
		

protected:
	 Stream *_Serial;	
};

#endif
