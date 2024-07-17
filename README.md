# SunScanner

Introducing SunScanner, a TI MSP432-powered smart device that tracks the sun to harvest solar energy.
It uses the GPS coordinates to understand the sun positioning in every place in the entire word.

This project is developed by a team of three students of the University of Trento, in the context of the course of Embedded Systems and IoT.


## Index Folder

- [Feauteres](#feauteres)
- [System Requirements](#system-requirements)
- [Schematic Project](#schematic-project)
  -  [Pin Scheme](#pin-scheme)
  -  [Project Layout](#project-layout)
- [Demo video](#demo-video)
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

## Schematic Project
![immagine](./.scheme)

### Pin Scheme
|component|port|pin|
|--|--|--|
|Servo_PWM|2|5
|Servo_PWM|2|6|
|Voltage|6|6|
|GPS|3|2/3|
|ESP32|xx|xx|


### Project Layout
```
├── README.md
├── MSP432
│   └── MSP_Sensors
|       ├── MSP_Sensors.ino
|       ├── Adafruit_Sensor.h
|       ├── DHT.h
|       ├── DHT_U.h
|       ├── DHT.cpp
|       └── DHT_U.cpp
│       └──
└── backend
    ├── index.js
    ├── package.json
    ├── AI
    │   ├── Products.py
    │   └── imageRecognition.py
    ├── app
    │   ├── app.js
    │   ├── product.js
    │   ├── sensor.js
    │   ├── wishlist.js
    │   └── models
    │       ├── product.js
    │       └── sensor.js
    └── static
        ├── index.css
        ├── index.html
        └── index.js
```





## Demo Video

![Immagine di copertina](link)
[Sun Scanner](https://youtube.com)

## Group members
|Name|Email|
|--|--|
|Alessandro Bianchi Ceriani|a.bianchiceriani@studenti.unitn.it|
|Gerardo Chianucci|gerardo.chianucci@studentu.unitn.it|
|Daniele Visentin|daniele.visentin@studenti.unitn.it|

## Credits

[Conversion .png to HEX](https://nununoisy.github.io/JSFormer/)

