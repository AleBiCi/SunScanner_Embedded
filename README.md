# SunScanner
<div align='center'> <img src = "img_solar_cell.jpg" alt = "Frame01" width = "500"> </div>

Introducing SunScanner, a TI MSP432-powered smart device that tracks the sun to harvest solar energy.
It uses the GPS coordinates to understand the sun positioning in every place in the entire word.

This project is developed by a team of three students of the University of Trento, in the context of the course of Embedded Systems and IoT.


## Index Folder

- [Features](#features)
- [System Requirements](#system-requirements)
- [Get Started](#get-started)
- [Schematic Project](#schematic-project)
  -  [Pin Scheme](#pin-scheme)
  -  [Project Layout](#project-layout)
- [Demo video](#demo-video)
- [Presentation](#presentation)
- [Gruoup members](#group-members)
- [Credits](#credits)


## Features

- Global Positioning System: to set automatically the solar cell directly the sun
- 2x Gear box: For increase the movement of servo to 180 degree to 360
- Possibility to manually adjust the position of the Solar Cell
- View in Real - Time Voltage catch the sun through the Solar Cell


## System Requirements

- Texas Instruments MSP432 - P401R
- BoosterPack MKII
- 2x Servo Motors MS18
- GPS Ublox neo 6m
- CCS - Code Composer Studio

## Get Started
In order to build and upload the project yourself, first clone the 'stable' branch of the repo:
```
git clone https://github.com/AleBiCi/SunScanner_Embedded/tree/stable
```
Then import the CCS project folder in your local CCS Workspace (you might have to edit the include options according to your requirements, refer to the guide posted in the Moodle page).
You should now be able to flash the board.

## Schematic Project
<img src = "SunScanner_Schema.png" alt = "Frame01" width = "500"> </div>

### Pin Scheme
|component|port|pin|
|--|--|--|
|Servo_PWM|2|5
|Servo_PWM|2|6|
|Voltage|6|6|
|GPS|3|2/3|
|ESP32_|15|16|
|_ESP32|P2.3|P2.2|


### Project Layout
```
├── README.md
└── MSP432
   └── SunScanner
       └──libs
          └──gps
             ├── gps.h
             └──NMEAparser.h
          └──servo
             ├── servo_control.h
             ├── servo_sun_tracker.h
             └──Sun_az_alt.h
          ├── menu.ino
          ├── finite_state_machine.h
          ├── delay.h
          └── scenes.h
       └──libs
          └──servo
             └──Sun_az_alt.c
          ├── menu.ino
          ├── finite_state_machine.h
          ├── delay.h
          └── scenes.h
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

We collaborated closely with eachother, both in presence and through Github.

## Credits
[Conversion .png to HEX - JSFormer app](https://nununoisy.github.io/JSFormer/)
