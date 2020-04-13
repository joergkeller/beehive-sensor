/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * The sensors are read in intervals and the sensor data message
 * is sent using LoRa. The controller then goes to deep sleep to
 * reduce power.
 * A manual mode stops sending data but continuous to read raw data.
 * - USB/Battery voltage measurement (internal)
 * - DS18B20 temperature sensors (multiple) are read from pin D5 (GPIO5)
 * - DHT22 temperature/humidity sensor is read from pin D4 (GPIO4)
 * - Weight measured with load cell and HX711 ADC from pins A0/A1 (GPIO2/3)
 * - Push Button to switch to manual mode with pullup on pin D3 (GPIO7)
 * - LED to indicate manual mode (active low) on pin A2 (GPIO1)
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
#include "StateMachine.h"
#include "Interaction.h"

#define UNDEFINED_VALUE -32768
#define MESSAGE_VERSION 0

typedef struct {
  byte version;
  short battery;
  short weight;
  struct {
    short roof;
  } humidity;
  struct {
    short roof;
    short upper;
    short middle;
    short lower;
    short drop;
    short outer;
  } temperature;
}__attribute((packed)) beesensor_t;

typedef union {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
} message_t;

#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

unsigned long getTime();
void onSwitchManualMode();

#define RAW_MEASURE_INTERVAL    (4*SEC)   // Dragino only allows 8s, 4s, 2s, 1s
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

unsigned long seqNumber = 0L;
unsigned long lastMeasureMs = 0L;
unsigned long lastTransmissionMs = 0L;
unsigned long sleptMs = 0L;

typedef enum               {JOIN,   MEASURE,   TRANSMIT,   SLEEP,   MANUAL } States;
const char* stateNames[] = {"Join", "Measure", "Transmit", "Sleep", "Manual"};
StateMachine node(5, stateNames, getTime);

Interaction interaction;

SensorReader sensor = SensorReader();
#if defined(__ASR6501__)
  TimerEvent_t wakeupTimer;
  CubeCellLoRa radio = CubeCellLoRa();
#else
  DraginoLoRa radio = DraginoLoRa();
#endif
  

/* Setup ******************************************/

void setup() {
  Serial.begin(115200);
  delay(500);

  #if defined(__ASR6501__)
    boardInitMcu();
  #endif
  sensor.begin();
  initializeMessage();
  radio.begin();
  interaction.begin(onSwitchManualMode);

  node.onEnter(JOIN, beginJoin);
  node.onState(JOIN, joining);
  node.onTimeout(JOIN, JOIN_WAIT, onJoinTimeout);
  node.onState(MEASURE, measure);
  node.onEnter(TRANSMIT, sendMessage);
  node.onState(TRANSMIT, transmitting);
  node.onTimeout(TRANSMIT, TRANSMISSION_WAIT, onTransmitTimeout);
  node.onEnter(SLEEP, powerDown);
  node.onState(SLEEP, sleeping);
  node.onTimeout(SLEEP, MEASURE_INTERVAL, onSleepTimeout);
  node.onExit(SLEEP, powerUp);
  node.onEnter(MANUAL, beginManual);
  node.onState(MANUAL, manualMode);
  node.onExit(MANUAL, endManual);

  node.toState(JOIN);
}

void initializeMessage() {
  Serial.print("Start Beehive LoRa script with sensor message v");
  Serial.println(MESSAGE_VERSION);
  message[0].sensor.version = MESSAGE_VERSION;
  message[0].sensor.temperature.upper = UNDEFINED_VALUE;
  message[0].sensor.temperature.middle = UNDEFINED_VALUE;
  message[0].sensor.temperature.lower = UNDEFINED_VALUE;
  message[0].sensor.temperature.drop = UNDEFINED_VALUE;
  message[0].sensor.temperature.outer = UNDEFINED_VALUE;
  message[0].sensor.temperature.roof = UNDEFINED_VALUE;
  message[0].sensor.humidity.roof = UNDEFINED_VALUE;
  message[0].sensor.weight = UNDEFINED_VALUE;
  message[0].sensor.battery = UNDEFINED_VALUE;
  message[1].sensor.version = MESSAGE_VERSION;
  message[1].sensor.temperature.upper = UNDEFINED_VALUE;
  message[1].sensor.temperature.middle = UNDEFINED_VALUE;
  message[1].sensor.temperature.lower = UNDEFINED_VALUE;
  message[1].sensor.temperature.drop = UNDEFINED_VALUE;
  message[1].sensor.temperature.outer = UNDEFINED_VALUE;
  message[1].sensor.temperature.roof = UNDEFINED_VALUE;
  message[1].sensor.humidity.roof = UNDEFINED_VALUE;
  message[1].sensor.weight = UNDEFINED_VALUE;
  message[1].sensor.battery = UNDEFINED_VALUE;
}

/* Loop ******************************************/

bool unconditionalTransmit();
bool hasChanged(byte index);
bool hasChangedWeight(short lastValue, short nextValue);
bool hasChangedTemperature(short lastValue, short nextValue);
bool hasChangedHumidity(short lastValue, short nextValue);

void loop() {
  node.loop();
  radio.tick();
}

/* Event handler ******************************************/

// JOIN ---------------------------

void beginJoin() {
  radio.join();
}

void joining() {
  if (!radio.isJoining()) {
    node.toState(MEASURE);
  }
}

void onJoinTimeout() {
  node.toState(JOIN); // try again
}

// MEASURE ---------------------------

void measure() {
  lastMeasureMs = getTime();
  byte index = (lastMsgIndex + 1) % 2;
  readSensors(index);
  printSensorData(index);
  if (unconditionalTransmit() || hasChanged(index)) {
    node.toState(TRANSMIT);
  } else {
    Serial.println("No relevant change");
    node.toState(SLEEP);
  }
}

// TRANSMIT ---------------------------

void sendMessage() {
  byte index = (lastMsgIndex + 1) % 2;
  lastTransmissionMs = getTime();
  seqNumber = radio.send(message[index].bytes, sizeof(message[index]), CONFIRMATION);
  lastMsgIndex = index;
}

void transmitting() {
  if (!radio.isTransmitting()) {
    node.toState(SLEEP);
  }
}

void onTransmitTimeout() {
  radio.clear();
  node.toState(SLEEP);
}

// SLEEP ---------------------------

void powerDown() {
  sensor.powerDown();

  uint32_t timeToWake = (lastMeasureMs + MEASURE_INTERVAL) - getTime();
  Serial.print(timeToWake / 1000); Serial.println(" s sleeping");
  delay(1);
  Serial.flush();
  #ifdef USBCON
    USBDevice.detach();
  #endif

  #if defined(__ASR6501__)
    TimerInit(&wakeupTimer, onSleepTimeout);
    TimerSetValue(&wakeupTimer, timeToWake);
    TimerStart(&wakeupTimer);
    sleptMs += timeToWake;
  #endif
}

void sleeping() {
  #if defined(__ASR6501__)
    lowPowerHandler();
  #else
    unsigned long timeToWake = (lastMeasureMs + MEASURE_INTERVAL) - getTime();
    if (timeToWake >= 8000) {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      sleptMs += 8000;
    } else if (timeToWake >= 4000) {
      LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
      sleptMs += 4000;
    } else if (timeToWake >= 2000) {
      LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
      sleptMs += 2000;
    } else {
      LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
      sleptMs += 1000;
    }
  #endif
}

void onSleepTimeout() {
  node.toState(MEASURE);
} 

void powerUp() {
  #ifdef USBCON
    USBDevice.init();
    USBDevice.attach();
  #endif
  sensor.powerUp();
  radio.reset(seqNumber);
}

// MANUAL ---------------------------

void onSwitchManualMode() {
  if (!interaction.checkSwitchPressed()) return;

  if (node.state() == MANUAL) {
    node.toState(JOIN);
  } else {
    node.toState(MANUAL);
  }
}

void beginManual() {
  measureRawData();

  #if defined(__ASR6501__)
    TimerInit(&wakeupTimer, onManualTimeout);
    TimerSetValue(&wakeupTimer, RAW_MEASURE_INTERVAL);
    TimerStart(&wakeupTimer);
    sleptMs += RAW_MEASURE_INTERVAL;
  #endif
}

void onManualTimeout() {
  measureRawData();

  #if defined(__ASR6501__)
    TimerStart(&wakeupTimer);
    sleptMs += RAW_MEASURE_INTERVAL;
  #endif
  // remain in manual state
}

void manualMode() {
  Serial.flush();
  #ifdef USBCON
    USBDevice.detach();
  #endif

  #if defined(__ASR6501__)
    lowPowerHandler();
  #else
    #if RAW_MEASURE_INTERVAL >= 8000
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      sleptMs += 8000;
    #elif  RAW_MEASURE_INTERVAL >= 4000
      LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
      sleptMs += 4000;
    #elif  RAW_MEASURE_INTERVAL >= 2000
      LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
      sleptMs += 2000;
    #else
      LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
      sleptMs += 1000;
    #endif
    measureRawData();
  #endif

  #ifdef USBCON
    USBDevice.init();
    USBDevice.attach();
  #endif
  delay(1);
}

void measureRawData() {
  interaction.setLed(true);
  sensor.listTemperatureSensors();
  sensor.listRawWeight();
  readSensors(1);
  printSensorData(1);
  delay(1);
  interaction.setLed(false);
}

void endManual() {
  interaction.setLed(false);
  #if defined(__ASR6501__)
    TimerStop(&wakeupTimer);
  #endif
}

/* Helper methods ******************************************/

unsigned long getTime() {
  return millis() + sleptMs;
}

inline
boolean isUndefined(float value) {
  return isnan(value) || value == -127.0f;
}

inline
boolean isDefined(float value) { return ! isUndefined(value); }

inline
short asShort(float value) {
  if (isUndefined(value)) return UNDEFINED_VALUE;
  return value * 100;
}

void readSensors(byte index) {
  sensor.startReading();
  float outerTemperature = sensor.getOuterTemperature();
  float weight = sensor.getWeight();
  if (isDefined(outerTemperature) && isDefined(weight)) {
    weight -= sensor.getWeightCompensation(outerTemperature);
  }

  message[index].sensor.temperature.upper = asShort(sensor.getUpperTemperature());
  message[index].sensor.temperature.middle = asShort(sensor.getMiddleTemperature());
  message[index].sensor.temperature.lower = asShort(sensor.getLowerTemperature());
  message[index].sensor.temperature.drop = asShort(sensor.getDropTemperature());
  message[index].sensor.temperature.outer = asShort(outerTemperature);
  message[index].sensor.temperature.roof = asShort(sensor.getRoofTemperature());
  message[index].sensor.humidity.roof = asShort(sensor.getRoofHumidity());
  message[index].sensor.weight = asShort(weight);
  message[index].sensor.battery = asShort(sensor.getVoltage());

  sensor.stopReading();
}

void printSensorData(byte index) {
  print(message[index].sensor.weight, " kg");
  print(message[index].sensor.temperature.upper, " C upper level");
  print(message[index].sensor.temperature.middle, " C middle level");
  print(message[index].sensor.temperature.lower, " C lower level");
  print(message[index].sensor.temperature.drop, " C drop");
  print(message[index].sensor.temperature.outer, " C outside");
  print(message[index].sensor.temperature.roof, " C roof");
  print(message[index].sensor.humidity.roof, " % rel roof");
  print(message[index].sensor.battery, " Vbat");
}

inline
void print(short compactValue, String suffix) {
  if (compactValue != UNDEFINED_VALUE) {
    Serial.print(compactValue / 100.0);
    Serial.println(suffix);
  }
}

inline
bool unconditionalTransmit() {
  unsigned long transmissionInterval = getTime() - lastTransmissionMs;
  boolean unconditionalTransmit = transmissionInterval >= (UNCONDITIONAL_INTERVAL - (MEASURE_INTERVAL/2));
  if (unconditionalTransmit) {
    Serial.print(transmissionInterval / 1000); Serial.println("s since transmission");
  }
  return unconditionalTransmit;
}

inline
bool hasChangedValue(short lastValue, short nextValue, short limit) {
  return abs(lastValue - nextValue) >= limit;
}

inline
bool hasChanged(byte index) {
  return hasChangedValue(message[lastMsgIndex].sensor.weight, message[index].sensor.weight, LIMIT_WEIGHT_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.upper, message[index].sensor.temperature.upper, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.middle, message[index].sensor.temperature.middle, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.lower, message[index].sensor.temperature.lower, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.drop, message[index].sensor.temperature.drop, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.outer, message[index].sensor.temperature.outer, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.roof, message[index].sensor.temperature.roof, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.humidity.roof, message[index].sensor.humidity.roof, LIMIT_HUMIDITY_DIFF);
}
