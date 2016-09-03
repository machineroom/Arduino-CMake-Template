#include <Dhcp.h>
#include <Ethernet.h>

#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void print(const char* format, ...) { 
    char tmp[128];
    va_list args;
    va_start (args, format);
    vsnprintf (tmp, sizeof(tmp), format, args);
    Serial.print(tmp);
    Serial.flush();
    va_end (args);
}

int main (int argc, char**argv) {
    int rc;
    uint8_t mac[] = { 0x01, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    init();
    Serial.begin(9600);
    print("main() enter\n\r");
    EthernetClass eth;
    IPAddress ip_adr; 
    eth.begin(mac);
    ip_adr = eth.localIP();
    print("DHCP IP = %d.%d.%d.%d\r\n", ip_adr[0], ip_adr[1], ip_adr[2], ip_adr[3]);
}

