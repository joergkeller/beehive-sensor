/**********************************************************
 * Sensor calibration for load cell calibration and
 * temperature sensor addresses.
 **********************************************************/
#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

  // load cell calibration (calculate from raw values during setup phase)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)
  #define TEMPERATURE_FACTOR 0.0e0  // gradient of trendline
  #define TEMPERATURE_OFFSET 0.0e0  // offset of trendline

  // set unique DS18B20 addresses of used sensors
  #define THERMOMETER_COUNT 5 // number of 1-wire thermometers, addresses below
  #define THERMOMETER_OUTER 0
  const DeviceAddress thermometer[THERMOMETER_COUNT] = {
    { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 }, // Aussentemperatur
    { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 }, // Temperatur Kälteloch
    { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 }, // Temperatur 200mm
    { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 }, // Temperatur 300mm
    { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 }  // Temperatur 400mm
  };

#endif
