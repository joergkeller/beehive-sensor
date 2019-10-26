/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * After setup, the sensors are read in intervals and the 
 * sensor data message is sent using LoRa.
 * - DS18B20 temperature sensors (multiple) are read from pin D3
 * - DHT22 temperature/humidity sensor is read from pin D4
 * - Weight sensor is simulated
 * - Battery measurement is simulated
 * ---
 * message version (aka command):
 *  0: sensor data v0
 **********************************************************/

#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
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

#define DHT_PIN 4
#define ONEWIRE_PIN 3
#define TEMPERATURE_PRECISION 10

#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

#define MESSAGE_VERSION 0
#define INTERVAL          (5*MIN)
#define TRANSMISSION_WAIT (30*SEC)

union {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
} message;

unsigned long lastTransmissionMs = 0L;

DeviceAddress lowerThermometer =  { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 };
DeviceAddress middleThermometer = { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 };
DeviceAddress upperThermometer =  { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 };

DHT dht(DHT_PIN, DHT22);

OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

DraginoLoRa radio = DraginoLoRa();
  

/* Setup ******************************************/

void setup() {
  Serial.begin(115200);
  Serial.print("Start LoRa test script with sensor message v");
  Serial.println(MESSAGE_VERSION);

  // initialize sensor message
  message.sensor.version = MESSAGE_VERSION;

  dht.begin();
  sensors.begin();
  radio.begin();

  listSensors();
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
  if (isnan(value) || value == -127.0) return WINT_MIN;
  return value * 100;
}

void readSensors() {
  sensors.requestTemperatures();

  // read real sensors instead of dummy data below
  message.sensor.temperature.outer = asShort(dht.readTemperature());
  message.sensor.temperature.upper = asShort(sensors.getTempC(upperThermometer));
  message.sensor.temperature.middle = asShort(sensors.getTempC(middleThermometer));
  message.sensor.temperature.lower = asShort(sensors.getTempC(lowerThermometer));
  message.sensor.humidity.outer = asShort(dht.readHumidity());
  message.sensor.weight = asShort(NAN);
  message.sensor.battery = asShort(NAN);
}

void sendLoRaMessage() {
  Serial.println();
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
  
  // send message.bytes using the LoRa transmitter
  Serial.print("Message: ");
  printBufferAsString(message.bytes, sizeof(message)); 
  radio.send(message.bytes, sizeof(message));
}

void listSensors() {
  byte deviceCount = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(deviceCount, DEC);
  Serial.println(" temperature sensor");

  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  for (byte i = 0; i < deviceCount; i++) {
    DeviceAddress addr;
    if (sensors.getAddress(addr, i)) {
        Serial.print("Device Address ");
        Serial.print(i);
        Serial.print(": ");
        printBufferAsArray(addr, sizeof(addr));
    } else {
      Serial.print("Unable to find address for Device ");
      Serial.println(i);
    }
  }
}

void printBufferAsArray(byte* buffer, int length) {
  Serial.print("{ 0x");
  for (uint8_t i = 0; i < length; i++) {
    if (i > 0) Serial.print(", 0x");
    if (buffer[i] < 16) Serial.print("0");
    Serial.print(buffer[i], HEX);
  }
  Serial.println(" }");
}

void printBufferAsString(byte* buffer, int length) {
  Serial.print("\"");
  for (uint8_t i = 0; i < length; i++) {
    if (buffer[i] < 16) Serial.print("0");
    Serial.print(buffer[i], HEX);
  }
  Serial.println("\"");
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
