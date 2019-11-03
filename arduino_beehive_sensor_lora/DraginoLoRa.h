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

      // Set up the channels used by the things network
      LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(1, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7B),  BAND_CENTI);      // g-band
      LMIC_setupChannel(2, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(3, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(4, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(5, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(6, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(7, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(8, 868100000, DR_RANGE_MAP(DR_FSK, DR_FSK),  BAND_MILLI);      // g2-

      // Disable link check validation
      LMIC_setLinkCheckMode(0);
  
      // TTN uses SF9 for its RX2 window.
      LMIC.dn2Dr = DR_SF9;
  
      // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
      LMIC_setDrTxpow(DR_SF12,14);
    }

    void send(uint8_t *message, uint8_t len) {
        // Check if there is not a current TX/RX job running
        if (isTransmitting()) {
            Serial.println(F("OP_TXRXPEND, not sending"));
        } else {
            // Prepare upstream data transmission at the next possible time.
            LMIC_setTxData2(1, message, len, 1);
            Serial.println(F("Sending uplink packet"));
        }
        // Next TX is scheduled after TX_COMPLETE event.
    }

    bool isTransmitting() {
      return LMIC.opmode & OP_TXRXPEND;
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

void onEvent(ev_t ev) {
    if (ev == EV_TXCOMPLETE) {
        Serial.println(F("EV_TXCOMPLETE"));
        // includes waiting for RX windows
        if (LMIC.txrxFlags & TXRX_ACK) {
          Serial.println(F("Received ack"));
        }
        if (LMIC.dataLen) {
          Serial.println(F("Received "));
          Serial.println(LMIC.dataLen);
          Serial.println(F(" bytes of payload"));
        }
    } else if (ev == EV_SCAN_TIMEOUT) {
        Serial.println(F("EV_SCAN_TIMEOUT"));
    } else if (ev == EV_BEACON_FOUND) {
        Serial.println(F("EV_BEACON_FOUND"));
    } else if (ev == EV_BEACON_MISSED) {
        Serial.println(F("EV_BEACON_MISSED"));
    } else if (ev == EV_BEACON_TRACKED) {
        Serial.println(F("EV_BEACON_TRACKED"));
    } else if (ev == EV_JOINING) {
        Serial.println(F("EV_JOINING"));
    } else if (ev == EV_JOINED) {
        Serial.println(F("EV_JOINED"));
    } else if (ev == EV_RFU1) {
        Serial.println(F("EV_RFU1"));
    } else if (ev == EV_JOIN_FAILED) {
        Serial.println(F("EV_JOIN_FAILED"));
    } else if (ev == EV_REJOIN_FAILED) {
        Serial.println(F("EV_REJOIN_FAILED"));
    } else if (ev == EV_LOST_TSYNC) {
        Serial.println(F("EV_LOST_TSYNC"));
    } else if (ev == EV_RESET) {
        Serial.println(F("EV_RESET"));
    } else if (ev == EV_RXCOMPLETE) {
        Serial.println(F("EV_RXCOMPLETE"));
    } else if (ev == EV_LINK_DEAD) {
        Serial.println(F("EV_LINK_DEAD"));
    } else if (ev == EV_LINK_ALIVE) {
        Serial.println(F("EV_LINK_ALIVE"));
    }
}

#endif
