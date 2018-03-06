#include <EEPROM.h>

#include <ESP8266NetBIOS.h>

/*******************************
  Required Libraries
 *******************************/

#include <ESP8266WiFi.h>          //builtin library for ESP8266 Arduino Core
#include <SoftwareSerial.h>
#include "FS.h"                   //builtin library for ESP8266 Arduino Core
#include "config.h"               //package builtin configuration file
#include "init.h"                 //package builtin configuration file
#include "Nextion.h"
#include <time.h>
#include <Time.h>
//#include "DHT.h"
#include <Wire.h>
#include <SimpleTimer.h>
#include "DHTesp.h"

DHTesp dht;

/*******************************
  Database variables
 *******************************/
WiFiServer server(5555);
WiFiClient wifi_client;
//DHT dht(DHTPin, DHTTYPE);
SimpleTimer timer;
SoftwareSerial HMISerial(SW_SER_RX,SW_SER_TX);  // RX,TX  see NexConfig.h - using software serial for ESP8266
char buffer[100] = {0};
float humidity = -1;
float temp = -1;

//vars to save buttons state
bool NexFanA_state = false;
bool NexFanB_state = false;
bool NexLight_state = false;
bool NexPlug_state = false;
/*
 * Declare a crop object [page id:0,component id:1, component name: "q0"].
 */
//
//NexText NexTemp = NexText(0, 10, "NexTemp");
//NexText NexHumid = NexText(0, 11, "NexHumid");
//NexText NexTime = NexText(0, 8, "NexTime");
//NexText NexTimeA = NexText(0, 9, "NexTimeA");
//NexText NexMonth = NexText(0, 7, "NexMonth");
//NexText NexDate = NexText(0, 6, "NexDate");
//NexCrop NexFanA = NexCrop(0, 2, "NexFanA");
//NexCrop NexLight = NexCrop(0, 3, "NexLight");
//NexCrop NexPlug = NexCrop(0, 4, "NexPlug");
//NexCrop NexFanB = NexCrop(0, 5, "NexFanB");

/*
   Register object textNumber, buttonPlus, buttonMinus, to the touch event list.
 */
//NexTouch *nex_listen_list[] =
//{
//  &NexTemp,
//  &NexHumid,
//  &NexTime,
//  &NexTimeA,
//  &NexMonth,
//  &NexDate,
//  &NexFanA,
//  &NexLight,
//  &NexPlug,
//  &NexFanB,
//  NULL
//};
//
//void update_buttons()
//{
//    NexFanA.setPic(db_array_value[1]);
//    NexLight.setPic(db_array_value[2]);
//    NexPlug.setPic(db_array_value[3]);
//    NexFanB.setPic(db_array_value[4]);
//}
//
//void NexFanAPushCallback(void *ptr)
//{
//   NexFanA_state = !NexFanA_state;
//    uint32_t number = NexFanA_state;
//    NexFanA.setPic(number);
//    PCF_write(L0,NexFanA_state);
//
//}
//
//void NexLightPushCallback(void *ptr)
//{
//   NexLight_state = !NexLight_state;
//    uint32_t number = NexLight_state;
//    NexLight.setPic(number);
//
//    PCF_write(L1,NexLight_state);
//}
//
//void NexPlugPushCallback(void *ptr)
//{
//   NexPlug_state = !NexPlug_state;
//    uint32_t number = NexPlug_state;
//    NexPlug.setPic(number);
//
//    PCF_write(L2,NexPlug_state);
//}
//
//void NexFanBPushCallback(void *ptr)
//{
//   NexFanB_state = !NexFanB_state;
//    uint32_t number = NexFanB_state;
//    NexFanB.setPic(number);
//
//    PCF_write(L3,NexFanB_state);
//}
//

/*****************************
  Booting the system
 ******************************/

void boot()
{
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);
    Serial.begin(9600);
    //Serial.swap();
    //SPIFFS.begin();
    Serial.println("System started");
    WiFi.mode(WIFI_STA);
}

/*******************************
  Connect to WiFi network
 *******************************/

void connectWiFi()
{
    WiFi.hostname(HOSTNAME);
    ++ wifi_reconnect_tries;
    boolean networkScan = false;
    int n = WiFi.scanNetworks();
    delay(300);
    for (int i = 0; i < n; ++i) {

        if (WiFi.SSID(i) == WIFI_SSID) {
            String this_print = String(WIFI_SSID) + " is available";
            Serial.println(this_print);
            networkScan = true;
            break;
        }
    }
    if(networkScan) {
        if (wifi_reconnect_tries > 1) {
            Serial.print("Retrying:: ");
        }
        String this_print = "Connecting to " + String(WIFI_SSID);
        Serial.println(this_print);
        long wifi_initiate = millis();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            if (WiFi.status() == WL_CONNECTED) {
                break;
            }
            if ((millis() - wifi_initiate) > 15000L) {
                break;
            }
            delay(500);
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print(" Connected!!");
            Serial.println("");
            Serial.println(WiFi.localIP());
            wifi_reconnect_tries = 0;
        } else if ((WiFi.status() != WL_CONNECTED) && (wifi_reconnect_tries > 3)) {
            String this_print = " Failed to connect to " + String(WIFI_SSID) + " Rebooting...";
            Serial.println(this_print);
        }
    } else {
        Serial.print(WIFI_SSID);
        Serial.print(" is offline");
        Serial.println("");
        if (wifi_reconnect_tries > 3) {
            wifi_check_time = 300000L;
            wifi_reconnect_tries = 0;
            Serial.println("System will try again after 5 minutes");
        }
    }
}


/***************************
  Keep Aliving Loop
 ***************************/

void keeplive()
{
    //client.loop();
    if((WiFi.status() != WL_CONNECTED) && ((millis() - wifi_reconnect_time) > wifi_check_time)) {
        wifi_check_time = 15000L;
        wifi_reconnect_time = millis();
        connectWiFi();
    }
    delay(1);
}


/*******************************
  IP Address to String conversion
 *******************************/

String ipToString(IPAddress ip){
    String s="";
    for (int i=0; i<4; i++)
        s += i  ? "." + String(ip[i]) : String(ip[i]);
    return s;
}


/*******************************
  Formating File System
 *******************************/

void format()
{
    Serial.println("initiating SPIFF file system format");
    if(SPIFFS.format()) {
        Serial.println("Succeeded to format file system");
    }
}

/*******************************
  Driving output pins
 *******************************/

uint8_t Data = 0;

uint8_t PCF_read8(uint8_t address)
{
    Wire.beginTransmission(address);
    Wire.requestFrom(address, 1);
    uint8_t data = Wire.read();
    Wire.endTransmission();
    return data;
}

void PCF_write8(uint8_t address,uint8_t value)
{
    Data = value;
    Wire.beginTransmission(address);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t PCF_toggle(uint8_t pin)
{
    /*uint8_t Data = PCF_read8(PCF_OUT_ADDRESS);*/

    Data ^=  1 << pin;
    PCF_write8(PCF_OUT_ADDRESS,Data);

    Serial.print("Write : ");
    Serial.println(Data);
    return (Data>>pin)&0x01;
}

uint8_t PCF_toggle_all(uint8_t pin)
{
    /*uint8_t Data = PCF_read8(PCF_OUT_ADDRESS);*/
    if(pin == 1)
        Data = 0xff;
    else Data = 0;

    PCF_write8(PCF_OUT_ADDRESS,Data);

    Serial.print("Write : ");
    Serial.println(Data);
    return pin;
}


void PCF_write(uint8_t pin, uint8_t val)
{
    uint8_t _data = PCF_read8(PCF_OUT_ADDRESS);
    if(val)
        _data |=  (1 << pin);
    else
        _data &=  ~(1 << pin);
    PCF_write8(PCF_OUT_ADDRESS,_data);
}


uint8_t PCF_read(uint8_t address, uint8_t pin)
{
    uint8_t _data = PCF_read8(address);
    return (_data>>pin)&0x01;
}


uint8_t PCF_detect_low_pin(){
    uint8_t data = PCF_read8(PCF_IN_ADDRESS);
    for(uint8_t i = 0; i<8; i++){
        if(data & (0x01<<i))
            return i;
    }

    return 8;

}

/*******************************
  Listen for tcp commands
 *******************************/

void tcp_listen(){

    if (!wifi_client.connected()) {
        // try to connect to a new client
        wifi_client = server.available();
    } else {
        // read data from the connected client
        if (wifi_client.available() > 0) {
            char req = wifi_client.read();
            /*Serial.write(wifi_client.read());*/
            if(req == 'L'){
                char buf [30];
                wifi_client.write(PCF_toggle(wifi_client.read()-48)+48);
                /*wifi_client.write('\n');*/
                return;
            }
            if(req == 'H'){
                char buf [30];
                sprintf (buf, "%2.1f", humidity);
                wifi_client.write((const char*)&buf[0],(size_t)(2));
                /*wifi_client.write('\n');*/
                return;

            }
            if(req == 'T'){
                char buf [30];
                sprintf (buf, "%2.1f", temp);
                wifi_client.write((const char*)&buf[0],(size_t)(2));
                /*wifi_client.write('\n');*/
                return;
            }
            if(req == 'A'){
                char buf [30];
                sprintf (buf, "S%2.1f_%2.1f_%d",
                        temp,humidity,Data&0xff);
                wifi_client.write((const char*)&buf[0],(size_t)(30));
                /*wifi_client.write('\n');*/
                return;
            }
            if(req == 'J'){
                char buf [30];
                wifi_client.write(PCF_toggle_all(wifi_client.read()-48));
                /*wifi_client.write('\n');*/
                return;
            }
            while(wifi_client.read()!=-1);
            /*wifi_client.write(req);*/
            /*wifi_client.write("ERROR\n");*/


        }
    }
}

/*******************************
 Update sensors, time
 *******************************/


void update_info(){

    /*read and show time on LCD*/

    time_t now = time(nullptr);
    setTime(now);
    //  NexMonth.setText(monthStr(month()));
    memset(buffer, 0, sizeof(buffer));
    itoa(day(now), buffer, 10);
    //  NexDate.setText(buffer);
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, 5, "%03d:%03d:03d", hour(),minute(),second());
    //  NexTime.setText(buffer);

    float Humi = dht.getHumidity();
    float Temp = dht.getTemperature();

    Serial.print(dht.getStatusString());
    Serial.print("\n");
    Serial.print(Humi, 3);
    Serial.print("\n");
    Serial.print(Temp, 3);
    Serial.print("\n");


    //uint8_t Humi = dht.readHumidity();
    //uint8_t Temp = dht.readTemperature();
    /*[>float hic = dht.computeHeatIndex(t, h, false);       <]*/
    // Check if any reads failed and exit early (to try again).
    if (isnan(Humi) || isnan(Temp)) {
        Serial.println("Failed to read from DHT sensor!");
        humidity = -1;
        temp = -1;
    }
    else{

        humidity = Humi;
        temp = Temp;
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, 4, "%dC", temp);
        //      NexTemp.setText(buffer);
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, 4, "%d%", humidity);
        //      NexHumid.setText(buffer);
    }

}

/*******************************
PCF gpio interrupt
 *******************************/

void interrupt(){

    uint8_t pin_num = PCF_detect_low_pin();
    if(pin_num == 8) return;

    Serial.print("Togle pin : ");
    Serial.println(pin_num);
    PCF_toggle(pin_num);

}




/*******************************
  Setup Function
 *******************************/

void setup() {

    boot();     //necessary to call at first during setup function for proper functioning
    pinMode(INT_PIN, INPUT_PULLUP);
    dht.setup(DHTPin,DHTesp::DHT22); // data pin 2

    attachInterrupt(INT_PIN, interrupt, FALLING);

    timer.setInterval(15000, update_info);//set timer for updating temp
    Wire.begin(SDA_PIN,SCL_PIN);//start i2c
    /*PCF_write8(0); //all ssr off*/
    PCF_write8(PCF_IN_ADDRESS,0x0); //all ssr off*/
    PCF_write8(PCF_OUT_ADDRESS,0x0); //all ssr off*/
    /*PCF_read8();*/
    //
    //
    //    //LCD init
    //    nexInit();
    //    NexFanA.attachPush(NexFanAPushCallback);
    //    NexLight.attachPush(NexLightPushCallback);
    //    NexPlug.attachPush(NexPlugPushCallback);
    //    NexFanB.attachPush(NexFanBPushCallback);
    //    update_buttons();

    //DHT senrosr init
    //dht.begin();

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
    //update_info();
}


/*******************************
  Loop Function
 *******************************/
void loop()
{
    timer.run();
    tcp_listen();
    keeplive();   //necessary to call keep alive for proper functioning
    //nexLoop(nex_listen_list);

}




/*static unsigned long last_interrupt_time = 0;*/
/*unsigned long interrupt_time = millis();*/

/*if (interrupt_time - last_interrupt_time > 200) */
/*{*/
/*Serial.println("interrupt0");*/
/*}*/
/*last_interrupt_time = interrupt_time; */
/*}*/



/*******************************
  End of the file
 *******************************/
