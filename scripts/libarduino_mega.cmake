# This file is based on the work of:
#
# http://mjo.tc/atelier/2009/02/arduino-cli.html
# http://johanneshoff.com/arduino-command-line.html
# http://www.arduino.cc/playground/Code/CmakeBuild
# http://www.tmpsantos.com.br/en/2010/12/arduino-uno-ubuntu-cmake/
# http://forum.arduino.cc/index.php?topic=244741.0


set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
enable_language(ASM)

set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/lib/mega")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/lib/mega")

# C only fine tunning
set(TUNNING_FLAGS "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums") 

set(CMAKE_CXX_FLAGS "-mmcu=${ARDUINO_MEGA_MCU} -DF_CPU=${ARDUINO_MEGA_FCPU} -Os")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${TUNNING_FLAGS} -Wall -Wstrict-prototypes -std=gnu99")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

set(ARDUINO_MEGA_LIB_DIR "${ARDUINO_MEGA_ROOT}/hardware/arduino/avr/libraries")
set(ARDUINO_MEGA_CORE_DIR "${ARDUINO_MEGA_ROOT}/hardware/arduino/avr/cores/arduino")
set(ARDUINO_MEGA_PINS_DIR "${ARDUINO_MEGA_ROOT}/hardware/arduino/avr/variants/${ARDUINO_MEGA_BOARD}")
set(ARDUINO_MEGA_AVR_DIR "${ARDUINO_MEGA_ROOT}/hardware/tools/avr/avr/include")
set(AVRDUDE_CONFIG "${ARDUINO_MEGA_ROOT}/hardware/tools/avr/etc/avrdude.conf")

include_directories(${ARDUINO_MEGA_PINS_DIR})
include_directories(${ARDUINO_MEGA_CORE_DIR})
include_directories(${ARDUINO_MEGA_AVR_DIR})
include_directories(${ARDUINO_MEGA_LIB_DIR}/SPI/src)

set(ARDUINO_MEGA_SOURCE_FILES
	${ARDUINO_MEGA_CORE_DIR}/wiring_pulse.S
	${ARDUINO_MEGA_CORE_DIR}/wiring_digital.c
	${ARDUINO_MEGA_CORE_DIR}/wiring.c
	${ARDUINO_MEGA_CORE_DIR}/WInterrupts.c
	${ARDUINO_MEGA_CORE_DIR}/wiring_pulse.c
	${ARDUINO_MEGA_CORE_DIR}/wiring_shift.c
	${ARDUINO_MEGA_CORE_DIR}/hooks.c 
	${ARDUINO_MEGA_CORE_DIR}/wiring_analog.c
	${ARDUINO_MEGA_CORE_DIR}/WMath.cpp
	${ARDUINO_MEGA_CORE_DIR}/IPAddress.cpp
	${ARDUINO_MEGA_CORE_DIR}/Tone.cpp
	${ARDUINO_MEGA_CORE_DIR}/HardwareSerial2.cpp
	${ARDUINO_MEGA_CORE_DIR}/Print.cpp
	${ARDUINO_MEGA_CORE_DIR}/new.cpp
	${ARDUINO_MEGA_CORE_DIR}/HardwareSerial0.cpp
	${ARDUINO_MEGA_CORE_DIR}/HardwareSerial.cpp
	${ARDUINO_MEGA_CORE_DIR}/WString.cpp
	${ARDUINO_MEGA_CORE_DIR}/abi.cpp
	${ARDUINO_MEGA_CORE_DIR}/USBCore.cpp
	${ARDUINO_MEGA_CORE_DIR}/Stream.cpp
	${ARDUINO_MEGA_CORE_DIR}/CDC.cpp
	${ARDUINO_MEGA_LIB_DIR}/SPI/src/SPI.cpp
)

add_library(core STATIC  ${ARDUINO_MEGA_SOURCE_FILES})

set(PORT $ENV{ARDUINO_MEGA_PORT})
if (NOT PORT)
    set(PORT ${ARDUINO_MEGA_PORT})
endif()

macro(arduino TARGET_NAME TARGET_SOURCE_FILES TARGET_LIBS)

  add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})
  target_link_libraries (${TARGET_NAME} ${TARGET_LIBS})
  
  add_custom_target(${TARGET_NAME}.elf )
  add_dependencies(${TARGET_NAME}.elf core ${TARGET_NAME})

  add_custom_command(TARGET ${TARGET_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_CXX_COMPILER}  -w -Os -Wl,--gc-sections -mmcu=${ARDUINO_MEGA_MCU} -o ${LIBRARY_OUTPUT_PATH}/${TARGET_NAME}.elf 
             -L${LIBRARY_OUTPUT_PATH} -Wl,--start-group -l${TARGET_NAME} -lm -lcore -l${TARGET_LIBS} -Wl,--end-group
  )
  

  add_custom_target(${TARGET_NAME}.hex)
  add_dependencies(${TARGET_NAME}.hex ${TARGET_NAME}.elf)

  add_custom_command(TARGET ${TARGET_NAME}.hex POST_BUILD
      COMMAND ${ARDUINO_MEGA_OBJCOPY} -O ihex -R .eeprom ${LIBRARY_OUTPUT_PATH}/${TARGET_NAME}.elf ${LIBRARY_OUTPUT_PATH}/${TARGET_NAME}.hex
  )
  
  add_custom_target(${TARGET_NAME}.upload)
  add_dependencies(${TARGET_NAME}.upload ${TARGET_NAME}.hex)

  add_custom_command(TARGET ${TARGET_NAME}.upload POST_BUILD
      COMMAND ${ARDUINO_MEGA_AVRDUDE} -C${AVRDUDE_CONFIG} -v -p${ARDUINO_MEGA_MCU} -c${ARDUINO_MEGA_PROTOCOL}  -P${PORT} -b${ARDUINO_MEGA_UPLOAD_SPEED} -D -Uflash:w:${LIBRARY_OUTPUT_PATH}/${TARGET_NAME}.hex:i
  )
endmacro()
