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
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>          // OTA
#include <ArduinoOTA.h>       // OTA
#include <ArduinoJson.h>
extern "C" {
  // include timer library for esp8266
  #include "user_interface.h"
}
#include "WifiClientPrint.h"
#include "config.h"


//
// Types
//
typedef struct Sensor {
  unsigned int value;
  unsigned int threshold;
  unsigned int counter[2];
  unsigned int revsPerKWh;
  bool state;
  bool stateOld;
} Sensor;

//
// Globals
//
ESP8266WebServer g_server(80);
os_timer_t g_timer;
Sensor g_sensors[CONFIG_NUM_SENSORS];






//
// LED
//
void LED_init() {
  pinMode(PIN_BOARD_LED, OUTPUT); 
}

void LED_board_set(bool enabled) {
  if (enabled) {
    digitalWrite(PIN_BOARD_LED, HIGH); 
  } else {
    digitalWrite(PIN_BOARD_LED, LOW); 
  }
}


//
// Timer
//
void SENSOR_setStatusLed(int index, bool enabled) {
  int pin;
  switch(index) {
    case 0: pin = PIN_STATUS_LED_1; break;
    case 1: pin = PIN_STATUS_LED_2; break;
    default: return;
  }
  
  if (enabled) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH); 
  }
}

void SENSOR_setSensorLed(int index, bool enabled) {
  int pin;
  switch(index) {
    case 0: pin = PIN_SENSOR_LED_1; break;
    case 1: pin = PIN_SENSOR_LED_2; break;
    default: return;
  }
  
  if (enabled) {
    digitalWrite(pin, HIGH); 
  } else {
    digitalWrite(pin, LOW); 
  }
}

void SENSOR_sample(void *pArg) {
  const int index = 0;

  // read sensor value through adc
  g_sensors[index].value = analogRead(PIN_SENSOR);

  // debounce the sensor value to prevent that a flickering or jittering 
  // sensorValue will falsly increase the counter.
  static int counter = 0;
  if (g_sensors[index].value > g_sensors[index].threshold) {
    
    if (counter > 3) {
      g_sensors[index].state = true;
    } else {
      counter++;
    }
    
  } else if (g_sensors[index].value <= g_sensors[index].threshold) {
    
    if (counter < -3) {
      g_sensors[index].state = false;
    } else {
      counter--;
    }
  } 
  
  // check if there was a rising or falling edge of the signal
  if (g_sensors[index].state != g_sensors[index].stateOld) {
    if (g_sensors[index].state) {
      g_sensors[index].counter[0]++;
      g_sensors[index].counter[1]++;
    }
    SENSOR_setStatusLed(0, g_sensors[index].state);
  }

  g_sensors[index].stateOld = g_sensors[index].state;
} 

void SENSOR_switchMultiplexer(int index) {
  // TODO
}

void SENSOR_init() {
  pinMode(PIN_STATUS_LED_1, OUTPUT); 
  pinMode(PIN_SENSOR_LED_1, OUTPUT); 
  pinMode(PIN_STATUS_LED_2, OUTPUT); 
  pinMode(PIN_SENSOR_LED_2, OUTPUT); 
  SENSOR_setStatusLed(0, false);
  SENSOR_setStatusLed(1, false);
  SENSOR_setSensorLed(0, true);
  SENSOR_setSensorLed(1, true);
  SENSOR_switchMultiplexer(0);

  for (int i = 0; i < CONFIG_NUM_SENSORS; i++) {
    g_sensors[i].value = 0;
    g_sensors[i].threshold = 150;
    g_sensors[i].revsPerKWh = 75;
    g_sensors[i].counter[0] = 0;
    g_sensors[i].counter[1] = 0;
    g_sensors[i].state = false;
    g_sensors[i].stateOld = false;
  }
   
  os_timer_setfn(&g_timer, SENSOR_sample, NULL);
  os_timer_arm(&g_timer, 20, true);
}


//
// WiFi
//
void WIFI_enableOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA: Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void WIFI_init() {
  Serial.print("Connecting to ");
  Serial.println(CONFIG_WIFI_SSID);

  WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

#ifdef CONFIG_ENABLE_OTA
  Serial.println("OTA update is enabled!");
  WIFI_enableOTA();
#else  
  Serial.println("OTA update is disabled!");
#endif
}


//
// WEB
//
void WEB_handleNotFound() {
#ifdef CONFIG_DEBUG_NOTFOUND
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += g_server.uri();
  message += "\nMethod: ";
  message += (g_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += g_server.args();
  message += "\n";

  for (int i = 0; i < g_server.args(); i++) {
    message += " " + g_server.argName(i) + ": " + g_server.arg(i) + "\n";
  }

  g_server.send(404, "text/plain", message);
#else
  g_server.send(404, "text/plain", "Not Found!");
#endif
}

void WEB_handleRoot() {
  char temp[500];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, sizeof(temp),
    "<html>\
      <head>\
        <meta http-equiv='refresh' content='5'/>\
        <title>WifiEnergyMonitor for Ferraris</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
        <h1>WifiEnergyMonitor for Ferraris-Counter (ESP8266)!</h1>\
        <p>Version: %d.%d.%d</p>\
        <p>Uptime: %02d:%02d:%02d</p>\
      </body>\
    </html>",

    APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH,
    hr, min % 60, sec % 60
  );
  g_server.send(200, "text/html", temp);
}

template<class T> void WEB_sendJson(const T& json) {
  g_server.setContentLength(json.measureLength());
  g_server.send(200, "application/json", "");
  WiFiClientPrint<> p(g_server.client());
  json.printTo(p);
  p.stop(); // Calls p.flush() and WifiClient.stop()
}

void WEB_handleApiInfo() {
  String version = "";
  version += APP_VERSION_MAJOR;
  version += ".";
  version += APP_VERSION_MINOR;
  version += ".";
  version += APP_VERSION_PATCH;

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  String uptime = "";
  uptime += hr;
  uptime += ":";
  uptime += min % 60;
  uptime += ":";
  uptime += sec % 60;

  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = APP_NAME;
  root["version"] = version;
  root["uptime"] = uptime;
  WEB_sendJson(root);
}

void WEB_handleApiThreshold(int indexMeter) {
  switch (g_server.method()) {
    case HTTP_GET:
      {
        String content = "";
        content += g_sensors[indexMeter].threshold;
        g_server.send(200, "text/plain", content);
      }
      break;
    case HTTP_PUT:
      {
        String value = g_server.arg("value");
        if (value) {
          g_sensors[indexMeter].threshold = value.toInt();
          g_server.send(200, "text/plain", "OK!");
        } else {
          g_server.send(400, "text/plain", "Missing argument 'value'!");
        }      
      }
      break;
    default:
      {
        g_server.send(405, "text/plain", "The resource doesn't support the specified HTTP verb.!");
      }
      break;
  }
}

void WEB_handleApiRevsPerKWh(int indexMeter) {
  switch (g_server.method()) {
    case HTTP_GET:
      {
        String content = "";
        content += g_sensors[indexMeter].revsPerKWh;
        g_server.send(200, "text/plain", content);
      }
      break;
    case HTTP_PUT:
      {
        String value = g_server.arg("value");
        if (value) {
          g_sensors[indexMeter].revsPerKWh = value.toInt();
          g_server.send(200, "text/plain", "OK!");
        } else {
          g_server.send(400, "text/plain", "Missing argument 'value'!");
        }      
      }
      break;
    default:
      {
        g_server.send(405, "text/plain", "The resource doesn't support the specified HTTP verb.!");
      }
      break;
  }
}

void WEB_handleApiCounterRevolutions(int indexMeter, int indexCounter) {
  switch (g_server.method()) {
    case HTTP_GET:
      {
        String content = "";
        content += g_sensors[indexMeter].counter[indexCounter];
        g_server.send(200, "text/plain", content);
      }
      break;
    case HTTP_PUT:
      {
        String value = g_server.arg("value");
        if (value) {
          g_sensors[indexMeter].counter[indexCounter] = value.toInt();
          g_server.send(200, "text/plain", "OK!");
        } else {
          g_server.send(400, "text/plain", "Missing argument 'value'!");
        }      
      }
      break;
    default:
      {
        g_server.send(405, "text/plain", "The resource doesn't support the specified HTTP verb.!");
      }
      break;
  }
}

void WEB_handleApiCounterKWh(int indexMeter, int indexCounter) {
  switch (g_server.method()) {
    case HTTP_GET:
      {
        String content = "";
        content += static_cast<double>(g_sensors[indexMeter].counter[indexCounter]) / g_sensors[indexMeter].revsPerKWh;
        g_server.send(200, "text/plain", content);
      }
      break;
    case HTTP_PUT:
      {
        String value = g_server.arg("value");
        if (value) {
          g_sensors[indexMeter].counter[indexCounter] = strtod(value.c_str(), 0) * g_sensors[indexMeter].revsPerKWh;
          g_server.send(200, "text/plain", "OK!");
        } else {
          g_server.send(400, "text/plain", "Missing argument 'value'!");
        }      
      }
      break;
    default:
      {
        g_server.send(405, "text/plain", "The resource doesn't support the specified HTTP verb.!");
      }
      break;
  }
}



void WEB_buildCounter(int indexMeter, int indexCounter, JsonObject& jsonObject) {
  jsonObject["id"] = indexCounter + 1;
  jsonObject["revolutions"] = g_sensors[indexMeter].counter[indexCounter];
  jsonObject["kWh"] = static_cast<double>(g_sensors[indexMeter].counter[indexCounter]) / g_sensors[indexMeter].revsPerKWh;  
}

void WEB_handleApiCounter(int indexMeter, int indexCounter) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  WEB_buildCounter(indexMeter, indexCounter, root);
  WEB_sendJson(root);
}

void WEB_handleApiCounters(int indexMeter) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonArray& root = jsonBuffer.createArray();
  JsonObject& counter1 = root.createNestedObject();
  WEB_buildCounter(indexMeter, 0, counter1);
  JsonObject& counter2 = root.createNestedObject();
  WEB_buildCounter(indexMeter, 1, counter2);
  WEB_sendJson(root);
}

void WEB_buildMeter(int indexMeter, JsonObject& jsonObject) {
  jsonObject["id"] = indexMeter + 1;
  jsonObject["value"] = g_sensors[indexMeter].value;
  jsonObject["state"] = g_sensors[indexMeter].state;
  jsonObject["threshold"] = g_sensors[indexMeter].threshold;
  jsonObject["revsPerKWh"] = g_sensors[indexMeter].revsPerKWh;

  JsonArray& counters = jsonObject.createNestedArray("counters");
  JsonObject& counter1 = counters.createNestedObject();
  WEB_buildCounter(indexMeter, 0, counter1);
  JsonObject& counter2 = counters.createNestedObject();
  WEB_buildCounter(indexMeter, 1, counter2);
}

void WEB_handleApiMeter(int indexMeter) {
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  WEB_buildMeter(indexMeter, root);
  WEB_sendJson(root);
}

void WEB_handleApiMeters() {
  StaticJsonBuffer<600> jsonBuffer;
  JsonArray& root = jsonBuffer.createArray();
  JsonObject& meter1 = root.createNestedObject();
  WEB_buildMeter(0, meter1);
  JsonObject& meter2 = root.createNestedObject();
  WEB_buildMeter(1, meter2);
  WEB_sendJson(root);
}

void WEB_init() {
  g_server.onNotFound(WEB_handleNotFound);
  g_server.on("/", HTTP_GET, WEB_handleRoot);

  //
  // API interface
  //
  g_server.on("/api/ping", []() {
    g_server.send(200, "text/plain", "pong");
  });
  
  g_server.on("/api/info", HTTP_GET, WEB_handleApiInfo);
  g_server.on("/api/meters", HTTP_GET, WEB_handleApiMeters);
  g_server.on("/api/meters/1", HTTP_GET, [](){ WEB_handleApiMeter(0); });
  g_server.on("/api/meters/1/threshold", HTTP_ANY, [](){ WEB_handleApiThreshold(0); });
  g_server.on("/api/meters/1/revsPerKWh", HTTP_ANY, [](){ WEB_handleApiRevsPerKWh(0); });
  g_server.on("/api/meters/1/counters", HTTP_GET, [](){ WEB_handleApiCounters(0); });
  g_server.on("/api/meters/1/counters/1", HTTP_GET, [](){ WEB_handleApiCounter(0, 0); });
  g_server.on("/api/meters/1/counters/1/revolutions", HTTP_ANY, [](){ WEB_handleApiCounterRevolutions(0, 0); });
  g_server.on("/api/meters/1/counters/1/kWh", HTTP_ANY, [](){ WEB_handleApiCounterKWh(0, 0); });
  g_server.on("/api/meters/1/counters/2", HTTP_GET, [](){ WEB_handleApiCounter(0, 1); });
  g_server.on("/api/meters/1/counters/2/revolutions", HTTP_ANY, [](){ WEB_handleApiCounterRevolutions(0, 1); });
  g_server.on("/api/meters/1/counters/1/kWh", HTTP_ANY, [](){ WEB_handleApiCounterKWh(0, 1); });
  g_server.on("/api/meters/2", HTTP_GET, [](){ WEB_handleApiMeter(1); });
  g_server.on("/api/meters/2/threshold", HTTP_ANY, [](){ WEB_handleApiThreshold(1); });
  g_server.on("/api/meters/2/revsPerKWh", HTTP_ANY, [](){ WEB_handleApiRevsPerKWh(1); });
  g_server.on("/api/meters/2/counters", HTTP_GET, [](){ WEB_handleApiCounters(1); });
  g_server.on("/api/meters/2/counters/1", HTTP_GET, [](){ WEB_handleApiCounter(1, 0); });
  g_server.on("/api/meters/1/counters/1/revolutions", HTTP_ANY, [](){ WEB_handleApiCounterRevolutions(1, 0); });
  g_server.on("/api/meters/1/counters/1/kWh", HTTP_ANY, [](){ WEB_handleApiCounterKWh(1, 0); });
  g_server.on("/api/meters/2/counters/2", HTTP_GET, [](){ WEB_handleApiCounter(1, 1); });
  g_server.on("/api/meters/1/counters/1/revolutions", HTTP_ANY, [](){ WEB_handleApiCounterRevolutions(1, 1); });
  g_server.on("/api/meters/1/counters/1/kWh", HTTP_ANY, [](){ WEB_handleApiCounterKWh(1, 1); });
  g_server.begin();
  Serial.println("HTTP g_server started");
}


//
// Setup
//
void setup() {
  // Init board
  LED_init();
  LED_board_set(true);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  // Init sensor
  SENSOR_init();

  // Connect WiFi 
  WIFI_init();

  // Start server interface
  WEB_init();

  // Go, go, go...
  Serial.println("READY");
}


//
// LOOP
//
void loop() {
  g_server.handleClient();
#ifdef CONFIG_ENABLE_OTA
  ArduinoOTA.handle();
#endif
}




