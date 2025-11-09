#include "connector.h"
#include "rgbSensor.h"

void rgbSensor(void){
       uint16_t red_data, green_data, blue_data;
    uint8_t reg;
    uint8_t data[2];

    // Red
    reg = rgbCommandBit | rgbRdata;
    if (i2c_write_read(i2c, rgbAddr, &reg, 1, data, 2) != 0) {
        printk("i2c read error (Red)!\n");
    }
    red_data = ((uint16_t)data[1] << 8) | data[0];

    // Green
    reg = rgbCommandBit | rgbGdata;
    if (i2c_write_read(i2c, rgbAddr, &reg, 1, data, 2) != 0) {
        printk("i2c read error (Green)!\n");
    }
    green_data = ((uint16_t)data[1] << 8) | data[0];

    // Blue
    reg = rgbCommandBit | rgbBdata;
    if (i2c_write_read(i2c, rgbAddr, &reg, 1, data, 2) != 0) {
        printk("i2c read error (Blue)!\n");
    }
    blue_data = ((uint16_t)data[1] << 8) | data[0];

    printk("\r\nR: %5u  G: %5u  B: %5u\n",red_data, green_data, blue_data);

    k_msleep(2000);
}