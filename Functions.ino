    /*************************************************************************************************************************
      Function definitions: Any modification may leads towards system failure
    *************************************************************************************************************************
    *
    *
    *************************************************************************************************************************
      Booting the system
    *************************************************************************************************************************/
    void boot()
    {  
      pinMode(16, OUTPUT);
      digitalWrite(16, HIGH);
      Serial.begin(9600);
      //Serial.swap();
      //SPIFFS.begin();
      Serial.println("System started");
    //  if(!fetchDatabase()) {
    //    if(!creatDatabase()) {
    //      Serial.println("Problem generating database file");
    //      Serial.println("Formating file system");
    //      if(SPIFFS.format()) {
    //        Serial.println("Succeeded in formating file system");
    //        creatDatabase();
    //      }
    //    }
    //  }
      //client.setServer(MQTT_SERVER, 1883);
      //client.setCallback(callback);
      WiFi.mode(WIFI_STA);
    }

    /*************************************************************************************************************************
      Keep Aliving Loop
    *************************************************************************************************************************/

    void keeplive()
    {
      client.loop();
      if((WiFi.status() != WL_CONNECTED) && ((millis() - wifi_reconnect_time) > wifi_check_time)) {
          wifi_check_time = 15000L;
          wifi_reconnect_time = millis();
          connectWiFi();
        }
      delay(1);
    }

    /*************************************************************************************************************************
      Connect to WiFi network
    *************************************************************************************************************************/

    void connectWiFi()
    {
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

    /*************************************************************************************************************************
      Connect to MQTT Broker
    *************************************************************************************************************************/
    //
    //void connectMQTT()
    //{
    //  if (mqtt_reconnect_tries > 1) {
    //    Serial.print("Retrying:: ");
    //  }
    //  Serial.print("Connecting to mqtt server: ");
    //  Serial.println(MQTT_SERVER);
    //  client.connect(SKETCH_ID, MQTT_USER, MQTT_PASSWORD);
    //  delay(500);
    //  if (client.connected()) {
    //    send("units-bootup", String(SKETCH_ID) + "-bootup"); // Initial system status publish to server
    //    client.subscribe(MQTT_SUBSCRIBE_TOPIC); // Subscribe to your MQTT topic
    //    client.subscribe("subunit-ping"); // Subscribe to Ping
    //    Serial.println(".. Connected!!");
    //    mqtt_reconnect_tries = 0;
    //    scheduled_reboot = false;
    //    updateDatabase(db_array[0], 0);
    //    db_array_value[0] = 0;
    //  } else {
    //
    //    Serial.print("Failed to connect to mqtt server, rc=");
    //    Serial.print(client.state());
    //    Serial.println("");
    //  }
    //}




    /*************************************************************************************************************************
      IP Address to String conversion
    *************************************************************************************************************************/

    String ipToString(IPAddress ip){
      String s="";
      for (int i=0; i<4; i++)
        s += i  ? "." + String(ip[i]) : String(ip[i]);
      return s;
    }


    /*************************************************************************************************************************
      Formating File System
    *************************************************************************************************************************/

    void format()
    {
      Serial.println("initiating SPIFF file system format");
      if(SPIFFS.format()) {
        Serial.println("Succeeded to format file system");
      }
    }

    /*************************************************************************************************************************
      Driving output pins
    *************************************************************************************************************************/



    uint8_t PCF_read8(uint8_t address)
    {
      Wire.beginTransmission(address);
      Wire.requestFrom(address, 1);
      uint8_t _data = Wire.read();
      Wire.endTransmission();
      return _data;
    }

    void PCF_write8(uint8_t address,uint8_t value)
    {
      Wire.beginTransmission(address);
      Wire.write(value);
      Wire.endTransmission();
    }

    uint8_t PCF_toggle(uint8_t pin)
    {
      uint8_t _data = PCF_read8(PCF_OUT_ADDRESS);
      _data ^=  (1 << pin);
      PCF_write8(PCF_OUT_ADDRESS,_data); 
      return _data>>pin&0x01;
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


void tcp_listen(){

 if (!wifi_client.connected()) {
        // try to connect to a new client
        wifi_client = server.available();
    } else {
        // read data from the connected client
        if (wifi_client.available() > 0) {
        char buf [4];
        char req = wifi_client.read();
            /*Serial.write(wifi_client.read());*/
            if(req == 'L'){
                wifi_client.write(PCF_toggle(wifi_client.read()-48)+48);
                wifi_client.write('\n');
            }
            if(req == 'H'){
                sprintf (buf, "%d", humidity);
                wifi_client.write((const char*)&buf[0],(size_t)(2));
                wifi_client.write('\n');
            }
            if(req == 'T'){
                sprintf (buf, "%d", temp);
                wifi_client.write((const char*)&buf[0],(size_t)(2));
                wifi_client.write('\n');
            }
        }
    }
}
