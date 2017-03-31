"use strict";

const express = require("express");
const router = express.Router();
const Utils = require("../lib/Utils.js");
const Brain = require("../lib/Brain.js");

router.get('/write', function(request, response) {
    const content = request.param("content");

    Brain.getInstance().write(content, function (result) {
        Utils.respond(response, (result === Brain.RESULT_OK) ? 200 : 500, {status: result});
    });
});

router.get('/register', function(request, response) {
    const url = request.param("url");

    Brain.getInstance().addListener(url);

    response.status(200).json({status: Brain.RESULT_OK});
});

module.exports = router;
