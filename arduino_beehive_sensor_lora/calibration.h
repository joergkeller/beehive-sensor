/**********************************************************
 * Sensor calibration for load cell calibration and
 * temperature sensor addresses.
 **********************************************************/
#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

  // load cell calibration (calculate from raw values during setup phase)
  #define LOADCELL_OFFSET -43496    // zero_reading: raw value measured with zero weight
  #define LOADCELL_DIVIDER 25365    // real_weight / (weighted_reading - zero_reading)

  // set unique DS18B20 addresses of used sensors
  const DeviceAddress lowerThermometer =  { 0x28, 0xC2, 0xE1, 0x78, 0x0A, 0x00, 0x00, 0x26 };
  const DeviceAddress middleThermometer = { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 };
  const DeviceAddress upperThermometer =  { 0x28, 0x3F, 0x1C, 0x31, 0x02, 0x00, 0x00, 0x02 };

#endif
