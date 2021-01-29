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

// see credentials.h, calibration.h
#define KROKUS       1      // CubeCell - OTAA
#define SHAKRA       2      // CubeCell - OTAA
#define GOTTHARD     3      // CubeCell - OTAA
#define WITCHES      4      // CubeCell - OTAA
#define CUBE_CELL_1 11      // CubeCell - OTAA
#define TEST_123    12      // Dragino  - ABP

// see credentials.h, calibration.h
#define DEVICE_ID   KROKUS
#define DEVICE_NAME krokus

#ifdef ARDUINO_AVR_FEATHER32U4
  #define DEBUG(str)            ;
  #define DEBUG2(str1,str2)     ;
  #define DEBUG3(str1,val,str2) ;
#else
  #define DEBUG(str)                Serial.println(str)
  #define DEBUG2(str1,str2)       { Serial.print(str1); Serial.println(str2); }
  #define DEBUG3(str1,val,str2)   { Serial.print(str1); Serial.print(val,DEC); Serial.println(str2); }
#endif

#include "SensorReader.h"
#include "StateMachine.h"

#if defined(__ASR6501__)
  #include "CubeCellLoRa.h"
#else
  #include <Adafruit_SleepyDog.h>
  #include "DraginoLoRa.h"
#endif

#ifndef ARDUINO_AVR_FEATHER32U4
  #include "Interaction.h"
#endif

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
    short other[THERMOMETER_COUNT];
  } temperature;
}__attribute((packed)) beesensor_t;

typedef union {
  beesensor_t sensor;
  byte bytes[1+2+2+2+2+(THERMOMETER_COUNT*2)];
} message_t;

#define MS 1L
#define SEC  (1000*MS)
#define MIN  (60*SEC)
#define HOUR (60*MIN)
#define DAY  (24*HOUR)

unsigned long getTime();
void onSwitchManualMode();

#define RAW_MEASURE_INTERVAL    (4*SEC)   // Dragino only allows 8s, 4s, 2s, 1s
#define MEASURE_INTERVAL        (5*MIN)
#define UNCONDITIONAL_INTERVAL  (30*MIN)
#define CONFIRMATION_INTERVAL   (12*HOUR)
#define RESET_INTERVAL          (6*DAY)
#define JOIN_WAIT               (60*MIN)
#define TRANSMISSION_WAIT       (15*SEC)
#define MAX_TRANSMISSION_FAIL   5

#define LIMIT_WEIGHT_DIFF       10  // 0.100 kg
#define LIMIT_TEMPERATURE_DIFF  50  // 0.50 degrees
#define LIMIT_HUMIDITY_DIFF    200  // 2.0 %

message_t message[2];
byte lastMsgIndex = 0;

unsigned long seqNumber = 0L;
unsigned long lastMeasureMs = 0L;
unsigned long lastTransmissionMs = 0L;
unsigned long lastConfirmationMs = 0L;
unsigned long sleptMs = 0L;
boolean       requireConfirmation = false;
unsigned int  transmissionFailed = 0;

#ifdef ARDUINO_AVR_FEATHER32U4
typedef enum               {JOIN,   MEASURE,   TRANSMIT,   SLEEP} States;
const char* stateNames[] = {"Join", "Measure", "Transmit", "Sleep"};
#else
typedef enum               {JOIN,   MEASURE,   TRANSMIT,   SLEEP,   MANUAL } States;
const char* stateNames[] = {"Join", "Measure", "Transmit", "Sleep", "Manual"};
#endif
StateMachine node(5, stateNames, getTime);

#ifndef ARDUINO_AVR_FEATHER32U4
Interaction interaction;
#endif

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
  #ifndef ARDUINO_AVR_FEATHER32U4
  interaction.begin(onSwitchManualMode);
  #endif

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
  #ifndef ARDUINO_AVR_FEATHER32U4
  node.onEnter(MANUAL, beginManual);
  node.onState(MANUAL, manualMode);
  node.onExit(MANUAL, endManual);
  #endif

  node.toState(JOIN);
}

#ifndef ARDUINO_AVR_FEATHER32U4
  #define xstr(x) str(x)
  #define str(x) #x
  #define ABOUT_MESSAGE "Start '" xstr(DEVICE_NAME) "' beehive LoRa script with sensor message v" xstr(MESSAGE_VERSION)
#endif

void initializeMessage() {
  #ifndef ARDUINO_AVR_FEATHER32U4
    Serial.print(ABOUT_MESSAGE);
    Serial.print(" (");
    Serial.print(sizeof(message[0]));
    Serial.println(" bytes)");
  #endif
  for (int m = 0; m < 2; m++) {
    message[m].sensor.version = MESSAGE_VERSION;
    message[m].sensor.battery = UNDEFINED_VALUE;
    message[m].sensor.weight = UNDEFINED_VALUE;
    message[m].sensor.humidity.roof = UNDEFINED_VALUE;
    message[m].sensor.temperature.roof = UNDEFINED_VALUE;
    for (int i = 0; i < THERMOMETER_COUNT; i++) {
      message[m].sensor.temperature.other[i] = UNDEFINED_VALUE;
    }
  }
}

/* Loop ******************************************/

void loop() {
  node.loop();
  radio.tick();
}

/* Event handler ******************************************/

bool unconditionalTransmit();
bool withConfirmation();
bool hasChanged(byte index);
bool hasChangedWeight(short lastValue, short nextValue);
bool hasChangedTemperature(short lastValue, short nextValue);
bool hasChangedHumidity(short lastValue, short nextValue);

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
  #ifndef ARDUINO_AVR_FEATHER32U4
    printSensorData(index);
  #endif
  if (unconditionalTransmit() || hasChanged(index)) {
    node.toState(TRANSMIT);
  } else {
    DEBUG("No changes");
    node.toState(SLEEP);
  }
}

// TRANSMIT ---------------------------

void sendMessage() {
  byte index = (lastMsgIndex + 1) % 2;
  lastTransmissionMs = getTime();
  requireConfirmation = withConfirmation();
  seqNumber = radio.send(message[index].bytes, sizeof(message[index]), requireConfirmation);
  lastMsgIndex = index;
}

void transmitting() {
  if (!radio.isTransmitting()) { // successful transmission!
    if (requireConfirmation) {
      transmissionFailed = 0;
      requireConfirmation = false;
      lastConfirmationMs = getTime();
    }
    node.toState(SLEEP);
  }
}

void onTransmitTimeout() {
  transmissionFailed++;
  radio.clear();
  node.toState(SLEEP);
  #if defined(__ASR6501__)
    if (transmissionFailed > MAX_TRANSMISSION_FAIL) {
      HW_Reset(0);
    }
  #endif
}

// SLEEP ---------------------------

void powerDown() {
  sensor.powerDown();

  uint32_t timeToWake = (lastMeasureMs + MEASURE_INTERVAL) - getTime();
  DEBUG2((timeToWake / 1000), " s sleeping");
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
  #elif defined(__SAMD21G18A__)
    unsigned long timeToWake = (lastMeasureMs + MEASURE_INTERVAL) - getTime();
    sleptMs += Watchdog.sleep(timeToWake);
  #else
    unsigned long timeToWake = (lastMeasureMs + MEASURE_INTERVAL) - getTime();
    sleptMs += Watchdog.sleep(min(timeToWake, 8*SEC));
  #endif
}

void onSleepTimeout() {
  node.toState(MEASURE);
} 

void powerUp() {
  #if defined(__ASR6501__)
    if (getTime() >= RESET_INTERVAL) {
      HW_Reset(0);
    }
  #endif
  #ifdef USBCON
  #if defined(__ASR6501__)
    USBDevice.init();
  #endif
    USBDevice.attach();
  #endif
  sensor.powerUp();
  radio.reset(seqNumber);
}

// MANUAL ---------------------------

#ifndef ARDUINO_AVR_FEATHER32U4
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
  #elif defined(__SAMD21G18A__)
    sleptMs += Watchdog.sleep(RAW_MEASURE_INTERVAL);
  #else
    sleptMs += Watchdog.sleep(min(RAW_MEASURE_INTERVAL, 8*SEC));
    measureRawData();
  #endif

  #ifdef USBCON
  #if defined(__ASR6501__)
    USBDevice.init();
  #endif
    USBDevice.attach();
  #endif
  delay(1);
}

void measureRawData() {
  interaction.setLed(true);
  sensor.listTemperatureSensors();
  sensor.listRawWeight();
  readSensors(1);
  #if defined(__ASR6501__)
    printSensorData(1);
  #endif
  delay(1);
  interaction.setLed(false);
}

void endManual() {
  interaction.setLed(false);
  #if defined(__ASR6501__)
    TimerStop(&wakeupTimer);
  #endif
}
#endif

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
  message[index].sensor.battery = asShort(sensor.getVoltage());
  message[index].sensor.weight = asShort(sensor.getCompensatedWeight());
  message[index].sensor.humidity.roof = asShort(sensor.getRoofHumidity());
  message[index].sensor.temperature.roof = asShort(sensor.getRoofTemperature());
  #ifndef ARDUINO_AVR_FEATHER32U4
  for (int i = 0; i < THERMOMETER_COUNT; i++) {
    message[index].sensor.temperature.other[i] = asShort(sensor.getTemperature(i));
  }
  #endif
  sensor.stopReading();
}

void printSensorData(byte index) {
  print(message[index].sensor.weight, " kg");
  for (int i = 0; i < THERMOMETER_COUNT; i++) {
    print(message[index].sensor.temperature.other[i], String(" C level ") + i);
  }
  print(message[index].sensor.temperature.roof, " C roof");
  print(message[index].sensor.humidity.roof, " % rel roof");
  print(message[index].sensor.battery, " Vbat");
}

void print(short compactValue, String suffix) {
  if (compactValue != UNDEFINED_VALUE) {
    DEBUG2((compactValue / 100.0), suffix);
  }
}

inline
bool unconditionalTransmit() {
  unsigned long transmissionInterval = getTime() - lastTransmissionMs;
  boolean unconditionalTransmit = transmissionInterval >= (UNCONDITIONAL_INTERVAL - (MEASURE_INTERVAL/2));
  if (unconditionalTransmit) {
    DEBUG2((transmissionInterval / 1000), "s since transmission");
  }
  return unconditionalTransmit;
}

inline
boolean withConfirmation() {
  #if defined(__ASR6501__)
    if (transmissionFailed > 0) {
      DEBUG("Require confirmation after fail");
      return true;
    }
    unsigned long confirmationInterval = getTime() - lastConfirmationMs;
    boolean confirmation = confirmationInterval >= CONFIRMATION_INTERVAL;
    if (confirmation) {
      DEBUG("Require confirmation");
    }
    return confirmation;
  #else
    return false;
  #endif
}

inline
bool hasChangedValue(short lastValue, short nextValue, short limit) {
  return abs(lastValue - nextValue) >= limit;
}

bool hasChanged(byte index) {
  for (int i = 0; i < THERMOMETER_COUNT; i++) {
    if (hasChangedValue(message[lastMsgIndex].sensor.temperature.other[i], message[index].sensor.temperature.other[i], LIMIT_TEMPERATURE_DIFF)) {
      return true;
    }
  }
  return hasChangedValue(message[lastMsgIndex].sensor.weight, message[index].sensor.weight, LIMIT_WEIGHT_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.temperature.roof, message[index].sensor.temperature.roof, LIMIT_TEMPERATURE_DIFF)
      || hasChangedValue(message[lastMsgIndex].sensor.humidity.roof, message[index].sensor.humidity.roof, LIMIT_HUMIDITY_DIFF);
}
