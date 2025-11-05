void accelerometerSensor(); //i2c2
void brightnessSensor();    //analog 
void earthSensor();         //analog
void rgbSensor();           //i2c3
void temperatureSensor();   //i2c2
void gpsSensor();           //UART

enum Mode {TEST, NORMAL, ADVANCED};
extern enum Mode mode;
extern const struct device *i2c2;
extern const struct device *i2c3;
extern const struct device *uart;

