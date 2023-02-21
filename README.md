![Library Version](https://img.shields.io/badge/Version-2.1.0-green)

# AIS DEVIO NB-SHIELD I & DEVIO NB-SHIELD I Plus Library 
This library only supports the [DEVIO NB-SHIELD I & DEVIO NB-SHIELD I Plus](https://aisplayground.ais.co.th/marketplace/products/64DkezcxuRk).
If you have any questions or concerns, please reach out to us on [DEVIO FB Fanpage](https://www.facebook.com/AISDEVIO)

## AIS BC95-B8 & BC95-G API
The AIS BC95 API Library is an Arduino IDE library that has been developed to support platforms that use UDP, MQTT (BC95-G only)

### AIS BC95 API Example code

#### Call the AIS BC95 API library:
```cpp
#include "AIS_BC95_API.h"
AIS_BC95_API nb;
```
#### Initializing the AIS BC95 API Library:
**UDP**
```cpp
nb.begin(address,port);    
```
**MQTT**
```cpp
nb.begin();    
setupMQTT(); 
//nb.connectMQTT(host, port, clientID, username, password)
nb.setCallback(callback);     
```
#### Send Data:
 **UDP**
```cpp
nb.sendMsgSTR(host,port,payload);  // Send data in String 
// or
nb.sendMsgHEX(host,port,payload);  // Send data in HexString   
```
**MQTT**
```cpp
nb.publish(topic,payload,pubQoS,pubRetained);  
``` 
 
## AIS Magellan Library

The AIS Magellan Library is an SDK for use with the Magellan IoT Platform and the Arduino IDE.

### Magellan SDK Example Code
#### Calling the Magellan Library:
```cpp
#include "Magellan_BC95.h"
Magellan_BC95 magel;
```
#### Initializing the Magellan Library:
```cpp
magel.begin();           //init Magellan LIB
```
#### Payload Data: 
Please ensure that the payload is in JSON format, for example:
```cpp
payload="{\"Temperature\":"+Temperature+",\"Humidity\":"+Humidity+"}";
```
#### Reporting Data:
Please ensure that the payload is in JSON format, for example:
```cpp
magel.report(payload);
```
#### Magellan payload format example 
**Location**
```cpp
payload="{\"Location\":"Latitude,Longitude"}";
```
**Battery Status on Dashboard**\
Battery must be in the range of 0-100 %
```cpp
payload="{\"Battery\":100}";   
```
**Lamp Status**\
Send status using 0 or 1.
```cpp
payload="{\"Lamp\":0}"; 
payload="{\"Lamp\":1}";
```
**Note** For more examples, please refer to the example code included in the Arduino IDE.
