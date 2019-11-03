function Decoder(bytes, port) {
  // Decode an ThingSpeak formatted uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 2A 12 FD 12 07 80 07 ED 07 8F 19 0E 06 4B 01 (15 bytes)
  // {
  //   "field1": 18.1,
  //   "field2": 19.2,
  //   "field3": 20.29,
  //   "field4": -7.5,
  //   "field6": 65.43,
  //   "field7": 3.31,
  //   "field8": 15.5,
  //   "sensor": {
  //     "battery": 3.31,
  //     "humidity": {
  //       "outer": 65.43
  //     },
  //     "temperature": {
  //       "lower": 18.1,
  //       "middle": 19.2,
  //       "outer": -7.5,
  //       "upper": 20.29
  //     },
  //     "version": 42,
  //     "weight": 15.5
  //   },
  //   "status": "version 42, 3.31 V"
  // }

  function asShort(index) {
    var x = (bytes[index+1] << 8) | bytes[index];
    if ((x & 0x8000) > 0) {
      return -(x ^ 0xffff) - 1;
    }
    return x;
  }

  function asFloat(index) {
    var value = asShort(index);
    if (value == -32768) {
      return null;
    } else {
      return value / 100.0;
    }
  }

  var sensorData = {
    version: bytes[0],
    temperature: {
      outer: asFloat(1),
      lower:  asFloat(3),
      middle:  asFloat(5),
      upper:   asFloat(7)
    },
    humidity: {
      outer: asFloat(9)
    },
    weight: asFloat(11),
    battery: asFloat(13)
  };

  return { // ThingSpeak format
    field1: sensorData.temperature.lower,
    field2: sensorData.temperature.middle,
    field3: sensorData.temperature.upper,
    field4: sensorData.temperature.outer,
    field6: sensorData.humidity.outer,
    field7: sensorData.battery,
    field8: sensorData.weight,
    status: 'version ' + sensorData.version + ', ' + sensorData.battery + " V",
    sensor: sensorData // ignored by ThingSpeak
  }
}
