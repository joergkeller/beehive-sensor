function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 2A 12 FD 12 07 80 07 ED 07 8F 19 0E 06 4B 01 (15 bytes)
  // {
  //   "battery": 3.31,
  //     "humidity": {
  //   "outer": 65.43
  // },
  //   "temperature": {
  //   "lower": 18.1,
  //       "middle": 19.2,
  //       "outer": -7.5,
  //       "upper": 20.29
  // },
  //   "version": 42,
  //     "weight": 15.5
  // }

  function asShort(index) {
    var x = (bytes[index+1] << 8) | bytes[index];
    if ((x & 0x8000) > 0) {
      return -(x ^ 0xffff) - 1;
    }
    return x;
  }
  
  function asFloat(index) {
    return asShort(index) / 100.0;
  } 
  
  return {
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
}
