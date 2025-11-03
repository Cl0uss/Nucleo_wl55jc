cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(finalProject)

target_sources(app PRIVATE 
        src/main.c
        sensors/accelerometerSensor.c
        sensors/brightnessSensor.c
        sensors/earthSensor.c
        sensors/gpsSensor.c
        sensors/rgbSensor.c
        sensors/temperatureSensor.c
        
        )

zephyr_include_directories(headers)