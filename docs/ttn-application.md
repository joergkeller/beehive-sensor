# The Things Network TTN

## Application / device setup
TTN > Application > xxx > Devices

## Decoding
TTN > Application > xxx > Payload Formats > Custom

Since the LoRa message is binary encoded (compact size), a decoding of the custom format is needed.

## TTN Http Integration
TTN > Application > xxx > Integrations

See https://thethingsnetwork.org/docs/applications/http/

| Settings              | Value |
| ----------------------|-------|
| Access Key            | <downlink-key> (generate new key) |
| URL                   | https://rt9jhy41md.execute-api.us-west-2.amazonaws.com/test/beesensor |
| Method                | POST                  |
| Authorization         | -                     |
| Custom Header Name    | x-api-key             |
| Custom Header Value   | <aws-api-key> (copy from AWS) |

## Testing the setup
 TTN > Application > xxx > Devices > Simulate Uplink
 
 Example message: ```2A 12 FD 00 00 00 00 00 00 C5 0D 00 00 E8 03```