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

typedef union message_t {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
};

#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

#define SETUP_DURATION          (0*SEC)
#define MEASURE_INTERVAL        (5*MIN)
#define UNCONDITIONAL_INTERVAL  (30*MIN)
#define CONFIRMATION            false
#define TRANSMISSION_WAIT       (10*SEC)

message_t message[2];
byte lastMsgIndex = 0;

enum States { INITIAL, SETUP, MEASURE, TRANSMIT, SLEEP };
enum States currentState = INITIAL;

unsigned long seqNumber = 0L;
unsigned long lastMeasureMs = 0L;
unsigned long lastTransmissionMs = 0L;
unsigned long sleptMs = 0L;

SensorReader sensor = SensorReader();
DraginoLoRa radio = DraginoLoRa();
  

/* Setup ******************************************/

void setup() {
  Serial.begin(115200);
  Serial.print("Start LoRa test script with sensor message v");
  Serial.println(MESSAGE_VERSION);

  initializeMessage();

  os_init();
  
  sensor.begin();
  radio.begin();

  currentState = SETUP;
  Serial.println("Do Setup");
  sensor.listTemperatureSensors();
}

void initializeMessage() {
  message[0].sensor.version = MESSAGE_VERSION;
  message[1].sensor.version = MESSAGE_VERSION;
  message[lastMsgIndex].sensor.temperature.upper = WINT_MIN;
  message[lastMsgIndex].sensor.temperature.middle = WINT_MIN;
  message[lastMsgIndex].sensor.temperature.lower = WINT_MIN;
  message[lastMsgIndex].sensor.temperature.outer = WINT_MIN;
  message[lastMsgIndex].sensor.humidity.outer = WINT_MIN;
  message[lastMsgIndex].sensor.weight = WINT_MIN;
  message[lastMsgIndex].sensor.battery = WINT_MIN;
}

/* Loop ******************************************/

void loop() {
  if (currentState == SETUP) {
    // SETUP ---------------------------
    sensor.listRawWeight();
    readSensors(1);
    printSensorData(1);
    if (getTime() >= SETUP_DURATION) {
      currentState = MEASURE;
      Serial.println("\nDo Measure");
    }
  } else if (currentState == MEASURE) {
    // MEASURE ---------------------------
    byte index = (lastMsgIndex + 1) % 2;
    lastMeasureMs = getTime();
    readSensors(index);
    printSensorData(index);
    if (hasChanged(index) || getTime() >= lastTransmissionMs + UNCONDITIONAL_INTERVAL) {
      lastTransmissionMs = getTime();
      seqNumber = radio.send(message[index].bytes, sizeof(message[index]), CONFIRMATION);
      lastMsgIndex = index;
      currentState = TRANSMIT;
      Serial.println("Do Transmit");
    } else {
      Serial.println("No relevant change");
      currentState = SLEEP;
      Serial.println("Go Sleep");
      powerDown();
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
    if (getTime() >= lastMeasureMs + MEASURE_INTERVAL) {
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

void readSensors(byte index) {
  sensor.startReading();
  
  message[index].sensor.temperature.upper = asShort(sensor.getUpperTemperature());
  message[index].sensor.temperature.middle = asShort(sensor.getMiddleTemperature());
  message[index].sensor.temperature.lower = asShort(sensor.getLowerTemperature());
  message[index].sensor.temperature.outer = asShort(sensor.getOuterTemperature());
  message[index].sensor.humidity.outer = asShort(sensor.getOuterHumidity());
  message[index].sensor.weight = asShort(sensor.getWeight());
  message[index].sensor.battery = asShort(sensor.getVoltage());

  sensor.stopReading();
}

void printSensorData(byte index) {
  if (message[index].sensor.weight > WINT_MIN) {
    Serial.print(message[index].sensor.weight / 100.0);
    Serial.println(" kg");
  }
  if (message[index].sensor.temperature.lower > WINT_MIN) {
    Serial.print(message[index].sensor.temperature.lower / 100.0);
    Serial.println(" C lower level");
  }
  if (message[index].sensor.temperature.middle > WINT_MIN) {
    Serial.print(message[index].sensor.temperature.middle / 100.0);
    Serial.println(" C middle level");
  }
  if (message[index].sensor.temperature.upper > WINT_MIN) {
    Serial.print(message[index].sensor.temperature.upper / 100.0);
    Serial.println(" C upper level");
  }
  if (message[index].sensor.temperature.outer > WINT_MIN) {
    Serial.print(message[index].sensor.temperature.outer / 100.0);
    Serial.println(" C outside");
  }
  if (message[index].sensor.humidity.outer > WINT_MIN) {
    Serial.print(message[index].sensor.humidity.outer / 100.0);
    Serial.println(" % rel");
  }
  if (message[index].sensor.battery > WINT_MIN) {
    Serial.print(message[index].sensor.battery / 100.0);
    Serial.println(" Vcc");
  }
}

bool hasChanged(byte index) {
  return hasChangedWeight(message[lastMsgIndex].sensor.weight, message[index].sensor.weight)
      || hasChangedTemperature(message[lastMsgIndex].sensor.temperature.lower, message[index].sensor.temperature.lower)
      || hasChangedTemperature(message[lastMsgIndex].sensor.temperature.middle, message[index].sensor.temperature.middle)
      || hasChangedTemperature(message[lastMsgIndex].sensor.temperature.upper, message[index].sensor.temperature.upper)
      || hasChangedTemperature(message[lastMsgIndex].sensor.temperature.outer, message[index].sensor.temperature.outer)
      || hasChangedHumidity(message[lastMsgIndex].sensor.humidity.outer, message[index].sensor.humidity.outer);
}

bool hasChangedWeight(short lastValue, short nextValue) {
  return abs(lastValue - nextValue) >= 5; // 0.05 kg
}

bool hasChangedTemperature(short lastValue, short nextValue) {
  return abs(lastValue - nextValue) >= 50; // 0.50 degrees
}

bool hasChangedHumidity(short lastValue, short nextValue) {
  return abs(lastValue - nextValue) >= 50; // 0.50 %
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
