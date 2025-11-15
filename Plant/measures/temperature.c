#include "../src/connector.h"

uint8_t humidAddr = 0xE5;
uint8_t tempAddr  = 0xE3;


uint8_t tmphmdData[2];

uint16_t humidityRaw;
uint16_t temperatureRaw;
float humidity;
float temperature;

void temperatureMeasure(){
    while (true) {
        i2c_write_read(i2c,tempAndHumidAddr,&humidAddr,1,&tmphmdData,2);
        humidityRaw = ((uint16_t)tmphmdData[0] << 8) | tmphmdData[1];

        i2c_write_read(i2c,tempAndHumidAddr,&tempAddr,1,&tmphmdData,2);
        temperatureRaw = ((uint16_t)tmphmdData[0] << 8) | tmphmdData[1];

        temperature = (175.72f * (float)temperatureRaw) / 65536.0f - 46.85f;
        humidity = (125.0f * (float)humidityRaw) / 65536.0f - 6.0f; 

         printk("\r\033[KTemp: %.2f°C, Humidity: %.2f%%", temperature, humidity);
         k_msleep(500);
    }
    
}