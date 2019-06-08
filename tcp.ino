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
            if(req == 'L'){
                wifi_client.write(PCF_toggle(wifi_client.read()-48)+48);
                DataChanged = true;
                return;
            }
            if(req == 'H'){
                char buf [30];
                sprintf (buf, "%2.1f", humidity);
                wifi_client.write((const char*)&buf[0],(size_t)(2));
                return;

            }
            if(req == 'T'){
                char buf [30];
                sprintf (buf, "%2.1f", temp);
                wifi_client.write((const char*)&buf[0],(size_t)(2));
                return;
            }
            if(req == 'A'){
                char buf [30];
                sprintf (buf, "S%2.1f_%2.1f_%d",
                        temp,humidity,Data&0xff);
                wifi_client.write((const char*)&buf[0],(size_t)(30));
                return;
            }
            if(req == 'J'){
                char buf [30];
                wifi_client.write(PCF_toggle_all(wifi_client.read()-48));
                DataChanged = true;
                return;
            }

            while(wifi_client.read()!=-1);
            /*my_serial.write(Data);*/

        }
    }
}
