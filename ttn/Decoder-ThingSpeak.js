function Decoder(bytes, port) {
  // Decode an ThingSpeak formatted uplink message from a buffer
  // (array) of bytes to an object of fields.

  // Payload: 00 7E 01 AB 00 00 80 00 80 00 80 00 80 00 80 00 80 00 80 (19 bytes)
  //   {
  //     "field1": null, // Aussentemperatur
  //     "field2": null, // Kälteloch
  //     "field3": null, // Temperatur 200mm
  //     "field4": null, // Temperatur 300mm
  //     "field5": null, // Temperatur 400mm
  //     "field6": null, // Dachtemperatur (DHT22)
  //     "field7": null, // Dachfeuchtigkeit (DHT22)
  //     "field8": 1.71, // Gewicht
  //     "status": "version 0, 3.82 V",
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
