set(SOURCE_FILES
    Dhcp.cpp  
    Dns.cpp  
    EthernetClient.cpp  
    Ethernet.cpp  
    EthernetServer.cpp  
    EthernetUdp.cpp  
    utility/w5100.cpp
    utility/socket.cpp
)
include_directories(./ ./utility ${ARDUINO_MEGA_ROOT}/hardware/arduino/avr/libraries/SPI/src) 

add_library(WIZ STATIC ${SOURCE_FILES})

