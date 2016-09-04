#include <assert.h>

#include <Ethernet.h>
#include <socket.h>

#include "opsprot.h"


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

static int16_t blocking_recv(SOCKET s, uint8_t * buf, int16_t len) {
    uint8_t status;
    uint16_t avail;
    status = W5100.readSnSR(s);
    while (status != SnSR::ESTABLISHED) {
        delay(10);
        status = W5100.readSnSR(s);
    }
    print ("status = 0x%X\n\r", status);
    avail = W5100.getRXReceivedSize(s);
    while (avail == 0) {
        delay(10);
        avail = W5100.getRXReceivedSize(s);
    }
    print ("avail = 0x%X\n\r", avail);
    return recv(s, buf, len);
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
    
    SOCKET sock = 1;   //TODO not clear how to manage these. Now that DHCP is done we should have all 4 available?
    uint16_t port = 555;
    if (socket(sock, SnMR::TCP, port, 0) == 1) {
        if (listen(sock) == 1) {
            print ("listening on %d...\n\r", port);
            while (1) {
                OPSWriteLinkCommand cmd;
                assert (OPSWriteLinkCommandBasicSize == sizeof(cmd));
                uint16_t received;
                uint8_t buf[OPSWriteLinkCommandBasicSize];
                received = blocking_recv(sock, buf, sizeof(buf));
                if (received >= 0) {
                    print ("received = %d\n\r", received);
                } else {
                    print ("*E* failed recv()\n\r");
                    break;
                }
            }
        } else {
            print ("*E* failed listen()\n\r");
        }
    } else {
        print ("*E* failed socket()\n\r");
    }
}

