#include "Magellan_BC95.h"
Magellan_BC95 magel;

void setup() {
  Serial.begin(9600);
  magel.begin();           //Init Magellan LIB
}

void loop() {
  Serial.println("---------- Clock ------------"); // Date,Time
  dateTime timeClock = magel.getClock();
  Serial.println("date : "+timeClock.date);
  Serial.println("time : "+timeClock.time);
  Serial.println("------------ End ------------");
  delay(1000);
  Serial.println("---------- Radio ------------"); // Date,Time
  radio radioStat = magel.getRadioStat();
  Serial.println("pci : "+radioStat.pci);
  Serial.println("rsrq : "+radioStat.rsrq);
  Serial.println("------------ End ------------");
  delay(1000);
  Serial.println("---------- Signal -----------"); // Signal Quality Report 
  Serial.println("Signal : "+magel.getSignal());
  Serial.println("------------ End ------------");
  delay(1000);
  Serial.println("------------ Ping -----------"); // Test IP Network Connectivity to A Remote Host
  String address  = "8.8.8.8"; 
  pingRESP pingIP = magel.pingIP(address);
  Serial.print("Status : ");
  Serial.println(pingIP.status);
  Serial.println("Addr : "+pingIP.addr);
  Serial.println("ttl : "+pingIP.ttl);
  Serial.println("rtt : "+pingIP.rtt);
  Serial.println("------------ End ------------");
  delay(1000);
}