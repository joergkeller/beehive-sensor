aws dynamodb get-item --table-name SensorData --key file://test/query/key.json --output json

aws dynamodb query --table-name SensorData \
    --key-condition-expression "dev_id=:dev AND #ts>=:ts" \
    --expression-attribute-names "#ts=timestamp" \
    --expression-attribute-values file://test/query/query.json \
    --projection-expression "#ts,payload_fields.sensor,metadata.data_rate" \
    --output json

