'use strict';

let instance;

const TEMPERATURE_THRESHOLD = 1;
const SOUND_LEVEL_THRESHOLD = 400;
const LIGHT_LEVEL_THRESHOLD = 250;

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

            instance.getApi().setTemperatureThreshold(TEMPERATURE_THRESHOLD, function () {
                instance.getApi().setSoundLevelThreshold(SOUND_LEVEL_THRESHOLD, function () {
                    instance.getApi().setLightLevelThreshold(LIGHT_LEVEL_THRESHOLD);
                });
            });
        }
        return instance;
    }

    toggleStreaming() {
        if (this.isStreaming()) {
            this.stopStreaming();
        } else {
            this.startStreaming();
        }

        this.setIsStreaming(!this.isStreaming());
    }

    stopStreaming() {
        $("#streamingView").remove();
        $("#streamingContainer").append('<img id="streamingView" src="https://' + location.hostname + ':3001/stream/snapshot.jpeg" width="100%" height="100%" />');
    }

    startStreaming() {
        $("#streamingView").remove();
        $("#streamingContainer").append('<img id="streamingView" src="https://' + location.hostname + ':3001/stream/video.mjpeg" width="100%" height="100%" />');
    }
}