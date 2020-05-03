### Query

## GET /sensor/{devId}

See https://aws.amazon.com/de/blogs/compute/using-amazon-api-gateway-as-a-proxy-for-dynamodb/

Integration request: 

| Settings              | Value |
| ----------------------|-------|
| Integration Type      | AWS Service |
| AWS Region            | eu-central-1 |
| AWS Service           | DynamoDB |
| HTTP method           | POST |
| Action                | Query |
| Execution role        | \<IAM role with AmazonDynamoDBReadOnlyAccess, AmazonAPIGatewayPushToCloudWatchLogs\>

Mapping template application/json:
~~~
{
    "TableName": "SensorData",
    "KeyConditionExpression": "dev_id=:dev AND #ts>=:ts",
    "ExpressionAttributeNames": {
        "#ts": "timestamp"
    },
    "ExpressionAttributeValues": {
      ":dev": {
        "S": "$input.params('devid')"
      },
      ":ts": {
        "S": "$input.params('from')"
      }
    },
    "ProjectionExpression": "#ts,payload_fields.sensor,metadata"
}
~~~

Integration response 200 - Mapping template application/json:
~~~
#set($inputRoot = $input.path('$'))
#set($inputRoot = $input.path('$'))
{
    "count": $inputRoot.Count,
    "readings": [
        #foreach($elem in $inputRoot.Items) {
            "timestamp": "$elem.timestamp.S",
            "sensor": {
                "version": "$elem.payload_fields.M.sensor.M.version.N",
                "battery": "$elem.payload_fields.M.sensor.M.battery.N",
                "weight": "$elem.payload_fields.M.sensor.M.weight.N",
                "temperature": {
                    "roof": "$elem.payload_fields.M.sensor.M.temperature.M.roof.N",
                    "upper": "$elem.payload_fields.M.sensor.M.temperature.M.upper.N",
                    "middle": "$elem.payload_fields.M.sensor.M.temperature.M.middle.N",
                    "lower": "$elem.payload_fields.M.sensor.M.temperature.M.lower.N",
                    "drop": "$elem.payload_fields.M.sensor.M.temperature.M.drop.N",
                    "outer": "$elem.payload_fields.M.sensor.M.temperature.M.outer.N"
                },
                "humidity": {
                    "roof": "$elem.payload_fields.M.sensor.M.humidity.M.roof.N"
                }
            },
            "data_rate": "$elem.metadata.M.data_rate.S",
            "gtw_id": "$elem.metadata.M.gateways.L[0].M.gtw_id.S",
            "rssi": "$elem.metadata.M.gateways.L[0].M.rssi.N",
            "snr": "$elem.metadata.M.gateways.L[0].M.snr.N",
            "frequency": "$elem.metadata.M.frequency.N",
            "gateways": [
                #foreach($gw in $elem.metadata.M.gateways.L) {
                  "gtw_id": "$gw.M.gtw_id.S",
                  "channel": "$gw.M.channel.N",
                  "rssi": "$gw.M.rssi.N",
                  "snr": "$gw.M.snr.N",
                  "latitude": "$gw.M.latitude.N",
                  "longitude": "$gw.M.longitude.N",
                  "altitude": "$gw.M.altitude.N"
                }#if($foreach.hasNext),#end
            #end
            ]
        }#if($foreach.hasNext),#end
	#end
    ]
}
~~~

Example https://xxxxxxx.execute-api.eu-central-1.amazonaws.com/v0/sensor/cube-cell-1:
~~~
{
    "count": 23,
    "readings": [
         {
            "timestamp": "2020-01-29T00:38:53.695888554Z",
            "data_rate": "SF7BW125",
            "sensor": {
                "version": "0",
                "weight": "2.96",
                "temperature": {
                    "lower": "6.87",
                    "middle": "",
                    "upper": "",
                    "outer": "7.1"
                },
                "humidity": {
                    "outer": "84.6"
                },
                "battery": "4.06"
            }
        },
        {...}	
    ]
}
~~~