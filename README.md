# SunScanner
<div align='center'> <img src = "img_solar_cell.jpg" alt = "Frame01" width = "500"> </div>

Introducing SunScanner, a TI MSP432-powered smart device that tracks the sun to harvest solar energy.
It uses GPS coordinates to infer the sun's position and a Pan-and-Tilt-like contraption to orient its built-in solar panel accordingly.

This project is developed by a team of three students of the University of Trento, in the context of the course of Embedded Systems and IoT.

## Index Folder

- [Features](#features)
- [System Requirements](#system-requirements)
- [Get Started](#get-started)
  -  [MSP Programming](#msp-programming)
  -  [ESP32-S3 Programming](#esp32-s3-programming)
- [Schematic Project](#schematic-project)
  -  [Pin Scheme](#pin-scheme)
  -  [Project Layout](#project-layout)
- [Demo video](#demo-video)
- [Presentation](#presentation)
- [Gruoup members](#group-members)
- [Credits](#credits)


## Features

- Global Positioning System: to automatically set the orientation of the solar cell - Reach for the Sun!
- 2x Gear box: To increase the range of movement of the Azimuth servo fro 180 to 360 degrees
- Possibility to manually adjust the position of the Solar Cell
- View Real-time Voltage readings from the Solar Cell
- ESP32-S3 as a gateway to the IoT - more processing power, Telegram Bot communication

## System Requirements

- Texas Instruments MSP432P401R
- BoosterPack edu. MKII
- ESP32-S3-DevKitC-1
- 2x Servo Motors MS18
- GPS Ublox neo 6m
- CCS - Code Composer Studio
- PlatformIO in Visual Studio Code

## Get Started

### MSP432 Programming
In order to build and upload the project yourself, first clone the 'main' branch of the repo:
```
git clone https://github.com/AleBiCi/SunScanner_Embedded/
```
Then import the 'SunScanner' CCS project folder in your local CCS Workspace (you might have to edit the include options according to your requirements, refer to the guide posted in the Moodle page).
You should now be able to flash the MSP432P401R board.
### ESP32-S3 Programming
Inside the 'ESP' folder you'll find the PlatformIO project that you have to upload onto the ESP32-S3 board.
If PlatformIO IDE isn't already set up on your machine, follow the official guide: [Start from here](https://docs.platformio.org/en/latest/core/installation/index.html) and end up [Here!](https://docs.platformio.org/en/latest/integration/ide/vscode.html)
Then import the project, flash it onto the board and you're good to go!

## Schematic Project
<img src = "SunScanner_Schema.png" alt = "Frame01" width = "500"> </div>

### Pin Scheme
|component|port|pin|
|--|--|--|
|Servo_PWM - Elevation|2|5|
|Servo_PWM - Azimuth|2|6|
|Voltage Reading|6|6|
|UART2 - GPS|3|RX:2 - TX:3|
|UART1 on ESP32|/|RX:15 - TX:16|
|UART1 on MSP to ESP32|2|RX:2 - TX:3|

### Project Layout
```
├── CCS Project
│   ├── .DS_Store
│   ├── .gitignore
│   └── SunScanner
│       ├── ccs
│       │   └── startup_msp432p401r_ccs.c
│       ├── libs
│       │   ├── .DS_Store
│       │   ├── ESP_control.h
│       │   ├── LcdDriver
│       │   │   ├── Crystalfontz128x128_ST7735.h
│       │   │   └── HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h
│       │   ├── delay.h
│       │   ├── finite_state_machine.h
│       │   ├── gps
│       │   │   ├── NMEAParser.h
│       │   │   └── gps.h
│       │   ├── menu.h
│       │   ├── scenes.h
│       │   └── servo
│       │       ├── servo_control.h
│       │       └── servo_sun_tracker.h
│       ├── main.c
│       ├── msp432p401r.cmd
│       ├── src
│       │   ├── .DS_Store
│       │   ├── LcdDriver
│       │   │   ├── Crystalfontz128x128_ST7735.c
│       │   │   └── HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.c
│       │   └── NMEAParser.c
│       ├── system_msp432p401r.c
│       └── targetConfigs
├── ESP
│   ├── lib
│   │   ├── Az_alt
│   │   │   ├── Sun_Az_Alt.cpp
│   │   │   └── Sun_Az_Alt.h
│   │   └── README
│   ├── platformio.ini
│   ├── src
│   │   └── main.cpp
│   └── test
├── LICENSE
└── README.md
```


## Demo Video
[Sun Scanner](https://youtu.be/7Bo-yUQaSLg)

## Presentation
[Link Presentation](https://docs.google.com/presentation/d/1R6tl_jSoiLLPsk6RGqctIMuoVqyj2pdu-Sd7vOVIlfg/edit?usp=sharing)

## Group members
|Name|Email|
|--|--|
|Alessandro Bianchi Ceriani|a.bianchiceriani@studenti.unitn.it|
|Gerardo Chianucci|gerardo.chianucci@studentu.unitn.it|
|Daniele Visentin|daniele.visentin@studenti.unitn.it|

We collaborated closely with eachother, both in presence and through Github

## Credits

[Generate grlib Graphics_Image elements from image files - JSFormer applet](https://nununoisy.github.io/JSFormer/)

