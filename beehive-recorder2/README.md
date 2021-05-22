# TTN v3 Community Edition


https://www.thethingsnetwork.org/docs/the-things-stack/

## Payload decoder

see ```../ttn/Decoder.js```

# AWS Default Integration

To allow the integration, in the [TTN console (v3)](https://eu1.cloud.thethings.network/console/) an API key needs to created.
The given permissions were not sufficient.

The AWS integration can be created using a [provided CloudFormation description](https://s3.amazonaws.com/thethingsindustries/integration-aws/latest/community.template.json), 
locally versioned in ```src/main/cf/iot-integration```.

| Parameter | Value | Description |
|-----------|-------|-------------|
| ThingTypeName | lorawan | default |
| ThingNameScheme | DeviceID | DeviceID or DevEUI |
| ThingShadowMetrics | Enabled | last connection etc. |
| ClusterAddress | eu1.cloud.thethings.network | default |
| ApplicationID | bienen-test | given id |
| ApplicationAPIKey | **** | Secret value of the generated TTN API key |

```
aws cloudformation deploy --stack-name test-beehive-sensors --template-file src/main/cf/iot-integration/community.template.json --capabilities CAPABILITY_IAM --parameter-overrides file://parameters.json --profile beehive-build
```

## Nested Stack

https://www.trek10.com/blog/cloudformation-nested-stacks-primer

```
aws cloudformation deploy --stack-name test-beehive-storage --template-file src/main/cf/datastore/sensordata.template.yaml --capabilities CAPABILITY_IAM --profile beehive-build
aws cloudformation deploy --stack-name test-beehive-message --template-file src/main/cf/job/message-recorder.template.yaml --capabilities CAPABILITY_IAM --parameter-overrides TargetTable=BeeHive_Test --profile beehive-build
```

```
aws cloudformation package --template-file cf/beehive.template.yaml --output-template beehive.packaged.yaml --s3-bucket beehive-mapper-dev --profile beehive-build
aws cloudformation deploy --stack-name Test-Beehive --template-file beehive.packaged.yaml --capabilities CAPABILITY_IAM CAPABILITY_AUTO_EXPAND --profile beehive-build
```
