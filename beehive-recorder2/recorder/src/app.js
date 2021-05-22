const axios = require('axios')
const AWS = require('aws-sdk');

const region = process.env.AWS_REGION;
const dbTable = process.env.TARGET_TABLE_NAME; // 'Test-Beehive-Datastore-SOYTDMQR274O-SensorDataTable-17ZXGC931ZF87';
const url = 'https://api.thingspeak.com/update.json';

const devices = require('./devices.json');

/**
 * Handler to define in AWS lambda as 'src/app.lambdaHandler'
 *
 * Event doc: https://docs.aws.amazon.com/apigateway/latest/developerguide/set-up-lambda-proxy-integrations.html#api-gateway-simple-proxy-for-lambda-input-format
 * @param {Object} event - API Gateway Lambda Proxy Input Format
 *
 * Context doc: https://docs.aws.amazon.com/lambda/latest/dg/nodejs-prog-model-context.html
 * @param {Object} context
 *
 * Return doc: https://docs.aws.amazon.com/apigateway/latest/developerguide/set-up-lambda-proxy-integrations.html
 * @returns {Object} object - API Gateway Lambda Proxy Output Format
 *
 */
exports.handler = async (event, context) => {
    if (event.body) {
        return messageHandler(event.body);
    } else {
        return messageHandler(event);
    }
};

async function messageHandler(message) {
    try {
        console.log("Event = " + JSON.stringify(message));
        await sendToDb(message);
        console.log('Payload recorded');

        const device = findDevice(message.dev_id);
        if (device && device.thingspeak) {
            await sendToThingspeak(device, message.payload_fields);
            console.log("Device shown on Thingspeak channel ${device.thingspeak.channel_id} with ${message.payload_fields}");
        }
        return {
            statusCode: 201
        };
    } catch (err) {
        console.log(err);
        return err;
    }
}

function toIsoLocal(date, timezone) {
    return date
        .toLocaleString("en-CA", {timeZone: timezone, hour12: false})
        .replace(/, /, "T");
}

async function sendToDb(payload) {
    payload.dev_id = payload.end_device_ids.device_id;
    payload.app_id = payload.end_device_ids.application_ids.application_id;
    payload.timestamp = toIsoLocal(new Date(payload.received_at), "Europe/Zurich");
    payload.payload_fields = payload.uplink_message.decoded_payload;
    delete payload.uplink_message.decoded_payload;
    var params = {
        TableName: dbTable,
        Item: payload
    };

    AWS.config.update({region: region});
    const docClient = new AWS.DynamoDB.DocumentClient({apiVersion: '2012-08-10'});
    return docClient.put(params).promise();
}

function findDevice(devId) {
    console.log('Device is ' + devId);
    const device = devices.find(dev => dev.dev_id === devId);
    return device;
}

async function sendToThingspeak(device, payload) {
    payload.channel_id = device.thingspeak.channel_id;
    payload.api_key = device.thingspeak.api_key;
    return axios.post(url, payload);
}
