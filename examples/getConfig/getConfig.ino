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

String threshold;

void setup() 
{
  Serial.begin(9600);
  magel.begin();           //Init Magellan LIB
}

void loop() 
{
  /*
    Example get config data from Magellan IoT platform
  */
  threshold = magel.getConfig("threshold"); //Get configuration data from Magellan
  Serial.print("threshold = ");
  
  if(threshold.indexOf("40300")!=-1)        //Response data is 40300
  {
    Serial.println("Device has not registered or it has not configuraton profile in Magellan Platform");
  }
  else
  {
    Serial.println(threshold);              //Config data from Magellan IoT platform
  }
  Serial.println();
}
