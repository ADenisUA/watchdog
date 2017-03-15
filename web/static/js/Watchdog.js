'use strict';

let instance;

const TEMPERATURE_THRESHOLD = 1;
const SOUND_LEVEL_THRESHOLD = 200;
const LIGHT_LEVEL_THRESHOLD = 300;

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
            instance.getApi().setTemperatureThreshold(TEMPERATURE_THRESHOLD);
            instance.getApi().setSoundLevelThreshold(SOUND_LEVEL_THRESHOLD);
            instance.getApi().setLightLevelThreshold(LIGHT_LEVEL_THRESHOLD);
        }
        return instance;
    }
}