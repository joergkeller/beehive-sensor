```
arduino-cli config init --dest-dir .
```

```
arduino-cli core list
arduino-cli core update-index
arduino-cli core install CubeCell:CubeCell
arduino-cli core install arduino:avr
arduino-cli board listall CubeCell
arduino-cli board listall Uno
```

```
arduino-cli board list
arduino-cli compile --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=5,LORAWAN_CLASS=0,LORAWAN_NETMODE=0,LORAWAN_ADR=0,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=1,LORAWAN_RGB=1,LORAWAN_DebugLevel=0 arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
arduino-cli compile -u -p COM25 --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=5,LORAWAN_CLASS=0,LORAWAN_NETMODE=0,LORAWAN_ADR=0,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=1,LORAWAN_RGB=1,LORAWAN_DebugLevel=0 arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
--- or for dragino LoRa shield/board:
arduino-cli compile --fqbn=arduino:avr:uno  arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino 
```