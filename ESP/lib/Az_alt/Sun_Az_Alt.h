#ifndef __SUN_AZ_ALT_H__
#define __SUN_AZ_ALT_H__

#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif /* M_PI */

typedef struct tm tm;

/*
Function that return current julian date (decimal)
*/
float julian_day(time_t utc_time_point);

/*
Function that calculate sun Azimuth and Altitude given a place Latitude, Longitude, Altitude
and time.
The result is pointed by "Az" and "El"
*/
void SolarAzEl(tm* time_info_ptr, float Lat, float Lon, float Alt, float* Az, float* El);

#endif //__SUN_AZ_ALT_H__

