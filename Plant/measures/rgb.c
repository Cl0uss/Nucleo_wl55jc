#include "../src/connector.h"

uint8_t clearAddr = 0x80 | 0x14;
uint8_t redAddr   = 0x80 | 0x16;
uint8_t greenAddr = 0x80 | 0x18;
uint8_t blueAddr  = 0x80 | 0x1A;

uint8_t rgbData[2];

void rgbMeasure () {
        i2c_write_read(i2c,rgbAddr,&clearAddr,1,&rgbData,2);
        clear = ((uint16_t)rgbData[1] << 8) | rgbData[0];
        
        i2c_write_read(i2c,rgbAddr,&redAddr,1,&rgbData,2);
        red   = ((uint16_t)rgbData[1] << 8) | rgbData[0];

        i2c_write_read(i2c,rgbAddr,&greenAddr,1,&rgbData,2);
        green = ((uint16_t)rgbData[1] << 8) | rgbData[0];

        i2c_write_read(i2c,rgbAddr,&blueAddr,1,&rgbData,2);
        blue  = ((uint16_t)rgbData[1] << 8) | rgbData[0];

}
