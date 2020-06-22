/* 
  Example for report random data to Magellan IoT Platform
  please use payload with json format e.g. {"Temperature":25.5,"Humidity":90.4,"Light":45}
  and {"Location":"13.001,100.44"} for plot the location on MAP
                    |        |
                latitude  longitude 

  and {"Lamp":0} or {"Lamp":1"} for show the Lamp status on dashboard
  
  Please login and enjoy with https://magellan.ais.co.th
  
  If you have any questions, please see more details at https://magellan.ais.co.th
  
         " Plug NB-IoT Shield on Arduino Mega "
    1. Jump Pin 8 (RX) NB to Pin 48 Mega 
    2. Jump Pin 9 (TX) NB to Pin 46 Mega
*/

#include "Magellan_BC95.h"
Magellan_BC95 magel;          

String payload;

void setup() 
{
  Serial.begin(9600);
  magel.begin();           //Init Magellan LIB
}

void loop() 
{
  //Positive Number Type
  unsigned int Positive_Number = 1234;
  payload ="{\"Positive_Number\":"+String(Positive_Number)+"}";
  magel.report(payload);  //Report data positive number type
  delay(5000);            //Delay 5 second

  //Negative Number Type
  int Negative_Number = -1234;
  payload ="{\"Negative_Number\":"+String(Negative_Number)+"}";
  magel.report(payload);  //Report data negative number type
  delay(5000);            //Delay 5 second
  
  //Floating Point Number Type
  float Floating_Point_Number = 12.34;
  payload ="{\"Floating_Point_Number\":"+String(Floating_Point_Number)+"}";
  magel.report(payload);  //Report data decimal number type
  delay(5000);            //Delay 5 second

  //Text Type
  String Text = "\"Hello World\"";
  payload ="{\"Text\":"+String(Text)+"}";
  magel.report(payload);  //Report data text type
  delay(5000);            //Delay 5 second

  //Location Type
  String Location = "\"13.764980,100.538335\"";
  payload ="{\"Location\":"+String(Location)+"}";
  magel.report(payload);  //Report data location type
  delay(5000);            //Delay 5 second

  //Boolean Type
  bool Boolean = 1;
  payload ="{\"Boolean\":"+String(Boolean)+"}";
  magel.report(payload);  //Report data boolean type
  delay(5000);            //Delay 5 second
}
