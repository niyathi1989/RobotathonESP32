set(srcs
    "main.c"
    "arduino_main.cpp"
    "motors.cpp"
    "color_sensor.cpp"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES bluepad32 bluepad32_arduino arduino btstack nvs_flash callbacks)