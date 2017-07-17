/*************************************************************************************************************************
  Configure connection parameters
*************************************************************************************************************************/

const char* SKETCH_ID = "nextion";                  //Non-overlaping name of sketch
const char* SKETCH_VERSION = "0.1";                //Sketch version
const char* MQTT_PUBLISH_TOPIC = "nextion-out";     //Topic name to publish on mqtt
const char* MQTT_SUBSCRIBE_TOPIC = "nextion-in";    //Topic to subscribe at mqtt for message reception

//const char* WIFI_SSID = "Stotis";
const char* WIFI_SSID = "prapisai_gulta";
//const char* WIFI_PASSWORD = "turekbabkiu";
const char* WIFI_PASSWORD = "bukvyras";
//const char* MQTT_SERVER = "192.168.0.30";          //IP Adress of Machine running MQTT Broker
//const char* MQTT_USER = "pi";                      //MQTT Broker User Name
//const char* MQTT_PASSWORD = "ChawaDheN"; ,          //MQTT Broker Password


#define PCF_ADDRESS 0x38
#define DHTTYPE DHT11   // DHT 11
#define HOSTNAME "svetaine1"
#define SDA_PIN 14 
#define SCL_PIN 12
#define SW_SER_RX 4
#define SW_SER_TX 5
#define L0 0
#define L1 1
#define L2 2
#define L3 3
#define L4 4
#define L5 5
#define L6 6
#define L6 7


#define B0 10
#define B1 9
#define B2 0
#define B3 13
#define B4 15
// DHT Sensor
#define DHTPin  2


/*************************************************************************************************************************
  Configure database parameters
*************************************************************************************************************************/

int db_array_default_value = 0;                    //default value to write in database e.g. default gpio state
