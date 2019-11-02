/**********************************************************
 * LoRa sample code snipped: Reading sensors and sending
 * LoRa messages.
 * ---
 * After setup, the sensors are read in intervals and the 
 * sensor data message is sent using LoRa.
 * - USB voltage measurement
 * - DS18B20 temperature sensors (multiple) are read from pin D3
 * - DHT22 temperature/humidity sensor is read from pin D4
 * - Weight measured with load cell and HX711 ADC from pins A0/A1
 * ---
 * message version (aka command):
 *  0: sensor data v0
 **********************************************************/

#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "HX711.h"
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

#define LOADCELL_DOUT_PIN  A0
#define LOADCELL_SCK_PIN  A1
#define LOADCELL_OFFSET -43496
#define LOADCELL_DIVIDER 25365
#define SETUP_SAMPLING 100
#define OPERATIONAL_SAMPLING 10

#define MS 1L
#define SEC (1000*MS)
#define MIN (60*SEC)

#define MESSAGE_VERSION 0
#define INTERVAL          (5*MIN)
#define TRANSMISSION_WAIT (30*SEC)
#define SETUP_DURATION    (2*MIN)

union {
  beesensor_t sensor;
  byte bytes[1+8+2+2+2];
} message;

unsigned long lastTransmissionMs = 0L;

// set unique DS18B20 addresses of used sensors
DeviceAddress lowerThermometer =  { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 };
DeviceAddress middleThermometer = { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 };
DeviceAddress upperThermometer =  { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 };

DHT dht(DHT_PIN, DHT22);

OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

HX711 scale;

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
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(LOADCELL_DIVIDER);
  scale.set_offset(LOADCELL_OFFSET);

  listSensors();
}

/* Loop ******************************************/

void loop() {
  if (millis() < SETUP_DURATION) {
    readSensors();
    printSensorData(true);
  } else if (lastTransmissionMs == 0L || millis() >= lastTransmissionMs + INTERVAL) {
    lastTransmissionMs += INTERVAL;
    readSensors();
    printSensorData(false);
    Serial.print("Message: ");
    printBufferAsString(message.bytes, sizeof(message)); 
    radio.send(message.bytes, sizeof(message));
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
  // power up
  scale.power_up();
  
  // DS18B20 temperature sensors on one-wire bus
  sensors.requestTemperatures();
  message.sensor.temperature.upper = asShort(sensors.getTempC(upperThermometer));
  message.sensor.temperature.middle = asShort(sensors.getTempC(middleThermometer));
  message.sensor.temperature.lower = asShort(sensors.getTempC(lowerThermometer));

  // DHTxx sensor
  message.sensor.temperature.outer = asShort(dht.readTemperature());
  message.sensor.humidity.outer = asShort(dht.readHumidity());

  // HX711 with load cell
  message.sensor.weight = asShort(scale.get_units(OPERATIONAL_SAMPLING));

  // voltage
  message.sensor.battery = asShort(readVcc() / 1023.0);
}

void printSensorData(bool rawWeightReading) {
  Serial.println();
  if (rawWeightReading) {
    Serial.println("Setup mode");
    Serial.println("----------");
    Serial.print("Raw weight read: ");
    Serial.println(scale.read_average(SETUP_SAMPLING));
  }
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

// see https://forum.arduino.cc/index.php?topic=120693.msg908179#msg908179
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void sleep() {
  // shut down
  scale.power_down();
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
