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

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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
      sensors.begin();
      scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
      scale.set_scale(LOADCELL_DIVIDER);
      scale.set_offset(LOADCELL_OFFSET);
    }

    void powerDown() {
      scale.power_down();
    }

    void powerUp() {
      scale.power_up();
      dht.begin();
    }

    void listTemperatureSensors() {
      sensors.begin();
      byte deviceCount = sensors.getDeviceCount();
      Serial.print("\nFound ");
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

    void listRawWeight() {
      long value = scale.read_average(SETUP_SAMPLING);
      Serial.print("Raw weight read: ");
      Serial.println(value);
    }

    void startReading() {
      sensors.setResolution(TEMPERATURE_PRECISION);
      sensors.requestTemperatures();
      dht.read(true);
    }

    void stopReading() {}

    // DS18B20 temperature sensors on one-wire bus
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

    // DHTxx sensor
    float getRoofTemperature() { return dht.readTemperature(); }
    float getRoofHumidity() { return dht.readHumidity(); }

    // HX711 with load cell
    float getWeight() { return scale.get_units(OPERATIONAL_SAMPLING); }

    float getCompensatedWeight() {
      float weight = getWeight();
      float outerTemperature = sensors.getTempC(thermometer[THERMOMETER_OUTER]);
      if (isnan(outerTemperature) || outerTemperature == -127.0f) {
        return weight;
      } else {
        return weight - (TEMPERATURE_FACTOR * outerTemperature) - TEMPERATURE_OFFSET;
      }
    }

    // battery voltage
    float getVoltage() { return readVcc() / 1000.0; }

  private:
    DHT dht = DHT(DHT_PIN, DHT22);
    OneWire oneWire = OneWire(ONEWIRE_PIN);
    DallasTemperature sensors = DallasTemperature(&oneWire);
    HX711 scale = HX711();

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
      #else
        // see https://forum.arduino.cc/index.php?topic=120693.msg908179#msg908179
        // Read 1.1V reference against AVcc
        ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
        delay(2); // Wait for Vref to settle
        ADCSRA |= _BV(ADSC); // Convert
        while (bit_is_set(ADCSRA, ADSC));
        long result = ADCL;
        result |= ADCH<<8;
        result = 1126400L / result; // Back-calculate AVcc in mV
        return result;
      #endif
      interrupts();
    }

};

#endif
