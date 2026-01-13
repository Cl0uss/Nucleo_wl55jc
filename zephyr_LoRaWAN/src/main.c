#include "connector.h"

// ===================   COMMUNICATION   ===================

const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c2));;
const struct device *uart;
const struct device *adc;
const struct device *port;
struct measDataQueue gpsMsg;

// =================== COMMUNICATION END ===================

    // ===================   SENSOR VALUES   ===================

float axisX;
float axisY;
float axisZ;
float lightValue;
uint16_t clear;
uint16_t red;
uint16_t blue;
uint16_t green;
float soilValue;
float tempValue;
float humValue;
int16_t soilRawVal;
int16_t brightnessRawVal;
uint32_t distanceVal;

    // =================== SENSOR VALUES END ===================

const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios);
const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios);

	/* Customize based on network configuration */
#define LORAWAN_DEV_EUI			{ 0x79, 0x39, 0x32, 0x35, 0x59, 0x37, 0x91, 0x94 } // Use your own DEV_EUI
#define LORAWAN_JOIN_EUI		{ 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0xFC, 0x4D }
#define LORAWAN_APP_KEY			{ 0xf3, 0x1c, 0x2e, 0x8b, 0xc6, 0x71, 0x28, 0x1d, 0x51, 0x16, 0xf0, 0x8f, 0xf0, 0xb7, 0x92, 0x8f }

#define DELAY K_MSEC(10000)  /* 30 seconds */
#define MAX_PAYLOAD_SIZE   30
#define NUM_MAX_RETRIES    30

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(lorawan_class_a);

static void dl_callback(uint8_t port, uint8_t flags, int16_t rssi, int8_t snr,
                        uint8_t len, const uint8_t *hex_data)
{
    LOG_INF("DL port=%u, len=%u", port, len);

    if (!hex_data || len == 0) {
        return;
    }

    LOG_HEXDUMP_INF(hex_data, len, "DL payload:");

    /* Convert to C-string (uppercased), safe */
    char cmd[16];
    size_t n = (len < (sizeof(cmd) - 1)) ? len : (sizeof(cmd) - 1);

    for (size_t i = 0; i < n; i++) {
        char c = (char)hex_data[i];
        cmd[i] = (char)toupper((unsigned char)c);
    }
    cmd[n] = '\0';

    /* Trim possible trailing \r \n \0 spaces */
    while (n > 0 && (cmd[n-1] == '\r' || cmd[n-1] == '\n' || cmd[n-1] == ' ' || cmd[n-1] == '\0')) {
        cmd[n-1] = '\0';
        n--;
    }

    if (strcmp(cmd, "OFF") == 0) {
        rgbChange(0);
    } else if (strcmp(cmd, "RED") == 0) {
        rgbChange(1);
    } else if (strcmp(cmd, "GREEN") == 0) {
        rgbChange(2);
    } else {
        LOG_WRN("Unknown cmd: '%s'", cmd);
    }
}


static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
	uint8_t unused, max_size;

	lorawan_get_payload_sizes(&unused, &max_size);
	LOG_INF("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}


static inline void put_le16(uint8_t *buf, int idx, uint16_t v)
{
    buf[idx]     = (uint8_t)(v & 0xFF);
    buf[idx + 1] = (uint8_t)(v >> 8);
}

int main(void)
{

	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	uint8_t dev_eui[] = LORAWAN_DEV_EUI;
	uint8_t join_eui[] = LORAWAN_JOIN_EUI;
	uint8_t app_key[] = LORAWAN_APP_KEY;
	int ret;
	uint8_t joining_retries = 0 ;
	
	struct lorawan_downlink_cb downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = dl_callback
	};

	lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s: device not ready.", lora_dev->name);
		return 0;
	}

	if (!gpio_is_ready_dt(&led_r) || !gpio_is_ready_dt(&led_g)) {
		LOG_ERR("LED GPIOs not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("LED R init failed: %d", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&led_g, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("LED G init failed: %d", ret);
		return 0;
	}

#if defined(CONFIG_LORAMAC_REGION_EU868)

	ret = lorawan_set_region(LORAWAN_REGION_EU868);
	if (ret < 0) {
		LOG_ERR("lorawan_set_region failed: %d", ret);
		return 0;
	}
#endif

	ret = lorawan_start();
	if (ret < 0) {
		LOG_ERR("lorawan_start failed: %d", ret);
		return 0;
	}
	
	lorawan_enable_adr(false);

	lorawan_register_downlink_callback(&downlink_cb);
	lorawan_register_dr_changed_callback(lorwan_datarate_changed);

	join_cfg.mode = LORAWAN_ACT_OTAA;
	join_cfg.dev_eui = dev_eui;
	join_cfg.otaa.join_eui = join_eui;
	join_cfg.otaa.app_key = app_key;
	join_cfg.otaa.nwk_key = app_key;
	
	printf("\r\n DEV_EUI: ");
    for (int i = 0; i < sizeof(dev_eui); ++i) printf("%02x", dev_eui[i]);
    printf("\r\n APP_EUI: ");
    for (int i = 0; i < sizeof(join_eui); ++i) printf("%02x", join_eui[i]);
    printf("\r\n APP_KEY: ");
    for (int i = 0; i < sizeof(app_key); ++i) printf("%02x", app_key[i]);
    printf("\r\n");

	LOG_INF("Waiting 10 seconds to join...");
	k_sleep(K_SECONDS(10));  // Wait 30 seconds


	LOG_INF("Joining network over OTAA");

	
	while ((ret = lorawan_join(&join_cfg)) < 0) {
		joining_retries++;
		if (joining_retries > NUM_MAX_RETRIES){
			LOG_ERR("Number max of retries (%d/%d) reached, finishing program", joining_retries, NUM_MAX_RETRIES);
			return 0;
		}
		LOG_ERR("Join failed (%d), retrying in 30 seconds, try #%d...", ret, joining_retries);
		k_sleep(K_SECONDS(30));
	}
		
	LOG_INF("Starting to send data...");
	
	static uint16_t counter = 0;		
	uint8_t data[MAX_PAYLOAD_SIZE];
	int16_t temp = 0;
	int16_t hum = 0;
	uint8_t len = 0;
	while (1) {
		temp = 0;
		hum = 0;
		len = 0;
		temperatureMeasure();
		temp = (int16_t)lroundf(tempValue * 10.0f);
		hum = (int16_t)lroundf(humValue * 10.0f);
		put_le16(data, len, (uint16_t)counter); len +=2;
		put_le16(data, len, (uint16_t)temp); len += 2;
		put_le16(data, len, (uint16_t)hum); len += 2;
		ret = lorawan_send(1, data, len, LORAWAN_MSG_UNCONFIRMED);

		if (ret == -EAGAIN) {
			LOG_ERR("lorawan_send failed: %d. Continuing...", ret);
			k_sleep(DELAY);
			continue;
		}

		if (ret < 0) {
			LOG_ERR("lorawan_send failed: %d", ret);
			return 0;
		}

		LOG_INF("Data sent!! (data counting #%04d)", counter);
		counter = (counter < 255) ? counter + 1 : 0;
		printk("temp - %.1f%%, hum - %.1f%%\n",tempValue,humValue);
		k_sleep(DELAY);

	}
}
