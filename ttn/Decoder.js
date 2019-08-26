function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 00 2A 12 FD 12 07 80 07 ED 07 DD 09 8F 19 4E 25 0E 06 4B 01 (20 bytes)
  // {
  //   "version": 0,
  //   "hiveId": 42,
  //   "battery": 3.31,
  //   "humidity": {
  //     "aussen": 65.43,
  //     "dach": 95.5
  //   },
  //   "temperature": {
  //     "aussen": -7.5,
  //     "dach": 25.25,
  //     "mitte": 19.2,
  //     "oben": 20.29,
  //     "unten": 18.1
  //   },
  //   "weight": 15.5
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
    hiveId: bytes[1],
    temperature: {
      aussen: asFloat(2),
      unten:  asFloat(4),
      mitte:  asFloat(6),
      oben:   asFloat(8),
      dach:   asFloat(10)
    },
    humidity: {
      aussen: asFloat(12),
      dach:   asFloat(14)
    },
    weight: asFloat(16),
    battery: asFloat(18)
  };
}
