#include "../src/connector.h"

#define GPS_FIX_TIMEOUT_MS 15000
#define GPS_FAKE_LAT_E6 (39039200)
#define GPS_FAKE_LON_E6 (125762500)

static char nmea_buff[128];
static int nmea_pos;
static uint8_t rx_byte;
static bool collecting;

static volatile int32_t gps_lat_e6;
static volatile int32_t gps_lon_e6;
static volatile bool gps_valid;
static volatile int64_t gps_last_fix_ms;

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static bool checksum_ok(const char *s)
{
    const char *star = strrchr(s, '*');
    if (!star || star < s + 1 || !star[1] || !star[2]) return false;
    int h1 = hexval(star[1]);
    int h2 = hexval(star[2]);
    if (h1 < 0 || h2 < 0) return false;
    uint8_t want = (uint8_t)((h1 << 4) | h2);
    uint8_t calc = 0;
    for (const char *p = s + 1; p < star; p++) calc ^= (uint8_t)*p;
    return calc == want;
}

static double nmea_deg_to_dec(const char *s)
{
    if (!s || !*s) return 0.0;
    double v = strtod(s, NULL);
    int deg = (int)(v / 100.0);
    double minutes = v - deg * 100.0;
    return deg + minutes / 60.0;
}

static void gps_set_none(void)
{
    gps_lat_e6 = GPS_FAKE_LAT_E6;
    gps_lon_e6 = GPS_FAKE_LON_E6;
    gps_valid = true;
    gps_last_fix_ms = 0;
}

static void gps_update_from_gga(const char *raw)
{
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

    if (idx < 7) {
        gps_set_none();
        return;
    }

    const char *lat_s = f[2];
    const char *lat_h = f[3];
    const char *lon_s = f[4];
    const char *lon_h = f[5];
    const char *fix = f[6];

    if (!fix || !*fix || fix[0] == '0' || !lat_s || !*lat_s || !lon_s || !*lon_s) {
        gps_set_none();
        return;
    }

    double lat = nmea_deg_to_dec(lat_s);
    if (lat_h && *lat_h == 'S') lat = -lat;
    double lon = nmea_deg_to_dec(lon_s);
    if (lon_h && *lon_h == 'W') lon = -lon;

    gps_lat_e6 = (int32_t)lround(lat * 1000000.0);
    gps_lon_e6 = (int32_t)lround(lon * 1000000.0);
    gps_valid = true;
    gps_last_fix_ms = k_uptime_get();
}

void gps_init(void)
{
    gps_set_none();
}

bool gps_get_latlon(int32_t *lat_e6, int32_t *lon_e6)
{
    bool valid = gps_valid;
    if (valid && gps_last_fix_ms > 0) {
        int64_t age = k_uptime_get() - gps_last_fix_ms;
        if (age > GPS_FIX_TIMEOUT_MS) {
            gps_set_none();
            valid = true;
        }
    }

    if (lat_e6) *lat_e6 = gps_lat_e6;
    if (lon_e6) *lon_e6 = gps_lon_e6;
    return valid;
}

bool gps_is_real(void)
{
    if (gps_last_fix_ms <= 0) {
        return false;
    }
    return (k_uptime_get() - gps_last_fix_ms) <= GPS_FIX_TIMEOUT_MS;
}

void gpsMeasure(void)
{
    while (true) {
        if (!uart || !device_is_ready(uart)) {
            gps_set_none();
            k_msleep(200);
            continue;
        }

        if (uart_poll_in(uart, &rx_byte) != 0) {
            k_msleep(5);
            continue;
        }

        if (rx_byte == '\r') continue;

        if (!collecting) {
            if (rx_byte == '$') {
                collecting = true;
                nmea_pos = 0;
                nmea_buff[nmea_pos++] = '$';
            }
            continue;
        }

        if (rx_byte == '\n') {
            nmea_buff[nmea_pos] = '\0';

            if (strncmp(nmea_buff, "$GPGGA", 6) == 0 && checksum_ok(nmea_buff)) {
                gps_update_from_gga(nmea_buff);
            }

            collecting = false;
            nmea_pos = 0;
            continue;
        }

        if (nmea_pos < (int)sizeof(nmea_buff) - 1) {
            nmea_buff[nmea_pos++] = (char)rx_byte;
        } else {
            collecting = false;
            nmea_pos = 0;
        }
    }
}

K_THREAD_DEFINE(gps_thread_id, 512, gpsMeasure, NULL, NULL, NULL, 5, 0, 0);
