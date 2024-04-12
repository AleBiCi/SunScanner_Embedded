#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <vector>
#include "NMEAParser.h"

#define MAX_ELEMENTS 14


// Sample GNRMC sentence for debugging
// line.assign("$GNRMC,140212.00,A,4604.18179,N,01108.18041,E,0.234,,210224,,,A*69\n");

bool replaceChar(char* m, const char c1, const char c2) {
    int l = strlen(m);
    bool found = false;
    for (size_t i = 0; i < l; i++)
    {
        if(m[i] == c1)
        {
            m[i] = c2;
            found = true;
        }
    }
    return found;
}

MessageNMEA::MessageNMEA() : m(nullptr) {
    std::cout << "Creato Messaggio NMEA vuoto" << std::endl;
}

MessageNMEA::MessageNMEA(char* mess) {
    std::cout << "Creato Messaggio NMEA" << std::endl;
    this->m = new char[strlen(mess) + 1];
    strcpy(this->m, mess);
}

MessageNMEA::~MessageNMEA() {
    std::cout << "Distrutto Messaggio NMEA" << std::endl;
    delete[] this->m;
}

MessageRMC::MessageRMC() : MessageNMEA() {
    std::cout << "Creato Messaggio RMC vuoto" << std::endl;
    this->message_ID = new char[7];
    this->checksum = new char[3];
}

MessageRMC::MessageRMC(char* mess) : MessageNMEA(mess) {
    std::cout << "Creato Messaggio RMC" << std::endl;
    this->message_ID = new char[7];
    this->checksum = new char[3];
}

MessageRMC::~MessageRMC() {
    std::cout << "Distrutto Messaggio RMC" << std::endl;
    delete[] this->message_ID;
    delete[] this->checksum;
}

void MessageNMEA::setMessage(char* mess) {
    std::cout << "Copia Messaggio" << std::endl;

    delete[] this->m;
    if(mess!=nullptr)
        this->m = new char[strlen(mess) + 1];
    strcpy(this->m, mess);
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
        /* No more delimiters; this is the last token.  */
        *stringp = NULL;
    return begin;
}

bool MessageRMC::validateChecksum() {
    /*
    Workflow to calculate NMEA checksum for a given message:
        1. compute XOR of every char of the message between '$' and '*' with themselves
        2. divide XOR into two nibbles (4 bit fields) (they correspond to the two characters of the checksum)
        3. convert the nibbles into hexadecimal characters checkByte1 and checkByte2
        4. compare the two checksum characters with the computed checkBytes
    */
    uint8_t checkByte1Original = (this->checksum[0] <= 0x09) ? (this->checksum[0] + '0') : (this->checksum[0] - 10 + 'A');
    uint8_t checkByte2Original = (this->checksum[1] <= 0x09) ? (this->checksum[1] + '0') : (this->checksum[1] - 10 + 'A');

    uint8_t XOR = 0;
    size_t limit = strchr(this->m, '*') - this->m; // limit points to first occurence of '*'

    for (size_t i = 1; i < limit; ++i) {    // start at i = 1 to skip '$'
        XOR^=this->m[i];                    // computes XOR
    }

    uint8_t nibble1 = (XOR & 0xF0) >> 4;    // separate the nibbles
    uint8_t nibble2 = XOR & 0x0F;

    uint8_t checkByte1 = (nibble1 <= 0x09) ? (nibble1 + '0') : (nibble1 - 10 + 'A');    // convert the nibbles into hex values
    uint8_t checkByte2 = (nibble2 <= 0x09) ? (nibble2 + '0') : (nibble2 - 10 + 'A');

    return (checkByte1 == checkByte1Original || checkByte2 == checkByte2Original); // returns true if XOR is same as checksum
}


bool MessageRMC::parseRMC() {
    bool error = false;
    // Initializing a copy of the message pointer from the second char (excludes the '$' start char)
    char* input_mess;
    if(this->m != nullptr) input_mess = strdup(this->m+1);
    // Vector of tokens
    std::vector<char*> tokens = {};
//    char* tokens[MAX_ELEMENTS];
//    for(int i=0; i<MAX_ELEMENTS; i++) {
//        tokens[i] = new char[20];
//    }

    // Pointer to start of each token
    char* tok_ptr;
    
    // Replacing '*' and '\n' chars in input message with ','
    bool found = replaceChar(input_mess, '*', ',');
    found = replaceChar(input_mess, '\n', '\0');

    int iter = 0;
    // Tokenizing input_mess with strsep using ',' as delimiter
    while ((tok_ptr = my_strsep(&input_mess, ",")) != nullptr) {
        // If the token is empty, push an empty string in the array
        if(strlen(tok_ptr)!=0) {
            strcpy(tokens[iter], tok_ptr);
        } else {
            strcpy(tokens[iter], " ");
        }
        iter++;
    }

    // Casting and assignment of checksum and status fields to check for sentence validity
    
    this->status = *tokens[2];
    (strlen(tokens[MAX_ELEMENTS-1])!=0) ? strcpy(this->checksum, tokens[MAX_ELEMENTS-1]) : strcpy(this->checksum, " ");
    
    // Checking for validity : calculated checksum chars have to match the originals + status field has to be valid (A = valid mess, V = not valid)
    if (!validateChecksum() || strcmp(&this->status, "A")!=0) {
        error = true; // if checksum comparison and validity status check didn't give positive result, return false
    } else return false;

    // CAST AND ASSIGN VALUES TO EACH FIELD
    // Generally, if value of tokens field is null -> "" (nothing) is copied in the corresponding attribute
    
    // ID
    (strlen(tokens[0])!=0) ? strcpy(this->message_ID, tokens[0]) : strcpy(this->message_ID, " ");

    // Latitude
    if(strlen(tokens[3])!=0) {
        this->latitude = std::strtod(tokens[3], nullptr);
        // Converting acquired DMM (Degrees, Decimal Minutes) latitude into DD (Decimal Degrees)
        double degrees = floor(this->latitude / 100.0);
        double minutes = this->latitude - degrees * 100.0;
        this->latitude = degrees + minutes / 60.0;
    } else this->latitude = -1;
    
    // Lat_dir (N,S)
    (strlen(tokens[4])!=0) ? this->lat_dir = *tokens[4] : this->lat_dir = ' ';
    
    // Longitude
    if(strlen(tokens[5])!=0) {
        this->longitude = std::strtod(tokens[5], nullptr);
        // Converting acquired DMM (Degrees, Decimal Minutes) latitude into DD (Decimal Degrees)
        double degrees = floor(this->longitude / 100.0);
        double minutes = this->longitude - degrees * 100.0;
        this->longitude = degrees + minutes / 60.0;
    } else this->longitude = -1;

    // Lon_dir (E,W)
    (strlen(tokens[6])!=0) ? this->lon_dir = *tokens[6] : this->lon_dir = ' ';
    
    char* tmp = new char[3]; // Temporary char* to store the 2 characters corresponding to hh, mm, ss/dd, mo, yy + terminator
    
    // Time
    if(strlen(tokens[1])!=0) {
            // Hours
        strncpy(tmp, tokens[1], 2); // Copying two chars at a time to tmp from tokens[1] (time field)
        tmp[3] = '\0'; // Adding terminator character to tmp
        this->time.tm_hour = std::strtol(tmp, nullptr, 10); // Converting the two characters to integer and assigning that to time.tm_hour
        strcpy(tmp, ""); // Flush tmp

            // Minutes
        strncpy(tmp, tokens[1]+2, 2);
        tmp[3] = '\0';
        this->time.tm_min = std::strtol(tmp, nullptr, 10);
        strcpy(tmp, "");

            // Seconds
        strncpy(tmp, tokens[1]+4, 2);
        tmp[3] = '\0';
        this->time.tm_sec = std::strtol(tmp, nullptr, 10);
        strcpy(tmp, "");
    }

    // Date
    if(strlen(tokens[9])!=0) {
            // Day
        strncpy(tmp, tokens[9], 2); // Copying two chars at a time to tmp from tokens[9] (day field)
        tmp[3] = '\0'; // Adding terminator character to tmp
        this->time.tm_mday = std::strtol(tmp, nullptr, 10); // Converting the two characters to integer and assigning that to time.tm_mday
        strcpy(tmp, ""); // Flush tmp

            // Month
        strncpy(tmp, tokens[9]+2, 2);
        tmp[3] = '\0';
        this->time.tm_mon = std::strtol(tmp, nullptr, 10) + 1; // Month is [0-11]
        strcpy(tmp, "");

            // Year
        strncpy(tmp, tokens[9]+4, 2);
        tmp[3] = '\0';
        this->time.tm_year = std::strtol(tmp, nullptr, 10) + 2000; // Year is from 1900
        strcpy(tmp, "");
    }

    // Mode
    (strlen(tokens[12])!=0) ? this->mode = *tokens[12] : this->mode = ' ';
    
    delete[] input_mess;
    return (error);
}

void MessageRMC::printSent() {
    std::cout << "ID : " << this->message_ID << std::endl;
    std::cout << "Time : " << this->time.tm_hour << ":" << this->time.tm_min << ":" << this->time.tm_sec << std::endl;
    std::cout << "Date : " << this->time.tm_mday << "/" << this->time.tm_mon << "/" << this->time.tm_year << std::endl;
    std::cout << "Latitude : " << this->latitude << std::endl;
    std::cout << "Lat Dir : " << this->lat_dir << std::endl;
    std::cout << "Longitude : " << this->longitude << std::endl;
    std::cout << "Lon Dir : " << this->lon_dir << std::endl;
    std::cout << "Mode : " << this->mode << std::endl;
    std::cout << "Checksum : " << this->checksum << std::endl;
}
