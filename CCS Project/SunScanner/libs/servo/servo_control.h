#ifndef SERVO_CONTROL_H_
#define SERVO_CONTROL_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Before doing anything with a servo configure_pwm_timer() and
 * setup_servo() must be called
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PWM_PERIOD 200
#define PWM_LOW    5
#define PWM_HIGH   25

const Timer_A_UpModeConfig servo_timer_config =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source (12 MHz)
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // SMCLK/1 = 3MHz
        1200,                                    // 1200 tick period (0.1 ms)
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

uint16_t pwm_counter;

typedef struct Tservo{
    uint32_t angle;  //value between 10 and 50
    uint16_t port;
    uint16_t pin;
} Tservo;

/*move servo at the corresponding angle in degrees*/
void set_servo_angle(Tservo* _servo, uint32_t _angle){
    _angle = _angle >= 180 ? 180 : _angle;
    _angle = _angle <= 0   ? 0   : _angle;
    _servo->angle = PWM_LOW + (((_angle*100000) / 180) * (PWM_HIGH - PWM_LOW))/100000;
}

/*return the position of servo in degrees*/
uint16_t get_servo_angle(const Tservo _servo){
    uint16_t _ang =9 * (_servo.angle - PWM_LOW);
    return _ang;
}

void configure_pwm_timer(){
    /* Configuring Timer_A1 for Up Mode */
    Timer_A_configureUpMode(TIMER_A1_BASE, &servo_timer_config);

    /* Enabling interrupts and starting the timer */
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    pwm_counter = 0;
}

void setup_servo(Tservo* _servo, uint16_t _port, uint16_t _pin, uint16_t _angle){
    _servo->port = _port;
    _servo->pin = _pin;
    set_servo_angle(_servo,_angle);
    GPIO_setAsOutputPin(_port, _pin);
}

void pwm_servo_control(Tservo* _servo){
    if(pwm_counter < _servo->angle){
        GPIO_setOutputHighOnPin(_servo->port, _servo->pin);
    }else{
        GPIO_setOutputLowOnPin(_servo->port, _servo->pin);
    }
}


#endif /* SERVO_CONTROL_H_ */
