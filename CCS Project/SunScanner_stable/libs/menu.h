#ifndef LIBS_MENU_H_
#define LIBS_MENU_H_

#include <scenes.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"

#define MAX_ADC14 16383

/* Graphic library context */
Graphics_Context g_sContext;

/* ADC results buffer */
static uint16_t resultsBuffer[2];
uint16_t analogRead;

char *home_menu_names[4] = { (char*)"Specs", (char*)"GPS", (char*)"Move", (char*)"History"};
char *home_menu_move[3] = { (char*)"Automatic", (char*)"Zenit", (char*)"Manual"};

int v_out = 0, phi = 0, theta = 0;


char sun_rise_time[6] = "05:41";
char sun_set_time[6] = "21:03";

int alt1 = 1, depth = 1, alt2 = 1;
int alt1Prev, alt2Prev, depthPrev;

/* ADC results buffer + Joystick stuff */
static uint16_t resultsBuffer[2];
uint8_t joystick_state[2] = {false, false};
const uint16_t resting = 8191, threshold = 1500, min_val = 0, max_val = 16383;
uint8_t count = 0;

/* FUNCTIONS */

// Returns false if going left/down and true if going right/up
int active_pos(uint16_t value){
    if(value < min_val + threshold) return 1;
    else if(value > (max_val - threshold)) return 2;
    return 0;
}

// Returns true if the joystick is in the resting position
bool resting_pos(uint16_t value){
    return (value < resting + threshold && value > (resting - threshold));
}

void drawRectangle(Graphics_Context *g_sContext, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    Graphics_Rectangle rect = { x1, y1, x2, y2 };
    Graphics_drawRectangle(g_sContext, &rect);
}

void drawText(Graphics_Context *g_sContext, int x, int y, const char *text) {
    Graphics_drawString(g_sContext, (int8_t *)text, -1, x, y, false);
}


float realTension(int read_value) {
    return v_out = (read_value * 6.6) / MAX_ADC14; // Real tension on panel is between 0V - 6.6V, and our read value is between 0 - 2^14
}


void pacMan_1_n(int limit, int *val){
    if(*val<1){
        *val = limit;
    }
    if(*val>limit){
        *val = 1;
    }
}

void menu_keep_range(int limit, int *val){
    if(*val<1){
        *val = 1;
    }
    if(*val>limit){
        *val = limit;
    }
}

//----------------------------------------------------------------------------------------------
void drawMenuElement(Graphics_Context *g_sContext, int x, int y, const Graphics_Image *image, const char* name) {
    Graphics_drawImage(g_sContext, image, x, y);
    // Imposta il cursore e disegna il testo
    Graphics_drawString(g_sContext, (int8_t *)name, -1, x + 35, y + 8, OPAQUE_TEXT);
}


//----------------------------------------------------------------------------------------------

void drawMenu1(int i) {
//  Graphics_clearDisplay(&g_sContext);

  switch (i) {
    case 1:
      drawMenuElement(&g_sContext, 8, 8, &HISTORY_UNCOMP, home_menu_names[3]);
      drawRectangle(&g_sContext, 4, 47, 124, 81);
      drawMenuElement(&g_sContext, 8, 51, &SPECS_UNCOMP, home_menu_names[0]);
      drawMenuElement(&g_sContext, 8, 94, &GPS_UNCOMP, home_menu_names[1]);
      break;

    case 2:
      drawMenuElement(&g_sContext, 8, 8, &SPECS_UNCOMP, home_menu_names[0]);
      drawRectangle(&g_sContext, 4, 47, 124, 81);
      drawMenuElement(&g_sContext, 8, 51, &GPS_UNCOMP, home_menu_names[1]);
      drawMenuElement(&g_sContext, 8, 94, &MOVE_UNCOMP, home_menu_names[2]);
      break;

    case 3:
      drawMenuElement(&g_sContext, 8, 8, &GPS_UNCOMP, home_menu_names[1]);
      drawRectangle(&g_sContext, 4, 47, 124, 81);
      drawMenuElement(&g_sContext, 8, 51, &MOVE_UNCOMP, home_menu_names[2]);
      drawMenuElement(&g_sContext, 8, 94, &HISTORY_UNCOMP, home_menu_names[3]);
      break;

    case 4:
      drawMenuElement(&g_sContext, 8, 8, &MOVE_UNCOMP, home_menu_names[2]);
      drawRectangle(&g_sContext, 4, 47, 124, 81);
      drawMenuElement(&g_sContext, 8, 51, &HISTORY_UNCOMP, home_menu_names[3]);
      drawMenuElement(&g_sContext, 8, 94, &SPECS_UNCOMP, home_menu_names[0]);
      break;
  }
}

void drawMenu2(int i, int sel) {
  char tension_string[10]; tension_string[0] = '\0';
  char phi_string[5]; phi_string[0] = '\0';
  char theta_string[5]; theta_string[0] = '\0';

  switch (i) {
    //SPECS
    case 1:
      //v_out = Tensione(PIN_1);
      sprintf(tension_string, "%5f",((float)analogRead / (float)MAX_ADC14 * 6.6)/*realTension(v_out)*/);
      drawMenuElement(&g_sContext, 8, 51, &VOLTAGE_UNCOMP, tension_string);
      break;

    //GPS
    case 2:
      sprintf(phi_string, "%d", phi*2); // *2 because we used half the value to control the servos
      drawMenuElement(&g_sContext, 8, 8, &PHI_UNCOMP, phi_string);

      sprintf(theta_string, "%d", theta);
      drawMenuElement(&g_sContext, 8, 51, &THETA_UNCOMP, theta_string);
      break;

    //MOVE
    case 3:
      switch (sel) {
        case 1:
          drawMenuElement(&g_sContext, 8, 8, &JOYSTICK_UNCOMP, home_menu_move[2]);
          drawRectangle(&g_sContext, 4, 47, 124, 81);
          drawMenuElement(&g_sContext, 8, 51, &MOVE_UNCOMP, home_menu_move[0]);
          drawMenuElement(&g_sContext, 8, 94, &MOVE_1_UNCOMP, home_menu_move[1]);
          break;

        case 2:
          drawMenuElement(&g_sContext, 8, 8, &MOVE_UNCOMP, home_menu_move[0]);
          drawRectangle(&g_sContext, 4, 47, 124, 81);
          drawMenuElement(&g_sContext, 8, 51, &MOVE_1_UNCOMP, home_menu_move[1]);
          drawMenuElement(&g_sContext, 8, 94, &JOYSTICK_UNCOMP, home_menu_move[2]);
          break;

        case 3:
          drawMenuElement(&g_sContext, 8, 8, &MOVE_1_UNCOMP, home_menu_move[1]);
          drawRectangle(&g_sContext, 4, 47, 124, 81);
          drawMenuElement(&g_sContext, 8, 51, &JOYSTICK_UNCOMP, home_menu_move[2]);
          drawMenuElement(&g_sContext, 8, 94, &MOVE_UNCOMP, home_menu_move[0]);
          break;
      }

      break;

    //HISTORY
    case 4:
      drawMenuElement(&g_sContext, 8, 8, &MAX_IRRADIANCE_UNCOMP, sun_rise_time);
      drawMenuElement(&g_sContext, 8, 51, &MIN_IRRADIANCE_UNCOMP, sun_set_time);

      break;
  }
}

void drawMenu3(int i) {
//  Graphics_clearDisplay(&g_sContext);

  switch (i) {
    case 1:
      Graphics_clearDisplay(&g_sContext);
      break;

    case 2:
      Graphics_clearDisplay(&g_sContext);
      break;

    case 3:
      Graphics_drawImage(&g_sContext, &UP_ARROW_UNCOMP, 56,28);
      drawText(&g_sContext, 54, 21, "UP");

      Graphics_drawImage(&g_sContext, &BACK_ARROW_UNCOMP, 25,56);
      drawText(&g_sContext, 19, 42, "LEFT");

      Graphics_drawImage(&g_sContext, &DOWN_ARROW_UNCOMP, 56,84);
      drawText(&g_sContext, 49, 114, "DOWN");

      Graphics_drawImage(&g_sContext, &CHANGE_ARROW, 87,56);
      drawText(&g_sContext, 89, 49, "RIGHT");

      Graphics_drawImage(&g_sContext, &SQ_DOT_UNCOMP, 60,60);

      break;
  }
}


void selMenu(int i, int j, int sel){

  switch(j){
  case 1:
    drawMenu1(i);
    break;
  case 2:
    drawMenu2(i,sel);
    break;
  case 3:
    drawMenu3(sel);
    break;
  }
}


void startupAnimation(const Graphics_Context *gContext, const Graphics_Image *frames[], int nFrames) {
    int i, j;
    for(i=0; i<nFrames; i++) {
        Graphics_drawImage(gContext, frames[i], 0, 0);
        for(j=100000; j>0; j--);
        if(i!=nFrames-1) Graphics_clearDisplay(gContext);
    }
    for(j=10000000; j>0; j--);
    Graphics_clearDisplay(gContext);
    for(i=nFrames; i>0; i--) {
        Graphics_drawImage(gContext, frames[i], 0, 0);
        for(j=100000; j>0; j--);
        Graphics_clearDisplay(gContext);
    }
}

void setup_menu(){
    GPIO_setAsInputPin(GPIO_PORT_P5, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT5);

    GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN5);
    Interrupt_enableInterrupt(INT_PORT3);

    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

    /* Clear screen */
    Graphics_clearDisplay(&g_sContext);

    /* ADC14 */
    /* Configures Pin 6.0 and 4.4 as ADC input */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    /*pin per lettura valori pannello*/
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN6, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initializing ADC (ADCOSC/64/8) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_32, ADC_DIVIDER_8, 0);

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM1 (A15, A9)  with repeat)
     * with internal 2.5v reference */
    MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0,
                                        ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                        ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);

    MAP_ADC14_configureConversionMemory(ADC_MEM1,
                                        ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                        ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    MAP_ADC14_configureConversionMemory(ADC_MEM2,
                                        ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                        ADC_INPUT_A14, ADC_NONDIFFERENTIAL_INPUTS);

    /* Enabling the interrupt when a conversion on channel 1 (end of sequence)
     *  is complete and enabling conversions */
    ADC14_enableInterrupt(ADC_INT1);
    ADC14_enableInterrupt(ADC_INT2);
    MAP_Interrupt_enableInterrupt(INT_ADC14);

    /* Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();


    alt1Prev = alt1;
    alt2Prev = alt2;
    depthPrev = depth;

    /* STARTUP ANIMATION */
    //const Graphics_Image* frame_list[7] = {&frame1_UNCOMP, &frame2_UNCOMP, &frame3_UNCOMP, &frame4_UNCOMP, &frame5_UNCOMP, &frame6_UNCOMP, &frame7_UNCOMP};
    //startupAnimation(&g_sContext, frame_list, 7);

    selMenu(1, 1, 1);
}

#endif /* LIBS_MENU_H_ */
