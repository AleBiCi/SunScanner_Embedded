#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "finite_state_machine.h"
#include "servo/Sun_az_alt.h"
#include "menu.h"

void init_clk();

/*
 * main.c
 */
void main(void)
{
    /* Stop WDT  */
    WDT_A_holdTimer();

    init_clk();
    init_delay();
    setup_menu();
    gps_UART_setup();
    gps_setup(EUSCI_A2_BASE);
    setup_servo_panel(&solar_panel, GPIO_PORT_P2, GPIO_PIN5, GPIO_PORT_P2, GPIO_PIN6);

    while(1){
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }
    }
}

void init_clk(){
    /* Set 2 flash wait states for Flash bank 0 and 1*/
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);

    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

void SysTick_Handler(void)
{
    Tick++;
}

/* GPIO ISR */
void PORT5_IRQHandler(void) // BUTTON 1
{
    uint32_t status;
    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);

    if(status & GPIO_PIN1) {
        depth++;
        alt1 == 3 ? menu_keep_range(3, &depth) : menu_keep_range(2, &depth);
    }
    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);
}

void PORT3_IRQHandler(void) // BUTTON 2
{
    uint32_t status;
    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);

    if(status & GPIO_PIN5) {
        depth--;
        alt1 == 3 ? menu_keep_range(3, &depth) : menu_keep_range(2, &depth);
        if(current_state == STATE_MANUAL){
            current_state = STATE_NAVIGATION;
        }
    }
    GPIO_clearInterruptFlag(GPIO_PORT_P3, status);
}

void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    /* ADC_MEM1 conversion completed */
    if(status & ADC_INT1)
    {
        /* Store ADC14 conversion results */
        resultsBuffer[0] = ADC14_getResult(ADC_MEM0);
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1);

        /* Determine if JoyStick button is pressed */
        if (!(P4IN & GPIO_PIN1)){
            depth = 1;
            alt1 = 1;
        }


        if(active_pos(resultsBuffer[0])!=0 && !joystick_state[0] ){ // X Movement
            joystick_state[0] = true;
            if(current_state == STATE_NAVIGATION){
                switch(active_pos(resultsBuffer[0])) {
                    case 1: --depth; break;
                    case 2: ++depth; break;
                }
                alt1 == 3 ? menu_keep_range(3, &depth) : menu_keep_range(2, &depth);
            }
            if(current_state == STATE_MANUAL){
                switch(active_pos(resultsBuffer[1])) {
                    case 1:
                        set_servo_angle(&(solar_panel.azimuth),get_servo_angle(solar_panel.azimuth)+20);
                        break;
                    case 2:
                        set_servo_angle(&(solar_panel.azimuth),get_servo_angle(solar_panel.azimuth)-20);
                        break;
                }
            }
        }
        if(resting_pos(resultsBuffer[0])){
            joystick_state[0] = false;
        }

        if(active_pos(resultsBuffer[1])!=0 && !(joystick_state[1])){ // Y Movement
            joystick_state[1] = true;
            if(current_state == STATE_NAVIGATION){
                switch(active_pos(resultsBuffer[1])) {
                    case 1:
                        depth == 1 ? ++alt1 : ++alt2;
                        break;
                    case 2:
                        depth == 1 ? --alt1 : --alt2;
                        break;
                    default:
                        break;
                }
                depth == 1 ? pacMan_1_n(4, &alt1) : pacMan_1_n(3, &alt2);
            }
            if(current_state == STATE_MANUAL){
                switch(active_pos(resultsBuffer[1])) {
                    case 1:
                        set_servo_angle(&(solar_panel.elevation),get_servo_angle(solar_panel.elevation)+10);
                        break;
                    case 2:
                        set_servo_angle(&(solar_panel.elevation),get_servo_angle(solar_panel.elevation)-10);
                        break;
                    default:
                        break;
                }
            }
        }
        if(resting_pos(resultsBuffer[1])){
            joystick_state[1] = false;
        }
    }
    if(status & ADC_INT2){
        analogRead = ADC14_getResult(ADC_MEM2);
        if(depth == 2 && alt1 == 1){
            selMenu(alt1,depth,alt2);
        }
    }
}

void TA1_0_IRQHandler(void)
{
    if(++pwm_counter > PWM_PERIOD){
        pwm_counter = 0;
    }
    pwm_panel_control(&solar_panel);
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
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
