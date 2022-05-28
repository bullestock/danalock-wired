idf_component_register(SRCS console.cpp encoder.cpp led.cpp main.cpp motor.cpp switches.cpp
                    INCLUDE_DIRS "$ENV{IDF_PATH/components/esp8266/include}")

OPTION(SIMULATE "Simulate" OFF)
IF(SIMULATE)
    ADD_DEFINITIONS(-DSIMULATE)
ENDIF()

# This breaks ESP-IDF as of 4.3
#component_compile_options(-std=c++17)

