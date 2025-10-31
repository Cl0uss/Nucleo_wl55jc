#include "connector.h"
#define ADDR          0x29
#define CDATA         0x14
#define COMMAND_BIT   0x80


// === Register in TCS34725 definitions ===
void setRGB(uint16_t current, uint16_t red, uint16_t yellow){
    if (current <= red)                             rgb_set(&led,1,0,0);
    else if (current > red && current <= yellow)    rgb_set(&led,1,1,0);
    else                                            rgb_set(&led,0,1,0);
}


void sensor() {
    uint16_t current_data;
    uint16_t saved_data;
    uint16_t red_number;
    uint16_t yellow_number;
    while (true){
        uint8_t reg = COMMAND_BIT | CDATA;
        uint8_t data[2];
        if ( (i2c_write_read(i2c,ADDR,&reg,1,data,2)) != 0) {
            printk("i2c read error!\n");
        }
        current_data = ((uint16_t)data[1]<<8) | data[0];
        if (!saved){
            saved_data=current_data;
            red_number = saved_data * 0.33f;
            yellow_number = saved_data * 0.66f;
            saved=true;
        }
        setRGB(current_data,red_number,yellow_number);
        printk("\r\033[KSaved Clear: %u     Current: %u     %u - %u", saved_data, current_data, red_number, yellow_number);
        k_msleep(2000);
    }
}