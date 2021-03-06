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
        sendToDb(body);
        console.log('Payload recorded');
        const device = findDevice(body.dev_id);
        if (device && device.thingspeak) {
            await sendToThingspeak(device, body.payload_fields);
            console.log('Device shown on Thingspeak #' + device.thingspeak.channel_id);
        }
        return {
            statusCode: 201
        };
    } catch (err) {
        console.log(err);
        return err;
    }
};

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

async function sendToDb(payload) {
    payload.timestamp = payload.metadata.time;
    var params = {
        TableName: dbTable,
        Item: payload
    };

    AWS.config.update({region: region});
    const docClient = new AWS.DynamoDB.DocumentClient({apiVersion: '2012-08-10'});
    return docClient.put(params).promise();
}