# Beehive sensor data
Weight, temperature and humidity measurement in a beehive using Arduino and LoRaWan (TTN).

## Overview
- Arduino (Uno + Dragino LoRa Shield, Dragino LoRa Mini Dev, CubeCell LoRa Dev)
    - Temperature sensors using 1-wire (D5/GPIO5)
    - DHT11/22 temparature and humidity sensor (D4/GPIO4)
    - Weight cell with HX711 converter (Dout: A0/GPIO2, Sck: A1/GPIO3)
    - LoRaWan connection (TTN)
    - Solar power with rechargeable battery
    - Button (D3/GPIO0) with LED (A2/GPIO1) to disable temporary (manual mode)
    - Evtl. slave devices with RS-485?
    
- TTN-Gateway/Network/Application
    - Authorization and encoding (OTAA/ABP)
    - Direct ThingSpeak integration plugin (1 device, 1 application, 1 channel)
    - HTTP Integration plugin
    - see [TTN application](./docs/ttn-application.md)
    
- Mapping and routing sensor data eg. AWS Api Gateway
    - Simple extension eg. AWS Lambda (data store, device specific routing, alarming, custom app)
    - see [AWS serverless](./docs/aws-serverless.md)
    
- Dashboard and visualization with ThingSpeak channel
    - Simple to setup and share
    - Mobile app available
    - Matlab analysis possible
    - Limited to 8 fields/channel
    - No linking of multiple channels (beehives)
    - Limited user interaction (drilldown)
       
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
