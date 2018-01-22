/*
The MIT License (MIT)

Copyright (c) 2017 sebakrau

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

// 
// Hardware Settings
//
#define APP_NAME              "WifiEnergyMonitorFerraris8266"
#define APP_VERSION_MAJOR     1
#define APP_VERSION_MINOR     1
#define APP_VERSION_PATCH     0


#define PIN_BOARD_LED         D0  // GPIO16, blue LED on NodeMCU
#define PIN_SENSOR            A0  // ADC, sensor diode on ES-Fer

#define PIN_STATUS_LED_1      D1  // GPIO5, red status LED on first ES-Fer
#define PIN_SENSOR_LED_1      D2  // GPIO4, sensor LED on first ES-Fer
#define PIN_STATUS_LED_2      D3  // GPIO0, red status LED on second ES-Fer
#define PIN_SENSOR_LED_2      D4  // GPIO2, sensor LED on second ES-Fer

//
// Configuration Settings
//
//#define CONFIG_DEBUG_NOTFOUND   // uncomment to return debug info when url not found.
#define CONFIG_ENABLE_OTA       // uncomment to enable firmware update over the air (OTA)
#define CONFIG_NUM_SENSORS    2

// 
// Network Settings
//
#define CONFIG_WIFI_SSID      "xxx"
#define CONFIG_WIFI_PASSWORD  "xxx"

