#include "../src/connector.h"

static char nmeaBuff[128];
static int nmeaPos = 0;
static uint8_t rxByte;

void gpsMeasure(){
    bool gpsReady = false;
    while(!gpsReady){
        while (uart_poll_in(uart, &rxByte) == 0) {

            if (rxByte == '\n') {
                nmeaBuff[nmeaPos] = '\0';
                printk("GPS: %s\n",nmeaBuff);
                gpsReady = true;
                nmeaPos=0;
            }
            else if (nmeaPos < sizeof(nmeaBuff) - 1) nmeaBuff[nmeaPos++] = rxByte;

        }
    }
}