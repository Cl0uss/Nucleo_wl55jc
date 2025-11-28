#include "connector.h"
#include <stdlib.h>
#include <string.h>

static double nmea_deg_to_dec(const char *s) {
    if (!s || !*s) return 0.0;
    double v = strtod(s, NULL);
    int deg = (int)(v / 100.0);
    double minutes = v - deg * 100.0;
    return deg + minutes / 60.0;
}

void gpsToHuman(int mode) {
    const char *raw = gpsMsg.gpsQ;

    if (!raw[0]) {
        printk("GPS: NO SATELLITE CONNECTION\n");
        return;
    }

    if (strncmp(raw, "$GPGGA", 6) != 0) {
        printk("GPS raw: %s\n", raw);
        return;
    }

    char buf[128];
    strncpy(buf, raw, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    char *f[15] = {0};
    int idx = 0;
    char *copy = buf;
    char *token;
    while (idx < 15 && (token = strsep(&copy, ",")) != NULL) {
        f[idx++] = token;
    }

    if (idx < 10) {
        printk("GPS: incomplete data (%s)\n", raw);
        return;
    }

    const char *utc   = f[1];
    const char *lat_s = f[2];
    const char *lat_h = f[3];
    const char *lon_s = f[4];
    const char *lon_h = f[5];
    const char *fix   = f[6];
    const char *sats  = f[7];
    const char *alt   = f[9];

    int sats_num = (sats && *sats) ? atoi(sats) : 0;
    bool has_fix = (fix && *fix && fix[0] != '0');
    if (!has_fix || sats_num == 0) {
        printk("GPS: NO SATELLITE CONNECTION\n");
        return;
    }

    double lat = nmea_deg_to_dec(lat_s);
    if (lat_h && *lat_h == 'S') lat = -lat;
    double lon = nmea_deg_to_dec(lon_s);
    if (lon_h && *lon_h == 'W') lon = -lon;

    char utc_time[9] = "??:??:??";
    char shifted_time[9] = "??:??:??";
    const char *time_to_use = utc_time;

    if (utc && strlen(utc) >= 6 &&
        isdigit((unsigned char)utc[0]) && isdigit((unsigned char)utc[1]) &&
        isdigit((unsigned char)utc[2]) && isdigit((unsigned char)utc[3]) &&
        isdigit((unsigned char)utc[4]) && isdigit((unsigned char)utc[5])) {

        int hh = (utc[0] - '0') * 10 + (utc[1] - '0');
        int mm = (utc[2] - '0') * 10 + (utc[3] - '0');
        int ss = (utc[4] - '0') * 10 + (utc[5] - '0');

        snprintf(utc_time, sizeof(utc_time), "%02d:%02d:%02d", hh, mm, ss);

        if (mode == 1) {
            time_to_use = utc_time;
        } else {
            hh = (hh + 1) % 24;
            snprintf(shifted_time, sizeof(shifted_time), "%02d:%02d:%02d", hh, mm, ss);
            time_to_use = shifted_time;
        }
    }
    if (mode == 1)    printk("GPS: #Sats: %s Lat(UTC): %.6f %c Long(UTC): %.6f %c Altitude: %s m GPS time: %s\n",
                        (sats && *sats) ? sats : "?",
                        fabs(lat), lat >= 0 ? 'N' : 'S',
                        fabs(lon), lon >= 0 ? 'E' : 'W',
                        (alt && *alt) ? alt : "?",
                        time_to_use);

    else                printk("GPS: #Sats: %s Lat(UTC): %.6f %c Long(UTC): %.6f %c Altitude: %s m GPS time to Local: %s\n",
                        (sats && *sats) ? sats : "?",
                        fabs(lat), lat >= 0 ? 'N' : 'S',
                        fabs(lon), lon >= 0 ? 'E' : 'W',
                        (alt && *alt) ? alt : "?",
                        time_to_use);

}
