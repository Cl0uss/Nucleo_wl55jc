void accelerometerSensor(); //i2c2
void brightnessSensor();    //analog 
void earthSensor();         //analog
void rgbSensor();           //i2c2
void temperatureSensor();   //i2c2
void gpsSensor();           //UART



#define STACKSIZE 2048


enum Mode {TEST, NORMAL, ADVANCED};
extern enum Mode mode;
extern const struct device *i2c2;

