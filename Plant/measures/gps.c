#include "../src/connector.h"

static char nmeaBuff[128];
static int nmeaPos = 0;
static uint8_t rxByte;
static bool collecting = false;

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static bool checksum_ok(const char *s) {
    const char *star = strrchr(s, '*');
    if (!star || star < s + 1 || !star[1] || !star[2]) return false;
    int h1 = hexval(star[1]), h2 = hexval(star[2]);
    if (h1 < 0 || h2 < 0) return false;
    uint8_t want = (uint8_t)((h1 << 4) | h2);
    uint8_t calc = 0;
    for (const char *p = s + 1; p < star; p++) calc ^= (uint8_t)*p;
    return calc == want;
}

void gpsMeasure(void) {
    while (true) {
        while (permission) {
            while (uart_poll_in(uart, &rxByte) == 0) {
                if (rxByte == '\r') continue;

                if (!collecting) {
                    if (rxByte == '$') {
                        collecting = true;
                        nmeaPos = 0;
                        nmeaBuff[nmeaPos++] = '$';
                    }
                    continue;
                }

                if (rxByte == '\n') {
                    nmeaBuff[nmeaPos] = '\0';

                    bool ok = strncmp(nmeaBuff, "$GPGGA", 6) == 0 && checksum_ok(nmeaBuff);
                    if (ok) {
                        struct measDataQueue msg = {0};
                        size_t len = nmeaPos < sizeof(msg.gpsQ) - 1 ? nmeaPos : sizeof(msg.gpsQ) - 1;
                        memcpy(msg.gpsQ, nmeaBuff, len);
                        msg.gpsQ[len] = '\0';
                        k_msgq_put(&messageQueue, &msg, K_NO_WAIT);
                        permission = false;
                    }

                    collecting = false;
                    nmeaPos = 0;
                    continue;
                }

                if (nmeaPos < (int)sizeof(nmeaBuff) - 1) {
                    nmeaBuff[nmeaPos++] = rxByte;
                } else {
                    collecting = false;
                    nmeaPos = 0;
                }
            }
        }
    }
}
