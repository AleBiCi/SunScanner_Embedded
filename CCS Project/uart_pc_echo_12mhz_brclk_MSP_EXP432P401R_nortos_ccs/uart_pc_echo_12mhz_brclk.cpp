
/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 UART - PC Echo with 12MHz BRCLK
 *
 * Description: This demo echoes back characters received via a PC serial port.
 * SMCLK/DCO is used as a clock source and the device is put in LPM0
 * The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
 * when the UART is idle and turned on when a receive edge is detected.
 * Note that level shifter hardware is needed to shift between RS232 and MSP
 * voltage levels.
 *
 *               MSP432P401
 *             -----------------
 *            |                 |
 *            |                 |
 *            |                 |
 *       RST -|     P1.3/UCA0TXD|----> PC (echo)
 *            |                 |
 *            |                 |
 *            |     P1.2/UCA0RXD|<---- PC
 *            |                 |
 *
 *******************************************************************************/

/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* UART driver include */
#include <ti/drivers/UART.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* Custom Includes */
#include <NMEAParser.hpp>

#define MAX_BUF_LEN 100

char RXBuffer[MAX_BUF_LEN];

MessageRMC sentence;

int counter = 0;
int iterFixed = 0;
int iterNotFixed = 0;
char RXData;
bool is_acquiring = false;
bool once = true;

// Messages to configure NEO-6M
    uint8_t UBX_CFG_MSG_GGA_OFF[] = {                   // UBX-CFG-MSG NMEA-GGA Off on All interfaces
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,             // Header
        0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
        0xFF, 0x23};                                    // Checksum

    uint8_t UBX_CFG_MSG_GSV_OFF[] = {                   // UBX-CFG-MSG NMEA-GSV Off on All interfaces
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
        0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x38};

    uint8_t UBX_CFG_MSG_GSA_OFF[] = {                   // UBX-CFG-MSG NMEA-GSA Off on All interfaces
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
        0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x31};

    uint8_t UBX_CFG_MSG_VTG_OFF[] = {                   // UBX-CFG-MSG NMEA-VTG Off on All interfaces
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
        0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x46};

    uint8_t UBX_CFG_MSG_GLL_OFF[] = {                   // UBX-CFG-MSG NMEA-GLL Off on All interfaces
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
        0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x2A};

/* Helper functions */

void PrintChar(uint32_t UART, char c)
{
    /*Send the char through the selected UART*/
    UART_transmitData(EUSCI_A0_BASE, c);
}

void PrintString(uint32_t UART, char *string)
{
    /*while the string is not the null character*/
    while(*string)
    {
        PrintChar(UART, *string);
        string++;
    }
}

void PrintInteger(uint32_t UART, int integer)
{
    if(integer == 0) {
        PrintChar(UART, '0');
    }

    if(integer < 0) {
        PrintChar(UART, '-');
    }

    char b[10];
    int digit = integer;

    uint8_t i = 0;
    while(digit) {
        b[i++] = digit % 10;
        digit /= 10;
    }

    while(i) {
        PrintChar(UART, '0' + b[i-1]);
        i--;
    }
}

/*A basic printf for the MSP432. In order to use it properly you need to initialize the correct UART peripheral.
 * The following formats are supported:
 * %c = for char variables
 * %s = for string variables
 * %i = for unsigned integers
 * USAGE...
 *
 * MSPrintf(EUSCI_A0_BASE, "Formated string %c, %s, %i", character, string, integer)*/

void MSPrintf(uint32_t UART, const char *fs, ...)
{
    va_list valist;
    va_start(valist, fs);
    int i;
    char *s;

    while(*fs)
    {
        if(*fs != '%')
        {
            UART_transmitData(UART, *fs);
            fs++;
        }
        else
        {
            switch(*++fs)
            {
            case 'c':
                i = va_arg(valist, int);
                PrintChar(UART, (char)i);
                break;
            case 's':
                s = va_arg(valist, char*);
                PrintString(UART, s);
                break;
            case 'i':
                i = va_arg(valist, int);
                PrintInteger(UART, i);
                break;
            }

            ++fs;
        }
    }
}



/* FSM stuff */

typedef enum {
    STATE_SETUP,
    STATE_ACQUISITION,
    //STATE_PARSING,
    STATE_IDLE
} State_t;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

void fn_GPSsetup(void);
void fn_acquisition(void);
//void fn_parsing(void);
void fn_idle(void);

State_t current_state = STATE_SETUP;

StateMachine_t fsm[] = {
                      {STATE_SETUP, fn_GPSsetup},
                      {STATE_ACQUISITION, fn_acquisition},
                      //{STATE_PARSING, fn_parsing},
                      {STATE_IDLE, fn_idle}
};


void fn_GPSsetup() {
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
    int i;
    for(i=0; i<16; i++) {
        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GGA_OFF[i]);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GSV_OFF[i]);
    }
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GSA_OFF[i]);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_VTG_OFF[i]);
    }
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GLL_OFF[i]);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    //CHANGE STATE
    current_state = STATE_ACQUISITION;

    for(i=1000000; i>0; --i);
}

void fn_acquisition() {
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);

    while((iterFixed < 10) && (iterNotFixed < 20) && (counter<30)) {
        /* Received a normal character --> put it in the buffer and echo it back to the loopback UART */
        RXData = UART_receiveData(EUSCI_A2_BASE);

        //Temporary buffer to hold RXData and terminator
        char temp[2];
        temp[0] = RXData;
        temp[1] = '\0';

        // Concatenate the character to RXBuffer
        strcat(RXBuffer, temp);

        /* Received a break character --> prepare buffer for parsing + blink led + clear buffer */

        if(RXData=='\n') {
            if(strlen(RXBuffer)>10) {
                sentence.setMessage(&RXBuffer[0]);
                int i;
//                sentence.setMessage((char*)"$GNRMC,140212.00,A,4604.18179,N,01108.18041,E,0.234,,210224,,,A*69\n");
                for(i=0; i<strlen(sentence.m); i++) {
                    UART_transmitData(EUSCI_A0_BASE, sentence.m[i]);
                }

                if(!strlen(sentence.m)<10) {
                    if(sentence.parseRMC()) { // NOT FIXED
                        iterNotFixed++;
                    } else {
                        iterFixed++;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        for(i=1000; i>0; i--);
                        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        for(i=1000; i>0; i--);
                    }
                } else iterNotFixed++;

                strcpy(RXBuffer, "");

                counter++;
            } else strcpy(RXBuffer, "");
        } else if(strlen(RXBuffer)==MAX_BUF_LEN) {
            strcpy(RXBuffer, "");
        }
    }
    //CHANGE STATE
    current_state = STATE_IDLE;
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
}

void fn_idle(void) {
    if(once) {
        PrintString(EUSCI_A0_BASE, "MESSAGE: ");
        PrintString(EUSCI_A0_BASE, sentence.m);
        PrintString(EUSCI_A0_BASE, "\n");
        PrintString(EUSCI_A0_BASE, "CHECKSUM: ");
        PrintString(EUSCI_A0_BASE, sentence.checksum);
        PrintString(EUSCI_A0_BASE, "\n");
        PrintString(EUSCI_A0_BASE, "STATUS: ");
        PrintChar(EUSCI_A0_BASE, sentence.status);
        PrintString(EUSCI_A0_BASE, "\n");
//        PrintString(EUSCI_A0_BASE, "Time -> HH:MM:SS, DD/MM/YYYY\n");
//        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_hour);
//        PrintString(EUSCI_A0_BASE, ":");
//        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_min);
//        PrintString(EUSCI_A0_BASE, ":");
//        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_sec);
//        PrintString(EUSCI_A0_BASE, ", ");
//        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_mday);
//        PrintString(EUSCI_A0_BASE, "/");
//        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_mon);
//        PrintString(EUSCI_A0_BASE, "/");
//        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_year);
        MSPrintf(EUSCI_A0_BASE, "Time: %i:%i:%i, %i/%i/%i\n", sentence.time.tm_hour, sentence.time.tm_min, sentence.time.tm_sec, sentence.time.tm_mday, sentence.time.tm_mon, sentence.time.tm_year);
        PrintString(EUSCI_A0_BASE, "\n");
        once=!once;
    }
    int i;
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
    for(i=0; i<1000000; i++);
}

//![Simple UART Config]
/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 9600 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
const eUSCI_UART_ConfigV1 uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
        EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
};

//![Simple UART Config]

int main(void)
{
    /* Halting WDT  */
    WDT_A_holdTimer();

    /* Selecting P1.2 and P1.3 in UART mode --> PC loopback */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* Selecting P3.2 and P3.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    // Sets led pin as output
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //Red
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    //Green
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    //Blue
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);


    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    //![Simple UART Example]
    /* Configuring UART Module */
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A0_BASE);
    UART_enableModule(EUSCI_A2_BASE);
    //![Simple UART Example]
    Interrupt_enableMaster();
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(EUSCI_A_UART_RECEIVE_INTERRUPT);


    // INITIALIZING THE RXBUFFER
    RXBuffer[0] = '\0';

    while(1)
    {
        //PCM_gotoLPM0InterruptSafe();
        (*fsm[current_state].state_function)();
    }
}

/* EUSCI A0 UART ISR - Echoes data back to PC host */
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        UART_transmitData(EUSCI_A0_BASE, UART_receiveData(EUSCI_A0_BASE));
    }

    UART_clearInterruptFlag(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
}
