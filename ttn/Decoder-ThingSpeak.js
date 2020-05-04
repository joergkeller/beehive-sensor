function Decoder(bytes, port) {
  // Decode an ThingSpeak formatted uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 00 88 01 25 00 8E 12 AC 08 0E 08 14 08 9E 07 C9 07 91 07 (19 bytes)
  // {
  //   "field1": 20.62, // Aussentemperatur
  //   "field2": 20.68, // Kälteloch
  //   "field3": 19.5,  // Temperatur 200mm
  //   "field4": 19.93, // Temperatur 300mm
  //   "field5": 19.37, // Temperatur 400mm
  //   "field6": 22.2,  // Dachtemperatur (DHT22)
  //   "field7": 47.5,  // Dachfeuchtigkeit (DHT22)
  //   "field8": 0.37,  // Gewicht
  //   "status": "version 0, 3.92 V",
  //   "sensor": {
  //     "version": 0,
  //     "battery": 3.92,
  //     "weight": 0.37,
  //     "humidity": {
  //       "roof": 47.5
  //     },
  //     "temperature": {
  //       "drop": 20.68,
  //       "lower": 19.5,
  //       "middle": 19.93,
  //       "outer": 20.62,
  //       "roof": 22.2,
  //       "upper": 19.37
  //     }
  //   }
  // }

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
      outer: asFloat(9),
      drop: asFloat(11),
      lower: asFloat(13),
      middle: asFloat(15),
      upper: asFloat(17)
    }
  };

  return { // ThingSpeak format
    field1: sensorData.temperature.outer,
    field2: sensorData.temperature.drop,
    field3: sensorData.temperature.lower,
    field4: sensorData.temperature.middle,
    field5: sensorData.temperature.upper,
    field6: sensorData.temperature.roof,
    field7: sensorData.humidity.roof,
    field8: sensorData.weight,
    status: 'version ' + sensorData.version + ', ' + sensorData.battery + " V",
    sensor: sensorData // ignored by ThingSpeak
  }
}
