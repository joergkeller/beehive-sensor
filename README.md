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
   
## Checkout this project
Local installation of 'git' assumed.
~~~
git clone https://github.com/joergkeller/beehive-sensor.git
cd beehive-sensor
git submodule init
git submodule init
~~~
