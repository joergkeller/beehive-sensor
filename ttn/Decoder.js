function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 2A 12 FD 00 00 00 00 00 00 C5 0D 00 00 E8 03 (15 bytes)
  // {
  //   "hiveId": 42,
  //   "humidity": {
  //     "aussen": 0,
  //     "dach": 10
  //   },
  //   "temperature": {
  //     "aussen": -7.5,
  //     "dach": 35.25,
  //     "mitte": 0,
  //     "oben": 0,
  //     "unten": 0
  //   }
  // }

  function asShort(index) { 
    var x = (bytes[index+1] << 8) | bytes[index];
    if ((x & 0xf000) > 0) {
      return -(x ^ 0xffff) - 1;
    }
    return x;
  }
  
  function asFloat(index) {
    return asShort(index) / 100.0;
  } 
  
  return {
    hiveId: bytes[0],
    temperature: {
      aussen: asFloat(1),
      unten:  asFloat(3),
      mitte:  asFloat(5),
      oben:   asFloat(7),
      dach:   asFloat(9)
    },
    humidity: {
      aussen: asFloat(11),
      dach:   asFloat(13)
    }
  };
}
