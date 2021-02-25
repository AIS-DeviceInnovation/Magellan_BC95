
![Library Version](https://img.shields.io/badge/Version-1.1.1-green)

# AIS Library

## AIS BC95 API
AIS BC95 API Library, used on arduino, have been developed for 
any platform. This library include API such as UDP.
### Example for use the AIS BC95 API
#### Call the AIS BC95 API library:
```cpp
#include "AIS_BC95_API.h"
AIS_BC95_API nb;
```
#### Initial AIS BC95 API Library:
```cpp
nb.begin(serverIP,serverPort);    
```

#### Send Data:
```cpp
nb.sendMsgSTR(serverIP,serverPort,payload);  // Send data in String 
// or
nb.sendMsgHEX(serverIP,serverPort,payload);  // Send data in HexString   
```
 **Note** please see more in the example code  
 
## AIS Magellan Library
AIS Magellan Library, a software development kit used on arduino platform, have been developed for 
Magellan IoT Platform.  

### Example for use the Magellan SDK
#### Call the Magellan library:
```cpp
#include "Magellan_BC95.h"
Magellan_BC95 magel;
```
#### Initial Magellan Library:
```cpp
magel.begin();           //init Magellan LIB
```
#### Payload Data: 
Please use the payload in JSON format 

**Example**\
{"Temperature":25,"Humidity":90}

```cpp
payload="{\"Temperature\":"+Temperature+",\"Humidity\":"+Humidity+"}";
```
#### Report Data:
The example code report payload data to Magellan IoT Platform.
```cpp
magel.report(payload);
```
#### Example Magellan payload format
Please the location payload data as below format.\
**Example**
```cpp
payload="{\"Location\":"Latitude,Longitude"}";
```
Show battery on dashboard\
Battery is range in 0-100 %.\
**Example**
```cpp
payload="{\"Battery\":100}";   
```
Show Lamp status on dashbord\
please use 0 or 1 to send status\
**Example**
```cpp
payload="{\"Lamp\":0}"; 
payload="{\"Lamp\":1}";
```
**Note** please see more in the example code 
