```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
arduino-cli config init --dest-dir .
```

```
arduino-cli core list
arduino-cli core update-index
arduino-cli core install CubeCell:CubeCell@1.2.0
arduino-cli core install arduino:avr
arduino-cli board listall CubeCell
arduino-cli board listall Uno
```

```
arduino-cli board list
arduino-cli compile --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=6,LORAWAN_CLASS=0,LORAWAN_DEVEUI=0,LORAWAN_NETMODE=0,LORAWAN_ADR=1,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=0,LORAWAN_RGB=0,LORAWAN_DebugLevel=0 arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
arduino-cli compile -u -p COM25 --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=6,LORAWAN_CLASS=0,LORAWAN_DEVEUI=0,LORAWAN_NETMODE=0,LORAWAN_ADR=1,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=0,LORAWAN_RGB=0,LORAWAN_DebugLevel=0 arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
--- or for dragino LoRa shield/board:
arduino-cli compile --fqbn=arduino:avr:uno  arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
minicom -D /dev/ttyUSB0 -b 115200 
```