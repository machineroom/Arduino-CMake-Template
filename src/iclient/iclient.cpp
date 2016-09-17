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
        received = recv(SOCK, buf, sizeof(buf));
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
                    }
                    print ("\tdone OCMD_Reset\n\r");
                    break;
                case OCMD_WriteLink:
                case OCMD_ReadLink:
                case OCMD_Analyse:
                case OCMD_TestError:
                case OCMD_Poke16:
                case OCMD_Poke32:
                case OCMD_Peek16:
                case OCMD_Peek32:
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
    attachInterrupt(0, w5100int, FALLING);
    Serial.begin(9600);
    print("main() enter\n\r");
    EthernetClass eth;
    IPAddress ip_adr; 
    eth.begin(mac);
    ip_adr = eth.localIP();
    print("DHCP IP = %d.%d.%d.%d\r\n", ip_adr[0], ip_adr[1], ip_adr[2], ip_adr[3]);
    
    pinMode(2,INPUT);
    W5100.writeIMR(0xEF);   // enable global interrupts
    print("w5100 interrupts enabled\r\n");
    
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

