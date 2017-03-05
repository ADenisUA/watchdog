"use strict";

var BtSerial = module.exports = function BtSerial() {
    var _connection = null;
    var _name = null;
    var _this = this;

    this.connect = function(callback) {

        if (_connection != null) {
            if (callback) callback(_name);
            return;
        }

        const bluetooth = require('node-bluetooth');
        const device = new bluetooth.DeviceINQ();

        device
            .on('finished',  console.log.bind(console, 'finished'))
            .on('found', function found(address, name){
                console.log('Found: ' + address + ' with name ' + name);

                if (_connection != null) {
                    if (callback) callback(_name);
                    return;
                }

                if (name.indexOf("Makeblock") > -1) {
                    device.findSerialPortChannel(address, function (channel) {
                        console.log('Found RFCOMM channel for serial port on %s: ', name, channel);

                        // make bluetooth connect to remote device
                        bluetooth.connect(address, channel, function (error, connection) {

                            var result = name;
                            if (error) {
                                result = error;
                            }

                            _connection = connection;
                            _name = name;

                            console.log('Connected ', result);

                            if (callback) callback(result);
                        });

                    });
                }
            }).inquire();
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