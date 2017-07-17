/*************************************************************************************************************************
*    HomeAutomation 2016
*    Version 3.0
*    Created By: Waqas Ahmed
*         Email: ahmed@hobbytronics.com.pk
*    All Rights Reserved © 2016 HobbyTronics Pakistan
*    
*    Configure connection parameters in config.h
*    Any modification in Functions, init.h tabs and whereever mentioned may leads towards failure of software
*    Download necessary libraries from the links mentioned before the library including statement
*
*
*************************************************************************************************************************
  Required Libraries
*************************************************************************************************************************/

#include <ESP8266WiFi.h>          //builtin library for ESP8266 Arduino Core
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "FS.h"                   //builtin library for ESP8266 Arduino Core
#include "config.h"               //package builtin configuration file
#include "init.h"                 //package builtin configuration file
#include "Nextion.h"
#include <time.h>
#include <Time.h>
#include "DHT.h"
#include <Wire.h>
#include <SimpleTimer.h>


/*************************************************************************************************************************
  Database variables
*************************************************************************************************************************/
WiFiServer server(5555);
WiFiClient wifi_client;
DHT dht(DHTPin, DHTTYPE);
SimpleTimer timer;
SoftwareSerial HMISerial(SW_SER_RX,SW_SER_TX);  // RX,TX  see NexConfig.h - using software serial for ESP8266
String db_array[] = {"Reboot", "q0", "q1", "q2", "q3"};
int db_array_len = 5;
char buffer[100] = {0};

//vars to save buttons state
bool NexFanA_state = false;
bool NexFanB_state = false;
bool NexLight_state = false;
bool NexPlug_state = false;
/*
 * Declare a crop object [page id:0,component id:1, component name: "q0"]. 
 */

NexText NexTemp = NexText(0, 10, "NexTemp");
NexText NexHumid = NexText(0, 11, "NexHumid");
NexText NexTime = NexText(0, 8, "NexTime");
NexText NexTimeA = NexText(0, 9, "NexTimeA");
NexText NexMonth = NexText(0, 7, "NexMonth");
NexText NexDate = NexText(0, 6, "NexDate");
NexCrop NexFanA = NexCrop(0, 2, "NexFanA");
NexCrop NexLight = NexCrop(0, 3, "NexLight");
NexCrop NexPlug = NexCrop(0, 4, "NexPlug");
NexCrop NexFanB = NexCrop(0, 5, "NexFanB");

/*
   Register object textNumber, buttonPlus, buttonMinus, to the touch event list.
*/
NexTouch *nex_listen_list[] =
{
  &NexTemp,
  &NexHumid,
  &NexTime,
  &NexTimeA,
  &NexMonth,
  &NexDate,
  &NexFanA,
  &NexLight,
  &NexPlug,
  &NexFanB,
  NULL
};

void update_buttons()
{
    NexFanA.setPic(db_array_value[1]);
    NexLight.setPic(db_array_value[2]);
    NexPlug.setPic(db_array_value[3]);
    NexFanB.setPic(db_array_value[4]);
}

void NexFanAPushCallback(void *ptr)
{
   NexFanA_state = !NexFanA_state;
    uint32_t number = NexFanA_state;
    NexFanA.setPic(number);
    PCF_write(L0,NexFanA_state);

}

void NexLightPushCallback(void *ptr)
{
   NexLight_state = !NexLight_state;
    uint32_t number = NexLight_state;
    NexLight.setPic(number);

    PCF_write(L1,NexLight_state);
}

void NexPlugPushCallback(void *ptr)
{
   NexPlug_state = !NexPlug_state;
    uint32_t number = NexPlug_state;
    NexPlug.setPic(number);

    PCF_write(L2,NexPlug_state);
}

void NexFanBPushCallback(void *ptr)
{
   NexFanB_state = !NexFanB_state;
    uint32_t number = NexFanB_state;
    NexFanB.setPic(number);

    PCF_write(L3,NexFanB_state);
}



/*************************************************************************************************************************
  Setup Function
*************************************************************************************************************************/

void setup() {

    boot();     //necessary to call at first during setup function for proper functioning
    pinMode(B0, INPUT_PULLUP);
    pinMode(B1, INPUT_PULLUP);
    pinMode(B2, INPUT_PULLUP);
    pinMode(B3, INPUT_PULLUP);
    pinMode(B4, INPUT);

    attachInterrupt(B0, interrupt1, FALLING);
    attachInterrupt(B1, interrupt2, FALLING);
    attachInterrupt(B2, interrupt2, FALLING);
    attachInterrupt(B3, interrupt3, CHANGE);
    attachInterrupt(B4, interrupt4, FALLING);

    timer.setInterval(35000, update_info);//set timer for updating temp
    Wire.begin(SDA_PIN,SCL_PIN);//start i2c
    /*PCF_write8(0); //all ssr off*/
    PCF_write8(0xFF); //all ssr off*/
    /*PCF_read8();*/


    //LCD init
    nexInit();
    NexFanA.attachPush(NexFanAPushCallback);
    NexLight.attachPush(NexLightPushCallback);
    NexPlug.attachPush(NexPlugPushCallback);
    NexFanB.attachPush(NexFanBPushCallback);
    update_buttons();

    //DHT senrosr init
    dht.begin(); 


    connectWiFi();  //necessary to call at the last during setup function for proper functioning

    //start tcp server
    server.begin(); 

    //configure ntp server and get time
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("\nWaiting for time");
    while (!time(nullptr)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("");
}


/*************************************************************************************************************************
  Loop Function
*************************************************************************************************************************/
void loop()
{
    timer.run();
    tcp_listen();
   

  /*keeplive();   //necessary to call keep alive for proper functioning*/
  nexLoop(nex_listen_list);

    /*PCF_toggle(0);*/
    /*PCF_toggle(1);*/
    /*PCF_toggle(2);*/
    /*PCF_toggle(3);*/

}


void interrupt0(){
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) 
    {
        Serial.println("interrupt0");
    }
    last_interrupt_time = interrupt_time; 
}


void interrupt1(){

    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) 
    {
        Serial.println("interrupt1");
    }
    last_interrupt_time = interrupt_time; 
}


void interrupt2(){

    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) 
    {
        Serial.println("interrupt2");
    }
    last_interrupt_time = interrupt_time; 
}

void interrupt3(){

        Serial.println("interrupt3");
        return;

    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) 
    {
        Serial.println("interrupt3");
    }
    last_interrupt_time = interrupt_time; 
}


void interrupt4(){

    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) 
    {
        Serial.println("interrupt4");
    }
    last_interrupt_time = interrupt_time; 
}

void update_info(){

/*read and show time on LCD*/

  time_t now = time(nullptr);
  setTime(now);
  NexMonth.setText(monthStr(month()));
  memset(buffer, 0, sizeof(buffer));
  itoa(day(now), buffer, 10);
  NexDate.setText(buffer);
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, 5, "%03d:%03d:03d", hour(),minute(),second());
  NexTime.setText(buffer);



  int h = dht.readHumidity();
  int t = dht.readTemperature();
  /*[>float hic = dht.computeHeatIndex(t, h, false);       <]*/
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
  }
  else{
      memset(buffer, 0, sizeof(buffer));
      snprintf(buffer, 4, "%dC", t);
      NexTemp.setText(buffer);
      memset(buffer, 0, sizeof(buffer));
      snprintf(buffer, 4, "%d%", h);
      NexHumid.setText(buffer);
    }

}

/*************************************************************************************************************************
  End of the file
    *************************************************************************************************************************/

