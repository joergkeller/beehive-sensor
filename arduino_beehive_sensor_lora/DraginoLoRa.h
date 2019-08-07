/**********************************************************
 * Wrapper code for the lmic libray.
 * ---
 * Code from https://github.com/SensorsIot/LoRa
 * Initialization and sending moved to a class, other code
 * seems to be obsolete (testing required).
 **********************************************************/
#ifndef DRAGINOLORA
#define DRAGINOLORA

#include <lmic.h>
#include <hal/hal.h>
#include "credentials.h"

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

class DraginoLoRa {
  public:

    void begin() {
      os_init();
  
      // Reset the MAC state. Session and pending data transfers will be discarded.
      LMIC_reset();
  
      // Set static session parameters.
      LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  
      // Disable link check validation
      LMIC_setLinkCheckMode(0);
  
      // TTN uses SF9 for its RX2 window.
      LMIC.dn2Dr = DR_SF9;
  
      // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
      LMIC_setDrTxpow(DR_SF12,14);
    }

    void send(uint8_t *message, uint8_t len) {
        // Check if there is not a current TX/RX job running
        if (LMIC.opmode & OP_TXRXPEND) {
            Serial.println(F("OP_TXRXPEND, not sending"));
        } else {
            // Prepare upstream data transmission at the next possible time.
            LMIC_setTxData2(1, message, len, 0);
            Serial.println(F("Sending uplink packet..."));
        }
        // Next TX is scheduled after TX_COMPLETE event.
    }

    void printlnMessage(uint8_t *message, uint8_t len) {
      Serial.print("0x ");
      for (int i = 0; i < len; i++) {
        if (message[i] < 16) Serial.print('0');
        Serial.print(message[i], HEX);
        Serial.print(' ');
      }
      Serial.println();
    }
};

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL_SEC = 20;
    
// Pin mapping Dragino Shield
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void do_send(osjob_t* j){
    // Payload to send (uplink)
    static uint8_t message[] = "hi";

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, message, sizeof(message)-1, 0);
        Serial.println(F("Sending uplink packet..."));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent(ev_t ev) {
    if (ev == EV_TXCOMPLETE) {
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        // Schedule next transmission
        os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL_SEC), do_send);
    }
}

#endif
