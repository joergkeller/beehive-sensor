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
arduino-cli compile arduino_beehive_sensor_lora --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=6,LORAWAN_CLASS=0,LORAWAN_DEVEUI=0,LORAWAN_NETMODE=0,LORAWAN_ADR=1,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=1,LORAWAN_RGB=0,LORAWAN_DebugLevel=0
arduino-cli compile arduino_beehive_sensor_lora -u -p COM25 --fqbn=CubeCell:CubeCell:CubeCell-Board:LORAWAN_REGION=6,LORAWAN_CLASS=0,LORAWAN_DEVEUI=0,LORAWAN_NETMODE=0,LORAWAN_ADR=1,LORAWAN_UPLINKMODE=1,LORAWAN_Net_Reserve=0,LORAWAN_AT_SUPPORT=1,LORAWAN_RGB=0,LORAWAN_DebugLevel=0
--- or for dragino LoRa shield/board:
sed -i -e '/#define DEVICE_ID/s/KROKUS/TEST_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
sed -i -e '/#define DEVICE_NAME/s/krokus/test_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
arduino-cli compile arduino_beehive_sensor_lora --fqbn=arduino:avr:uno
--- or for feather32u4 
sed -i -e '/#define DEVICE_ID/s/KROKUS/TEST_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
sed -i -e '/#define DEVICE_NAME/s/krokus/test_123/' arduino_beehive_sensor_lora/arduino_beehive_sensor_lora.ino
arduino-cli compile arduino_beehive_sensor_lora --fqbn=adafruit:avr:feather32u4

minicom -D /dev/ttyUSB0 -b 115200 
```
The compile-time arguments are defined in the boards.txt file of the [CubeCell arduino core](https://github.com/HelTecAutomation/ASR650x-Arduino).

| Value | LORAWAN_REGION |
|-------|----------------|
| 0     | AS923(AS1) |
| 1     | AS923(AS2) |
| 2     | AU915 |
| 3     | CN470 |
| 4     | CN779 |
| 5     | EU433 |
| 6     | EU868 |
| 7     | KR920 |
| 8     | IN865 |
| 9     | US915 |
| 10    | US915_HYBRID |

| Value | LORAWAN_CLASS |
|-------|---------------|
| 0     | CLASS_A |
| 1     | CLASS_B |
| 2     | CLASS_C |

| Value | LORAWAN_DEVEUI |
|-------|----------------|
 | 0     | CUSTOM |
 | 1     | Generate By ChipID |

| Value | LORAWAN_NETMODE |
|-------|-----------------|
| 0     | OTAA |
| 1     | ABP |

| Value | LORAWAN_UPLINKMODE |
|-------|--------------------|
| 0     | CONFIRMED |
| 1     | UNCONFIRMED |

| Value | LORAWAN_DebugLevel |
|-------|--------------------|
 | 0     | None |
 | 1     | Freq |
 | 2     | Freq && DIO |


