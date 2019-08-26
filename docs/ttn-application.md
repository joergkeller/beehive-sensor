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
 
 Example message: ```00 2A 12 FD 12 07 80 07 ED 07 DD 09 8F 19 4E 25 0E 06 4B 01```