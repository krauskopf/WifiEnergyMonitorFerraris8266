# WiFi Energy Monitor for electromechanical induction watt-hour meters ('Ferraris-Zähler')

## Usage
The board has a RESTfull http interface which returns all data as json objects.
There is an API description in [OpenAPI 2.0](https://github.com/OAI/OpenAPI-Specification) format in the folder `api`. 
Use the [Swagger Editor](https://editor.swagger.io/) to open. The editor also allows you to generate client code 
to access the API from the runtime/language of your choice.

## Build instructions

### Hardware
You can find a [fritzing](http://fritzing.org) schematic in the folder `hardware` which shows the pin layout. 

### Firmware
To build the arduino sketch
- Install Arduino IDE
 - Use the Arduino Board Manager to install the ESP8266 toolchain (https://github.com/esp8266/Arduino) 
 - Use the Arduino Library Manager to install ArduinoJSON (https://bblanchon.github.io/ArduinoJson/)
- Open the sketch and modify the settings in the `config.h` header. You have to set the SSID and password of your wifi network.
- Compile and download to nodemcu.

## History
- 2017-oct-01: 0.1.0 - Created first version.
- 2017-oct-12: 1.0.0 - Completed API.

## Credits
- Sebastian Krauskopf (sebakrau)

## License
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
