'use strict';

let instance;

const TEMPERATURE_THRESHOLD = 1;
const SOUND_LEVEL_THRESHOLD = 400;
const LIGHT_LEVEL_THRESHOLD = 250;
const SPEED = 90;

class Watchdog {

    constructor() {
        console.log("Watchdog started");

        let _api = new Api();

        this.getApi = function() {
            return _api;
        };


        let _isStreaming = false;
        let _isLedOn = false;

        this.setIsStreaming = function(value) {
            _isStreaming = value;
        };

        this.isStreaming = function() {
            return _isStreaming;
        }
    }

    static getInstance() {
        if (!instance) {
            instance = new Watchdog();
            instance.getApi().listen(function(data) {
                $("#output").html(data);
            });

            instance.getApi().setTimestamp((Math.round(new Date().getTime()/1000)), function () {
                instance.getApi().setTemperatureThreshold(TEMPERATURE_THRESHOLD, function () {
                    instance.getApi().setSoundLevelThreshold(SOUND_LEVEL_THRESHOLD, function () {
                        instance.getApi().setLightLevelThreshold(LIGHT_LEVEL_THRESHOLD, function () {

                        });
                    });
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
        stopWebrtc();

        $("#streamingView").remove();
        $("#streamingContainer").prepend('<img id="streamingView" src="https://' + location.hostname + ':3001/stream/snapshot.jpeg?timestamp=' +  (new Date()).getTime() + '" />');
    }

    startStreaming() {
        $("#streamingView").remove();
        $("#streamingContainer").prepend('<video id="streamingView" autoplay muted />');
        startWebRtc();
    }

    startStreamingFallback() {
        stopWebrtc();

        $("#streamingView").remove();
        $("#streamingContainer").prepend('<img id="streamingView" src="https://' + location.hostname + ':3001/stream/video.mjpeg" />');
    }
}