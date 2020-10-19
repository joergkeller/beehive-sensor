```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
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
arduino-cli compile arduino_beehive_sensor_lora --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=5,LORAWAN_CLASS=0,LORAWAN_NETMODE=0,LORAWAN_ADR=0,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=1,LORAWAN_RGB=1,LORAWAN_DebugLevel=0
arduino-cli compile arduino_beehive_sensor_lora -u -p COM25 --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=5,LORAWAN_CLASS=0,LORAWAN_NETMODE=0,LORAWAN_ADR=0,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=1,LORAWAN_RGB=1,LORAWAN_DebugLevel=0
--- or for dragino LoRa shield/board:
sed -i -e '/#define DEVICE_ID/s/KROKUS/TEST_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
sed -i -e '/#define DEVICE_NAME/s/krokus/test_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
arduino-cli compile arduino_beehive_sensor_lora --fqbn=arduino:avr:uno
--- or for feather32u4 
sed -i -e '/#define DEVICE_ID/s/KROKUS/TEST_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
sed -i -e '/#define DEVICE_NAME/s/krokus/test_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
arduino-cli compile arduino_beehive_sensor_lora --fqbn=adafruit:avr:feather32u4
```