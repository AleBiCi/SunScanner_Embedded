#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define LED_PIN         BIT0   // LED rosso sulla scheda LaunchPad

uint8_t RXData = 0;

const eUSCI_UART_ConfigV1 uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,  // SMCLK Clock Source
    78,                              // BRDIV
    2,                               // UCxBRF
    0,                               // UCxBRS
    EUSCI_A_UART_NO_PARITY,          // No Parity
    EUSCI_A_UART_MSB_FIRST,          // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,       // One stop bit
    EUSCI_A_UART_MODE,               // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
    EUSCI_A_UART_8_BIT_LEN           // 8 bit data length
};

int main(void)
{
    WDT_A_holdTimer();  // Disabilita il Watchdog Timer

    // Configura i pin P3.2 e P3.3 come funzione periferica UART
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    // Configura il pin P1.0 come output per il LED
    GPIO_setAsOutputPin(GPIO_PORT_P1, LED_PIN);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, LED_PIN);

    // Configura il DCO a 12MHz
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    // Inizializza e abilita il modulo UART
    UART_initModule(EUSCI_A2_BASE, &uartConfig);
    UART_enableModule(EUSCI_A2_BASE);

    // Abilita le interruzioni UART
    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableMaster();

    while(1)
    {
        RXData = UART_receiveData(EUSCI_A2_BASE);  // Leggi i dati ricevuti

        if(RXData=='\n') {
                    //UART_transmitData(EUSCI_A2_BASE, 1);
                    // Accendi il LED per indicare la ricezione di dati
                    GPIO_toggleOutputOnPin(GPIO_PORT_P1, LED_PIN);
                }
        // Metti il microcontrollore in modalità LPM0 per risparmiare energia
        //PCM_gotoLPM0InterruptSafe();
    }
}
//
//// ISR per la gestione delle interruzioni UART
//void EUSCIA2_IRQHandler(void)
//{
//    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
//
//    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
//    {
//        RXData = UCA2RXBUF;
//        RXData = UART_receiveData(EUSCI_A2_BASE);  // Leggi i dati ricevuti
//        if(RXData=='\n') {
//            //UART_transmitData(EUSCI_A2_BASE, 1);
//            // Accendi il LED per indicare la ricezione di dati
//            GPIO_toggleOutputOnPin(GPIO_PORT_P1, LED_PIN);
//        }
//
//        // Pulisci la flag di interruzione
//        UART_clearInterruptFlag(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
//    }
//}
