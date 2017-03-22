"use strict";

var Utils = {};
Utils.callFunction = function(callback, params) {
    try {
        if (callback) callback(params);
    } catch (e) {
        console.log("Exception on callback execution", e);
    }
};

Utils.respond = function(response, status, data) {
    if (response && !response.finished) {
        response.status(status).json(data);
        return true;
    } else {
        return false;
    }
};

module.exports = Utils;