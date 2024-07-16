#ifndef SERVO_SUN_TRACKER_H_
#define SERVO_SUN_TRACKER_H_

#include "servo_control.h"
#include "Sun_az_alt.h"

/* *
 * Important!!!
 * before using servo call setup_servo_panel(), also to make everything work
 * call pwm_panel_control() in the timer isr that controls the servo motors
 * */


typedef struct Tservo_panel{
    Tservo azimuth, elevation;
}Tservo_panel;

Tservo_panel solar_panel;

/* *
 *
 * */
void setup_servo_panel(Tservo_panel* controller, uint16_t port_az, uint16_t pin_az, uint16_t port_el, uint16_t pin_el){
    configure_pwm_timer();
    setup_servo(&(controller->azimuth),port_az,pin_az,0);
    setup_servo(&(controller->elevation),port_el,pin_el,0);
}

void point_sun(Tservo_panel* controller, tm* time_info_ptr, float Lat, float Lon, float Alt){
    float _az, _el;
    if(time_info_ptr->tm_year < 100) time_info_ptr->tm_year+=100;
    SolarAzEl(time_info_ptr,Lat,Lon,Alt,&_az, &_el);
    set_servo_angle(&(controller->azimuth), (_az/2));
    set_servo_angle(&(controller->elevation), _el);
}

void pwm_panel_control(Tservo_panel* controller){
    pwm_servo_control(&(controller->azimuth));
    pwm_servo_control(&(controller->elevation));
}

#endif /* SERVO_SUN_TRACKER_H_ */
