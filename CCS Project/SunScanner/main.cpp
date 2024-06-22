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
 * MSP432 UART
 *
 *  MCLK = HSMCLK = SMCLK = DCO of 24MHz
 *
 *               MSP432P401
 *             -----------------
 *            |                 |
 *       RST -|     P3.3/UCA0TXD|---> GPS_RX
 *            |                 |
 *           -|                 |
 *            |     P3.2/UCA0RXD|---> GPS_TX
 *            |                 |
 *            |             P1.0|---> LED
 *            |                 |
 *
 *******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <vector>

/* Custom includes */
#include "NMEAParser.h"

#define BUFFER_LEN

/* Global vars */
uint8_t RXData = 0;
std::vector<char> UART_buffer{};
MessageRMC sentence = MessageRMC();

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
const eUSCI_UART_ConfigV1 uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        13,                                      // BRDIV = 13
        0,                                       // UCxBRF = 0
        37,                                      // UCxBRS = 37
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_MSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
        EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
};

int main(void)
{
    /* Halting WDT  */
    WDT_A_holdTimer();

    /* Selecting P3.2 and P3.3 in UART mode and P1.0 as output (LED) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
             GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Setting DCO to 24MHz (upping Vcore) -> CPU operates at 24 MHz!*/
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A2_BASE);

    /* Enabling interrupts */
    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA2);
    Interrupt_enableSleepOnIsrExit();

    while(1)
    {
        // UART_transmitData(EUSCI_A2_BASE, TXData);
        Interrupt_enableSleepOnIsrExit();
        PCM_gotoLPM0InterruptSafe();
    }
}

/* EUSCI A0 UART ISR - Adds data to UART_buffer and sets sentence.m */
void EUSCIA2_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        RXData = UART_receiveData(EUSCI_A2_BASE);
        UART_buffer.push_back(RXData);  // Add received data to uart buffer

        if(RXData == '\n')              // Check if received byte is newline character
        {
            // Flash led and set sentence.m to content of UART_buffer
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            sentence.setMessage(&UART_buffer[0]);
        }
        sentence.parseRMC();
        sentence.printSent();
        Interrupt_disableSleepOnIsrExit();
    }

}

//
///* --COPYRIGHT--,BSD
// * Copyright (c) 2017, Texas Instruments Incorporated
// * All rights reserved.
// *
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions
// * are met:
// *
// * *  Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// *
// * *  Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// *
// * *  Neither the name of Texas Instruments Incorporated nor the names of
// *    its contributors may be used to endorse or promote products derived
// *    from this software without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// * --/COPYRIGHT--*/
///******************************************************************************
// * MSP432 UART - PC Echo with 12MHz BRCLK
// *
// * Description: This demo echoes back characters received via a PC serial port.
// * SMCLK/DCO is used as a clock source and the device is put in LPM0
// * The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
// * when the UART is idle and turned on when a receive edge is detected.
// * Note that level shifter hardware is needed to shift between RS232 and MSP
// * voltage levels.
// *
// *               MSP432P401
// *             -----------------
// *            |                 |
// *            |                 |
// *            |                 |
// *       RST -|     P1.3/UCA0TXD|----> PC (echo)
// *            |                 |
// *            |                 |
// *            |     P1.2/UCA0RXD|<---- PC
// *            |                 |
// *
// *******************************************************************************/
///* DriverLib Includes */
//#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//
///* UART driver include */
//#include <ti/drivers/UART.h>
//
///* Standard Includes */
//#include <stdint.h>
//#include <stdbool.h>
//#include <vector>
//
//#define MAX_BUF_LEN 80
//
//std::vector<char> RXBuffer;
//
//volatile int counter = 0;
//
//// Messages to configure NEO-6M
//    uint8_t UBX_CFG_MSG_GGA_OFF[] = {                   // UBX-CFG-MSG NMEA-GGA Off on All interfaces
//        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,             // Header
//        0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
//        0xFF, 0x23};                                    // Checksum
//
//    uint8_t UBX_CFG_MSG_GSV_OFF[] = {                   // UBX-CFG-MSG NMEA-GSV Off on All interfaces
//        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
//        0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0x02, 0x38};
//
//    uint8_t UBX_CFG_MSG_GSA_OFF[] = {                   // UBX-CFG-MSG NMEA-GSA Off on All interfaces
//        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
//        0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0x01, 0x31};
//
//    uint8_t UBX_CFG_MSG_VTG_OFF[] = {                   // UBX-CFG-MSG NMEA-VTG Off on All interfaces
//        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
//        0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0x04, 0x46};
//
//    uint8_t UBX_CFG_MSG_GLL_OFF[] = {                   // UBX-CFG-MSG NMEA-GLL Off on All interfaces
//        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
//        0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0x00, 0x2A};
//
///* FSM stuff */
//
//typedef enum {
//    STATE_SETUP,
//    STATE_ACQUISITION,
//    //STATE_PARSING,
//    STATE_IDLE
//} State_t;
//
//typedef struct{
//    State_t state;
//    void (*state_function)(void);
//} StateMachine_t;
//
//void fn_GPSsetup(void);
//void fn_acquisition(void);
////void fn_parsing(void);
//void fn_idle(void);
//
//State_t current_state = STATE_SETUP;
//
//StateMachine_t fsm[] = {
//                      {STATE_SETUP, fn_GPSsetup},
//                      {STATE_ACQUISITION, fn_acquisition},
//                      //{STATE_PARSING, fn_parsing},
//                      {STATE_IDLE, fn_idle}
//};
//
//void fn_GPSsetup() {
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    int i;
//    for(i=0; i<16; i++) {
//        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GGA_OFF[i]);
//    }
//    for(i=1000; i>0; i--);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//
//    for(i=0; i<16; i++) {
//        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GSV_OFF[i]);
//    }
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//
//    for(i=0; i<16; i++) {
//        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GSA_OFF[i]);
//    }
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//
//    for(i=0; i<16; i++) {
//        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_VTG_OFF[i]);
//    }
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//
//    for(i=0; i<16; i++) {
//        UART_transmitData(EUSCI_A2_BASE, UBX_CFG_MSG_GLL_OFF[i]);
//    }
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    for(i=1000; i>0; i--);
//
//    //CHANGE STATE
//    current_state = STATE_ACQUISITION;
//}
//
//void fn_acquisition() {
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
//
//    /* Enabling interrupts */
//    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//
//    Interrupt_enableInterrupt(INT_EUSCIA0);
//    Interrupt_enableInterrupt(INT_EUSCIA2);
//    if(counter<10) {
//    } else {
//    //CHANGE STATE
//        current_state = STATE_IDLE;
//
//        UART_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//        UART_disableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//
//        Interrupt_disableInterrupt(INT_EUSCIA0);
//        Interrupt_disableInterrupt(INT_EUSCIA2);
//    }
//}
//
//void fn_idle(void) {
//    int i;
//    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
//    for(i=0; i<1000; i++);
//}
//
////![Simple UART Config]
///* UART Configuration Parameter. These are the configuration parameters to
// * make the eUSCI A UART module to operate with a 9600 baud rate. These
// * values were calculated using the online calculator that TI provides
// * at:
// *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
// */
//const eUSCI_UART_ConfigV1 uartConfig =
//{
//        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
//        78,                                     // BRDIV = 78
//        2,                                       // UCxBRF = 2
//        0,                                       // UCxBRS = 0
//        EUSCI_A_UART_NO_PARITY,                  // No Parity
//        EUSCI_A_UART_LSB_FIRST,                  // LSB First
//        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
//        EUSCI_A_UART_MODE,                       // UART mode
//        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
//        EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
//};
//
////![Simple UART Config]
//
//int main(void)
//{
//    /* Halting WDT  */
//    WDT_A_holdTimer();
//
//    /* Selecting P1.2 and P1.3 in UART mode --> PC loopback */
//    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
//    /* Selecting P3.2 and P3.3 in UART mode */
//    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
//
//    // Sets led pin as output
//    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
//
//    //Red
//    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    //Green
//    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
//    //Blue
//    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
//
//
//    /* Setting DCO to 12MHz */
//    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
//
//    //![Simple UART Example]
//    /* Configuring UART Module */
//    UART_initModule(EUSCI_A0_BASE, &uartConfig);
//    UART_initModule(EUSCI_A2_BASE, &uartConfig);
//
//    /* Enable UART module */
//    UART_enableModule(EUSCI_A0_BASE);
//    UART_enableModule(EUSCI_A2_BASE);
//    Interrupt_enableMaster();
//    //![Simple UART Example]
//
//
//    while(1)
//    {
//        //PCM_gotoLPM0InterruptSafe();
//        (*fsm[current_state].state_function)();
//
//    }
//}
//
///* EUSCI A2 UART ISR - Echoes data back to PC host */
//void EUSCIA2_IRQHandler(void)
//{
//    char RXData;
//    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
//
//    /* Received a normal character --> put it in the buffer and echo it back to the loopback UART */
//    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
//    {
//        //RXBuffer.push_back(RXData);
//        if((RXData = UART_receiveData(EUSCI_A2_BASE))=='\n') { /* Received a break character --> prepare buffer for parsing + blink led + clear buffer */
//        UART_transmitData(EUSCI_A0_BASE, RXData);
//        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
//        int i;
//        for(i=1000; i>0; i--);
//        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
//
//        counter++;
//        }
//    }
//    UART_clearInterruptFlag(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
//}
//
///* EUSCI A0 UART ISR - Echoes data back to PC host */
//void EUSCIA0_IRQHandler(void)
//{
//    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
//
//    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
//    {
//        UART_transmitData(EUSCI_A0_BASE, UART_receiveData(EUSCI_A0_BASE));
//    }
//
//    UART_clearInterruptFlag(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
//}
