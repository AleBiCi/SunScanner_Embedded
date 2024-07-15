# Sun Scanner

Introducing SunScanner, a TI MSP432-powered smart device that tracks the sun to harvest solar energy.
It uses the GPS coordinates to understand the sun positioning in every place in the entire word.

This project is developed by a team of three students of the University of Trento, in the context of the course of Embedded Systems and IoT.


## Index Folder

- [Documentation](#documentation)
  - [Presentation](#presentation)
  - [Link video](#link-video)
  - [Schematic Project](#schematic-project)
- [Software](#software)
  - [GPS](#GPS)
    - [GPS code](#GPS-code)
  - [Servo Motor](#servo-motor)
    - [Servo Code](#servo-code)
  - [Menu - Display](#menu-display)
    - [Display Code](#display-code)
    - [Bitmap .png](#bitmap-png)
    - [Buttons / Analog Joystick code](#buttons-analog-joystick)


## Features

- Global Positioning System: to set automatically the solar cell directly the sun
- 2x Gear box: For increase the movement of servo to 180 degree to 360
- Possibility to manually adjust the position of the Solar Cell
- View in Real - Time Voltage catch the sun through the Solar Cell


## Resources used

- Texas Instruments MSP432 - P401R
- BoosterPack MKII
- 2x Servo Motor
- GPS Ublox neo 6m
- CCS - Code Composer Studio

## Schematic Project
![immagine](./.scheme)

### Pin scheme
|Pin|Description|
|--|--|
|GND|xx|
|5V|xx|
|P|VCC-servo_1|
|P|DATA-servo_1|
|P|GND-servo_1|
|P|VCC-servo_2|
|P|DATA-servo_2|
|P|GND-servo_2|
|P|VCC-solar_cell|
|P|GN-solar_cell|


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


## Credits

[Conversion .png to HEX](https://nununoisy.github.io/JSFormer/)

