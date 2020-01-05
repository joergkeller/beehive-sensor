/**********************************************************
 * Handling of user push-button and LED to switch to 
 * manual-mode (disable measure/send).
 **********************************************************/
#ifndef __INTERACTION_H__
#define __INTERACTION_H__

#if defined(__ASR6501__)
  #define BUTTON_PIN  GPIO0
  #define LED_PIN     GPIO1
#else
  #define BUTTON_PIN  3
  #define LED_PIN    A2
#endif

typedef void (*ButtonHandler)();

class Interaction {
  public:
    Interaction(ButtonHandler handler) {
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
