/**********************************************************
 * Sensor calibration for load cell calibration and DS18B20
 * temperature sensor addresses.
 * ---
 * Go to manual mode (press USR button) to list all sensors
 * and read raw weight readings.
 **********************************************************/
#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

#if DEVICE_ID == KROKUS

  #define LOADCELL_DIVIDER 10458    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET 481976    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR -9.6755E-02  // gradient of trendline
  #define TEMPERATURE_OFFSET +9.3877E-01  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x28, 0xD7, 0x47, 0x79, 0xA2, 0x16, 0x03, 0xC1 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_ID == SHAKRA

  #define LOADCELL_DIVIDER 10263    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET  17838    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR -0.1432  // gradient of trendline
  #define TEMPERATURE_OFFSET +3.4714  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x28, 0x23, 0x70, 0x30, 0x0A, 0x00, 0x00, 0xBE }, // 0: Aussentemperatur
    { 0x28, 0x19, 0xE0, 0x79, 0xA2, 0x16, 0x03, 0x2F }, // 1: Temperatur Kälteloch
    { 0x28, 0xF4, 0x3A, 0x79, 0xA2, 0x16, 0x03, 0xAF }, // 2: Temperatur 200mm
    { 0x28, 0xFE, 0xFF, 0x79, 0xA2, 0x16, 0x03, 0x90 }, // 3: Temperatur 300mm
    { 0x28, 0xD3, 0x2A, 0x79, 0xA2, 0x19, 0x03, 0xF8 }  // 4: Temperatur 400mm
  };

#elif DEVICE_ID == GOTTHARD

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR -7.1332E-03 // gradient of trendline
  #define TEMPERATURE_OFFSET -1.6111E+00 // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 }, // 0: Aussentemperatur
    { 0x28, 0x13, 0x41, 0x79, 0xA2, 0x16, 0x03, 0x1E }, // 1: Temperatur Kälteloch
    { 0x28, 0xA6, 0x47, 0x79, 0xA2, 0x16, 0x03, 0x6E }, // 2: Temperatur 200mm
    { 0x28, 0x3F, 0xE0, 0x79, 0xA2, 0x16, 0x03, 0x2B }, // 3: Temperatur 300mm
    { 0x28, 0x8F, 0xE1, 0x79, 0xA2, 0x16, 0x03, 0xE1 }  // 4: Temperatur 400mm
  };

#elif DEVICE_ID == WITCHES

  #define LOADCELL_DIVIDER  0    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET   0    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0 // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0 // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x28, 0xA6, 0x47, 0x79, 0xA2, 0x16, 0x03, 0x6E }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_ID == CUBE_CELL_1

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 }, // 0: Aussentemperatur
    { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_ID == TEST_123

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #ifdef ARDUINO_AVR_FEATHER32U4
      #define THERMOMETER_COUNT 0 // number of 1-wire thermometers
  #else
      #define THERMOMETER_COUNT 0 // number of 1-wire thermometers, addresses below
      #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
      const DeviceAddress thermometer[THERMOMETER_COUNT] = {
//        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
//        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
//        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
//        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
//        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
      };
  #endif

#endif

#endif
