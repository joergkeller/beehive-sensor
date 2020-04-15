'use strict';

var AWS = require('aws-sdk-mock');
AWS.mock('DynamoDB.DocumentClient', 'put', async function (params){
    return "successfully put item in database";
});

const app = require('../../src/app');
const chai = require('chai');
const expect = chai.expect;
var event, context;

describe('Tests index', function () {
    it('verifies successful response', async () => {
        event = require('../../events/event');
        const result = await app.lambdaHandler(event, context)

        expect(result.statusCode).to.equal(201);
        // expect(result.body.message).to.be.equal('Hello world, test-987')

    });
});
