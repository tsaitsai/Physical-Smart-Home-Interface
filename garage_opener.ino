/*
Author: Eric Tsai
License: CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
Date: 2015-04-11
File: garage_opener.ino
// gateway with MQTT subscribe/publish functions
// Edits requires ***************
// Change IP address of "server"
// Change MAC address if desired

*/



#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <stdlib.h>


//Ethernet
byte mac[]    = {  0x90, 0xA2, 0xDA, 0x0D, 0x31, 0xB9 };
byte server[] = { 192, 168, 1, 101 };		//MQTT broker
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
unsigned long keepalivetime=0;
unsigned long MQTT_reconnect=0;



int pin_relay = 5;
bool conn_ok;
bool relay_flag = 0;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("received MQTTTTT");
  
  //convert topic to int.
  int mytopic = atoi (topic);
  Serial.print(mytopic);
  
  //convert message to float
  payload[length] = '\0';
  float msg = atof( (const char *) payload);
  Serial.print(" = ");
  Serial.println(msg);

  
  if ((mytopic == 4132) || (mytopic == 2872))
  {
	Serial.println("topic received");
	relay_flag = 1;
  }
  

}//end call back


void setup() 
{
  Serial.begin (9600);
    
  //turn off relay
  pinMode (pin_relay, OUTPUT);
  digitalWrite(pin_relay, LOW);  //reed relay
 
  Serial.println("starting");
  
  //wait for IP address
  while (Ethernet.begin(mac) != 1)
 
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(3000);
  }
  
  //Ethernet.begin(mac, ip);
  Serial.println("ethernet OK");
  keepalivetime=millis();


  while (client.connect("garage_opener") != 1)
  {
    Serial.println("Error connecting to MQTT");
    delay(3000);
  }
  MQTT_reconnect = millis();
  Serial.println("setup complete");
    client.publish("fromArduino","hello world");
    client.subscribe("4132");	//4132 = topic for openhab garage button
    client.subscribe("2872");	//2872 = topic for smarthome button 2
}  // end of setup



void loop() 
{
  
  client.loop();
  
  
  if (relay_flag == 1)
  {
	Serial.println("Garage door open/close");
	digitalWrite(pin_relay, HIGH);
    delay(1200);
    digitalWrite(pin_relay, LOW);
	relay_flag = 0;		//turn off flag after button pressed
  }
  
  //check for MQTT connection
  if ((millis() - MQTT_reconnect) > 60000)  //check MQTT connection every X seconds
  {
    conn_ok = client.connected();
    if (conn_ok==1)
    {
      Serial.println("MQTT connected OK");
    }
    else
    {
      Serial.println("MQTT NOT connected OK");
    }
    
    //no connection, reconnect
    if (conn_ok == 0)
    {
      client.disconnect();
      delay(5000);
      while (client.connect("garage_opener") != 1)
      {

        Serial.println("Error connecting to MQTT");
        delay(4000);
      }
    }
    //Serial.println("reconnected to MQTT");
    MQTT_reconnect = millis();
    client.publish("fromArduino","still connected");
  } //end examining MQTT connection every X seconds.
    
    
}  // end of loop
