"use strict";

const bluetooth = require('node-bluetooth');


var BtSerial = module.exports = function BtSerial() {
    var _connection = null;
    var _this = this;
    var _callback = null;
    var _address = null;
    const device = new bluetooth.DeviceINQ();

    device
        .on('finished',  function() {
            console.log('Completed discovery');

            if (_address == null) {
                console.warn('Unable to find device. Retrying');
                _this.connect(_callback);
            }
        })
        .on('found', function found(address, name){
            console.log('Discovered device ' + name);

            if (_connection != null) {
                if (_callback) _callback();
                return;
            }

            if (name.indexOf("Makeblock") > -1) {
                _address = address;

                device.findSerialPortChannel(address, function (channel) {
                    console.log('Found RFCOMM channel for serial port on %s: ', name, channel);

                    // make bluetooth connect to remote device
                    bluetooth.connect(address, channel, function (error, connection) {

                        if (error) {
                            console.log('Unable to connect ', error);
                            _this.connect(_callback);
                        } else {
                            console.log('Connected to ', name);
                            _connection = connection;
                            if (_callback) _callback();
                        }
                    });

                });
            }
        });

    this.connect = function(callback) {
        _callback = callback;

        if (_connection != null) {
            if (_callback) _callback(_name);
            return;
        }

        device.inquire();
    };

    this.getConnection = function() {
        return _connection;
    };

    this.write = function(content, callback) {
        if (!_connection) {
            console.log("connection is null. re-connection");
            _this.connect(function () {
                _connection.write(new Buffer(content, 'utf-8'), callback);
            });
        } else {
            _connection.write(new Buffer(content, 'utf-8'), callback);
        }
    };
};