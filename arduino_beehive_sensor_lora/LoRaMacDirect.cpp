#if defined(__ASR6501__)
/**********************************************************
 * Wrapper code for the LoRa Mac libray.
 **********************************************************/
#include <Arduino.h>
//#include <board.h>
#include "LoRaMacDirect.h"

/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;

/*loraWan default Dr when adr disabled*/
#ifdef REGION_US915
int8_t defaultDrForNoAdr = 3;
#else
int8_t defaultDrForNoAdr = 5;
#endif
int8_t currentDrForNoAdr;

LoRaMacPrimitives_t macPrimitive;
LoRaMacCallback_t macCallback;

boolean LoRaDirect::joinPending = false;
boolean LoRaDirect::txPending = false;

void LoRaDirect::init() {
  macPrimitive.MacMcpsConfirm = mcpsConfirm;
  macPrimitive.MacMcpsIndication = mcpsIndication;
  macPrimitive.MacMlmeConfirm = mlmeConfirm;
  macPrimitive.MacMlmeIndication = mlmeIndication;
  macCallback.GetBatteryLevel = NULL;
  macCallback.GetTemperatureLevel = NULL;
  LoRaMacInitialization( &macPrimitive, NULL, ACTIVE_REGION);

  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_ADR;
  mibReq.Param.AdrEnable = LORAWAN_ADR;
  LoRaMacMibSetRequestConfirm( &mibReq );

  LoRaDirect::joinPending = false;
  LoRaDirect::txPending = false;
}

void LoRaDirect::tick() {
  Radio.IrqProcess();
}

void LoRaDirect::joinOTAA(uint8_t devEui[], uint8_t appEui[], uint8_t appKey[]) {
  Serial.println("Joining OTAA...");
  LoRaDirect::joinPending = true;
  
  MlmeReq_t mlmeReq;
  mlmeReq.Type = MLME_JOIN;
  mlmeReq.Req.Join.DevEui = devEui;
  mlmeReq.Req.Join.AppEui = appEui;
  mlmeReq.Req.Join.AppKey = appKey;
  LoRaMacMlmeRequest( &mlmeReq );
}


void LoRaDirect::joinABP(uint32_t deviceAddress, uint8_t nwkSessionKey[], uint8_t appSessionKey[]) {
  Serial.println("Joining ABP.");
  LoRaDirect::joinPending = false;
  MibRequestConfirm_t mibReq;

  mibReq.Type = MIB_NET_ID;
  mibReq.Param.NetID = 0;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_DEV_ADDR;
  mibReq.Param.DevAddr = deviceAddress;
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_NWK_SKEY;
  mibReq.Param.NwkSKey = nwkSessionKey;
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_APP_SKEY;
  mibReq.Param.AppSKey = appSessionKey;
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_NETWORK_JOINED;
  mibReq.Param.IsNetworkJoined = true;
  LoRaMacMibSetRequestConfirm( &mibReq );
}


LoRaMacStatus_t LoRaDirect::send(uint8_t applicationPort, uint8_t message[], uint8_t messageSize, bool confirmReception) {
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_DEVICE_CLASS;
  LoRaMacMibGetRequestConfirm( &mibReq );

  if( mibReq.Param.Class != CLASS_A ) {
    mibReq.Param.Class = CLASS_A;
    LoRaMacMibSetRequestConfirm( &mibReq );
  }

  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  if( LoRaMacQueryTxPossible( messageSize, &txInfo ) != LORAMAC_STATUS_OK ) {
    // Send empty frame in order to flush MAC commands
    mcpsReq.Type = MCPS_UNCONFIRMED;
    mcpsReq.Req.Unconfirmed.fBuffer = NULL;
    mcpsReq.Req.Unconfirmed.fBufferSize = 0;
    mcpsReq.Req.Unconfirmed.Datarate = DEFAULT_DATARATE;
  } else {
    if( confirmReception ) {
      mcpsReq.Type = MCPS_CONFIRMED;
      mcpsReq.Req.Confirmed.fPort = applicationPort;
      mcpsReq.Req.Confirmed.fBuffer = message;
      mcpsReq.Req.Confirmed.fBufferSize = messageSize;
      mcpsReq.Req.Confirmed.NbTrials = CONFIRMED_TRIALS;
      mcpsReq.Req.Confirmed.Datarate = DEFAULT_DATARATE;
    } else {
      mcpsReq.Type = MCPS_UNCONFIRMED;
      mcpsReq.Req.Unconfirmed.fPort = applicationPort;
      mcpsReq.Req.Unconfirmed.fBuffer = message;
      mcpsReq.Req.Unconfirmed.fBufferSize = messageSize;
      mcpsReq.Req.Unconfirmed.Datarate = DEFAULT_DATARATE;
    }
  }
  LoRaMacStatus_t status = LoRaMacMcpsRequest(&mcpsReq);
  if (status == LORAMAC_STATUS_OK) {
    LoRaDirect::txPending = true;
  }
  return status;
}

void LoRaDirect::mcpsConfirm( McpsConfirm_t *mcpsConfirm ) { 
  printStatus("MCPS Confirmation: ", mcpsConfirm->Status);
  Serial.print("Datarate: "); Serial.println(mcpsConfirm->Datarate);
  Serial.print("TxPower: "); Serial.println(mcpsConfirm->TxPower);
  Serial.print("Retries: "); Serial.println(mcpsConfirm->NbRetries);
  Serial.print("AckReceived: "); Serial.println(mcpsConfirm->AckReceived);
  Serial.print("Counter: "); Serial.println(mcpsConfirm->UpLinkCounter);
  LoRaDirect::txPending = false;
}

void LoRaDirect::mlmeConfirm( MlmeConfirm_t *mlmeConfirm ) { 
  printStatus("MLME Confirmation: ", mlmeConfirm->Status); 
  if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
    Serial.println("Joined!");
    LoRaDirect::joinPending = false;
  }
}

void LoRaDirect::mcpsIndication( McpsIndication_t *mcpsIndication ) {
  printStatus("MCPS Indication: ", mcpsIndication->Status);
  if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {
    return;
  }
//  printf( "receive data: rssi = %d, snr = %d, datarate = %d\r\n", mcpsIndication->Rssi, (int)mcpsIndication->Snr,(int)mcpsIndication->RxDatarate);

  if( mcpsIndication->FramePending ) {
    // The server signals that it has pending data to be sent.
    // We schedule an uplink as soon as possible to flush the server.
    Serial.println("Frame pending");
//    OnTxNextPacketTimerEvent();
  }

  if( mcpsIndication->RxData ) {
    Serial.print(mcpsIndication->RxSlot ? "RXWIN2 " : "RXWIN1 ");
    Serial.print(mcpsIndication->Port); Serial.print(": ");
    Serial.print(mcpsIndication->BufferSize); Serial.println(" bytes");
//    printf("+REV DATA:%s,RXSIZE %d,PORT %d\r\n",mcpsIndication->RxSlot?"RXWIN2":"RXWIN1",mcpsIndication->BufferSize,mcpsIndication->Port);
//    printf("+REV DATA:");
//    for( uint8_t i = 0; i < mcpsIndication->BufferSize; i++ ) {
//      printf("%02X",mcpsIndication->Buffer[i]);
//    }
//    printf("\r\n");
  }
}

void LoRaDirect::mlmeIndication( MlmeIndication_t *mlmeIndication ) {
  printStatus("MLME Indication: ", mlmeIndication->Status);
  switch( mlmeIndication->MlmeIndication ) {
    case MLME_SCHEDULE_UPLINK: { // The MAC signals that we shall provide an uplink as soon as possible
      Serial.println("Schedule Uplink");
//      OnTxNextPacketTimerEvent( );
      break;
    }
    default: {
      break;
    }
  }
}

void LoRaDirect::printStatus( char* prefix, LoRaMacEventInfoStatus_t status ) {
  Serial.print(prefix);
  switch (status) {
    case LORAMAC_EVENT_INFO_STATUS_OK: {
      Serial.println("STATUS_OK");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_ERROR: {
      Serial.println("STATUS_ERROR");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT: {
      Serial.println("STATUS_TX_TIMEOUT");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT: {
      Serial.println("STATUS_RX1_TIMEOUT");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT: {
      Serial.println("STATUS_RX2_TIMEOUT");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_RX1_ERROR: {
      Serial.println("STATUS_RX1_ERROR");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_RX2_ERROR: {
      Serial.println("STATUS_RX2_ERROR");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL: {
      Serial.println("STATUS_JOIN_FAIL");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED: {
      Serial.println("STATUS_DOWNLINK_REPEATED");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR: {
      Serial.println("STATUS_TX_DR_PAYLOAD_SIZE_ERROR");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS: {
      Serial.println("STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL: {
      Serial.println("STATUS_ADDRESS_FAIL");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_MIC_FAIL: {
      Serial.println("STATUS_MIC_FAIL");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL: {
      Serial.println("STATUS_MULTICAST_FAIL");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED: {
      Serial.println("STATUS_BEACON_LOCKED");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_BEACON_LOST: {
      Serial.println("STATUS_BEACON_LOST");
      break;
    }
    case LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND: {
      Serial.println("STATUS_BEACON_NOT_FOUND");
      break;
    }
  }
}


uint8_t BoardGetBatteryLevel() {
  return 255;
}
#endif
