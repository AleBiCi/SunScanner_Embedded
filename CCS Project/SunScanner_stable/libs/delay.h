#ifndef LIBS_DELAY_H_
#define LIBS_DELAY_H_

// Delay stuff
volatile uint32_t Tick;

void delay_ms(uint32_t delay) // assumes 1 ms tick.
{
    uint32_t start = Tick;

    while (Tick - start < delay) ; // nop
}

void init_delay(){
    SysTick_enableModule();
    SysTick_setPeriod(3000);
    SysTick_enableInterrupt();
}

#endif /* LIBS_DELAY_H_ */
