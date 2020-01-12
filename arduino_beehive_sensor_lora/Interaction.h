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

typedef void (*ButtonHandler)();

class Interaction {
  public:
    void begin(ButtonHandler handler) {
      pinMode(LED_PIN, OUTPUT);
      setLed(false);
      pinMode(BUTTON_PIN, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handler, FALLING);
    }

    bool checkSwitchPressed() {
      delayMicroseconds(5000);
      return ! digitalRead(BUTTON_PIN);
    }

    void setLed(bool state) {
        digitalWrite(LED_PIN, state ? LOW : HIGH); // LED is active low
    }
};

#endif
