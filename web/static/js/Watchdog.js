'use strict';

let instance;

const TEMPERATURE_THRESHOLD = 1;
const SOUND_LEVEL_THRESHOLD = 150;
const LIGHT_LEVEL_THRESHOLD = 50;
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
            instance.getApi().listen(instance.processIncomingData);

            instance.getApi().setTimestamp((Math.round(new Date().getTime()/1000)), function () {
                instance.getApi().setTemperatureThreshold(TEMPERATURE_THRESHOLD, function () {
                    instance.getApi().setSoundLevelThreshold(SOUND_LEVEL_THRESHOLD, function () {
                        instance.getApi().setLightLevelThreshold(LIGHT_LEVEL_THRESHOLD, function () {
                            instance.getApi().getTemperature(function () {
                                instance.getApi().getSoundLevel(function () {
                                    instance.getApi().getLightLevel(function () {

                                    });
                                });
                            });
                        });
                    });
                });
            });
        }
        return instance;
    }

    processIncomingData(data) {
        $("#output").html(data);

        if (data) {
            try {
                var id = null;
                if (data.indexOf("onTemperature") > -1) {
                    id = "#temperature";
                } else if (data.indexOf("onSoundLevel") > -1) {
                    id = "#sound";
                } else if (data.indexOf("onLightLevel") > -1) {
                    id = "#light";
                }

                if (id != null) {
                    data = JSON.parse(data.substring(data.indexOf("{")));
                    console.log(data);
                    $(id).html(data.value);
                }
            } catch (e) {
                console.log("Unable to parse incoming event", e);
            }
        }
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