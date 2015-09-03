set(ARDUINO_PROTOCOL "arduino")
set(ARDUINO_BOARD "standard")
set(ARDUINO_MCU "atmega328p")
set(ARDUINO_FCPU "16000000")
set(ARDUINO_UPLOAD_SPEED "115200")
set(ARDUINO_PORT "/dev/ttyACM1")
set(ARDUINO_ROOT "$ENV{HOME}/opt/arduino")
include(${CMAKE_SOURCE_DIR}/scripts/libarduino_avr.cmake)