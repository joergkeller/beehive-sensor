/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * After setup, the sensors are read in intervals and the 
 * sensor data message is sent using LoRa (simulated).
 * ---
 * message version (aka command):
 *  0: sensor data v0
 **********************************************************/

#include "DraginoLoRa.h"
//#include <Adafruit_SleepyDog.h>

typedef struct beesensor_t {
  byte version;
  struct { 
    short outer;
    short lower;
    short middle;
    short upper;
  } temperature; 
  struct {
    short outer;
  } humidity;
  short weight;
  short battery;
};

union {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
} message;

DraginoLoRa radio = DraginoLoRa();
    
#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

#define INTERVAL          (5*MIN)
#define TRANSMISSION_WAIT (30*SEC)
unsigned long lastTransmissionMs = 0L;

/* Setup ******************************************/

void setup() {
  Serial.begin(115200);
  Serial.println("Start LoRa test script with dummy message");

  // initialize sensor message
  message.sensor.version = 42;

  // setup sensor library
  radio.begin();
}

/* Loop ******************************************/

void loop() {
  if (lastTransmissionMs == 0L || millis() >= lastTransmissionMs + INTERVAL) {
    lastTransmissionMs += INTERVAL;
    // After each interval: measure and transmit
    readSensors();
    sendLoRaMessage();
  } else if (radio.isTransmitting() && millis() < lastTransmissionMs + TRANSMISSION_WAIT) {
    // Wait for transmission complete
  } else {
    sleep();
  }

  os_runloop_once();
}

/* Helper methods ******************************************/

short asShort(float value) {
  return value * 100;
}

void readSensors() {
  // read real sensors instead of dummy data below
  message.sensor.temperature.outer = asShort(-7.5);
  message.sensor.temperature.upper = asShort(20.3);
  message.sensor.temperature.middle = asShort(19.2);
  message.sensor.temperature.lower = asShort(18.1);
  message.sensor.humidity.outer = asShort(65.43);
  message.sensor.weight = asShort(15.5);
  message.sensor.battery = asShort(3.31);
}

void sendLoRaMessage() {
  Serial.println();
  Serial.print(message.sensor.temperature.outer / 100.0);
  Serial.println(" C outside");
  Serial.print(message.sensor.humidity.outer / 100.0);
  Serial.println(" % rel");
  
  // send message.bytes using the LoRa transmitter
  radio.printlnMessage(message.bytes, sizeof(message));
  radio.send(message.bytes, sizeof(message));
}

void sleep() {
  // shut down
  Serial.flush();
  #ifdef USBCON
    USBDevice.detach();
  #endif

  // for lower power consumption use deep sleep mode 
  // (as long a possible, will be approx. 8 sec).
  delay(8000); 
  //Watchdog.sleep();  

  // wake up
  #ifdef USBCON
    USBDevice.init();
    USBDevice.attach();
  #endif
  Serial.print('.');
}
