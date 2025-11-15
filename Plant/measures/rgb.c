#include "../src/connector.h"

uint8_t redAddr   = 0x80 | 0x16;
uint8_t greenAddr = 0x80 | 0x18;
uint8_t blueAddr  = 0x80 | 0x1A;

uint8_t rgbData[2];

uint16_t red;
uint16_t green;
uint16_t blue;

void rgbMeasure () {
        while (true){
        i2c_write_read(i2c,rgbAddr,&redAddr,1,&rgbData,2);
        red = ((uint16_t)rgbData[1] << 8) | rgbData[0];

        i2c_write_read(i2c,rgbAddr,&greenAddr,1,&rgbData,2);
        green = ((uint16_t)rgbData[1] << 8) | rgbData[0];

        i2c_write_read(i2c,rgbAddr,&blueAddr,1,&rgbData,2);
        blue = ((uint16_t)rgbData[1] << 8) | rgbData[0];

        printk("\r\033[Kred - %u\tgreen - %u\tblue - %u", red,green,blue);
        k_msleep(500);
}

}


/*  --- to read all colors + clear ---
i2c_write_read(i2c, rgbAddr, &reg, 1, buff, 8);

uint16_t clear = (buff[1] << 8) | buff[0];
uint16_t red   = (buff[3] << 8) | buff[2];
uint16_t green = (buff[5] << 8) | buff[4];
uint16_t blue  = (buff[7] << 8) | buff[6];
*/