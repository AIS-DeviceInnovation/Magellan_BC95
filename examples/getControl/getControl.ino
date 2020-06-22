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

String sw_state;
String sw;
String payload;

void setup() 
{
  Serial.begin(9600);
  magel.begin();             //Init Magellan LIB
  payload="{\"sw\":0}";      //Please provide payload with json format
  magel.report(payload);     //Initial switch data to Magellan IoT Platform
}

void loop() 
{
  /*
    Example get swich data from Magellan IoT platform
  */
  sw_state=magel.getControl("sw"); //Get switch data from Magellan IoT platform
  
  if(sw_state.indexOf("40300")==-1) //Response data is not 40300
  {
    sw=sw_state;
  }

  Serial.print("sw ");
  Serial.println(sw);
}
