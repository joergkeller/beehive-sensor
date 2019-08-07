/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * After setup, the sensors are read in intervals and the 
 * sensor data message is sent using LoRa (simulated).
 **********************************************************/

#include "DraginoLoRa.h"
//#include <Adafruit_SleepyDog.h>

typedef struct beesensor_t {
  byte hiveId;
  struct { 
    short aussen;
    short unten;
    short mitte;
    short oben;
    short dach;
  } temperature; 
  struct {
    short aussen;
    short dach;
  } humidity;
};

union {
  beesensor_t sensor;
  byte bytes[1+10+4];
} message;

DraginoLoRa radio = DraginoLoRa();
    
#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)
#define INTERVAL (1*MIN)
unsigned long nextTransmissionMs = 0L;

/* Setup ******************************************/

void setup() {
  Serial.begin(9600);
  Serial.println("Start LoRa test script");

  // setup sensor library
  radio.begin();
}

/* Loop ******************************************/

void loop() {
  if (millis() >= nextTransmissionMs) {
    nextTransmissionMs += INTERVAL;
    // After each interval: measure and transmit
    readSensors();
    sendLoRaMessage();
  }
  
  delay(8000); 
  // for lower power consumption use deep sleep mode 
  // (as long a possible, will be approx. 8 sec).
  //Watchdog.sleep();  
  //#ifdef USBCON
  //  USBDevice.attach();
  //#endif
  Serial.print('.');
}

/* Helper methods ******************************************/

short asShort(float value) {
  return value * 100;
}

void readSensors() {
  // read real sensors instead of dummy data below
  message.sensor.hiveId = 42;
  message.sensor.temperature.aussen = asShort(-7.5);
  message.sensor.temperature.dach = asShort(35.25);
  message.sensor.humidity.dach = asShort(10);
}

void sendLoRaMessage() {
  Serial.println();
  Serial.print(message.sensor.temperature.aussen / 100.0);
  Serial.println(" C aussen");
  Serial.print(message.sensor.temperature.dach / 100.0);
  Serial.println(" C dach");
  Serial.print(message.sensor.humidity.dach / 100.0);
  Serial.println(" % rel");
  
  // send message.bytes using the LoRa transmitter
  radio.printlnMessage(message.bytes, sizeof(message));
  radio.send(message.bytes, sizeof(message));
}
