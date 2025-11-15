#include "../src/connector.h"

uint8_t xAddr = 0x01;
uint8_t yAddr = 0x03;
uint8_t zAddr = 0x05;

uint8_t accData[2];

int16_t x;
int16_t y;
int16_t z;

void accelerometerMeasure() {
    while (true){
        i2c_write_read(i2c,accAddr,&xAddr,1,&accData,2);
        x = (int16_t) ((accData[0] << 8) | accData[1]) >> 2;   

        i2c_write_read(i2c,accAddr,&yAddr,1,&accData,2);
        y = (int16_t) ((accData[0] << 8) | accData[1]) >> 2;   
        
        i2c_write_read(i2c,accAddr,&zAddr,1,&accData,2);
        z = (int16_t) ((accData[0] << 8) | accData[1]) >> 2;   
        

        float xf = (float)x * 9.80665f / 4096.0f;
        float yf = (float)y * 9.80665f / 4096.0f;
        float zf = (float)z * 9.80665f / 4096.0f;

           printk("\r\033[KX axis: %.2f m/s², Y axis: %.2f m/s², Z axis: %.2f m/s²",xf, yf, zf);
        k_msleep(500);
    }
}