/**********************************************************
 * Wrapper code for the LoRa Mac libray.
 * See https://stackforce.github.io/LoRaMac-doc/index.html
 **********************************************************/
#ifndef __LORAMACDIRECT_H__
#define __LORAMACDIRECT_H__

#include <LoRaWan_102.h>

/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
#define CONFIRMED_TRIALS 8

/*!
 * Default datarate
 */
#define DEFAULT_DATARATE DR_2

class LoRaDirect {
public:
    void init();
    void tick();
    void joinOTAA(uint8_t devEui[], uint8_t appEui[], uint8_t appKey[]);
    void joinABP(uint32_t deviceAddress, uint8_t nwkSessionKey[], uint8_t appSessionKey[]);
    LoRaMacStatus_t send(uint8_t applicationPort, uint8_t message[], uint8_t messageSize, bool confirmReception);
    boolean isJoinPending() { return LoRaDirect::joinPending; }
    boolean isTxPending() { return LoRaDirect::txPending; }

private:
    static boolean joinPending;
    static boolean txPending;

    static void mcpsConfirm( McpsConfirm_t *mcpsConfirm );
    static void mlmeConfirm( MlmeConfirm_t *mlmeConfirm );
    static void mcpsIndication( McpsIndication_t *mcpsIndication );
    static void mlmeIndication( MlmeIndication_t *mlmeIndication );
    static void printStatus( char* prefix, LoRaMacEventInfoStatus_t status );
};

#endif
