const axios = require('axios')
const AWS = require('aws-sdk');

const region = 'eu-central-1';
const dbTable = 'SensorData';
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
exports.lambdaHandler = async (event, context) => {
    try {
        const body = JSON.parse(event.body);
        console.log(body);
        if (isPayloadV3(body)) {
            await handlePayloadV3(body);
        } else {
            await handlePayloadV2(body);
        }
        const feed = {message: 'Hello world'};
        return {
            statusCode: 201
        };
    } catch (err) {
        console.log(err);
        return err;
    }
};

function isPayloadV3(body) {
    if (body.uplink_message) {
        return true;
    } else {
        return false;
    }
}

async function handlePayloadV2(body) {
    sendToDbV2(body);
    console.log('Payload v2 recorded');
    const device = findDevice(body.dev_id);
    if (device && device.thingspeak) {
        await sendToThingspeak(device, body.payload_fields);
        console.log('Device shown on Thingspeak #' + device.thingspeak.channel_id);
    }
}

async function handlePayloadV3(body) {
    sendToDbV3(body);
    console.log('Payload v3 recorded');
    const device = findDevice(body.end_device_ids.device_id);
    if (device && device.thingspeak) {
        await sendToThingspeak(device, body.uplink_message.decoded_payload);
        console.log('Device shown on Thingspeak #' + device.thingspeak.channel_id);
    }
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

async function sendToDbV2(payload) {
    payload.timestamp = payload.metadata.time;
    var params = {
        TableName: dbTable,
        Item: payload
    };

    AWS.config.update({region: region});
    const docClient = new AWS.DynamoDB.DocumentClient({apiVersion: '2012-08-10'});
    return docClient.put(params).promise();
}

async function sendToDbV3(payload) {
    payload.dev_id = payload.end_device_ids.device_id;
    payload.app_id = payload.end_device_ids.application_ids.application_id;
    payload.timestamp = payload.received_at;
    payload.payload_fields = {};
    payload.payload_fields.sensor = payload.uplink_message.decoded_payload.sensor;
    var params = {
        TableName: dbTable,
        Item: payload
    };

    AWS.config.update({region: region});
    const docClient = new AWS.DynamoDB.DocumentClient({apiVersion: '2012-08-10'});
    return docClient.put(params).promise();
}