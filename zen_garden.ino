/*
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif 

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include <WiFi.h>
#endif

#define FASTLED_ESP8266_DMA
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "SinricPro.h"
#include "SinricProLight.h"

#define WIFI_SSID         "abcd"    
#define WIFI_PASS         "efgh"

#define APP_KEY           "app_key"      
#define APP_SECRET        "7APP_SECRET"   
#define LIGHT_ID         "LIGHT_ID"
#define BAUD_RATE         115200                // Change baudrate to your need

#define NUM_LEDS          61                   // how much LEDs are on the stripe
#define LED_PIN           41                   // LED stripe is connected to PIN 3

bool powerState;        
int globalBrightnessLocal = 10;
CRGB leds[NUM_LEDS];

bool onPowerState(const String &deviceId, bool &state) {
  powerState = state;
  if (state) {
    analogWrite(1,200);
    FastLED.setBrightness(map(globalBrightnessLocal, 0, 100, 0, 255));
  } else {
    analogWrite(1,0);
    FastLED.setBrightness(0);
  }
  FastLED.show();
  return true; // request handled properly
}

bool onBrightness(const String &deviceId, int &brightness) {
  globalBrightnessLocal = brightness;
  FastLED.setBrightness(map(brightness, 0, 100, 0, 255));
  FastLED.show();
  return true;
}

bool onAdjustBrightness(const String &deviceId, int brightnessDelta) {
  globalBrightnessLocal += brightnessDelta;
  brightnessDelta = globalBrightnessLocal;
  FastLED.setBrightness(map(globalBrightnessLocal, 0, 100, 0, 255));
  FastLED.show();
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  fill_solid(leds, NUM_LEDS, CRGB(g, r, b));
  FastLED.show();
  return true;
}

void setupFastLED() {
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(map(globalBrightnessLocal, 0, 100, 0, 255));
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");

  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
    WiFi.setAutoReconnect(true);
  #elif defined(ESP32)
    WiFi.setSleep(false); 
    WiFi.setAutoReconnect(true);
  #endif

  WiFi.begin(WIFI_SSID, WIFI_PASS); 

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", localIP.toString().c_str());
}

void setupSinricPro() {
  // get a new Light device from SinricPro
  SinricProLight &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(onPowerState);
  myLight.onBrightness(onBrightness);
  myLight.onAdjustBrightness(onAdjustBrightness);
  myLight.onColor(onColor);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  //SinricPro.restoreDeviceStates(true); // Uncomment to restore the last known state from the server.
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  setupFastLED();
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}