////////////////////// stk500boot.c ////////////////////

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/common.h>
#include <avr/sfr_defs.h>
#include <stdlib.h>


// *****************[ STK message constants ]***************************

#define MESSAGE_START 0x1B
#define TOKEN 0x0E

// *****************[ STK general command constants ]**************************

#define CMD_SIGN_ON 0x01
#define CMD_SET_PARAMETER 0x02
#define CMD_GET_PARAMETER 0x03
#define CMD_SET_DEVICE_PARAMETERS 0x04
#define CMD_OSCCAL 0x05
#define CMD_LOAD_ADDRESS 0x06
#define CMD_FIRMWARE_UPGRADE 0x07

// *****************[ STK ISP command constants ]******************************

#define CMD_ENTER_PROGMODE_ISP 0x10
#define CMD_LEAVE_PROGMODE_ISP 0x11
#define CMD_CHIP_ERASE_ISP 0x12
#define CMD_PROGRAM_FLASH_ISP 0x13
#define CMD_READ_FLASH_ISP 0x14
#define CMD_PROGRAM_EEPROM_ISP 0x15
#define CMD_READ_EEPROM_ISP 0x16
#define CMD_PROGRAM_FUSE_ISP 0x17
#define CMD_READ_FUSE_ISP 0x18
#define CMD_PROGRAM_LOCK_ISP 0x19
#define CMD_READ_LOCK_ISP 0x1A
#define CMD_READ_SIGNATURE_ISP 0x1B
#define CMD_READ_OSCCAL_ISP 0x1C
#define CMD_SPI_MULTI 0x1D

// *****************[ STK PP command constants ]*******************************

#define CMD_ENTER_PROGMODE_PP 0x20
#define CMD_LEAVE_PROGMODE_PP 0x21
#define CMD_CHIP_ERASE_PP 0x22
#define CMD_PROGRAM_FLASH_PP 0x23
#define CMD_READ_FLASH_PP 0x24
#define CMD_PROGRAM_EEPROM_PP 0x25
#define CMD_READ_EEPROM_PP 0x26
#define CMD_PROGRAM_FUSE_PP 0x27
#define CMD_READ_FUSE_PP 0x28
#define CMD_PROGRAM_LOCK_PP 0x29
#define CMD_READ_LOCK_PP 0x2A
#define CMD_READ_SIGNATURE_PP 0x2B
#define CMD_READ_OSCCAL_PP 0x2C
#define CMD_SET_CONTROL_STACK 0x2D

// *****************[ STK HVSP command constants ]*****************************

#define CMD_ENTER_PROGMODE_HVSP 0x30
#define CMD_LEAVE_PROGMODE_HVSP 0x31
#define CMD_CHIP_ERASE_HVSP 0x32
#define CMD_PROGRAM_FLASH_HVSP 0x33
#define CMD_READ_FLASH_HVSP 0x34
#define CMD_PROGRAM_EEPROM_HVSP 0x35
#define CMD_READ_EEPROM_HVSP 0x36
#define CMD_PROGRAM_FUSE_HVSP 0x37
#define CMD_READ_FUSE_HVSP 0x38
#define CMD_PROGRAM_LOCK_HVSP 0x39
#define CMD_READ_LOCK_HVSP 0x3A
#define CMD_READ_SIGNATURE_HVSP 0x3B
#define CMD_READ_OSCCAL_HVSP 0x3C

// *****************[ STK status constants ]***************************

// Success

#define STATUS_CMD_OK 0x00

// Warnings

#define STATUS_CMD_TOUT 0x80
#define STATUS_RDY_BSY_TOUT 0x81
#define STATUS_SET_PARAM_MISSING 0x82

// Errors

#define STATUS_CMD_FAILED 0xC0
#define STATUS_CKSUM_ERROR 0xC1
#define STATUS_CMD_UNKNOWN 0xC9

// *****************[ STK parameter constants ]***************************

#define PARAM_BUILD_NUMBER_LOW 0x80
#define PARAM_BUILD_NUMBER_HIGH 0x81
#define PARAM_HW_VER 0x90
#define PARAM_SW_MAJOR 0x91
#define PARAM_SW_MINOR 0x92
#define PARAM_VTARGET 0x94
#define PARAM_VADJUST 0x95
#define PARAM_OSC_PSCALE 0x96
#define PARAM_OSC_CMATCH 0x97
#define PARAM_SCK_DURATION 0x98
#define PARAM_TOPCARD_DETECT 0x9A
#define PARAM_STATUS 0x9C
#define PARAM_DATA 0x9D
#define PARAM_RESET_POLARITY 0x9E
#define PARAM_CONTROLLER_INIT 0x9F

// *****************[ STK answer constants ]***************************

#define ANSWER_CKSUM_ERROR 0xB0

// *****************[ end of STK constants ]***************************


// --------------------------------------------------------------------> 
// BOARD DEFINES
// --------------------------------------------------------------------> 
#ifdef _MEGA_BOARD_
    #define PROGLED_PORT         PORTF   
    #define PROGLED_DDR          DDRF
    #define PROGLED_PIN_RED      PINF3   // F3 is our Red LED 
    #define PROGLED_PIN_GREEN    PINF4   // F4 is our Green LED
    
    // We also need the port, ddr and pin for dipswitch #5 on the TCB
    #define USART_SELECT_PORT    PORTK   // Port
    #define USART_SELECT_DDR     DDRK    // Data direction
    #define USART_SELECT_REG     PINK    // Pin register
    #define USART_SELECT_PIN     PINK4   // Port K, pin 4 is attached to Dipswitch #5

#else
    #error "Board not defined!"
#endif


// If CPU frequency not defined in Makefile, throw an error
#ifndef F_CPU
    #error "F_CPU NOT DEFINED"
#endif


// UART Baudrate, AVRStudio AVRISP only accepts 115200 bps
#ifndef BAUDRATE
    #define BAUDRATE 115200
#endif


// Enable (1) or disable (0) USART double speed operation
// We want the double-speed option to get closer to 115,200 actual baud rate
#ifndef UART_BAUDRATE_DOUBLE_SPEED
    #if defined (__AVR_ATmega32__)
        #define UART_BAUDRATE_DOUBLE_SPEED 0
    #else
        #define UART_BAUDRATE_DOUBLE_SPEED 1    // U2Xn is bit 1 of UCSRnA
    #endif
#endif


// HW and SW version, reported to AVRISP, must match version of AVRStudio
#define CONFIG_PARAM_BUILD_NUMBER_LOW 0
#define CONFIG_PARAM_BUILD_NUMBER_HIGH 0
#define CONFIG_PARAM_HW_VER 0x0F
#define CONFIG_PARAM_SW_MAJOR 2
#define CONFIG_PARAM_SW_MINOR 0x0A

// The stk500boot.c file says:
/* Calculate the address where the bootloader starts from FLASHEND and BOOTSIZE
 * (adjust BOOTSIZE below and BOOTLOADER_ADDRESS in Makefile if you want to change the size of the bootloader) */
// The FLASHEND definition is presumably in Words not Bytes, which explains why they are multiplying BOOTSIZE (in bytes) * 2
// But why they were setting BOOTSIZE to 2048 (bytes) when in fact they are using an 8k bootloader doesn't make sense. 
// Krupki sets it directly to 2048 which makes sense as he claims his bootloader is only 2k (though it seems a bit larger in fact)

// APP_END only gets used in one location in this file - to prevent erasing portions of the bootloader section when flashing the program. 
// However, that is a pretty important piece. 
// For the Open Panzer TCB, we reserve 4k for the bootloader, which is half what the stock Arduino Mega does.
// The actual size in flash of this bootloader code is approximately 2,300 bytes.  
#define BOOTSIZE 4096
#define APP_END (FLASHEND - (2 * BOOTSIZE) + 1)

// Bootsize is also set by the two BOOTSZ1:0 fuses and in the BOOTLOADER_ADDRESS variable in the Makefile, so make sure all these three locations match each other! 


// Signature bytes are not available in avr-gcc io_xxx.h
#if defined (__AVR_ATmega8__)
    #define SIGNATURE_BYTES 0x1E9307
#elif defined (__AVR_ATmega16__)
    #define SIGNATURE_BYTES 0x1E9403
#elif defined (__AVR_ATmega32__)
    #define SIGNATURE_BYTES 0x1E9502
#elif defined (__AVR_ATmega8515__)
    #define SIGNATURE_BYTES 0x1E9306
#elif defined (__AVR_ATmega8535__)
    #define SIGNATURE_BYTES 0x1E9308
#elif defined (__AVR_ATmega162__)
    #define SIGNATURE_BYTES 0x1E9404
#elif defined (__AVR_ATmega128__)
    #define SIGNATURE_BYTES 0x1E9702
#elif defined (__AVR_ATmega1280__)
    #define SIGNATURE_BYTES 0x1E9703
#elif defined (__AVR_ATmega2560__)
    #define SIGNATURE_BYTES 0x1E9801
#elif defined (__AVR_ATmega2561__)
    #define SIGNATURE_BYTES 0x1e9802
#else
    #error "no signature definition for MCU available"
#endif


// USART Setup
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) || defined(__AVR_ATmega162__) \
|| defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)

    // Our ATMega has two USARTs, and the whole point of modifying this file is we want the user
    // to choose which one they want to flash with, via the Dipswitch #5 setting on the Open Panzer TCB board. 
    
    // Standard serial through the USB port will be selected when Dipswitch #5 is in the ON position
    #define UART_BAUD_RATE_LOW          UBRR0L
    #define UART_STATUS_REG             UCSR0A
    #define UART_CONTROL_REG            UCSR0B
    #define UART_ENABLE_TRANSMITTER     TXEN0
    #define UART_ENABLE_RECEIVER        RXEN0
    #define UART_TRANSMIT_COMPLETE      TXC0
    #define UART_RECEIVE_COMPLETE       RXC0
    #define UART_DATA_REG               UDR0
    #define UART_DOUBLE_SPEED           U2X0
    
    // Alternate serial will be Serial 1, selected when Dipswitch #5 is in the OFF position
    #define UART_ALT_BAUD_RATE_LOW      UBRR1L
    #define UART_ALT_STATUS_REG         UCSR1A
    #define UART_ALT_CONTROL_REG        UCSR1B
    #define UART_ALT_ENABLE_TRANSMITTER TXEN1
    #define UART_ALT_ENABLE_RECEIVER    RXEN1
    #define UART_ALT_TRANSMIT_COMPLETE  TXC1
    #define UART_ALT_RECEIVE_COMPLETE   RXC1
    #define UART_ALT_DATA_REG           UDR1
    #define UART_ALT_DOUBLE_SPEED       U2X1

#else
    #error "no UART definition for MCU available"
#endif


// Macro to calculate UBBR from XTAL and baudrate
#if defined(__AVR_ATmega32__) && UART_BAUDRATE_DOUBLE_SPEED
    #define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu / 4 / baudRate - 1) / 2)
#elif defined(__AVR_ATmega32__)
    #define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu / 8 / baudRate - 1) / 2)
#elif UART_BAUDRATE_DOUBLE_SPEED
    // See page 208 of ATmega2560 datasheet for formulas. The rounding bit they've added is fine.  
    #define UART_BAUD_SELECT(baudRate,xtalCpu) (((float)(xtalCpu))/(((float)(baudRate))*8.0)-1.0+0.5)
#else
    #define UART_BAUD_SELECT(baudRate,xtalCpu) (((float)(xtalCpu))/(((float)(baudRate))*16.0)-1.0+0.5)
#endif


// States used in the receive state machine
#define ST_START        0
#define ST_GET_SEQ_NUM  1
#define ST_MSG_SIZE_1   2
#define ST_MSG_SIZE_2   3
#define ST_GET_TOKEN    4
#define ST_GET_DATA     5
#define ST_GET_CHECK    6
#define ST_PROCESS      7

// use 16bit address variable for ATmegas with <= 64K flash
#if defined(RAMPZ)
typedef uint32_t address_t;
#else
typedef uint16_t address_t;
#endif


// Global variable - to store our UART selection setting
unsigned char useAltSerial;

// function prototypes
static void sendchar (char c);


// since this bootloader is not linked against the avr-gcc crt1 functions,
// to reduce the code size, we need to provide our own initialization
void __jumpMain (void) __attribute__ ( (naked)) __attribute__ ( (section (".init9")));
void __jumpMain (void)
{
    asm volatile (".set __stack, %0" :: "i" (RAMEND));
    asm volatile (" ldi 16,%0" :: "i" (RAMEND >> 8));
    asm volatile (" out %0,16" :: "i" (AVR_STACK_POINTER_HI_ADDR));
    asm volatile (" ldi 16,%0" :: "i" (RAMEND & 0x0ff));
    asm volatile (" out %0,16" :: "i" (AVR_STACK_POINTER_LO_ADDR));
    asm volatile (" clr __zero_reg__");                                     // GCC depends on register r1 set to 0
    asm volatile (" out %0, __zero_reg__" :: "I" (_SFR_IO_ADDR (SREG)));    // set SREG to 0
    asm volatile (" jmp main");                                             // jump to main()
}


// Send single byte to USART, wait until transmission is completed
static void sendchar (char c)
{
    if (useAltSerial)
    {
        UART_ALT_DATA_REG = c;                                                      // prepare transmission
        while (! (UART_ALT_STATUS_REG & (1 << UART_ALT_TRANSMIT_COMPLETE))) { ; }   // wait until byte sent
        UART_ALT_STATUS_REG |= (1 << UART_ALT_TRANSMIT_COMPLETE);                   // delete TXCflag
    }
    else
    {
        UART_DATA_REG = c;                                                          // prepare transmission
        while (! (UART_STATUS_REG & (1 << UART_TRANSMIT_COMPLETE))) { ; }           // wait until byte sent
        UART_STATUS_REG |= (1 << UART_TRANSMIT_COMPLETE);                           // delete TXCflag
    }
}

// Check serial port for data
static int Serial_Available (void)
{
    if (useAltSerial)
    {
        return (UART_ALT_STATUS_REG & (1 << UART_ALT_RECEIVE_COMPLETE));            // wait for data    
    }
    else
    {
        return (UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE));                    // wait for data
    }
}

// The stock stk500v2 bootloader sets the timeout to: 
// #define MAX_TIME_COUNT (F_CPU >> 1) - basically 8 million, which is about 9 seconds for them

// We set it to F_CPU / 8 which comes to 2 million instructions - about 3 seconds
#define MAX_TIME_COUNT (F_CPU >> 3)
static unsigned char recchar_timeout (void)
{
    char c;
    unsigned char incoming = 0;
    uint32_t count = 0;

    do 
    {
        // Check for incoming byte
        if (useAltSerial)
        {
            incoming = (UART_ALT_STATUS_REG & (1 << UART_ALT_RECEIVE_COMPLETE));
        }
        else
        {
            incoming = (UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE)); 
        }
        
        if (incoming == 0)
        {
            count++;
            
            // Timeout, exit bootloader
            if (count > MAX_TIME_COUNT) 
            {
                unsigned int data;
            #if (FLASHEND > 0x0FFFF)
                data = pgm_read_word_far (0);   // get the first word of the user program
            #else
                data = pgm_read_word_near (0);  // get the first word of the user program
            #endif

                if (data != 0xffff)             // make sure its valid before jumping to it.
                { 
                    asm volatile (
                        " clr r30\n"
                        " clr r31\n"
                        " ijmp\n"
                    );
                }
                count = 0;
            }
        }
    } while (incoming == 0);
    
    // Turn Green LED On when byte received. If we are flashing via USB cable, the two amber LEDs on the TCB will blink
    // during flash as a useful visual indicator. But if we are flashing over Bluetooth/FTDI over Serial 1 (alt serial), 
    // those amber LEDs will do nothing. So regardless of which serial port we are using, we flash the Green LED on any byte
    // received. It is turned on here, and turned off again in the main loop when the byte has been processed. 
    PROGLED_PORT |= (1 << PROGLED_PIN_GREEN);   
    
    // Now return the char received
    if (useAltSerial) { c = UART_ALT_DATA_REG; } else { c = UART_DATA_REG; }
    return c;
}

//*****************************************************************************
int main (void)
{
    address_t address = 0;
    address_t eraseAddress = 0;
    unsigned char msgParseState;
    unsigned int ii = 0;
    unsigned char checksum = 0;
    unsigned char seqNum = 0;
    unsigned int msgLength = 0;
    unsigned char msgBuffer[384];
    unsigned char c, *p;
    unsigned char isLeave = 0;
    unsigned long boot_timeout;
    unsigned long boot_timer;
    unsigned int boot_state;
    boot_timer = 0;
    boot_state = 0;
    boot_timeout = 50000; // Each boot_state=0 wait loop is about 39uS long, this number is how many times we will do the wait loop
                          // 25k will be roughly 1 second, 50k will be 2 seconds, etc.

    // PROG_PIN pulled low, indicate with LED that bootloader is active
    PROGLED_DDR       |=  (1 << PROGLED_PIN_RED);           // Set Red LED pin to output
    PROGLED_DDR       |=  (1 << PROGLED_PIN_GREEN);         // Set Green LED pin to output
    PROGLED_PORT      &= ~(1 << PROGLED_PIN_RED);           // Start with Red LED OFF
    PROGLED_PORT      &= ~(1 << PROGLED_PIN_GREEN);         // Start with Green LED OFF

    // Read Dipswitch #5 to determine which USART the user wants to flash with
    USART_SELECT_PORT |=  (1 << USART_SELECT_PIN);          // Enable internal pullup on pin
    USART_SELECT_DDR  &= ~(1 << USART_SELECT_PIN);          // Set pin to input
    // Give the pin a jilly-sec to get ready
    asm volatile (
        "\tnop\n"
        "\tnop\n"
        "\tnop\n"
    );
    // Now read the position of the pin into our global variable.
    // The physical dipswitch is held to ground when the switch is ON, and left floating (but pulled up with the internal pullup), when Off
    // If the switch is in the On position  (meaning, it will read low/ground), then we want to use regular serial.
    // If the switch is in the Off position (meaning, it will read high), then we want to use the alternate serial.
    useAltSerial = 0;                                            // Initialize to normal serial
    useAltSerial = USART_SELECT_REG & _BV(USART_SELECT_PIN);     // We have to AND with a full bit mask, not just the pin number

    // Init UART
    // set baudrate and enable USART receiver and transmiter without interrupts
#if UART_BAUDRATE_DOUBLE_SPEED
    if (useAltSerial)
    {
        UART_ALT_STATUS_REG     |= (1 << UART_ALT_DOUBLE_SPEED);    
    }
    else
    {
        UART_STATUS_REG         |= (1 << UART_DOUBLE_SPEED);        
    }
#endif
    if (useAltSerial)
    {
        UART_ALT_BAUD_RATE_LOW  = UART_BAUD_SELECT(BAUDRATE,F_CPU);
        UART_ALT_CONTROL_REG    = ((1 << UART_ALT_ENABLE_RECEIVER) | (1 << UART_ALT_ENABLE_TRANSMITTER));
    }
    else
    {
        UART_BAUD_RATE_LOW      = UART_BAUD_SELECT(BAUDRATE,F_CPU);
        UART_CONTROL_REG        = ((1 << UART_ENABLE_RECEIVER) | (1 << UART_ENABLE_TRANSMITTER));
    }
    asm volatile (      // wait until port has changed
        "\tnop\n"
        "\tnop\n"
        "\tnop\n"
    );    


    while (boot_state == 0) 
    {
        while ((!(Serial_Available())) && (boot_state == 0))    // wait for data
        { 

            boot_timer++;

            if (boot_timer > boot_timeout) {
                boot_state = 1;     // (after ++ -> boot_state=2 bootloader timeout, jump to main 0x00000 )
            }

            // About halfway through the wait we turn the Red LED on. If the bootloader keeps repeating for
            // infinity, this will cause the appearance of a blinking Red LED.
            // Of course, the bootloader will only repeat for infinity if there is no other program on the chip to go to. 
            if (! (boot_timer % 25000))
            {
                PROGLED_PORT |= (1 << PROGLED_PIN_RED); // turn Red LED On                
            }
        }

        boot_state++;       // ( if boot_state=1 bootloader received byte from UART, enter bootloader mode)
    }

    // Turn the Red LED on if it isn't already, and we'll leave it on for the remainder of the bootloader
    PROGLED_PORT |= (1 << PROGLED_PIN_RED); 

    // Bootloader got a command - process
    if (boot_state == 1) 
    {
        while (!isLeave) 
        {
            // Collect received bytes to a complete message
            msgParseState = ST_START;

            while (msgParseState != ST_PROCESS) 
            {

                if (boot_state == 1) 
                {
                    boot_state = 0;
                    if (useAltSerial) { c = UART_ALT_DATA_REG; } else { c = UART_DATA_REG; }
                } 
                else 
                {
                    PROGLED_PORT &= ~(1 << PROGLED_PIN_GREEN);  // Turn Green LED Off while waiting for a byte. 
                    c = recchar_timeout();                      // We will turn it back on when byte received in recchar_timeout()
                }

                switch (msgParseState) 
                {

                    case ST_START:

                        if (c == MESSAGE_START) 
                        {
                            msgParseState   = ST_GET_SEQ_NUM;
                            checksum        = MESSAGE_START^0;
                        }
                        break;

                    case ST_GET_SEQ_NUM:

                        if ( (c == 1) || (c == seqNum)) 
                        {
                            seqNum          = c;
                            msgParseState   = ST_MSG_SIZE_1;
                            checksum        ^= c;
                        }
                        else 
                        {
                            msgParseState   = ST_START;
                        }
                        break;

                    case ST_MSG_SIZE_1:
                        msgLength           = c << 8;
                        msgParseState       = ST_MSG_SIZE_2;
                        checksum            ^= c;
                        break;

                    case ST_MSG_SIZE_2:
                        msgLength           |= c;
                        msgParseState       = ST_GET_TOKEN;
                        checksum            ^= c;
                        break;

                    case ST_GET_TOKEN:
                        if (c == TOKEN) 
                        {
                            msgParseState   = ST_GET_DATA;
                            checksum        ^= c;
                            ii              = 0;
                        } 
                        else 
                        {
                            msgParseState   = ST_START;
                        }
                        break;

                    case ST_GET_DATA:
                        msgBuffer[ii++]     = c;
                        checksum            ^= c;
                        if (ii == msgLength) 
                        {
                            msgParseState   = ST_GET_CHECK;
                        }
                        break;

                    case ST_GET_CHECK:
                        if (c == checksum) 
                        {
                            msgParseState   = ST_PROCESS;
                        }
                        else 
                        {
                            msgParseState   = ST_START;
                        }
                        break;
                } // switch
            } // while(msgParseState)


            // Now process the STK500 commands, see Atmel Appnote AVR068
            switch (msgBuffer[0]) 
            {
                case CMD_SPI_MULTI: 
                    {
                        unsigned char answerByte;
                        unsigned char flag=0;

                        if (msgBuffer[4] == 0x30) 
                        {
                            unsigned char signatureIndex = msgBuffer[6];

                            if (signatureIndex == 0) 
                            {
                                answerByte = (SIGNATURE_BYTES >> 16) & 0x000000FF;
                            } 
                            else if (signatureIndex == 1) 
                            {
                                answerByte = (SIGNATURE_BYTES >> 8) & 0x000000FF;
                            } 
                            else 
                            {
                                answerByte = SIGNATURE_BYTES & 0x000000FF;
                            }
                        } 
                        else if (msgBuffer[4] & 0x50) 
                        {
                            answerByte = 0; //read fuse/lock bits not implemented, return dummy value
                        }
                        else 
                        {
                            answerByte = 0; // for all others command are not implemented, return dummy value for AVRDUDE happy <Worapoht>
                        }

                        if (!flag) 
                        {
                            msgLength       = 7;
                            msgBuffer[1]    = STATUS_CMD_OK;
                            msgBuffer[2]    = 0;
                            msgBuffer[3]    = msgBuffer[4];
                            msgBuffer[4]    = 0;
                            msgBuffer[5]    = answerByte;
                            msgBuffer[6]    = STATUS_CMD_OK;
                        }
                    }
                    break;

                case CMD_SIGN_ON:
                    msgLength           = 11;
                    msgBuffer[1]        = STATUS_CMD_OK;
                    msgBuffer[2]        = 8;
                    msgBuffer[3]        = 'A';
                    msgBuffer[4]        = 'V';
                    msgBuffer[5]        = 'R';
                    msgBuffer[6]        = 'I';
                    msgBuffer[7]        = 'S';
                    msgBuffer[8]        = 'P';
                    msgBuffer[9]        = '_';
                    msgBuffer[10]       = '2';
                    break;

                case CMD_GET_PARAMETER: 
                    {
                        unsigned char value;

                        switch (msgBuffer[1]) 
                        {
                            case PARAM_BUILD_NUMBER_LOW:
                                value = CONFIG_PARAM_BUILD_NUMBER_LOW;
                                break;
                            case PARAM_BUILD_NUMBER_HIGH:
                                value = CONFIG_PARAM_BUILD_NUMBER_HIGH;
                                break;
                            case PARAM_HW_VER:
                                value = CONFIG_PARAM_HW_VER;
                                break;
                            case PARAM_SW_MAJOR:
                                value = CONFIG_PARAM_SW_MAJOR;
                                break;
                            case PARAM_SW_MINOR:
                                value = CONFIG_PARAM_SW_MINOR;
                                break;
                            default:
                                value = 0;
                                break;
                        }
                        msgLength = 3;
                        msgBuffer[1] = STATUS_CMD_OK;
                        msgBuffer[2] = value;
                    }
                    break;

                case CMD_LEAVE_PROGMODE_ISP:
                    isLeave = 1;
                    //* fall through

                case CMD_SET_PARAMETER:
                case CMD_ENTER_PROGMODE_ISP:
                    msgLength = 2;
                    msgBuffer[1] = STATUS_CMD_OK;
                    break;

                case CMD_READ_SIGNATURE_ISP: 
                    {
                        unsigned char signatureIndex = msgBuffer[4];
                        unsigned char signature;

                        if (signatureIndex == 0) 
                        {
                            signature = (SIGNATURE_BYTES >> 16) & 0x000000FF;
                        }
                        else if (signatureIndex == 1) 
                        {
                            signature = (SIGNATURE_BYTES >> 8) & 0x000000FF;
                        }
                        else 
                        {
                            signature = SIGNATURE_BYTES & 0x000000FF;
                        }
                        msgLength = 4;
                        msgBuffer[1] = STATUS_CMD_OK;
                        msgBuffer[2] = signature;
                        msgBuffer[3] = STATUS_CMD_OK;
                    }
                    break;

                case CMD_READ_LOCK_ISP:
                    msgLength = 4;
                    msgBuffer[1] = STATUS_CMD_OK;
                    msgBuffer[2] = boot_lock_fuse_bits_get (GET_LOCK_BITS);
                    msgBuffer[3] = STATUS_CMD_OK;
                    break;

                case CMD_READ_FUSE_ISP: 
                    {
                        unsigned char fuseBits;

                        if (msgBuffer[2] == 0x50) 
                        {
                            if (msgBuffer[3] == 0x08)
                            { fuseBits = boot_lock_fuse_bits_get (GET_EXTENDED_FUSE_BITS); }
                            else
                            { fuseBits = boot_lock_fuse_bits_get (GET_LOW_FUSE_BITS); }
                        } 
                        else 
                        {
                            fuseBits = boot_lock_fuse_bits_get (GET_HIGH_FUSE_BITS);
                        }
                        msgLength = 4;
                        msgBuffer[1] = STATUS_CMD_OK;
                        msgBuffer[2] = fuseBits;
                        msgBuffer[3] = STATUS_CMD_OK;
                    }
                    break;

                case CMD_PROGRAM_LOCK_ISP: 
                    {
                        unsigned char lockBits = msgBuffer[4];
                        lockBits = (~lockBits) & 0x3C; // mask BLBxx bits
                        boot_lock_bits_set (lockBits); // and program it
                        boot_spm_busy_wait();
                        msgLength = 3;
                        msgBuffer[1] = STATUS_CMD_OK;
                        msgBuffer[2] = STATUS_CMD_OK;
                    }
                    break;

                case CMD_CHIP_ERASE_ISP:
                    eraseAddress = 0;
                    msgLength = 2;
                    msgBuffer[1] = STATUS_CMD_OK;
                    break;

                case CMD_LOAD_ADDRESS:
                    #if defined(RAMPZ)
                        address = ( ( (address_t) (msgBuffer[1]) << 24) | ( (address_t) (msgBuffer[2]) << 16) | ( (address_t) (msgBuffer[3]) << 8) | (msgBuffer[4])) << 1;
                    #else
                        address = ( ( (msgBuffer[3]) << 8) | (msgBuffer[4])) << 1; //convert word to byte address
                    #endif
                        msgLength = 2;
                        msgBuffer[1] = STATUS_CMD_OK;
                        break;

                case CMD_PROGRAM_FLASH_ISP:
                case CMD_PROGRAM_EEPROM_ISP: 
                    {
                        unsigned int size = ( (msgBuffer[1]) << 8) | msgBuffer[2];
                        unsigned char *p = msgBuffer+10;
                        unsigned int data;
                        unsigned char highByte, lowByte;
                        address_t tempaddress = address;

                        if (msgBuffer[0] == CMD_PROGRAM_FLASH_ISP) 
                        {
                            // erase only main section (bootloader protection)
                            if (eraseAddress < APP_END) 
                            {
                                boot_page_erase (eraseAddress); // Perform page erase
                                boot_spm_busy_wait(); // Wait until the memory is erased.
                                eraseAddress += SPM_PAGESIZE; // point to next page to be erase
                            }

                            // Write FLASH
                            do {
                                lowByte = *p++;
                                highByte = *p++;
                                data = (highByte << 8) | lowByte;
                                boot_page_fill (address,data);
                                address = address+2; // Select next word in memory
                                size -= 2; // Reduce number of bytes to write by two
                            } while (size); // Loop until all bytes written

                            boot_page_write (tempaddress);
                            boot_spm_busy_wait();
                            boot_rww_enable(); // Re-enable the RWW section
                        }
                        else 
                        {
                        #if (!defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__) && !defined(__AVR_ATmega2561__))
                            // write EEPROM
                            do {
                                EEARL = address; // Setup EEPROM address
                                EEARH = (address >> 8);
                                address++; // Select next EEPROM byte
                                EEDR = *p++; // get byte from buffer
                                EECR |= (1 << EEMWE); // Write data into EEPROM
                                EECR |= (1 << EEWE);

                                while (EECR & (1 << EEWE)) { ; } // Wait for write operation to finish

                                size--; // Decrease number of bytes to write
                            } while (size); // Loop until all bytes written
                        #endif
                        }
                        msgLength = 2;
                        msgBuffer[1] = STATUS_CMD_OK;
                    }
                    break;

                case CMD_READ_FLASH_ISP:
                case CMD_READ_EEPROM_ISP: 
                    {
                        unsigned int size = ( (msgBuffer[1]) << 8) | msgBuffer[2];
                        unsigned char *p = msgBuffer+1;
                        msgLength = size+3;
                        
                        *p++ = STATUS_CMD_OK;
                        if (msgBuffer[0] == CMD_READ_FLASH_ISP) 
                        {
                            unsigned int data;

                            // Read FLASH
                            do {
                            #if defined(RAMPZ)
                                data = pgm_read_word_far (address);
                            #else
                                data = pgm_read_word_near (address);
                            #endif
                                *p++ = (unsigned char) data; //LSB
                                *p++ = (unsigned char) (data >> 8); //MSB
                                address += 2; // Select next word in memory
                                size -= 2;
                            } while (size);
                        } 
                        else 
                        {
                            // Read EEPROM
                            do {
                                EEARL = address; // Setup EEPROM address
                                EEARH = ( (address >> 8));
                                address++; // Select next EEPROM byte
                                EECR |= (1 << EERE); // Read EEPROM
                                *p++ = EEDR; // Send EEPROM data
                                size--;
                            } while (size);
                        }
                        *p++ = STATUS_CMD_OK;
                    }
                    break;
                
                default:
                    msgLength = 2;
                    msgBuffer[1] = STATUS_CMD_FAILED;
                    break;
            }

            // Now send answer message back
            sendchar (MESSAGE_START);
            checksum = MESSAGE_START^0;
            
            sendchar (seqNum);
            checksum ^= seqNum;
            
            c = ( (msgLength >> 8) &0xFF);
            sendchar (c);
            checksum ^= c;
            
            c = msgLength&0x00FF;
            sendchar (c);
            checksum ^= c;
            
            sendchar (TOKEN);
            checksum ^= TOKEN;
            
            p = msgBuffer;
            while (msgLength) 
            {
                c = *p++;
                sendchar (c);
                checksum ^=c;
                msgLength--;
            }
            sendchar (checksum);
            seqNum++;

        }
    }

    PROGLED_PORT &= ~(1 << PROGLED_PIN_RED);    // Red LED OFF
    PROGLED_PORT &= ~(1 << PROGLED_PIN_GREEN);  // Green LED OFF
    asm volatile (
        "\tnop\n"
        "\tnop\n"
        "\tnop\n"
    ); 

    // Here we clear every serial flag and put double-speed back to off - let the Sketch set these up however it wants.
    if (useAltSerial)
    {
        UART_ALT_STATUS_REG &= 0xFC;        
    }
    else
    {
        UART_STATUS_REG &= 0xFC;
    }
    
    // Now leave bootloader
    boot_rww_enable();              // Enable application section
    asm volatile (
        " clr r30\n"
        " clr r31\n"
        " ijmp\n"
    );

    // Never return to stop GCC from generating exit return code.
    // Actually we will never reach this point, but the compiler doesn't know that.
    for (;;) { ; }
}
