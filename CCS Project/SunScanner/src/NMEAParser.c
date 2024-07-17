#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h> // for bool
#include <stdlib.h> // for std::strtod strtol
#include <stdint.h> // for uintn_t

#include "gps/NMEAParser.h"

#define MAX_ELEMENTS 14
#define MAX_BUFF_EL 100

// Sample GNRMC sentence for debugging
// line.assign("$GNRMC,140212.00,A,4604.18179,N,01108.18041,E,0.234,,210224,,,A*69\n");

bool replaceChar(char* m, const char c1, const char c2) {
    int l = strlen(m);
    bool found = false;
    size_t i;
    for (i = 0; i < l; i++)
    {
        if(m[i] == c1)
        {
            m[i] = c2;
            found = true;
        }
    }
    return found;
}

void initMessageRMC(MessageRMC *mess) {
    mess->m[0] = '\0';
    mess->message_ID[0] = '\0';
    mess->checksum[0] = '\0';
}

void setMessage(MessageRMC *mess, char* m) {
    mess->m[0] = '\0';
    if(m!=NULL) strcpy(mess->m, m);
}

// Code taken directly from glibc source, copy-pasted to make it portable since strsep() is only standard in POSIX systems
char *my_strsep(char **stringp, const char *delim)
{
    char *begin, *end;
    begin = *stringp;
    if (begin == NULL)
        return NULL;
    /* Find the end of the token.  */
    end = begin + strcspn(begin, delim);
    if (*end)
    {
        /* Terminate the token and set *STRINGP past NUL character.  */
        *end++ = '\0';
        *stringp = end;
    }
    else
        /* No more delimiters; mess is the last token.  */
        *stringp = NULL;
    return begin;
}

bool validateChecksum(MessageRMC *mess) {
    /*
    Workflow to calculate NMEA checksum for a given message:
        1. compute XOR of every char of the message between '$' and '*' with themselves
        2. divide XOR into two nibbles (4 bit fields) (they correspond to the two characters of the checksum)
        3. convert the nibbles into hexadecimal characters checkByte1 and checkByte2
        4. compare the two checksum characters with the computed checkBytes
    */

    uint8_t  checkByte1Original = mess->checksum[0];
    uint8_t checkByte2Original = mess->checksum[1];

    uint8_t XOR = 0;
    size_t limit = strchr(mess->m, '*') - mess->m; // limit points to first occurence of '*'
    size_t i;
    for (i = 1; i < limit; ++i) {    // start at i = 1 to skip '$'
        XOR^=mess->m[i];                    // computes XOR
    }

    uint8_t nibble1 = (XOR & 0xF0) >> 4;    // separate the nibbles
    uint8_t nibble2 = XOR & 0x0F;

    uint8_t checkByte1 = (nibble1 <= 0x09) ? (nibble1 + '0') : (nibble1 - 10 + 'A');    // convert the nibbles into hex values
    uint8_t checkByte2 = (nibble2 <= 0x09) ? (nibble2 + '0') : (nibble2 - 10 + 'A');

    return (checkByte1 == checkByte1Original && checkByte2 == checkByte2Original); // returns true if XOR is same as checksum
}


bool parseRMC(MessageRMC *mess) {
    // Initializing a copy of the message pointer from the second char (excludes the '$' start char)
    char* input_mess = NULL;
    if(strlen(mess->m)!=0) input_mess = strdup(mess->m+1);
    // Vector of tokens
    char *tokens[MAX_BUFF_EL];
    int i;
    for (i = 0; i < MAX_BUFF_EL; i++) {
        tokens[i] = (char *)malloc(MAX_BUFF_EL * sizeof(char));
    }

    // Pointer to start of each token
    char* tok_ptr = NULL;
    
    // Replacing '*' and '\n' chars in input message with ','
    bool found = replaceChar(input_mess, '*', ',');
    found = replaceChar(input_mess, '\n', ',');

    int iter = 0;
    // Tokenizing input_mess with strsep using ',' as delimiter
    while ((tok_ptr = my_strsep(&input_mess, ",")) != NULL) {
        // If the token is empty, push an empty string in the array
        if(strlen(tok_ptr)!=0) {
            strcpy(tokens[iter], tok_ptr);
        } else {
            strcpy(tokens[iter], " ");
        }
        iter++;
    }

    // Casting and assignment of checksum and status fields to check for sentence validity
    
    mess->status = tokens[2][0];
    (strlen(tokens[MAX_ELEMENTS-1])!=0) ? strcpy(mess->checksum, tokens[MAX_ELEMENTS-1]) : strcpy(mess->checksum, " ");
    
    // Checking for validity : calculated checksum chars have to match the originals + status field has to be valid (A = valid mess, V = not valid)
    if(!validateChecksum(mess) || strcmp(&mess->status, "A")!=0) {
        //return false; // if checksum comparison and validity status check didn't give positive result, return false
    }

    // CAST AND ASSIGN VALUES TO EACH FIELD
    // Generally, if value of tokens field is null -> "" (nothing) is copied in the corresponding attribute
    
    // ID
    (strlen(tokens[0])!=0) ? strcpy(mess->message_ID, tokens[0]) : strcpy(mess->message_ID, " ");

    // Latitude
    if(strlen(tokens[3])!=0) {
        mess->latitude = strtod(tokens[3], NULL);
        // Converting acquired DMM (Degrees, Decimal Minutes) latitude into DD (Decimal Degrees)
        double degrees = floor(mess->latitude / 100.0);
        double minutes = mess->latitude - degrees * 100.0;
        mess->latitude = degrees + minutes / 60.0;
    } else mess->latitude = -1;
    
    // Lat_dir (N,S)
    if(strlen(tokens[4])!=0) {
        mess->lat_dir = *tokens[4];
    } else mess->lat_dir = ' ';
    
    // Longitude
    if(strlen(tokens[5])!=0) {
        mess->longitude = strtod(tokens[5], NULL);
        // Converting acquired DMM (Degrees, Decimal Minutes) latitude into DD (Decimal Degrees)
        double degrees = floor(mess->longitude / 100.0);
        double minutes = mess->longitude - degrees * 100.0;
        mess->longitude = degrees + minutes / 60.0;
    } else mess->longitude = -1;

    // Lon_dir (E,W)
    if(strlen(tokens[6])!=0) {
        mess->lon_dir = *tokens[6];
     } else mess->lon_dir = ' ';
    
    char tmp[3]; // Temporary char* to store the 2 characters corresponding to hh, mm, ss/dd, mo, yy + terminator
    
    // Time
    if(strlen(tokens[1])!=0) {
            // Hours
        strncpy(tmp, tokens[1], 2); // Copying two chars at a time to tmp from tokens[1] (time field)
        tmp[2] = '\0'; // Adding terminator character to tmp
        mess->time.tm_hour = strtol(tmp, NULL, 10) + 2; // Converting the two characters to integer and assigning that to time.tm_hour, +2hours
        strcpy(tmp, ""); // Flush tmp

            // Minutes
        strncpy(tmp, tokens[1]+2, 2);
        tmp[2] = '\0';
        mess->time.tm_min = strtol(tmp, NULL, 10);
        strcpy(tmp, "");

            // Seconds
        strncpy(tmp, tokens[1]+4, 2);
        tmp[2] = '\0';
        mess->time.tm_sec = strtol(tmp, NULL, 10);
        strcpy(tmp, "");
    }

    // Date
    if(strlen(tokens[9])!=0) {
            // Day
        strncpy(tmp, tokens[9], 2); // Copying two chars at a time to tmp from tokens[9] (day field)
        tmp[2] = '\0'; // Adding terminator character to tmp
        mess->time.tm_mday = strtol(tmp, NULL, 10); // Converting the two characters to integer and assigning that to time.tm_mday
        strcpy(tmp, ""); // Flush tmp

            // Month
        strncpy(tmp, tokens[9]+2, 2);
        tmp[2] = '\0';
        mess->time.tm_mon = strtol(tmp, NULL, 10) - 1; // Month is [0-11]
        strcpy(tmp, "");

            // Year
        strncpy(tmp, tokens[9]+4, 2);
        tmp[2] = '\0';
        mess->time.tm_year = strtol(tmp, NULL, 10); // Year is from 1900
        mess->time.tm_isdst = true;
        strcpy(tmp, "");
    }

    // Mode
    if(strlen(tokens[12])!=0) {
        mess->mode = *tokens[12];
    } else mess->mode = ' ';
    for(i=0; i<MAX_BUFF_EL; i++) {
        free(tokens[i]);
    }

    return 0;
}

void printSent(MessageRMC *mess) {
    // std::cout << "ID : " << mess->message_ID << std::endl;
    // std::cout << "Time : " << mess->time.tm_hour << ":" << mess->time.tm_min << ":" << mess->time.tm_sec << std::endl;
    // std::cout << "Date : " << mess->time.tm_mday << "/" << mess->time.tm_mon << "/" << mess->time.tm_year << std::endl;
    // std::cout << "Latitude : " << mess->latitude << std::endl;
    // std::cout << "Lat Dir : " << mess->lat_dir << std::endl;
    // std::cout << "Longitude : " << mess->longitude << std::endl;
    // std::cout << "Lon Dir : " << mess->lon_dir << std::endl;
    // std::cout << "Mode : " << mess->mode << std::endl;
    // std::cout << "Checksum : " << mess->checksum << std::endl;
}
