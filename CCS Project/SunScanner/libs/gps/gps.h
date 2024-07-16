/*
 * gps.h
 *
 *  Created on: 15 lug 2024
 *      Author: alessandrobianchiceriani
 */

#ifndef LIBS_GPS_GPS_H_
#define LIBS_GPS_GPS_H_

#include <string.h>
#include <stdio.h>
#include <math.h>


/* Custom Includes */
#include "gps/NMEAParser.h"
#include "delay.h"

#define MAX_BUF_LEN 100

char RXBuffer[MAX_BUF_LEN];

const eUSCI_UART_ConfigV1 uartConfig =
    {
            EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source --> 12MHz
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

char RXData;


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

void gps_UART_setup() {
    /* Selecting P1.2 and P1.3 in UART mode --> PC loopback */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* Selecting P3.2 and P3.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    // DEBUGGING
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A0_BASE);
    UART_enableModule(EUSCI_A2_BASE);
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(EUSCI_A_UART_RECEIVE_INTERRUPT);
}

void gps_setup(uint32_t UART) {
    // INITIALIZING THE RXBUFFER
    RXBuffer[0] = '\0';

    int i;
    for(i=0; i<16; i++) {
        UART_transmitData(UART, UBX_CFG_MSG_GGA_OFF[i]);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(UART, UBX_CFG_MSG_GSV_OFF[i]);
    }
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(UART, UBX_CFG_MSG_GSA_OFF[i]);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(UART, UBX_CFG_MSG_VTG_OFF[i]);
    }
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);

    for(i=0; i<16; i++) {
        UART_transmitData(UART, UBX_CFG_MSG_GLL_OFF[i]);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    //CHANGE STATE
//    current_state = STATE_ACQUISITION;
    delay_ms(1000);
}


/* HELPER FUNCTIONS */

void PrintChar(uint32_t UART, char c)
{
    /*Send the char through the selected UART*/
    UART_transmitData(UART, c);
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

void PrintFloat(uint32_t UART, float num) {
    double fractional, integer;

    // Use modf to separate the integer and fractional parts
    fractional = modf(num, &integer);

    // Convert the double value of the integer part to an integer
    int intPart = (int)integer;

    // Handle the fractional part by shifting it
    int fracPart = 0;
    int multiplier = 1;

    while (fractional != 0.0) {
        fractional *= 10;
        int digit = (int)fractional;
        fractional -= digit;
        fracPart = fracPart * 10 + digit;
        multiplier *= 10;
    }

    PrintInteger(UART, intPart);
    delay_ms(1);
    PrintChar(UART, '.');
    delay_ms(1);
    PrintInteger(UART, fracPart);
    PrintChar(UART, '\n');
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

bool gps_acquisition(MessageRMC *sentence) {
    int counter = 0;
    int iterFixed = 0;
    int iterNotFixed = 0;

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
                //DEBUGGING
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                delay_ms(5);
                GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                delay_ms(5);

                setMessage(sentence, &RXBuffer[0]);
                int i;
                //                sentence.setMessage((char*)"$GNRMC,140212.00,A,4604.18179,N,01108.18041,E,0.234,,210224,,,A*69\n");
                for(i=0; i<strlen(sentence->m); i++) {
                     UART_transmitData(EUSCI_A0_BASE, sentence->m[i]);
                }

                if(!strlen(sentence->m)<10) {
                    if(parseRMC(sentence)) { // NOT FIXED
                        iterNotFixed++;
                    } else {
                        iterFixed++;
                    }
                } else iterNotFixed++;

                strcpy(RXBuffer, "");

                counter++;
            } else strcpy(RXBuffer, "");
        } else if(strlen(RXBuffer)==MAX_BUF_LEN) {
            strcpy(RXBuffer, "");
        }
    }

    // If acquisition was successful (iterFixed==10) return true, else we didn't receive enough valid messages
    return iterFixed>=10;
}

#endif /* LIBS_GPS_GPS_H_ */
