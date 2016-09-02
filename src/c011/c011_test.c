#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
 
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
    usart0_write("\r\n");  
}

void printsb (char *s, uint8_t b) {
    usart0_write(s);
    char tmp[8];
    snprintf (tmp, sizeof(tmp), "0x%X", b);
    usart0_write(tmp);
    usart0_write("\r\n");  
}

static uint8_t portl_mirror=0x00;

void setl (uint8_t bit) {
    portl_mirror |= (1<<bit);
    PORTL = portl_mirror;
}

void clearl (uint8_t bit) {
    portl_mirror &= ~(1<<bit);
    PORTL = portl_mirror;
}

void c011_reset(void) {
    _delay_ms(20);
    prints ("reset");
    setl(0);        //set reset
    _delay_ms(1);
    clearl(0);      //clear reset
    _delay_ms(10);
}

uint8_t outval=0x55;

void c011_write(uint8_t val) {
    //printsb ("write ",outval);
    PORTA = outval;
    setl(1);        // set Ivalid
}

ISR(INT0_vect) {
    //prints ("INT0 Iack");
    clearl(1);      // clear Ivalid
    outval++;
    c011_write(outval);
    return;
}

ISR(INT1_vect) {
    //prints ("INT1 Qvalid");
    setl(2);        // set Qack
    clearl(2);      // clear Qack
}

int main (void)
{
    cli();
    usart0_init();
    DDRA = 0xFF;    //port A 0:7 output 
    DDRC = 0x00;    //port C 0:7 input
    DDRL = 0x07;    //port L 0:2 output 0(reset), 1(IIvalid), 2(Qack)
    DDRD = 0x00;    //port D 0:7 input (INT 0,1 pins)
    //INT0&1 triggered on rising edge
    EICRA = (1<<ISC01) | (1<<ISC00) | (1<<ISC11) | (1<<ISC10);    
    EIMSK = (1<<INT0) | (1<<INT1); //enable INT0,1
    sei();

    c011_reset();
    c011_write(outval);
    while(1) {
        _delay_ms(1000);
    }
}


