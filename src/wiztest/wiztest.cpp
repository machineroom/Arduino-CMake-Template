#include <Dhcp.h>

#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void usart0_init(void) {
	UBRR0 = UBRR_VALUE;
	UCSR0B = (1<<TXEN0 | 1<<RXEN0); 	// tx/rx enable
}

void usart0_write(char *msg) {
    int l = strlen(msg);
    for (int i=0; i < l; i++) {
    	while ((UCSR0A & (1 << UDRE0)) == 0) {}; // Do nothing until UDR is ready for more data to be written to it
	    UDR0 = msg[i]; // Echo back the received byte back to the computer    
    }
}

void prints (char *s) {
    usart0_write(s);
}

void printsb (char *s, uint8_t b) {
    usart0_write(s);
    char tmp[8];
    snprintf (tmp, sizeof(tmp), "0x%X", b);
    usart0_write(tmp);
}

void prints4b (char *s, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
    usart0_write(s);
    char tmp[20];
    snprintf (tmp, sizeof(tmp), "0x%X 0x%X 0x%X 0x%X", b1, b2, b3, b4);
    usart0_write(tmp);
}

int main (int argc, char**argv) {
    DhcpClass dhcp;
    int rc;
    uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    usart0_init();
    IPAddress ip_adr; 
    ip_adr = dhcp.getLocalIp();
    prints4b("before DHCP IP = ", ip_adr[0], ip_adr[1], ip_adr[2], ip_adr[3]);
    prints ("\r\n");
    rc = dhcp.beginWithDHCP(mac);
    ip_adr = dhcp.getLocalIp();
    prints4b("after DHCP IP = ", ip_adr[0], ip_adr[1], ip_adr[2], ip_adr[3]);
    prints ("\r\n");
}

