#include <assert.h>

#include <Ethernet.h>
#include <socket.h>

#include "opsprot.h"
#include "opserror.h"


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
    print ("c011 reset\n\r");
    setl(0);        //set reset
    _delay_ms(1);
    clearl(0);      //clear reset
    _delay_ms(10);
}

static bool busy=false;

void c011_write(uint8_t val) {
    print ("c011 write 0x%X\n\r", val);
    while (busy) {
        delay(1);
    }
    print ("c011 write 0x%X after busy wait\n\r", val);
    PORTA = val;
    busy = true;
    setl(1);        // set Ivalid
}

void c011_int_iack() {
    print ("c011 iack\n\r");
    //data sent on link
    clearl(1);      // clear Ivalid
    busy = false;
}

static bool wait=true;

void c011_int_qvalid() {
    print ("c011 qvalid\n\r");
    // data received on link
    setl(2);        // set Qack
    wait = false;
    clearl(2);      // clear Qack
}

uint8_t c011_read(void) {
    while (wait) {
        delay(1);
    }
    wait = true;
    return PORTA;
}

uint8_t SOCK=1;

void processLinkOpsCmd(void) {
    uint16_t received;
    uint16_t sent;
    static bool opened = false;
    if (!opened) {
        uint8_t buf[OPSOpenCmdSize];
        /* linkops open() doesn't follow normal size,tag format so
           treat as a special case. doh! */
        OPSOpenCommand opencmd;
        received = recv(SOCK, (uint8_t *)&opencmd, sizeof(opencmd));
        if (received == OPSOpenCmdSize && opencmd.command_tag == OCMD_Open) {
            OPSOpenReply reply;
            print ("OCMD_Open\n\r");
            memset (reply.service_message, ' ', sizeof(reply.service_message));
            memcpy (reply.service_message,SERVICE_OPSOPEN,strlen(SERVICE_OPSOPEN));
            reply.reply_tag = OREPLY_Open;
            reply.status = STATUS_NOERROR;
            strncpy (reply.device_name, "james' C011 arduino thingie", sizeof(reply.device_name));
            sent = send(SOCK, (uint8_t *)&reply, (uint16_t)sizeof(reply));
            opened = true;
            print ("\tdone OCMD_Open\n\r");
        } else {
            print ("*E* expecting OPEN but didn't get it\n\r");
        }
        opened = true;
    } else {
        uint8_t buf[3];
        print ("wait command\n\r");
        received = recv(SOCK, buf, sizeof(buf));
        print ("rx = %d\n\r", received);
        if (received >= 0) {
            switch (buf[2]) {
                case OCMD_CommsSynchronous:
                    print ("OCMD_CommsSynchronous\n\r");
                    {
                        OPSCommsSynchronousReply reply;
                        reply.packet_size = OPSCommsSynchronousReplySize;
                        reply.reply_tag = OREPLY_CommsSynchronous;
                        reply.status = STATUS_NOERROR;
                        sent = send(SOCK, (uint8_t *)&reply, (uint16_t)sizeof(reply));
                    }
                    print ("\tdone OCMD_CommsSynchronous\n\r");
                    break;
                case OCMD_ErrorIgnore:
                    print ("OCMD_ErrorIgnore\n\r");
                    // no reply
                    print ("\tdone OCMD_ErrorIgnore\n\r");
                    break;
                case OCMD_Close:
                    print ("OCMD_Close\n\r");
                    {
                        OPSCloseReply reply;
                        reply.packet_size = OPSCloseReplySize;
                        reply.reply_tag = OREPLY_Close;
                        reply.status = STATUS_NOERROR;
                        sent = send(SOCK, (uint8_t *)&reply, (uint16_t)sizeof(reply));
                    }
                    print ("\tdone OCMD_Close\n\r");
                    break;
                case OCMD_Reset:
                    print ("OCMD_Reset\n\r");
                    {
                      char processor_id[4];
                      received = recv(SOCK, (uint8_t *)processor_id, sizeof(processor_id));
                      setl (3);
                      clearl (3);
                    }
                    print ("\tdone OCMD_Reset\n\r");
                    break;
                case OCMD_Analyse:
                    print ("OCMD_Analyse\n\r");
                    {
                      char processor_id[4];
                      received = recv(SOCK, (uint8_t *)processor_id, sizeof(processor_id));
                      setl (4);
                      //clearl (3);
                    }
                    print ("\tdone OCMD_Analyse\n\r");
                    break;
                case OCMD_Peek32:
                    print ("OCMD_Peek32\n\r");
                    {
                        char processor_id[4];
                        received = recv(SOCK, (uint8_t *)processor_id, sizeof(processor_id));
                        char peek_length[2];
                        received = recv(SOCK, (uint8_t *)peek_length, sizeof(peek_length));
                        char address[4];
                        received = recv(SOCK, (uint8_t *)address, sizeof(address));
                        uint16_t length;
                        length = peek_length[0];
                        length <<= 8;
                        length |= peek_length[1];
                        uint32_t addr;
                        addr = address[0];
                        addr <<= 8;
                        addr |= address[1];
                        addr <<= 8;
                        addr |= address[2];
                        addr <<= 8;
                        addr |= address[3];
                        print ("OCMD_Peek32 %d @ 0x%X\n\r", length, addr);
                        c011_write (1);   //transputer control byte = link peek
                        c011_write (address[0]);
                        c011_write (address[1]);
                        c011_write (address[2]);
                        c011_write (address[3]);
                        OPSPeek32Reply reply;
                        reply.packet_size = OPSPeek32ReplyBasicSize;
                        reply.reply_tag = OREPLY_Peek32;
                        reply.status = STATUS_NOERROR;
                        reply.processor_id[0] = reply.processor_id[1] = reply.processor_id[2] = reply.processor_id[3] = 0;
                        sent = send(SOCK, (uint8_t *)&reply, (uint16_t)sizeof(reply));
                        uint8_t bytes[4];
                        bytes[0] = c011_read();
                        bytes[1] = c011_read();
                        bytes[2] = c011_read();
                        bytes[3] = c011_read();
                        sent = send(SOCK, bytes, sizeof(bytes));
                    }
                    print ("\tdone OCMD_Peek32\n\r");
                    break;
                case OCMD_WriteLink:
                case OCMD_ReadLink:
                case OCMD_TestError:
                case OCMD_Poke16:
                case OCMD_Poke32:
                case OCMD_Peek16:
                case OCMD_ErrorDetect:
                case OCMD_CommsAsynchronous:
                case OCMD_AsyncWrite:
                case OCMD_Restart:
                default:
                    print ("unsupported op = %d\n\r",buf[2]);
                    break;
            }
        } else {
            print ("*E* failed recv()\n\r");
        }
    }
}

void w5100int() {
    uint8_t IR;
    do {
        IR = W5100.readSnIR(SOCK);
        print ("W5100 IR=0x%X\n\r",IR);
        switch (IR) {
            case SnIR::CON:
                print ("W5100 CON\n\r");
                break;
            case SnIR::DISCON:
                print ("W5100 DISCON\n\r");
                socket(SOCK, SnMR::TCP, 555, 0);
                listen(SOCK);
                break;
            case SnIR::RECV:
                print ("W5100 RECV\n\r");
                uint16_t avail;
                avail = W5100.getRXReceivedSize(SOCK);
                while (avail > 0) {
                    processLinkOpsCmd();
                    avail = W5100.getRXReceivedSize(SOCK);
                }
                break;
        }
        W5100.writeSnIR(1,IR);   // clear int
    } while (IR != 0);
}


int main (int argc, char**argv) {
    int rc;
    uint8_t mac[] = { 0x00, 0x08, 0xBE, 0xEF, 0xFE, 0xED };
    init();
    //mega mapping: 0=INT4=digital2=w5100 int
    //              1=INT5
    //              2=INT0=digital21
    //              3=INT1=digital20
    //              4=INT2
    //              5=INT3
    //              6=INT6
    //              7=INT7
    Serial.begin(9600);
    attachInterrupt(0/*INT4*/, w5100int, FALLING);
    attachInterrupt(2/*INT0*/, c011_int_iack, RISING);
    attachInterrupt(3/*INT1*/, c011_int_qvalid, RISING);
    print("main() enter\n\r");
    EthernetClass eth;
    IPAddress ip_adr; 
    eth.begin(mac);
    ip_adr = eth.localIP();
    print("DHCP IP = %d.%d.%d.%d\r\n", ip_adr[0], ip_adr[1], ip_adr[2], ip_adr[3]);
    
    DDRA = 0xFF;    //port A 0:7 output 
    DDRC = 0x00;    //port C 0:7 input
    DDRL = 0x1F;    //port L 0:4 output:
                    //          0: C011 reset
                    //          1: IIvalid
                    //          2: Qack
                    //          3: TRAM reset   (D 46)
                    //          4: TRAM analyse (D 45)

    pinMode(2,INPUT);   //W5100 interrupt (digital 2, INT4)
    pinMode(21,INPUT);  //C011 IACK interrupt (digital 21, INT0)
    pinMode(20,INPUT);  //C011 QVALID interrupt (digital 20, INT1)

    //DDRD = 0x00;    //port D 0:7 input (INT 0,1 pins)

    W5100.writeIMR(0xEF);   // enable global interrupts
    print("w5100 interrupts enabled\r\n");

    c011_reset();
    
    SOCKET sock = SOCK;   //TODO not clear how to manage these. Now that DHCP is done we should have all 4 available?
    uint16_t port = 555;
    if (socket(sock, SnMR::TCP, port, 0) == 1) {
        if (listen(sock) == 1) {
            print ("listening on %d...\n\r", port);
            while (1) {
                delay(10000);
            }
        } else {
            print ("*E* failed listen()\n\r");
        }
    } else {
        print ("*E* failed socket()\n\r");
    }
}

