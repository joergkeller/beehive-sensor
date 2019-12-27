/**********************************************************
 * Wrapper code for the loRa-Mac libray.
 * ---
 **********************************************************/
#ifndef __CUBECELLLORA_H__
#define __CUBECELLLORA_H__

#include "LoRaMacDirect.h"
#include "credentials.h"

class CubeCellLoRa {
  public:

    void begin() {
      lora.init();
    }

    void tick() {
      lora.tick();
    }

    void join() {
      lora.joinOTAA(DEV_EUI, APP_EUI, APP_KEY);
      //lora.joinABP(DEVADDR, NWKSKEY, APPSKEY);
    }

    void reset(unsigned long seqNumber) {
      // TODO uplink count
      //lora.joinABP(DEVADDR, NWKSKEY, APPSKEY);
    }

    unsigned long send(uint8_t *message, uint8_t len, bool confirmation) {
        // Check if there is not a current TX/RX job running
        if (isTransmitting()) {
            Serial.println(F("OP_TXRXPEND, not sending"));
        } else {
            // Prepare upstream data transmission at the next possible time.
            Serial.print("Message: ");
            printBufferAsString(message, len);
            LoRaMacStatus_t status = lora.send(1, message, len, confirmation);
            if (status == LORAMAC_STATUS_OK) {
              Serial.println(F("Sending uplink packet"));
            } else {
              Serial.println(F("Sending failed"));
            }

        }
        return seqNumber();
    }

    unsigned long seqNumber() {
      return 0;
    }

    void clear() {
    }

    bool isJoining() {
      return lora.isJoinPending();
    }

    bool isTransmitting() {
      return lora.isTxPending();
    }

  private:
    LoRaDirect lora = LoRaDirect();
  
    void printBufferAsString(byte* buffer, int length) {
      Serial.print("\"");
      for (uint8_t i = 0; i < length; i++) {
        if (buffer[i] < 16) Serial.print("0");
        Serial.print(buffer[i], HEX);
      }
      Serial.println("\"");
    }

};

#endif
