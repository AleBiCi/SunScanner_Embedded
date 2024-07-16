#ifndef LIBS_FINITE_STATE_MACHINE_H_
#define LIBS_FINITE_STATE_MACHINE_H_

#include "menu.h"
#include "delay.h"
#include "servo/servo_sun_tracker.h"
#include "gps/gps.h"

MessageRMC sentence;

typedef enum {
    STATE_NAVIGATION,
    STATE_POSITIONING,
    STATE_WAIT_AUTO,
    STATE_ZENIT,
    STATE_MANUAL,
    STATE_GPS_ACQUIRE,
    NUM_STATES
}State_t;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

void fn_NAVIGATION(void);
void fn_POSITIONING(void);
void fn_WAIT_AUTO(void);
void fn_ZENIT(void);
void fn_MANUAL(void);
void fn_WAIT_BACK(void);
void fn_GPS_ACQUIRE(void);

StateMachine_t fsm[] = {
                      {STATE_NAVIGATION, fn_NAVIGATION},
                      {STATE_POSITIONING, fn_POSITIONING},
                      {STATE_WAIT_AUTO, fn_WAIT_AUTO},
                      {STATE_ZENIT, fn_ZENIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_GPS_ACQUIRE, fn_GPS_ACQUIRE}
};

State_t current_state = STATE_NAVIGATION;

void fn_NAVIGATION(void){
    if(depthPrev!=depth || alt1Prev!=alt1 || alt2Prev!=alt2) {
        Graphics_clearDisplay(&g_sContext);
        delay_ms(100);
        selMenu(alt1, depth, alt2);
        depthPrev = depth;
        alt1Prev = alt1;
        alt2Prev = alt2;
        if(alt1 == 3 && alt2 == 2 && depth == 3){
            current_state = STATE_ZENIT;
        }
        if(alt1 == 3 && alt2 == 3 && depth == 3){
            current_state = STATE_MANUAL;
        }
        if(alt1 == 3 && alt2 == 1 && depth == 3){
            current_state = STATE_GPS_ACQUIRE;
        }
        phi = get_servo_angle(solar_panel.azimuth);
        theta = get_servo_angle(solar_panel.elevation);
    }
}

void fn_POSITIONING(void){
    point_sun(&solar_panel, &(sentence.time), sentence.latitude, sentence.longitude, 360.0); // We set the altitude based on external data --> Polo Ferrari: ~360m s.l.m

    current_state = STATE_NAVIGATION;
}

void fn_WAIT_AUTO(void){

}
void fn_ZENIT(void){
    set_servo_angle(&(solar_panel.elevation),90);
    current_state = STATE_NAVIGATION;
}
void fn_MANUAL(void){

}
void fn_WAIT_BACK(void){

}
void fn_GPS_ACQUIRE(void){
    ADC14_disableInterrupt(ADC_INT1);
    Interrupt_disableInterrupt(INT_ADC14);

    initMessageRMC(&sentence);

    if(!gps_acquisition(&sentence)) {
        // ACQUISITION UNSUCCESSFUL
        current_state = STATE_NAVIGATION;
    } else {
        // ACQUISITION SUCCESSFUL
        current_state = STATE_POSITIONING;
        PrintFloat(EUSCI_A0_BASE, sentence.latitude);
        PrintChar(EUSCI_A0_BASE, '\n');

        PrintFloat(EUSCI_A0_BASE, sentence.longitude);
        PrintChar(EUSCI_A0_BASE, '\n');

        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_mday);
        PrintChar(EUSCI_A0_BASE, ':');

        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_mon);
        PrintChar(EUSCI_A0_BASE, ':');

        PrintInteger(EUSCI_A0_BASE, sentence.time.tm_year);
        PrintChar(EUSCI_A0_BASE, '\n');
    }

    ADC14_enableInterrupt(ADC_INT1);
    Interrupt_enableInterrupt(INT_ADC14);
}



#endif /* LIBS_FINITE_STATE_MACHINE_H_ */
