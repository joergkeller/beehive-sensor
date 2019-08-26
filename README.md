# Beehive sensor data
Weight, temperature and humidity measurement in a beehive using Arduino and LoRaWan (TTN).

## Overview
- Arduino (Uno, Mini Pro, Moteino, Anarduino, ESP32)
    - Temperature, humidity, weight sensors
    - LoRaWan connection
    - Solar power with accumulator buffer (evtl. super capacitor?)
    - Evtl. slave devices with RS-485?
    
- TTN-Gateway/Network/Application
    - Authorization and encoding (OTAA possible)
    - HTTP Integration plugin
    - see [TTN application](./docs/ttn-application.md)
    
- Mapping and routing sensor data eg. AWS Api Gateway
    - Limited to 1:1 mapping TTN vs. ThingSpeak channel (?)
    - Simple extension eg. AWS Lambda (data store, device specific routing, custom app)
    - see [AWS serverless](./docs/aws-serverless.md)
    
- Dashboard and visualization eg. ThingSpeak channel
    - Simple to setup and share
    - Mobile app available
    - Matlab analysis possible
    - Limited to 8 fields/channel
    - No (?) linking of multiple channels (beehives)
    - Limited user interaction (drilldown)
   
## Transmitted LoRa message (binary encoded)
- Fixed size and order of measured values
- Values are transmitted as short integer values with 2 digits (-327.67 .. 327.67)
- Reserved value to represent null (-327.68) 
~~~
   {
     "version": 0,      // command id or version
     "hiveId": 42,      // when multiple hives per device
     "temperature": {
       "aussen": -7.5,  // °C
       "dach": 25.25,
       "mitte": 19.2,
       "oben": 20.29,
       "unten": 18.1
     },
     "humidity": {
       "aussen": 65.43, // % rel
       "dach": 95.5
     },
     "weight": 15.5,    // kilogram
     "battery": 3.31    // volts
   }
~~~   
   
## Checkout this project
Local installation of 'git' assumed.
~~~
git clone https://github.com/joergkeller/beehive-sensor.git
cd beehive-sensor
git submodule init
git submodule init
~~~
