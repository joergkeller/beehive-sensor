# AWS Serverless integration

## Rest Api
### Setup 
AWS > Services > API Gateway > Create API\
Add Ressource: e.g. /beesensor\
Add Method: e.g. POST

## Http integration
- See AWS [Request and Response Data Mapping Reference](https://docs.aws.amazon.com/apigateway/latest/developerguide/request-response-data-mappings.html)

Select resource method, then Integration Request:

| Settings | Value |
|----------|-------| 
| Integration type | HTTP |
| Use HTTP proxy integration    | false |
| HTTP method                   | POST |
| Endpoint URL                  | https://api.thingspeak.com/update.json |
| HTTP Headers > Content-Type   | 'application/json' | 
| Mapping Templates > application/json | see [http-integration-thingspeak-mapper.vtl](file:../beehive-mapper/aws/http-integration-thingspeak-mapper.vtl) |

## Lambda integration
    - Input / output
    - Device specific routing
    - Data storage
    - Build pipeline

### Logging
For an overview, see API Gateway > APIs > xxx > Dashboard.
This is sufficient to see the number of api calls, number of errors, and latency.

To setup logging, an IAM role has to be defined and linked first:\
AWS > Services > IAM > Role: Create 'AmazonAPIGatewayPushToCloudWatchLogs'

Click on the created role and copy the 'Role-ARN' value.\
AWS > Services > API Gateway > Settings > CloudWatch log role ARN: Insert the role arn.\
APIs > xxx > Stages > xxx > Logs/Tracing > CloudWatch Settings: Enable logs

AWS > Services > CloudWatch > Logs > API-Gateway-Execution-Logs_... > Log Stream (latest)

Using Insights:
~~~
fields @timestamp, @message
| filter @message like 'Sending request' or  @message like 'HTTP Method:' or @message like 'Received response' or  @message like 'Method completed'
| sort @timestamp desc
| limit 20
~~~

### OpenApi 3 documentation
AWS > Services > API Gateway > APIs > xxx > Stages > xxx > Export\
Export as Swagger or OpenAPI 3: [see yaml file](file:../beehive-mapper/aws/beehive-mapper-api-test-oas30.yaml)

## Model / Schema validation
see [ttn-http-integration-schema.json](file:../beehive-mapper/aws/ttn-http-integration-schema.json)

## Api key and usage plan
1) AWS > Services > API Gateway > APIs > xxx > Resources > xxx > Method Request > API Key Required = true
1) Deploy API
1) AWS > Services > API Gateway > Usage Plans: Create a plan
1) Add API Stage: Select the relevant API/Stage
1) API Keys: Add/Create a key to use

The API key value is expected as header field 'x-api-key', [see TTN Http Integration](file:./ttn-application.md)

