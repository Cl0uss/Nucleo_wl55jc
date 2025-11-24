#include "../src/connector.h"

static char nmeaBuff[128];
static int nmeaPos = 0;
static uint8_t rxByte;

void gpsMeasure(void) {
    here:
    bool gpsReady = false;
    while (!gpsReady && permission) {
        while (uart_poll_in(uart, &rxByte) == 0) {
            if (rxByte == '\r') continue;
            else if (rxByte == '\n') {
                nmeaBuff[nmeaPos] = '\0';

                struct measDataQueue msg = {0};
                msg.type = gpsDataQ;
                size_t len = nmeaPos < sizeof(msg.d.gpsQ) - 1 ? nmeaPos : sizeof(msg.d.gpsQ) - 1;
                memcpy(msg.d.gpsQ, nmeaBuff, len);
                msg.d.gpsQ[len] = '\0';

                int err = k_msgq_put(&messageQueue, &msg, K_NO_WAIT);
                gpsReady = true;
                permission = false;
                nmeaPos = 0;    
            } 
            else if (nmeaPos < sizeof(nmeaBuff) - 1) nmeaBuff[nmeaPos++] = rxByte;
            
        }
    }
    goto here;
}
