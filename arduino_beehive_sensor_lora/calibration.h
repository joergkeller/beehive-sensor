/**********************************************************
 * Sensor calibration for load cell calibration and DS18B20
 * temperature sensor addresses.
 * ---
 * Go to manual mode (press USR button) to list all sensors
 * and read raw weight readings.
 **********************************************************/
#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

#if DEVICE_NAME == krokus

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_NAME == shakra

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_NAME == gotthard

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_NAME == cube-cell-1

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#elif DEVICE_NAME == test-123

  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0 // 0-based index of temperature reading for weight compensation
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0: Aussentemperatur
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1: Temperatur Kälteloch
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2: Temperatur 200mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3: Temperatur 300mm
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 4: Temperatur 400mm
  };

#endif

#endif
