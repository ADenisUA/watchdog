"use strict";

const bluetooth = require('node-bluetooth');


var BtSerial = module.exports = function BtSerial() {
    var _connection = null;
    var _this = this;
    var _callback = null;
    var _address = null;
    const device = new bluetooth.DeviceINQ();
    const DEVICE_NAME = "Makeblock";

    // device
    //     .on('finished',  function() {
    //         console.log('Completed discovery');
    //
    //         if (_address == null) {
    //             console.warn('Unable to find device. Retrying');
    //             _this.connect(_callback);
    //         }
    //     })
    //     .on('found', function found(address, name){
    //         console.log('Discovered device ' + name + ' ' + address);
    //
    //         if (_connection == null) {
    //             _connect(name, address);
    //         }
    //
    //
    //     });

    this.connect = function(callback) {
        _callback = callback;

        if (_connection != null) {
            if (_callback) _callback();
            return;
        }

        device.listPairedDevices(function (devices) {

            console.log("Paired Devices");


            for (var i=0; i<devices.length; i++) {
                var _device = devices[i];
                if (_device && _device.name) {
                    console.log(_device.name, _device.address);
                    _connect(_device.name, _device.address);
                }
            }
        });
        //device.inquire();
    };

    var _connect = function(name, address) {
        if (name.indexOf(DEVICE_NAME) > -1) {
            _address = address;

            device.findSerialPortChannel(address, function (channel) {
                console.log('Found RFCOMM channel for serial port on %s: ', name, channel);

                if (channel < 0) {
                    _address = null;
                    console.log("Channel is " + channel + " retrying");
                    _this.connect(_callback);
                    return;
                }

                // make bluetooth connect to remote device
                bluetooth.connect(address, channel, function (error, connection) {

                    if (error) {
                        console.log('Unable to connect ', error);
                        _this.connect(_callback);
                        return;
                    } else {
                        console.log('Connected to ', name);
                        _connection = connection;
                        if (_callback) _callback();
                    }
                });

            });
        }
    }

    this.getConnection = function() {
        return _connection;
    };

    this.write = function(content, callback) {
        if (!_connection) {
            console.log("connection is null. connecting...");
            _this.connect(function () {
                _connection.write(new Buffer(content, 'utf-8'), callback);
            });
        } else {
            _connection.write(new Buffer(content, 'utf-8'), callback);
        }
    };
};