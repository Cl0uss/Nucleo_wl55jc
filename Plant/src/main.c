#include "connector.h"
#define nucleoLedB DT_ALIAS(led2)
#define nucleoLedG DT_ALIAS(led1)
#define nucleoLedR DT_ALIAS(led0)

bool buttonWasPressed = false;
const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0),gpios, {0});
struct gpio_callback button_cb_data;

const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios);
const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios);
const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(DT_ALIAS(ledb), gpios);

const struct device *i2c;
const struct device *uart;
const struct device *adc;
const struct device *port;

struct measDataQueue gpsMsg;

bool permission = true;
bool newStart = true;

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
// =================== NORMAL MODE ===================
struct {
    float minimumVal;
    float sumForMean;
    float meanVal;
    float maximumVal;
} tempNormalMode = { 0 };

struct {
    float minimumVal;
    float sumForMean;
    float meanVal;
    float maximumVal;
} humidityNormalMode = { 0 };

struct {
    float minimumVal;
    float sumForMean;
    float meanVal;
    float maximumVal;
} lightNormalMode = { 0 };

struct {
    float minimumVal;
    float sumForMean;
    float meanVal;
    float maximumVal;
} soilNormalMode = { 0 };

struct {
    float xMaximum;
    float xMinimum;
    float yMaximum;
    float yMinimum;
    float zMaximum;
    float zMinimum;
} accNormalMode = { 0 };

int rgbDominant[3] = {0};

// =================== NORMAL MODE FINISHED ===================


int16_t soilRawVal;
struct adc_channel_cfg soilCfg;
struct adc_sequence soilSeq;

int16_t brightnessRawVal;
struct adc_channel_cfg brightnessCfg;
struct adc_sequence brightnessSeq;


enum Mode {TEST, NORMAL, ADVANCED};
enum Mode mode = TEST;
int modeCount=1;

K_MSGQ_DEFINE(messageQueue, sizeof(struct measDataQueue), 8, 4);

bool measureTime = false;

int soilError;
int lightError;
uint8_t whoAmI;

void rgbLedInit() {

    gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_g, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_b, GPIO_OUTPUT_INACTIVE);
}

void i2cInit() {

    i2c = DEVICE_DT_GET(DT_NODELABEL(i2c2));
}

void uartInit() {

    uart = DEVICE_DT_GET(DT_NODELABEL(usart1));
}

void adcInit() {
    soilRawVal = 0;

    adc = DEVICE_DT_GET(DT_NODELABEL(adc1));


     soilCfg = (struct adc_channel_cfg){
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = soilAdcChannel,
        .differential     = 0,
    };

    soilError = adc_channel_setup(adc, &soilCfg);


    soilSeq = (struct adc_sequence){
    .channels    = BIT(soilAdcChannel),
    .buffer      = &soilRawVal,
    .buffer_size = sizeof(soilRawVal),
    .resolution  = adcRes,          
    };


    brightnessRawVal = 0;

    brightnessCfg = (struct adc_channel_cfg){
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = brightAdcChannel,
        .differential     = 0,
    };

    lightError = adc_channel_setup(adc, &brightnessCfg);


    brightnessSeq = (struct adc_sequence){
        .channels    = BIT(brightAdcChannel),
        .buffer      = &brightnessRawVal,
        .buffer_size = sizeof(brightnessRawVal),
        .resolution  = adcRes,
    };

}

uint8_t registers[2];
void registersInput(uint8_t first, uint8_t second) {
    
    registers[0] = first;
    registers[1] = second;
}

void rgbInit(){
    
    registersInput(0x80 | 0x00, 0x01);      // COMMAND BIT | ENABLE & PON
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
    k_msleep(3);

    registersInput(0x80 | 0x00, 0x3);       // COMMAND BIT | ENABLE & PON | AEN
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
    
    registersInput(0x80 | 0x01, 0xD5);      // COMMAND BIT | ATIME  & 101ms
    i2c_write(i2c,registers,sizeof(registers),rgbAddr);
}

void accelerometerInit(){
    
    registersInput(0x2A, 0x01);     //CTRL_REG1 & ACTIVE MODE
    i2c_write(i2c,registers,sizeof(registers),accAddr);

    registersInput(0x0E, 0x00);     //DYNAMIC RANGE & ±2g
    i2c_write(i2c,registers,sizeof(registers),accAddr);

}

void distanceInit() {

    // GPIOA from devicetree
    port = DEVICE_DT_GET(DT_NODELABEL(gpioa));
    if (!device_is_ready(port)) {
        printk("GPIOA not ready!\n");
        return;
    }

    // TRIG: PA5
    if (gpio_pin_configure(port, 5 /*PA5*/, GPIO_OUTPUT_INACTIVE)) {
        printk("Cannot configure TRIG\n");
        return;
    }

    // ECHO: PA6
    if (gpio_pin_configure(port, 6 /*PA6*/, GPIO_INPUT)) {
        printk("Cannot configure ECHO\n");
        return;
    }
}

void alertRaise() {
    return;
}

void manageData () {

    if (mode == TEST){
        if (red > green && red  >  blue)  rgbChange(1); //red
        if (green > red && green > blue)  rgbChange(2); //green
        if (blue > red  && blue  > green) rgbChange(3); //blue

    }

    else if (mode == NORMAL || mode == ADVANCED) {
        if (soilValue  < 0.0  || soilValue  > 100.0) alertRaise('s');
        if (lightValue < 0.0  || lightValue > 100.0) alertRaise('l');
        //if () for accelerometer
        if (tempValue < -10.0 || tempValue > 50.0) alertRaise('t');
        if (humValue  <  25.0 || humValue  > 75.0) alertRaise('h');

        if (newStart) {
            soilNormalMode.maximumVal = soilValue;
            soilNormalMode.minimumVal = soilValue;
            
            lightNormalMode.maximumVal = lightValue;
            lightNormalMode.minimumVal = lightValue;

            accNormalMode.xMaximum = axisX;
            accNormalMode.xMinimum = axisX;
            accNormalMode.yMaximum = axisY;
            accNormalMode.yMinimum = axisY;
            
            accNormalMode.zMaximum = axisZ;
            accNormalMode.zMinimum = axisZ;

            tempNormalMode.maximumVal = tempValue;
            tempNormalMode.minimumVal = tempValue;
            humidityNormalMode.maximumVal = humValue;
            humidityNormalMode.minimumVal = humValue;

        }
        else {
            if      (soilNormalMode.maximumVal < soilValue) soilNormalMode.maximumVal = soilValue;
            else if (soilNormalMode.minimumVal > soilValue) soilNormalMode.minimumVal = soilValue;
            
            if      (lightNormalMode.maximumVal < lightValue) lightNormalMode.maximumVal = lightValue;
            else if (lightNormalMode.minimumVal > lightValue) lightNormalMode.minimumVal = lightValue;

            if      (tempNormalMode.maximumVal < tempValue) tempNormalMode.maximumVal = tempValue;
            else if (tempNormalMode.minimumVal > tempValue) tempNormalMode.minimumVal = tempValue;

            if      (humidityNormalMode.maximumVal < humValue) humidityNormalMode.maximumVal = humValue;
            else if (humidityNormalMode.minimumVal > humValue) humidityNormalMode.minimumVal = humValue;

            if      (accNormalMode.xMaximum < axisX) accNormalMode.xMaximum = axisX;
            else if (accNormalMode.xMinimum > axisX) accNormalMode.xMinimum = axisX;

            if      (accNormalMode.yMaximum < axisY) accNormalMode.yMaximum = axisY;
            else if (accNormalMode.yMinimum > axisY) accNormalMode.yMinimum = axisY;

            if      (accNormalMode.zMaximum < axisZ) accNormalMode.zMaximum = axisZ;
            else if (accNormalMode.zMinimum > axisZ) accNormalMode.zMinimum = axisZ;
        }

        soilNormalMode.sumForMean     += soilValue;
        lightNormalMode.sumForMean    += lightValue;
        tempNormalMode.sumForMean     += tempValue;
        humidityNormalMode.sumForMean += humValue;

        if (red > green && red  >  blue)  rgbDominant[0] +=1; //red
        if (green > red && green > blue)  rgbDominant[1] +=1; //green
        if (blue > red  && blue  > green) rgbDominant[2] +=1; //blue

    }


        printk("SOIL MOISTURE: %.1f%%\n",soilValue);
        printk("Light: %.1f%%\n",lightValue);
        k_msgq_get(&messageQueue,&gpsMsg,K_NO_WAIT);
        printk("GPS: %s\n",gpsMsg.gpsQ);
        printk("COLOR SENSOR: Clear: %d Red: %d Green: %d Blue: %d\n", clear,red,green,blue);
        printk("ACCELEROMETERS: X_axis: %.2fm/s², Y_axis: %.2fm/s², Z_axis: %.2fm/s²\n",axisX,axisY,axisZ);
        printk("TEMP/HUM Temperature: %.1f°C,\tRelative Humidity: %.1f%%\n\n",tempValue,humValue);
        newStart = false;
}

K_THREAD_DEFINE(gpsThread,512,gpsMeasure,NULL,NULL,NULL,1,0,0);

void measures(){
    brightnessMeasure();

    rgbMeasure();

    accelerometerMeasure();

    temperatureMeasure();

    soilMeasure();

    while (permission) k_msleep(1);

    manageData();
    permission = true;          
}

void testSensors() {

    if (!device_is_ready(led_r.port) ||
    !device_is_ready(led_g.port) ||
    !device_is_ready(led_b.port)) {
    printk("LED ports not ready\n");
    //return;
    }

    if (!device_is_ready(i2c)) {
        printk("i2c not ready\n");
        //return;
    }

    if (!device_is_ready(uart)){
        printk("uart not ready\n");
        //return;
    }

    if (!device_is_ready(adc)) {
        printk("adc not ready\n");
        //return;
    }

    if (lightError < 0) {
        printk("adc_channel_setup brightness error: %d\n", lightError);
        //return;
    }

    if (soilError < 0) {
        printk("adc_channel_setup error: %d\n", soilError);
        //return;
    }

    i2c_write_read(i2c,rgbAddr,0x29 | 0x80,1,&whoAmI,1);
    if (whoAmI != 0) {
        printk("RGB sensor read  error %x\n",whoAmI);
        //return;
    }

    i2c_reg_read_byte(i2c, accAddr, 0x0D, &whoAmI);
    if (whoAmI != 0x1A) {
        printk("accelerometer sensor read error\n");
        return;
    }

    if (i2c_reg_read_byte(i2c, tempAndHumidAddr, 0xE7, &whoAmI) != 0) {
    printk("temp & hum read error\n");
    return;
}
}

void everyHourNormalMode (int quantity) {
    printk("\n\nHourly update:\n");
    // NM3
    tempNormalMode.meanVal = tempNormalMode.sumForMean / quantity;
    humidityNormalMode.meanVal = humidityNormalMode.sumForMean / quantity;
    lightNormalMode.meanVal = lightNormalMode.sumForMean / quantity;
    soilNormalMode.meanVal = soilNormalMode.sumForMean / quantity;

    printk("\ttemperature minimum - %.1f°C\n\ttemperature mean - %.1f°C\n\ttemperature maximum - %.1f°C\n\n",tempNormalMode.minimumVal,tempNormalMode.meanVal,tempNormalMode.maximumVal);
    printk("\thumidity minimum - %.1f%%\n\thumidity mean - %.1f%%\n\thumidity maximum - %.1f%%\n\n",humidityNormalMode.minimumVal,humidityNormalMode.meanVal,humidityNormalMode.maximumVal);
    printk("\tlight minimum - %.1f%%\n\tlight mean - %.1f%%\n\tlight maximum - %.1f%%\n\n",lightNormalMode.minimumVal,lightNormalMode.meanVal,lightNormalMode.maximumVal);
    printk("\tsoil minimum - %.1f%%\n\tsoil mean - %.1f%%\n\tsoil maximum - %.1f%%\n",soilNormalMode.minimumVal,soilNormalMode.meanVal,soilNormalMode.maximumVal);


    tempNormalMode.sumForMean = 0;
    humidityNormalMode.sumForMean = 0;
    lightNormalMode.sumForMean = 0;
    soilNormalMode.sumForMean = 0;

    // NM4
    if (rgbDominant[0] > rgbDominant[1] && rgbDominant[0] > rgbDominant[2]) printk("RED was dominant color\n");
    if (rgbDominant[1] > rgbDominant[0] && rgbDominant[1] > rgbDominant[2]) printk("GREEN was dominant color\n");
    if (rgbDominant[2] > rgbDominant[1] && rgbDominant[2] > rgbDominant[0]) printk("BLUE was dominant color\n");
    
    rgbDominant[0] = 0;
    rgbDominant[1] = 0;
    rgbDominant[2] = 0;

    //NM5
    printk("Minimum values of X_axis - %.2fm/s²   Y_axis - %.2fm/s²   Z_axis - %.2fm/s²\n",accNormalMode.xMinimum,accNormalMode.yMinimum,accNormalMode.zMinimum);
    printk("Maximum values of X_axis - %.2fm/s²   Y_axis - %.2fm/s²   Z_axis - %.2fm/s²\n\n",accNormalMode.xMaximum,accNormalMode.yMaximum,accNormalMode.zMaximum);
    newStart = true;
    
}

void buttonPressed() {
    buttonWasPressed = true;
    modeCount++;
    if (modeCount >= 4) modeCount = 1; 
    if (modeCount==1) mode = TEST;
    else if (modeCount==2) mode = NORMAL;
    else mode = ADVANCED;
}

void buttonInit() {
    gpio_pin_configure_dt(&button,GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb_data, buttonPressed, BIT(button.pin));
    gpio_add_callback(button.port,&button_cb_data);
}

void main(void) {
    rgbLedInit();
    i2cInit();
    uartInit();
    adcInit(); 
    rgbInit();
    accelerometerInit();
    //distanceInit();
    //distanceMeasure();
    buttonInit();

    static const struct gpio_dt_spec blueLed = GPIO_DT_SPEC_GET(nucleoLedB, gpios);
    gpio_pin_configure_dt(&blueLed, GPIO_OUTPUT_INACTIVE);

    static const struct gpio_dt_spec greenLed = GPIO_DT_SPEC_GET(nucleoLedG, gpios);
    gpio_pin_configure_dt(&greenLed, GPIO_OUTPUT_INACTIVE);

    static const struct gpio_dt_spec redLed = GPIO_DT_SPEC_GET(nucleoLedR, gpios);
    gpio_pin_configure_dt(&redLed, GPIO_OUTPUT_INACTIVE);
        
        while (true) {

            if (mode == TEST) {
                printk("\nTEST MODE\n");
                testSensors();
                gpio_pin_set_dt(&blueLed,1);
                gpio_pin_set_dt(&redLed,0);
                gpio_pin_set_dt(&greenLed,0);

                while (mode == TEST) {
                    measures();
                    for (int i = 0; i<2000; i++) {
                        if (buttonWasPressed) {
                            buttonWasPressed = false; 
                            break;
                        }
                        k_msleep(1);
                    }
                }
            }

            if (mode == NORMAL) {
                rgbChange(0);
                printk("\nNORMAL MODE\n");
                newStart = true;
                gpio_pin_set_dt(&blueLed,0);
                gpio_pin_set_dt(&redLed,0);
                gpio_pin_set_dt(&greenLed,1);

                int quantity = 0;
                int64_t startHourTimer = k_uptime_get();
                while (mode == NORMAL) {
                    measures();
                    quantity++;
                    if (k_uptime_get() - startHourTimer >= 15 * MSEC_PER_SEC) {
                        everyHourNormalMode(quantity);
                        quantity = 0;
                        startHourTimer = k_uptime_get();
                    } 
                    for (int i = 0; i<5000; i++) {
                        if (buttonWasPressed) {
                            buttonWasPressed = false; 
                            break;
                        }
                        k_msleep(1);
                    }
                }
            }

            if (mode == ADVANCED) {
                rgbChange(0);
                printk("\nADVANCED MODE\n");
                gpio_pin_set_dt(&blueLed,0);
                gpio_pin_set_dt(&redLed,1);
                gpio_pin_set_dt(&greenLed,0);

                int quantity = 0;
                int64_t startHourTimer = k_uptime_get();
                while (mode == ADVANCED) {
                    measures();
                    quantity++;
                    if (k_uptime_get() - startHourTimer >= 60 * MSEC_PER_SEC) {
                        everyHourNormalMode(quantity);
                        quantity = 0;
                        startHourTimer = k_uptime_get();
                    } 
                    for (int i = 0; i<10000; i++) {
                        if (buttonWasPressed) {
                            buttonWasPressed = false; 
                            break;
                        }
                        k_msleep(1);
                }
            }
        }
    }
}


/*TODO : figure out what limit values are for accelerometer then write them
write alertRaise function
make distance sensor transfer data to main file
*/