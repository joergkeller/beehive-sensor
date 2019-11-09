/**********************************************************
 * Wrapper code for the lmic libray.
 * ---
 * Code from https://github.com/SensorsIot/LoRa
 * Initialization and sending moved to a class, other code
 * seems to be obsolete (testing required).
 **********************************************************/
#ifndef __DRAGINOLORA_H__
#define __DRAGINOLORA_H__

#include <lmic.h>
#include <hal/hal.h>
#include "credentials.h"

// Pin mapping Dragino Shield
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

class DraginoLoRa {
  public:

    void begin() {
      // Set up the channels used by the things network - EU863-870
      // https://www.thethingsnetwork.org/docs/lorawan/frequency-plans.html
      LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
      LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
      LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

      // Disable link check validation
      LMIC_setLinkCheckMode(0);
  
      // TTN uses SF9 for its RX2 window.
      LMIC.dn2Dr = DR_SF9;

      reset(0);
    }

    void reset(unsigned long seqNumber) {
      // Reset the MAC state. Session and pending data transfers will be discarded.
      LMIC_reset();
  
      // Set static session parameters.
      LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

      // set sequence counter for uplink
      LMIC.seqnoUp = seqNumber;
  
      // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
      LMIC_setDrTxpow(DR_SF12, 14);
    }

    void send(uint8_t *message, uint8_t len, bool confirmation) {
        // Check if there is not a current TX/RX job running
        if (isTransmitting()) {
            Serial.println(F("OP_TXRXPEND, not sending"));
        } else {
            // Prepare upstream data transmission at the next possible time.
            Serial.print("Message: ");
            printBufferAsString(message, len); 
            LMIC_setTxData2(1, message, len, confirmation ? 1 : 0);
            Serial.println(F("Sending uplink packet"));
        }
        // Next TX is scheduled after TX_COMPLETE event.
    }

    unsigned long seqNumber() {
      return LMIC.seqnoUp;
    }

    void clear() {
      LMIC_clrTxData();
    }

    bool isTransmitting() {
      return LMIC.opmode & OP_TXRXPEND;
    }

  private:
    void printBufferAsString(byte* buffer, int length) {
      Serial.print("\"");
      for (uint8_t i = 0; i < length; i++) {
        if (buffer[i] < 16) Serial.print("0");
        Serial.print(buffer[i], HEX);
      }
      Serial.println("\"");
    }

};

static bool txPending = false;

void onEvent(ev_t ev) {
    if (ev == EV_TXCOMPLETE) {
        Serial.println(F("EV_TXCOMPLETE"));
        txPending = false;
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

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

#endif
