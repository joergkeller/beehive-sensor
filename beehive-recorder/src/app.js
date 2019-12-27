const axios = require('axios')

const url = 'https://api.thingspeak.com/update.json';
const sampleMessage = {
    api_key: "L4I0S01H3X70958G",
    channel_id: 941478,
    "field1": 0,
    "field2": null,
    "field3": null,
    "field4": 1.2,
    "field6": 98.1,
    "field7": 3.8,
    "field8": 1.7,
    "status": "version 0, 3.8 V",
    created_at: '2019-12-16T05:54:35.630735468Z'
};

const devices = [{
    dev_id: 'test-987',
    thingspeak: {
        channel_id: 941478,
        api_key: 'L4I0S01H3X70958G',
    }
}];

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
        const device = findDevice(body.dev_id);
        if (device && device.thingspeak) {
            await sendToThingspeak(device, body.payload_fields);
            console.log('Device shown on Thingspeak #' + device.thingspeak.channel_id);
        }
        const feed = {message: 'Hello world, ' + body.dev_id}; //await axios(url);
        return {
            statusCode: 201,
            body: feed
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