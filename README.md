# Beehive sensor data
Weight, temperature and humidity measurement in a beehive using Arduino and LoRaWan (TTN).\
[Deutscher Text](./README-de.md).

![Arduino compile test](https://github.com/joergkeller/beehive-sensor/workflows/Arduino%20compile%20test/badge.svg)

To support bee keepers in monitoring their bees with minimal interference, a set of sensors will measure and transmit the current state in short intervalls. Design goals:
* Energy efficiency. Devices should be able to run on batteries and solar cells for unlimited time.
* Low radiation. I do not know whether or not radio transmissions disturbe bees. Using a low-power technology like LoRa rather than GSM minimizes the effect.
* Low cost. Small microcontrollers are very affordable and sufficient for the task.

## System Overview
- **Arduino** (alternatively Uno + Dragino LoRa Shield, Dragino LoRa Mini Dev, or CubeCell LoRa Dev)\
  A small microcontroller has enough power to measure every couple of minutes. A rechargeable battery serves as buffer to measure during night and cloudy weather.\
  The device works without any human intervention except a button to put it to a 'manual mode' when working with the beehive.
    - Several (e.g. 5) DS18B20 temperature sensors using 1-wire
    - DHT11/22 temperature and humidity sensor
    - Weight cell with HX711 converter
    - LoRaWan connection (TTN)
    - Solar power and rechargeable battery (LiPo)
    - Push-Button with LED to disable temporary (manual mode)
    - Evtl. slave devices with RS-485?
    
- **LoRaWAN** TTN-Gateway/Network/Application\
  Beehives usually have no wired internet connection and high-speed connections using GSM and WLAN is avoided to keep radiation low.\
  [TheThingsNetwork](https://www.thethingsnetwork.org/) TTN is a global open LoRa network. If no gateway is available within some km near the beehives, an additional gateway can easily be installed. 
    - Transmits messages from the LoRa gateways to the internet, handles authorization (OTAA/ABP) and encoding/decoding of messages
    - Allows direct ThingSpeak integration (only 1 device per application with 1 channel)
    - HTTP integration allows to address any possible backend (many devices per application, see below)
    - see [TTN application](./docs/ttn-application.md)
    
- Dashboard and visualization with **ThingSpeak channel**\
  This is the quick and simple way to display the sensor data.
    - Simple to setup and share [ThingSpeak](https://thingspeak.com/)
    - Mobile app available
    - Matlab analysis possible
    - Limited to 8 fields/channel
    - No linking of multiple channels (beehives)
    - Limited user interaction (drilldown)

- **Mapping and routing** sensor data eg. using AWS (work in progress) \
  Alternative or extension to the ThingSpeak display. 
  Such a back-end application can be used to collect the sensor data and to feed a web application.
    - see [AWS serverless](./docs/aws-serverless.md)
    - Forwarding to ThingSpeak channels as above, but allowing many devices/channels per TTN application
    - Additionally storing sensor docs in DynamoDb, see [AWS recording](./docs/aws-recorder.md)
    - No restrictions of the number of fields or the way sensor data is displayed (own WebApp) 
    
       
## Device Hardware
Three boards have been tested:
- Arduino Uno + [Dragino LoRa Shield](https://www.dragino.com/products/lora/item/102-lora-shield.html)
    - Needs 5V supply
    - Many digital pins already used by shield
    - Classic Uno size and MCU (16MHz ATMega328P, 32KB FLASH, 2KB SRAM)
- [Dragino LoRa Mini Dev](https://www.dragino.com/products/lora/item/126-lora-mini-dev.html)
    - 3.3V (possible LiPo, not tested)
    - Small size, LoRa transmitter integrated
    - Classic Arduino MCU (16MHz ATMega328P, 32KB FLASH, 2KB SRAM)
- [Heltec CubeCell LoRa Dev](https://heltec.org/project/htcc-ab01/). My favorite!
    - 3.3V (possible LiPo, integrated solar charging logic)
    - Power efficient (1W solar cell with 230mAh LiPo loads on a single sunny day and has almost 2 weeks of buffer)
    - Small size, LoRa transmitter integrated
    - Stronger MCU (48 MHz ARM M0+, 128KB FLASH, 16KB SRAM)
    
Used pins:

| Pin Function | Wiring | Uno+Shield / Dragino | CubeCell |
| ------------ | ------ | ----------------- | -------- |
| 1-wire Temp sensor | pull-up resistor 4k7 to VCC | D5 | GPIO5 |
| DHT-11/22 | pull-up resistor 4k7 to VCC | D4 | GPIO4 |
| HX711 Dout | | A0 | GPIO2 |
| HX711 Sck | | A1 | GPIO3 |
| Push-Button | to GND (active low, triggers interrupt) | D3 | GPIO7 (aka battery test control) |
| LED | to VCC (active low) | A2 | GPIO1 |
       
## Transmitted LoRa message (binary encoded)
- The device measures about every 5 min
- Measures will be transmitted on significant changes or every 30 min (messages may get lost)
- Currently no uplink messages 
- Fixed size and order of measured values
- Values are transmitted as short integer values with 2 digits (-327.67 .. 327.67)
- Reserved value to represent null (-327.68) 
~~~
 "sensor": {
   "version": 0,  // command id or version
   "battery": 3.92,
   "weight": 0.37,
   "humidity": {
     "roof": 47.5
   },
   "temperature": {
     "drop": 20.68,
     "lower": 19.5,
     "middle": 19.93,
     "outer": 20.62,
     "roof": 22.2,
     "upper": 19.37
   }
 }
~~~   
   
## Checkout this project
Local installation of `git` and `Arduino IDE` assumed.\
Install the CubeCell board: https://heltec-automation-docs.readthedocs.io/en/latest/cubecell/quick_start.html
~~~
git clone https://github.com/joergkeller/beehive-sensor.git
cd beehive-sensor
git submodule init
git submodule update
~~~
To fetch the latest updates:
~~~
cd beehive-sensor
git stash
git pull
git submodule update
git stash pop
~~~

| Arduino Settings      | Value |
| ----------------------|-------|
| File > Preferences > Settings > Board Manager URLs | `http://resource.heltec.cn/download/package_CubeCell_index.json`
| File > Preferences > Settings > Sketchbook location | `C:\...\beehive-sensor` |
| File > Open... | `C:\...\beehive-sensor\arduino_beehive_sensor_lora\arduino_beehive_sensor_lora.ino` |
| Tools > Board | `CubeCell-Board` |
| Tools > LORAWAN_REGION | `REGION_EU868` or your region |
| Tools > LORAWAN_CLASS | `CLASS_A` |
| Tools > LORAWAN_NETMODE | `OTAA` |
| Tools > LORAWAN_ADR | `ON` |
| Tools > LORAWAN_UPLINKMODE | `UNCONFIRMED` |
| Tools > LORAWAN_NET_RESERVATION | `OFF` |
| Tools > LORAWAN_AT_SUPPORT | `OFF` |
| Tools > LORAWAN_RGB | `ACTIVE` or `DEACTIVE` |
| Monitor baud rate | `115200` |

The project already includes all required libraries (that is the reason for the submodule commands and for the local sketchbook location).

Then
1. Create free TTN account/application/device and enter OTAA/ABP authorization codes in `credentials.h`
1. Create free ThingSpeak account/channel and a TTN ThingSpeak integration with the appropriate values 
1. Compile/upload sketch
1. Activation with OTAA takes some time (even half an hour or so)
1. Press button to enter 'manual mode'
    - no LoRa messages sending, blinking LED instead
    - continous weight measuring for calibration, calculate offset/scale (eg. see [Excel sheet](./docs/Gewicht%20Eichung%20Loadcell.xlsx))
    - for temperature compensation, measure weights at different temperatures also 
    - scanning 1-wire temperature sensors one by one, note ids
    - press button again to start LoRa activation
1. Enter 1-wire sensor ids and weight calibration to `calibration.h`
1. Compile/upload sketch again 
    - Wait for OTAA activation
    - First measure should appear in the local monitor, as TTN data and in ThingSpeak channel