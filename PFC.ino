
uint8_t in2outRemap[8] = {

    0,
    1,
    2,
    3,
    4,
    7,
    6,
    5,
};

/*******************************
  Driving output pins
 *******************************/


uint8_t PCF_read8(uint8_t address)
{
    Wire.beginTransmission(address);
    Wire.requestFrom(address, (uint8_t) 1);
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
    Data ^=  1 << pin;
    PCF_write8(PCF_OUT_ADDRESS,Data);
    return (Data>>pin)&0x01;
}

uint8_t PCF_toggle_all(uint8_t pin)
{
    if(pin == 1)
        Data = 0xff;
    else Data = 0;

    PCF_write8(PCF_OUT_ADDRESS,Data);
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
PCF gpio interrupt
 *******************************/

void interrupt(){

    uint8_t pin_num = PCF_detect_low_pin();
    if(pin_num == 8) return;

    PCF_toggle(in2outRemap[pin_num]);
    DataChanged = true;

}


