/**********************************************************
 * Wrapper code for HX711, DHT-xx and DallasTemperature sensors.
 * ---
 * Acquiring sensor data from several sources.
 * Sensor names:
   - 1. Outer temperature - Aussentemperatur (used for weight compensation)
   - 2. Drop temperature - Kälteloch
   - 3. Lower temperature - Temperatur 200mm
   - 4. Middle temperature - Temperatur 300mm
   - 5. Upper temperature - Temperatur 400mm
   - 6. Roof temperature - Dachtemperatur (DHT22)
   - 7. Roof humidity - Dachfeuchtigkeit (DHT22)
   - 8. Weight - Gewicht
   - X. Battery voltage - Akkuspannung
 **********************************************************/
#ifndef __SENSORREADER_H__
#define __SENSORREADER_H__

#ifndef ARDUINO_AVR_FEATHER32U4
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#include <DHT.h>
#include <HX711.h>
#include "calibration.h"

#if defined(__ASR6501__)
  #define DHT_PIN            GPIO4
  #define ONEWIRE_PIN        GPIO5
  #define LOADCELL_DOUT_PIN  GPIO2
  #define LOADCELL_SCK_PIN   GPIO3
#else
  #define DHT_PIN             4
  #define ONEWIRE_PIN         5
  #define LOADCELL_DOUT_PIN  A0
  #define LOADCELL_SCK_PIN   A1
#endif

#define TEMPERATURE_PRECISION 12
#define SETUP_SAMPLING        20
#define OPERATIONAL_SAMPLING  10

class SensorReader {
  public:
    void begin() {
      dht.begin();
      #ifndef ARDUINO_AVR_FEATHER32U4
      sensors.begin();
      #endif
      scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
      initialize();
    }

    void powerDown() {
      #ifndef ARDUINO_AVR_FEATHER32U4
      scale.power_down();
      #endif
    }

    void powerUp() {
      #ifndef ARDUINO_AVR_FEATHER32U4
      scale.power_up();
      #endif
      dht.begin();
      initialize();
    }

    void initialize() {
      scaleIsReady = scale.wait_ready_retry(5, 200);
      if (!scaleIsReady) { DEBUG("Scale not ready"); }
      scale.set_scale(LOADCELL_DIVIDER);
      scale.set_offset(LOADCELL_OFFSET);
    }

    #ifndef ARDUINO_AVR_FEATHER32U4
    void listTemperatureSensors() {
      sensors.begin();
      byte deviceCount = sensors.getDeviceCount();
      DEBUG3("\nFound ", deviceCount, " temperature sensor");

      if (sensors.isParasitePowerMode()) {
        DEBUG2("Parasite power is: ", "ON");
      } else {
        DEBUG2("Parasite power is: ", "OFF");
      }

      for (byte i = 0; i < deviceCount; i++) {
        DeviceAddress addr;
        if (sensors.getAddress(addr, i)) {
            Serial.print("Device Address ");
            Serial.print(i);
            Serial.print(": ");
            printBufferAsArray(addr, sizeof(addr));
        } else {
          DEBUG2("Unable to find address for Device ", i);
        }
      }
    }

    void listRawWeight() {
      long value = scale.read_average(SETUP_SAMPLING);
      DEBUG2("Raw weight read: ", value);
    }
    #endif

    void startReading() {
      #ifndef ARDUINO_AVR_FEATHER32U4
      sensors.setResolution(TEMPERATURE_PRECISION);
      sensors.requestTemperatures();
      #endif
      dht.read(true);
    }

    void stopReading() {}

    // DS18B20 temperature sensors on one-wire bus
    #ifndef ARDUINO_AVR_FEATHER32U4
    float getTemperature(int index) {
      const uint8_t* addr = thermometer[index];
      float temperature = sensors.getTempC(addr);
      #if defined(__ASR6501__)
        Serial.print("Thermometer ");
        Serial.print(index);
        Serial.print(" @ 0x");
        for (int i = 0; i < 8; i++) {
            if (addr[i] < 0x10) Serial.print('0');
            Serial.print(addr[i], HEX);
        }
        Serial.print(" shows ");
        Serial.print(temperature);
        Serial.println(" C");
      #endif
      return temperature;
    }
    #endif

    // DHTxx sensor
    float getRoofTemperature() { return dht.readTemperature(); }
    float getRoofHumidity() { return dht.readHumidity(); }

    // HX711 with load cell
    float getWeight() { return scale.get_units(OPERATIONAL_SAMPLING); }

    float getCompensatedWeight() {
      if (!scaleIsReady) { return -127.0f; }
      #if THERMOMETER_COUNT == 0
        return getWeight();
      #else
        float weight = getWeight();
        float outerTemperature = sensors.getTempC(thermometer[THERMOMETER_OUTER]);
        if (isnan(outerTemperature) || outerTemperature == -127.0f) {
          return weight;
        } else {
          return weight - (TEMPERATURE_FACTOR * outerTemperature) - TEMPERATURE_OFFSET;
        }
      #endif
    }

    // battery voltage
    float getVoltage() { return readVcc() / 1000.0; }

  private:
    DHT dht = DHT(DHT_PIN, DHT22);
    #ifndef ARDUINO_AVR_FEATHER32U4
    OneWire oneWire = OneWire(ONEWIRE_PIN);
    DallasTemperature sensors = DallasTemperature(&oneWire);
    #endif
    HX711 scale = HX711();
    boolean scaleIsReady = false;

    void printBufferAsArray(byte* buffer, int length) {
      Serial.print("{ 0x");
      for (uint8_t i = 0; i < length; i++) {
        if (i > 0) Serial.print(", 0x");
        if (buffer[i] < 16) Serial.print("0");
        Serial.print(buffer[i], HEX);
      }
      Serial.println(" }");
    }

    long readVcc() {
      noInterrupts();
      #if defined(__ASR6501__)
        pinMode(VBAT_ADC_CTL, OUTPUT);
        digitalWrite(VBAT_ADC_CTL, LOW);
        long result = analogRead(ADC) * 2;
        digitalWrite(VBAT_ADC_CTL, HIGH);
        return result;
      #elif defined(__SAMD21G18A__)
        return 3300L; // TODO
      #else
        // see https://forum.arduino.cc/index.php?topic=120693.msg908179#msg908179
        // Read 1.1V reference against AVcc
        ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
        delay(2); // Wait for Vref to settle
        ADCSRA |= _BV(ADSC); // Convert
        while (bit_is_set(ADCSRA, ADSC));
        long result = ADCL;
        result |= ADCH<<8;
        result = 1126400L / result; // Back-calculate AVcc in mV (1.1V * 1024 * 1000)
        return result;
      #endif
      interrupts();
    }

};

#endif
