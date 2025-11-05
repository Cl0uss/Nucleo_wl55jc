# ==========================
# CMake configuration
# ==========================
cmake_minimum_required(VERSION 3.20.0)

# Найти Zephyr SDK (важно!)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(final_project)

# ==========================
# Папки с исходниками
# ==========================
# Добавляем заголовки
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
)

# Добавляем исходники сенсоров и main.c
target_sources(app PRIVATE
    src/main.c
    sensors/accelerometerSensor.c
    sensors/brightnessSensor.c
    sensors/earthSensor.c
    sensors/gpsSensor.c
    sensors/rgbSensor.c
    sensors/temperatureSensor.c
)
