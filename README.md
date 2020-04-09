# Beehive sensor data
Weight, temperature and humidity measurement in a beehive using Arduino and LoRaWan (TTN).

## Overview
- Arduino (Uno + Dragino LoRa Shield, Dragino LoRa Mini Dev, CubeCell LoRa Dev)
    - Temperature sensors using 1-wire
    - DHT11/22 temperature and humidity sensor
    - Weight cell with HX711 converter
    - LoRaWan connection (TTN)
    - Solar power with rechargeable battery
    - Push-Button with LED to disable temporary (manual mode)
    - Evtl. slave devices with RS-485?
    
- TTN-Gateway/Network/Application
    - Authorization and encoding (OTAA/ABP)
    - Direct ThingSpeak integration plugin (1 device, 1 application, 1 channel)
    - HTTP Integration plugin
    - see [TTN application](./docs/ttn-application.md)
    
- Mapping and routing sensor data eg. AWS Api Gateway
    - Simple extension eg. AWS Lambda (data store, device specific routing, alarming, custom app)
    - see [AWS serverless](./docs/aws-serverless.md)
    - Storing sensor docs in DynamoDb, see [AWS recording](./docs/aws-recorder.md)
    
- Dashboard and visualization with ThingSpeak channel
    - Simple to setup and share
    - Mobile app available
    - Matlab analysis possible
    - Limited to 8 fields/channel
    - No linking of multiple channels (beehives)
    - Limited user interaction (drilldown)
       
## Hardware
Three boards have been tested:
- Arduino Uno + [Dragino LoRa Shield](https://www.dragino.com/products/lora/item/102-lora-shield.html)
    - Needs 5V supply
    - Many digital pins already used by shield
    - Classic Uno size and MCU (16MHz ATMega328P, 32KB FLASH, 2KB SRAM)
- [Dragino LoRa Mini Dev](https://www.dragino.com/products/lora/item/126-lora-mini-dev.html)
    - 3.3V (possible LiPo, not tested)
    - Small size, LoRa transmitter integrated
    - Classic Arduino MCU (16MHz ATMega328P, 32KB FLASH, 2KB SRAM)
- [Heltec CubeCell LoRa Dev](https://heltec.org/project/htcc-ab01/)
    - 3.3V (LiPo, integrated solar charging logic)
    - Power efficient (1W solar cell with 230mAh LiPo loads on a single sunny day and has almost 2 weeks of buffer)
    - Small size, LoRa transmitter integrated
    - Stronger MCU (48 MHz ARM M0+, 128KB FLASH, 16KB SRAM)
    
Used pins:

| Pin Function | Wiring | Arduino / Dragino | CubeCell |
| ------------ | ------ | ----------------- | -------- |
| 1-wire Temp sensor | pull-up resistor 4k7 to VCC | D5 | GPIO5 |
| DHT-11/22 | pull-up resistor 4k7 to VCC | D4 | GPIO4 |
| HX711 Dout | | A0 | GPIO2 |
| HX711 Sck | | A1 | GPIO3 |
| Push-Button | to GND (active low) | D3 | GPIO7 (& battery test control) |
| LED | to VCC (active low) | A2 | GPIO1 |
       
## Transmitted LoRa message (binary encoded)
- Fixed size and order of measured values
- Values are transmitted as short integer values with 2 digits (-327.67 .. 327.67)
- Reserved value to represent null (-327.68) 
~~~
   {
     "version": 0,      // command id or version
     "battery": 3.31,
     "humidity": {
       "outer": 65.43
     },
     "temperature": {
       "lower": 18.1,
       "middle": 19.2,
       "outer": -7.5,
       "upper": 20.29
     },
     "weight": 15.5
   }
~~~   
   
## Checkout this project
Local installation of 'git' assumed.
~~~
git clone https://github.com/joergkeller/beehive-sensor.git
cd beehive-sensor
git submodule init
git submodule update
~~~
Then
- Create TTN account/application/device and enter OTAA/ABP authorization codes in `credentials.h`
- Compile/upload sketch
- Activation with OTAA takes some time
- Press button to enter 'manual mode'
    - no LoRa messages sending, blinking LED instead
    - continous weight measuring for calibration, calculate offset/scale (eg. see [Excel sheet](./docs/Gewicht%20Eichung%20Loadcell.xlsx)) 
    - scanning 1-wire temperature sensors one by one, note ids
    - press button again to start LoRa activation again
- Enter 1-wire sensor ids and weight calibration to `calibration.h`
- Compile/upload sketch again 