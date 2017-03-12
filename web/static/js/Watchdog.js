'use strict';

let instance;

class Watchdog {

    constructor() {
        console.log("Watchdog started");

        var _api = new Api();
        this.getApi = function() {
            return _api;
        }
    }

    static getInstance() {
        if (!instance) {
            instance = new Watchdog();
            instance.getApi().listen();
        }
        return instance;
    }
}