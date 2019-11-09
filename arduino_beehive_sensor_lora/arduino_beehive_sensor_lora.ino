/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * After setup, the sensors are read in intervals and the 
 * sensor data message is sent using LoRa. The controller
 * then goes to deep sleep to reduce power.
 * - USB voltage measurement (internal)
 * - DS18B20 temperature sensors (multiple) are read from pin D3
 * - DHT22 temperature/humidity sensor is read from pin D4
 * - Weight measured with load cell and HX711 ADC from pins A0/A1
 * ---
 * message version (aka command):
 *  0: sensor data v0
 **********************************************************/

#include <LowPower.h>
#include "SensorReader.h"
#include "DraginoLoRa.h"

#define MESSAGE_VERSION 0
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

#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

#define SETUP_DURATION    (0*SEC)
#define INTERVAL          (10*MIN)
#define CONFIRMATION      false
#define TRANSMISSION_WAIT (10*SEC)

union {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
} message;

enum States { INITIAL, SETUP, MEASURE, TRANSMIT, SLEEP };
enum States currentState = INITIAL;

unsigned long seqNumber = 0L;
unsigned long lastTransmissionMs = 0L;
unsigned long sleptMs = 0L;

SensorReader sensor = SensorReader();
DraginoLoRa radio = DraginoLoRa();
  

/* Setup ******************************************/

void setup() {
  Serial.begin(115200);
  Serial.print("Start LoRa test script with sensor message v");
  Serial.println(MESSAGE_VERSION);

  // initialize sensor message
  message.sensor.version = MESSAGE_VERSION;

  os_init();
  
  sensor.begin();
  radio.begin();

  currentState = SETUP;
  Serial.println("Do Setup");
  sensor.listTemperatureSensors();
}

/* Loop ******************************************/

void loop() {
  if (currentState == SETUP) {
    // SETUP ---------------------------
    sensor.listRawWeight();
    readSensors();
    printSensorData();
    if (getTime() >= SETUP_DURATION) {
      currentState = MEASURE;
      Serial.println("\nDo Measure");
    }
  } else if (currentState == MEASURE) {
    // MEASURE ---------------------------
    readSensors();
    printSensorData();
    lastTransmissionMs = getTime();
    seqNumber = radio.send(message.bytes, sizeof(message), CONFIRMATION);
    {
      currentState = TRANSMIT;
      Serial.println("Do Transmit");
    }
  } else if (currentState == TRANSMIT) {
    // TRANSMIT ---------------------------
    if (!radio.isTransmitting() || getTime() >= lastTransmissionMs + TRANSMISSION_WAIT) {
      if (radio.isTransmitting()) radio.clear();
      currentState = SLEEP;
      Serial.println("Go Sleep");
      powerDown();
    }
  } else {
    // SLEEP ---------------------------
    sleep();
    if (getTime() >= lastTransmissionMs + INTERVAL) {
      powerUp();
      currentState = MEASURE;
      Serial.println("\nDo Measure");
    }
  }

  os_runloop_once();
}

/* Helper methods ******************************************/

unsigned long getTime() {
  return millis() + sleptMs;
}

short asShort(float value) {
  if (isnan(value) || value == -127.0) return WINT_MIN;
  return value * 100;
}

void readSensors() {
  sensor.startReading();
  
  message.sensor.temperature.upper = asShort(sensor.getUpperTemperature());
  message.sensor.temperature.middle = asShort(sensor.getMiddleTemperature());
  message.sensor.temperature.lower = asShort(sensor.getLowerTemperature());
  message.sensor.temperature.outer = asShort(sensor.getOuterTemperature());
  message.sensor.humidity.outer = asShort(sensor.getOuterHumidity());
  message.sensor.weight = asShort(sensor.getWeight());
  message.sensor.battery = asShort(sensor.getVoltage());

  sensor.stopReading();
}

void printSensorData() {
  if (message.sensor.weight > WINT_MIN) {
    Serial.print(message.sensor.weight / 100.0);
    Serial.println(" kg");
  }
  if (message.sensor.temperature.lower > WINT_MIN) {
    Serial.print(message.sensor.temperature.lower / 100.0);
    Serial.println(" C lower level");
  }
  if (message.sensor.temperature.middle > WINT_MIN) {
    Serial.print(message.sensor.temperature.middle / 100.0);
    Serial.println(" C middle level");
  }
  if (message.sensor.temperature.upper > WINT_MIN) {
    Serial.print(message.sensor.temperature.upper / 100.0);
    Serial.println(" C upper level");
  }
  if (message.sensor.temperature.outer > WINT_MIN) {
    Serial.print(message.sensor.temperature.outer / 100.0);
    Serial.println(" C outside");
  }
  if (message.sensor.humidity.outer > WINT_MIN) {
    Serial.print(message.sensor.humidity.outer / 100.0);
    Serial.println(" % rel");
  }
  if (message.sensor.battery > WINT_MIN) {
    Serial.print(message.sensor.battery / 100.0);
    Serial.println(" Vcc");
  }
}

void powerDown() {
  sensor.powerDown();
}

void powerUp() {
  Serial.println('|');
  sensor.powerUp();
  radio.reset(seqNumber);
}

void sleep() {
  Serial.flush();
  #ifdef USBCON
    USBDevice.detach();
  #endif

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  sleptMs += 8000UL;

  #ifdef USBCON
    USBDevice.init();
    USBDevice.attach();
  #endif
  Serial.print('.');
}
