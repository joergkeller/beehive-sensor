/**********************************************************
 * Handling of user push-button and LED to switch to
 * manual-mode (disable measure/send).
 * ---
 * Arduino Uno/Dragino Mini Dev: Only digital pins 2/3 can
 * be used with interrupts.
 * - capacitor (eg. 200nF) parallel to the button to debounce?
 **********************************************************/
#ifndef __INTERACTION_H__
#define __INTERACTION_H__

#if defined(__ASR6501__)
  #define BUTTON_PIN  GPIO7
  #define LED_PIN     GPIO1
#else
  #define BUTTON_PIN  3
  #define LED_PIN    A2
#endif

#if(LoraWan_RGB==1)
#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel pixel(1, RGB, NEO_GRB + NEO_KHZ800);

void RGB_SET(uint8_t red, uint8_t green, uint8_t blue) {
  digitalWrite(Vext, LOW);  // Set power
  delay(1);
  pixel.begin();            // Initialize RGB strip object (REQUIRED)
  pixel.setPixelColor(0, pixel.Color(red, green, blue));
  pixel.show();             // Send the updated pixel colors to the hardware.
}

void RGB_ON(uint32_t color, uint32_t time) {
  uint8_t red,green,blue;
  red=(uint8_t)(color>>16);
  green=(uint8_t)(color>>8);
  blue=(uint8_t)color;
  RGB_SET(red, green, blue);
  if (time > 0) {
    delay(time);
  }
}

void RGB_OFF(void) {
  pixel.clear();            // Set all pixel colors to 'off'
  digitalWrite(Vext, HIGH); // Cut power
}
#endif

typedef void (*ButtonHandler)();

class Interaction {
  public:
    void begin(ButtonHandler handler) {
      // external LED
      pinMode(LED_PIN, OUTPUT);
      setLed(false);

      // internal Neopixel
      #if(LoraWan_RGB==1)
      pinMode(Vext,OUTPUT);
      #endif

      // internal/external user button
      pinMode(BUTTON_PIN, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handler, FALLING);
    }

    bool checkSwitchPressed() {
      delayMicroseconds(5000);
      return ! digitalRead(BUTTON_PIN);
    }

    void setLed(bool state) {
      digitalWrite(LED_PIN, state ? LOW : HIGH); // LED is active low
      #if(LoraWan_RGB==1)
        if (state) {
          RGB_SET(255, 0, 0);
        } else {
          RGB_OFF();
        }
      #endif
    }
};

#endif
