'use strict';

const app = require('../../src/app');
const chai = require('chai');
const expect = chai.expect;
var event, context;

describe('Tests index', function () {
    it('verifies successful response', async () => {
        event = require('../../events/event');
        const result = await app.lambdaHandler(event, context)

        expect(result).to.be.an('object');
        expect(result.statusCode).to.equal(201);
        expect(result.body).to.be.an('object');
        expect(result.body.message).to.be.equal('Hello world, test-987')

        // let response = JSON.parse(result.body);
        //
        // expect(response).to.be.an('object');
        // expect(response.message).to.be.equal("hello world");
        // expect(response.location).to.be.an("string");

    });
});
