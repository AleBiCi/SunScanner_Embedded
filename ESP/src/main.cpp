#include <Arduino.h>
#include "Sun_Az_Alt.h"
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#define START_SEQ 0xFF
#define STOP_SEQ 0xFF
#define IN_BUFF_SIZE 28
#define OUT_BUFF_SIZE 18 // Total size of the sentence
#define OUT_DATA_SIZE 14 // Size of data part of the sentence
#define POVO_ALT 360.0 // Altitude over sea level
/*
*     0-1           2-3      4-5        6-7       8-9      10-11      12-13     14-19     20-25      26-27     
* | START_BYTE | SEC (2) | MIN (2) | HOUR (2) | DAY (2) | MON (2) | YEAR (2) | LAT (6) | LON (6) | STOP_BYTE |
*
* w/ END_BYTE = START_BYTE == 11111111
*
*/

// Initialize Telegram BOT
#define BOTtoken "X"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
//                      Ale         Gerry
#define AUTH_USERS {"657098289", "984967381"}


// Define a structure to hold the parsed data
struct DataPacket {
  tm time_info;
  float lat;
  float lon;
};

DataPacket packet;

float azimuth  = 0., elevation = 0.;

// Buffer to store incoming data
uint8_t RXBuffer[IN_BUFF_SIZE];
uint8_t bufferIndex = 0;
bool startSequenceDetected = false;

//TELEGRAM BOT VARIABLES
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const char* ssid = "X";
const char* password = "X"; // REPLACE

int botRequestDelay = 1000;
bool ledState = LOW;

unsigned long lastTimeBotRan;

char* RX_buffer = nullptr;
char* TX_buffer = nullptr;

void setupBlink();
void receiveData();
void processData(uint8_t data[]);
void parseBuffer();
void sendDataMSP();

void connectionSetup();


//uint8_t test_send[BUFF_SIZE] = {START_SEQ, START_SEQ, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STOP_SEQ, STOP_SEQ};
// uint8_t data[DATA_SIZE] = {0, 0, 5, 3, 1, 5, 1, 6, 0, 7, 2, 4, 4, 6, 1, 1, 0, 0, 4, 6, 1, 1, 0, 0};


// Define the data to be sent
// uint8_t data[DATA_SIZE];
// Initialize the buffer with start and stop sequences
char to_send[OUT_BUFF_SIZE] = {START_SEQ, START_SEQ, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STOP_SEQ, STOP_SEQ};

//                              START                       SEC       MIN       HOUR      DAY       MON       YEAR      LAT                           LON                           STOP
//uint8_t to_send[BUFF_SIZE] = {START_SEQ, START_SEQ, '0', '0', '5', '3', '1', '5', '1', '6', '0', '7', '2', '4', '4', '6', '1', '1', '0', '0', '4', '6', '1', '1', '0', '0', START_SEQ, START_SEQ};


void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(9600);
  Serial1.begin(9600);

  // Allow some time for serial communication to initialize
  delay(1000);
  
  // Print initial message to indicate setup has completed
  Serial.println("ESP32-S3 is ready to receive data...");
  Serial.flush();
  int i=0;

  delay(1000);
  // To setup ESP WiFI connection
  // connectionSetup();
}

void loop() {
  // Check if there is data available in the serial buffer
  while (Serial1.available()) {
    // Read the incoming byte
    uint8_t byteReceived = Serial1.read();
    
    // Print the received byte in hexadecimal format for debugging
    Serial.print("Byte received: ");
    Serial.println(byteReceived, HEX);

    // State machine to detect the start sequence and fill the buffer
    if (startSequenceDetected) {
      // Store the byte in the buffer
      RXBuffer[bufferIndex++] = byteReceived;
      
      // Check if buffer is full
      if (bufferIndex == IN_BUFF_SIZE) {
        // Check for the stop sequence
        if (RXBuffer[IN_BUFF_SIZE-2] == STOP_SEQ && RXBuffer[IN_BUFF_SIZE-1] == STOP_SEQ) {
          Serial.println("Complete sentence received. Parsing...");
          parseBuffer();
        } else {
          Serial.println("Stop sequence not found. Discarding data.");
        }
        // Reset the buffer index
        bufferIndex = 0;
        startSequenceDetected = false;
      }
    } else if (byteReceived == START_SEQ) {
      // Detect start sequence
      if (bufferIndex == 0 || RXBuffer[bufferIndex - 1] == START_SEQ) {
        // Store the byte in the buffer
        RXBuffer[bufferIndex++] = byteReceived;
        
        // Check if the start sequence is detected
        if (bufferIndex == 2) {
          startSequenceDetected = true;
          bufferIndex = 2; // Reset buffer index to account for already stored start sequence
          Serial.println("Start sequence detected.");
        }
      } else {
        // Reset if not matching start sequence
        bufferIndex = 0;
      }
    } else {
      // Reset buffer index if not start sequence
      bufferIndex = 0;
    }
  } // If nothing in UART BUFFER control the BOT

  // CAN'T CONNECT TO BOT
  
  // if (millis() > lastTimeBotRan + botRequestDelay)
  // {
  //   int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  //   while (numNewMessages)
  //   {
  //     Serial.println("got response");
  //     handleNewMessages(numNewMessages);
  //     numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  //   }
  //   lastTimeBotRan = millis();
  // }

  // Add a small delay to avoid flooding the serial output
  delay(10);
}

void parseBuffer() {
  // Parse the buffer into the DataPacket structure
  int index = 2; // Start after the start sequence
  packet.time_info.tm_sec = (RXBuffer[index] * 10) + RXBuffer[index+1];
  index += 2;
  packet.time_info.tm_min = (RXBuffer[index] * 10) + RXBuffer[index+1];
  index += 2;
  packet.time_info.tm_hour = (RXBuffer[index] * 10) + RXBuffer[index+1];
  index += 2;
  packet.time_info.tm_mday = (RXBuffer[index] * 10) + RXBuffer[index+1];
  index += 2;
  packet.time_info.tm_mon = (RXBuffer[index] * 10) + RXBuffer[index+1];
  index += 2;
  packet.time_info.tm_year = (RXBuffer[index] * 10) + RXBuffer[index+1] + 100; // Since julian_day expects years from 1900
  index += 2;
  packet.lat = (RXBuffer[index] * 10) + RXBuffer[index+1] + RXBuffer[index+1]*0.5 + RXBuffer[index+1]*0.25 + RXBuffer[index+1]*0.125 + RXBuffer[index+1]*0.0625;
  index += 6;
  packet.lon = (RXBuffer[index] * 10) + RXBuffer[index+1] + RXBuffer[index+1]*0.5 + RXBuffer[index+1]*0.25 + RXBuffer[index+1]*0.125 + RXBuffer[index+1]*0.0625;
  
  // Print parsed data
  Serial.print("SEC: "); Serial.println(packet.time_info.tm_sec);
  Serial.print("MIN: "); Serial.println(packet.time_info.tm_min);
  Serial.print("HOUR: "); Serial.println(packet.time_info.tm_hour);
  Serial.print("DAY: "); Serial.println(packet.time_info.tm_mday);
  Serial.print("MON: "); Serial.println(packet.time_info.tm_mon);
  Serial.print("YEAR: "); Serial.println(packet.time_info.tm_year);
  Serial.print("LAT: "); Serial.println(packet.lat);
  Serial.print("LON: "); Serial.println(packet.lon);

  SolarAzEl(&(packet.time_info), packet.lat, packet.lon, 360.0, &azimuth, &elevation);
  Serial.print("AZ: "); Serial.println(azimuth);
  Serial.print("EL: "); Serial.println(elevation);

  sendDataMSP();
}

void sendDataMSP() {
  char az_str[7]; sprintf(az_str, "%6.4f", azimuth); // print 6 total digits, round off to 4 decimals
  Serial.println(az_str);
  char el_str[7]; sprintf(el_str, "%6.4f", elevation);
  Serial.println(el_str);
  
  memcpy(&to_send[2], az_str, 7);
  memcpy(&to_send[9], el_str, 7);
  for(int i=0; i<OUT_BUFF_SIZE; i++) {
    Serial1.print(to_send[i]);
  }
}




void connectionSetup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, ledState);

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
#ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
#endif
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
}


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    
    bool auth = false;
    for(auto a : AUTH_USERS) {
      if(chat_id == a) {
        auth=true;
        break;
      }
    }
    if(!auth) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    } else auth=!auth;

    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/led_on to turn GPIO ON \n";
      welcome += "/led_off to turn GPIO OFF \n";
      welcome += "/led_state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(LED_BUILTIN, ledState);
    }
    
    if (text == "/led_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(LED_BUILTIN, ledState);
    }
    
    if (text == "/led_state") {
      if (digitalRead(LED_BUILTIN)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }

    if(text == "/status") {
      // SEND STATUS VARIABLES saved locally
      char lat_str[20]; sprintf(lat_str, "LAT: %f\n", packet.lat);
      bot.sendMessage(chat_id, lat_str);
      char lon_str[20]; sprintf(lon_str, "LON: %f\n", packet.lon);
      bot.sendMessage(chat_id, lon_str);
      char date_time_str[18]; sprintf(date_time_str, "%d : %d : %d, %d/%d/%d\n", packet.time_info.tm_hour,  packet.time_info.tm_min, packet.time_info.tm_sec, packet.time_info.tm_mday, packet.time_info.tm_mon,packet.time_info.tm_year);
      bot.sendMessage(chat_id, date_time_str);
      char az_el_str[20]; sprintf(az_el_str, "Azimuth: %f, Elevation: %f", azimuth, elevation);
      bot.sendMessage(chat_id, az_el_str);
    }
  }
}

