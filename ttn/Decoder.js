function Decoder(bytes, port) {
  // Decode an ThingSpeak formatted uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 00 7E 01 AB 00 00 80 00 80 00 80 00 80 00 80 00 80 00 80 (19 bytes)
  //   {
  //     "sensor": {
  //       "version": 0,
  //       "battery": 3.82,
  //       "weight": 1.71,
  //       "humidity": {
  //         "roof": null
  //       },
  //       "temperature": {
  //         "roof": null,
  //         "upper": null,
  //         "middle": null,
  //         "lower": null,
  //         "drop": null,
  //         "outer": null
  //       }
  //     }
  //   }

  function asShort(index) {
    if (bytes.length < index) return null;
    var x = (bytes[index+1] << 8) | bytes[index];
    if ((x & 0x8000) > 0) {
      return -(x ^ 0xffff) - 1;
    }
    return x;
  }

  function asFloat(index) {
    var value = asShort(index);
    if (!value || value == -32768) {
      return null;
    } else {
      return value / 100.0;
    }
  }

  var sensorData = {
    version: bytes[0],
    battery: asFloat(1),
    weight: asFloat(3),
    humidity: {
      roof: asFloat(5)
    },
    temperature: {
      roof: asFloat(7),
      upper: asFloat(9),
      middle: asFloat(11),
      lower: asFloat(13),
      drop: asFloat(15),
      outer: asFloat(17)
    }
  };

  return {
    sensor: sensorData
  }
}
