/**********************************************************
 * Wrapper code for HX711, DHT-xx and DallasTemperature sensors.
 * ---
 * Acquiring sensor data from several sources.
 **********************************************************/
#ifndef __SENSORREADER_H__
#define __SENSORREADER_H__

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX711.h>
#include "calibration.h"

#define DHT_PIN 4

#define ONEWIRE_PIN 3
#define TEMPERATURE_PRECISION 12

#define LOADCELL_DOUT_PIN  A0
#define LOADCELL_SCK_PIN  A1
#define SETUP_SAMPLING 100
#define OPERATIONAL_SAMPLING 10


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
    }

    void listTemperatureSensors() {
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

    void listRawWeight() {
      long value = scale.read_average(SETUP_SAMPLING);
      Serial.print("Raw weight read: ");
      Serial.println(value);
    }

    void startReading() {
      sensors.requestTemperatures();
    }

    void stopReading() {}

    // DS18B20 temperature sensors on one-wire bus
    float getLowerTemperature() { return sensors.getTempC(lowerThermometer); }
    float getMiddleTemperature() { return sensors.getTempC(middleThermometer); }
    float getUpperTemperature() { return sensors.getTempC(upperThermometer); }

    // DHTxx sensor
    float getOuterTemperature() { return dht.readTemperature(); }
    float getOuterHumidity() { return dht.readHumidity(); }

    // HX711 with load cell
    float getWeight() { return scale.get_units(OPERATIONAL_SAMPLING); }

    // battery voltage
    float getVoltage() { return readVcc() / 1000.0; }

  private:
    const DHT dht = DHT(DHT_PIN, DHT22);
    const OneWire oneWire = OneWire(ONEWIRE_PIN);
    const DallasTemperature sensors = DallasTemperature(&oneWire);
    const HX711 scale = HX711();

    void printBufferAsArray(byte* buffer, int length) {
      Serial.print("{ 0x");
      for (uint8_t i = 0; i < length; i++) {
        if (i > 0) Serial.print(", 0x");
        if (buffer[i] < 16) Serial.print("0");
        Serial.print(buffer[i], HEX);
      }
      Serial.println(" }");
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

};

#endif
