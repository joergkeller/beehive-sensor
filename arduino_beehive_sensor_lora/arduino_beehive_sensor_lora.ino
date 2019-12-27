/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * After setup, the sensors are read in intervals and the 
 * sensor data message is sent using LoRa. The controller
 * then goes to deep sleep to reduce power.
 * - USB/Battery voltage measurement (internal)
 * - DS18B20 temperature sensors (multiple) are read from pin D3 (GPIO5)
 * - DHT22 temperature/humidity sensor is read from pin D4 (GPIO4)
 * - Weight measured with load cell and HX711 ADC from pins A0/A1 (GPIO2/3)
 * ---
 * message version (aka command):
 *  0: sensor data v0 (short/100)
 **********************************************************/

#if defined(__ASR6501__)
  #include "CubeCellLoRa.h"
#else
  #include <LowPower.h>
  #include "DraginoLoRa.h"
#endif
#include "SensorReader.h"

#define UNDEFINED_VALUE -32768
#define MESSAGE_VERSION 0

typedef struct {
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
}__attribute((packed)) beesensor_t;

typedef union {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
} message_t;

#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

#define SETUP_DURATION          (0*SEC)
#define MEASURE_INTERVAL        (5*MIN)
#define UNCONDITIONAL_INTERVAL  (30*MIN)
#define CONFIRMATION            false
#define JOIN_WAIT               (60*MIN)
#define TRANSMISSION_WAIT       (10*SEC)

#define LIMIT_WEIGHT_DIFF       10  // 0.100 kg
#define LIMIT_TEMPERATURE_DIFF  50  // 0.50 degrees
#define LIMIT_HUMIDITY_DIFF    200  // 2.0 %

message_t message[2];
byte lastMsgIndex = 0;

enum States { INITIAL, SETUP, JOIN, MEASURE, TRANSMIT, SLEEP };
enum States currentState = INITIAL;

unsigned long seqNumber = 0L;
unsigned long lastJoinMs = 0L;
unsigned long lastMeasureMs = 0L;
unsigned long lastTransmissionMs = 0L;
unsigned long sleptMs = 0L;

SensorReader sensor = SensorReader();
#if defined(__ASR6501__)
  TimerEvent_t wakeup;
  CubeCellLoRa radio = CubeCellLoRa();
#else
  DraginoLoRa radio = DraginoLoRa();
#endif
  

/* Setup ******************************************/

void setup() {
  Serial.begin(115200);
  delay(500);

  #if defined(__ASR6501__)
    BoardInitMcu();
    TimerInit(&wakeup, onWakeup);
  #endif
  sensor.begin();
  initializeMessage();
  radio.begin();

  if (SETUP_DURATION > 0) {
    currentState = SETUP;
    Serial.println("\nDo Setup");
  } else {
    currentState = JOIN;
    Serial.println("\nDo Join");
    lastJoinMs = getTime();
    radio.join();
  }
}

void initializeMessage() {
  Serial.print("Start Beehive LoRa script with sensor message v");
  Serial.println(MESSAGE_VERSION);
  message[0].sensor.version = MESSAGE_VERSION;
  message[1].sensor.version = MESSAGE_VERSION;
  message[lastMsgIndex].sensor.temperature.upper = UNDEFINED_VALUE;
  message[lastMsgIndex].sensor.temperature.middle = UNDEFINED_VALUE;
  message[lastMsgIndex].sensor.temperature.lower = UNDEFINED_VALUE;
  message[lastMsgIndex].sensor.temperature.outer = UNDEFINED_VALUE;
  message[lastMsgIndex].sensor.humidity.outer = UNDEFINED_VALUE;
  message[lastMsgIndex].sensor.weight = UNDEFINED_VALUE;
  message[lastMsgIndex].sensor.battery = UNDEFINED_VALUE;
}

/* Loop ******************************************/

bool hasChanged(byte index);
bool hasChangedWeight(short lastValue, short nextValue);
bool hasChangedTemperature(short lastValue, short nextValue);
bool hasChangedHumidity(short lastValue, short nextValue);

void loop() {
  if (currentState == SETUP) {
    // SETUP ---------------------------
    sensor.listTemperatureSensors();
    sensor.listRawWeight();
    readSensors(1);
    printSensorData(1);
    if (getTime() >= SETUP_DURATION) {
      currentState = JOIN;
      Serial.println("\nDo Join");
      lastJoinMs = getTime();
      radio.join();
    }
  } else if (currentState == JOIN) {
    // JOIN ---------------------------
    if (!radio.isJoining() || getTime() >= lastJoinMs + JOIN_WAIT) {
      if (radio.isJoining()) {
        lastJoinMs = getTime();
        radio.join();
      } else {
        currentState = MEASURE;
        Serial.println("\nDo Measure");
      }
    }
  } else if (currentState == MEASURE) {
    // MEASURE ---------------------------
    lastMeasureMs = getTime();
    byte index = (lastMsgIndex + 1) % 2;
    readSensors(index);
    printSensorData(index);
    unsigned long transmissionInterval = getTime() - lastTransmissionMs;
    boolean unconditionalTransmit = transmissionInterval >= (UNCONDITIONAL_INTERVAL - (MEASURE_INTERVAL/2));
    if (unconditionalTransmit) {
      Serial.print(transmissionInterval / 1000);
      Serial.println("s since transmission");
    }
    if (unconditionalTransmit || hasChanged(index)) {
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
    Serial.print('.');
    delay(1);
    Serial.flush();
    #ifdef USBCON
      USBDevice.detach();
    #endif

    #if defined(__ASR6501__)
      LowPower_Handler();
    #else
      //  delay(8000);
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      sleptMs += 8000UL;
      if (getTime() >= lastMeasureMs + MEASURE_INTERVAL) {
        powerUp();
        currentState = MEASURE;
        Serial.println("\nDo Measure");
      }
    #endif

    #ifdef USBCON
      USBDevice.init();
      USBDevice.attach();
    #endif
  }

  radio.tick();
}

/* Helper methods ******************************************/

unsigned long getTime() {
  return millis() + sleptMs;
}

short asShort(float value) {
  if (isnan(value) || value == -127.0f) return UNDEFINED_VALUE;
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
  print(message[index].sensor.weight, " kg");
  print(message[index].sensor.temperature.lower, " C lower level");
  print(message[index].sensor.temperature.middle, " C middle level");
  print(message[index].sensor.temperature.upper, " C upper level");
  print(message[index].sensor.temperature.outer, " C outside");
  print(message[index].sensor.humidity.outer, " % rel");
  print(message[index].sensor.battery, " Vbat");
}

void print(short compactValue, String suffix) {
  if (compactValue != UNDEFINED_VALUE) {
    Serial.print(compactValue / 100.0);
    Serial.println(suffix);
  }
}

bool hasChangedValue(short lastValue, short nextValue, short limit) {
  return abs(lastValue - nextValue) >= limit;
}

bool hasChanged(byte index) {
  return hasChangedValue(message[lastMsgIndex].sensor.weight, message[index].sensor.weight, LIMIT_WEIGHT_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.lower, message[index].sensor.temperature.lower, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.middle, message[index].sensor.temperature.middle, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.upper, message[index].sensor.temperature.upper, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.outer, message[index].sensor.temperature.outer, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.humidity.outer, message[index].sensor.humidity.outer, LIMIT_HUMIDITY_DIFF);
}

void powerDown() {
  sensor.powerDown();
  delay(1);
  #if defined(__ASR6501__)
    uint32_t timeToWake = (lastMeasureMs + MEASURE_INTERVAL) - getTime();
    Serial.print(timeToWake / 1000); Serial.println(" s sleeping");
    TimerSetValue(&wakeup, timeToWake);
    TimerStart(&wakeup);
    sleptMs += timeToWake;
  #endif
}

void onWakeup() {
  powerUp();
  currentState = MEASURE;
  Serial.println("\nDo Measure");
} 

void powerUp() {
  Serial.println('|');
  sensor.powerUp();
  radio.reset(seqNumber);
}
