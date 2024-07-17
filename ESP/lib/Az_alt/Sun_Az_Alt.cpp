#include "Sun_az_alt.h"

void SolarAzEl(tm* time_info_ptr, float Lat, float Lon, float Alt, float* Az, float* El) {
	time_t utc_time_point = mktime(time_info_ptr);
    float jd = julian_day(utc_time_point);
    float d = jd - 2451543.5;
    
    // Keplerian Elements for the Sun(geocentric)
    float w = 282.9404 + 4.70935e-5*d; // (longitude of perihelion degrees)
    // a = 1.000000; % (mean distance, a.u.)
    float e = 0.016709 - 1.151e-9*d; // (eccentricity)
    float M = fmod(356.0470 + 0.9856002585*d, 360.0); // (mean anomaly degrees)
        
    float L = w + M; // (Sun's mean longitude degrees)
    float oblecl = 23.4393 - 3.563e-7*d; // (Sun's obliquity of the ecliptic)
    // auxiliary angle
    float  E = M + (180 / M_PI)*e*sin(M*(M_PI / 180))*(1 + e*cos(M*(M_PI / 180)));
    // rectangular coordinates in the plane of the ecliptic(x axis toward perhilion)
    float x = cos(E*(M_PI / 180)) - e;
    float y = sin(E*(M_PI / 180))*sqrt(1 - pow(e, 2));
    // find the distance and true anomaly
    float r = sqrt(pow(x,2) + pow(y,2));
    float v = atan2(y, x)*(180 / M_PI);
    // find the longitude of the sun
    float lon = v + w;
    // compute the ecliptic rectangular coordinates
    float xeclip = r*cos(lon*(M_PI / 180));
    float yeclip = r*sin(lon*(M_PI / 180));
    float zeclip = 0.0;
    //rotate these coordinates to equitorial rectangular coordinates
    float xequat = xeclip;
    float yequat = yeclip*cos(oblecl*(M_PI / 180)) + zeclip * sin(oblecl*(M_PI / 180));
    float zequat = yeclip*sin(23.4406*(M_PI / 180)) + zeclip * cos(oblecl*(M_PI / 180));
    // convert equatorial rectangular coordinates to RA and Decl:
    r = sqrt(pow(xequat, 2) + pow(yequat, 2) + pow(zequat, 2)) - (Alt / 149598000); //roll up the altitude correction
    float RA = atan2(yequat, xequat)*(180 / M_PI);
    float delta = asin(zequat / r)*(180 / M_PI);
    // Following the RA DEC to Az Alt conversion sequence explained here :
    // http ://www.stargazing.net/kepler/altaz.html
    //	Find the J2000 value
    //	J2000 = jd - 2451545.0;
    //hourvec = datevec(UTC);
    //UTH = hourvec(:, 4) + hourvec(:, 5) / 60 + hourvec(:, 6) / 3600;
    tm *ptm;
    ptm = gmtime(&utc_time_point);
    float UTH = (float)ptm->tm_hour + (float)ptm->tm_min / 60 + (float)ptm->tm_sec / 3600;
    // Calculate local siderial time
    float GMST0 = fmod(L + 180, 360.0) / 15;
    float SIDTIME = GMST0 + UTH + Lon / 15;
    
    // Replace RA with hour angle HA
    float HA = (SIDTIME*15 - RA);
    // convert to rectangular coordinate system
    x = cos(HA*(M_PI / 180))*cos(delta*(M_PI / 180));
    y = sin(HA*(M_PI / 180))*cos(delta*(M_PI / 180));
    float z = sin(delta*(M_PI / 180));
    // rotate this along an axis going east - west.
    float xhor = x*cos((90 - Lat)*(M_PI / 180)) - z*sin((90 - Lat)*(M_PI / 180));
    float yhor = y;
    float zhor = x*sin((90 - Lat)*(M_PI / 180)) + z*cos((90 - Lat)*(M_PI / 180));
    
    // Find the h and AZ
    *Az = fmod(atan2(yhor, xhor)*(180 / M_PI) + 180, 360.0);
    *El = asin(zhor)*(180 / M_PI);
}

float julian_day(time_t utc_time_point){
    // Extract UTC Time
    struct tm* tm = gmtime(&utc_time_point);
    float year = tm->tm_year + 1900;
    float month = tm->tm_mon + 1;
    float day = tm->tm_mday;
    float hour = tm->tm_hour + 2;
    float min = tm->tm_min;
    float sec = tm->tm_sec;
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    float jd = floor(365.25*(year + 4716.0)) + floor(30.6001*(month + 1.0)) + 2.0 -
        floor(year / 100.0) + floor(floor(year / 100.0) / 4.0) + day - 1524.5 +
        (hour + min / 60 + sec / 3600) / 24;
    return jd;
}

