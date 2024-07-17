#ifndef LIBS_ESP_CONTROL_H_
#define LIBS_ESP_CONTROL_H_

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "gps/gps.h"
#define START_SEQ 0xFF
#define STOP_SEQ 0xFF
#define OUT_BUFF_SIZE 28
#define IN_BUFF_SIZE 18

typedef struct tm tm;
char to_send[OUT_BUFF_SIZE] = {START_SEQ, START_SEQ, '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', STOP_SEQ, STOP_SEQ};
char to_receive[18];
int buffer_index = 0;

float azimuth = 0.0, elevation = 0.0;

void parseMess(float *az, float *el);

void setupESPConnection() {
    /* Selecting P2.2 and P2.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A1_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A1_BASE);
}

void createStringToSend(tm *time_ptr, float lat, float lon) {

    char sec[3];
    char min[3];
    char hour[3];
    char mday[3];
    char mon[3];
    char year[3];
    char lat_str[7];
    char lon_str[7];

    sprintf(sec, "%d", time_ptr->tm_sec);
    sprintf(min, "%d", time_ptr->tm_min);
    sprintf(hour, "%d", time_ptr->tm_hour);
    sprintf(mday, "%d", time_ptr->tm_mday);
    sprintf(mon, "%d", time_ptr->tm_mon);
    sprintf(year, "%d", time_ptr->tm_year);
    sprintf(lat_str, "%f", lat);
    sprintf(lon_str, "%f", lon);

    memcpy(&to_send[2], sec, 2);
    memcpy(&to_send[4], min, 2);
    memcpy(&to_send[6], hour, 2);
    memcpy(&to_send[8], mday, 2);
    memcpy(&to_send[10], mon, 2);
    memcpy(&to_send[12], year, 2);
    memcpy(&to_send[14], lat_str, 6);
    memcpy(&to_send[20], lon_str, 6);

//    strcat(to_send, sec);
//    strcat(to_send, min);
//    strcat(to_send, hour);
//    strcat(to_send, mday);
//    strcat(to_send, mon);
//    strcat(to_send, year);
//    strcat(to_send, lat_str);
//    strcat(to_send, lon_str);
//    to_send[OUT_BUFF_SIZE - 2] = STOP_SEQ;
//    to_send[OUT_BUFF_SIZE - 1] = STOP_SEQ;
}

void sendToESP(uint32_t UART, char* mess) {
    if(mess!=NULL) {
        int i;
        for(i=0; i<OUT_BUFF_SIZE; i++) {
            PrintChar(UART, mess[i]);
        }
    }
}

void receiveFromESP() {
    char ESP_RXData = 0;
    bool startSequenceDetected = false;
    bool parsingDone = false;
    while(!parsingDone) {
        ESP_RXData = UART_receiveData(EUSCI_A1_BASE);

        // State machine to detect the start sequence and fill the buffer
        if (startSequenceDetected) {
            // Store the byte in the buffer
            to_receive[buffer_index++] = ESP_RXData;;

            // Check if buffer is full
            if (buffer_index == IN_BUFF_SIZE) {
                // Check for the stop sequence
                if (to_receive[IN_BUFF_SIZE-2] == STOP_SEQ && to_receive[IN_BUFF_SIZE-1] == STOP_SEQ) {
                    //Complete sentence received. Parsing...
                    parsingDone = true;
                    parseMess(&azimuth, &elevation);
                } else {
                    //Stop sequence not found. Discarding data.
                }
                // Reset the buffer index
                buffer_index = 0;
                startSequenceDetected = false;
            }
        } else if (ESP_RXData == START_SEQ) {
            // Detect start sequence
            if (buffer_index == 0 || to_receive[buffer_index - 1] == START_SEQ) {
                // Store the byte in the buffer
                to_receive[buffer_index++] = ESP_RXData;

                // Check if the start sequence is detected
                if (buffer_index == 2) {
                    startSequenceDetected = true;
                    buffer_index = 2; // Reset buffer index to account for already stored start sequence
                    // START SEQ DETECTED
                }
            } else {
                // Reset if not matching start sequence
                buffer_index = 0;
            }
        } else {
            // Reset buffer index if not start sequence
            buffer_index = 0;
        }
    }
}

void parseMess(float *az, float *el) {
//    char az_str[] = {to_receive[2], to_receive[3], to_receive[4], to_receive[5], to_receive[6], to_receive[7], to_receive[8]};
    char az_str[8];
    memcpy(&az_str, &to_receive[2], 7);
//    char el_str[] = {to_receive[9], to_receive[10], to_receive[11], to_receive[12], to_receive[13], to_receive[14], to_receive[15]};
    char el_str[8];
    memcpy(&el_str, &to_receive[9], 7);

    *az = strtod(az_str, NULL);
    *el = strtod(el_str, NULL);
}

#endif /* LIBS_ESP_CONTROL_H_ */
