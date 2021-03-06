var signalling_server_hostname = window.location.hostname;
var signalling_server_address = signalling_server_hostname + ":3001";


var ws = null;
var pc;
var datachannel;
var pcConfig = {"iceServers": [
    {"urls": [
        "stun:stun.l.google.com:19302",
        'stun:findreview.info:3478',
        'stun:stun1.l.google.com:19302',
        'stun:stun2.l.google.com:19302',
        'stun:stun3.l.google.com:19302',
        'stun:stun4.l.google.com:19302'/*,
        "stun:" + signalling_server_hostname + ":3478",
        "turn:" + signalling_server_hostname + ":3478"*/
    ]}
]};
var pcOptions = {
    optional: [
        // Deprecated:
        //{RtpDataChannels: false},
        //{DtlsSrtpKeyAgreement: true}
    ]
};
var mediaConstraints = {
    optional: [],
    mandatory: {
        OfferToReceiveAudio: false,
        OfferToReceiveVideo: true
    }
};

RTCPeerConnection = window.mozRTCPeerConnection || window.webkitRTCPeerConnection;
RTCSessionDescription = window.mozRTCSessionDescription || window.RTCSessionDescription;
RTCIceCandidate = window.mozRTCIceCandidate || window.RTCIceCandidate;
navigator.getUserMedia = navigator.getUserMedia || navigator.mozGetUserMedia || navigator.webkitGetUserMedia || navigator.msGetUserMedia;
var URL =  window.URL || window.webkitURL;

function createPeerConnection() {
    try {
        var pcConfig_ = pcConfig;
        console.log(JSON.stringify(pcConfig_));
        pc = new RTCPeerConnection(pcConfig_, pcOptions);
        pc.onicecandidate = onIceCandidate;
        pc.onaddstream = onRemoteStreamAdded;
        pc.onremovestream = onRemoteStreamRemoved;
        pc.ondatachannel = onDataChannel;
        console.log("peer connection successfully created!");
    } catch (e) {
        console.log("createPeerConnection() failed");
    }
}

function startWebRtc() {
    if ("WebSocket" in window) {
        server = signalling_server_address;

        var protocol = location.protocol === "https:" ? "wss:" : "ws:";
        ws = new WebSocket(protocol + '//' + server + '/stream/webrtc');

        function offer(stream) {
            createPeerConnection();
            if (stream) {
                pc.addStream(stream);
            }
            var command = {
                command_id: "offer",
                options: {
                    force_hw_vcodec: !isMobile(),
                    vformat: 30
                }
            };
            ws.send(JSON.stringify(command));
            console.log("offer(), command=" + JSON.stringify(command));
        }

        ws.onopen = function () {
            console.log("onopen()");

            var cast_mic = false;
            var cast_tab = false;
            var cast_camera = false;
            var cast_screen = false;
            var cast_window = false;
            var cast_application = false;
            var echo_cancellation = false;
            var localConstraints = {};
            if (cast_mic) {
                if (echo_cancellation)
                    localConstraints['audio'] = { optional: [{ echoCancellation: true }] };
                else
                    localConstraints['audio'] = { optional: [{ echoCancellation: false }] };
            } else if (cast_tab)
                localConstraints['audio'] = { mediaSource: "audioCapture" };
            else
                localConstraints['audio'] = false;
            if (cast_camera)
                localConstraints['video'] = true;
            else if (cast_screen)
                localConstraints['video'] = { frameRate: {ideal: 15, max: 30},
                    //width: {min: 640, max: 960},
                    //height: {min: 480, max: 720},
                    mozMediaSource: "screen",
                    mediaSource: "screen" };
            else if (cast_window)
                localConstraints['video'] = { frameRate: {ideal: 15, max: 30},
                    //width: {min: 640, max: 960},
                    //height: {min: 480, max: 720},
                    mozMediaSource: "window",
                    mediaSource: "window" };
            else if (cast_application)
                localConstraints['video'] = { frameRate: {ideal: 15, max: 30},
                    //width: {min: 640, max: 960},
                    //height:  {min: 480, max: 720},
                    mozMediaSource: "application",
                    mediaSource: "application" };
            else
                localConstraints['video'] = false;

            offer();
        };

        ws.onmessage = function (evt) {
            var msg = JSON.parse(evt.data);
            //console.log("message=" + msg);
            var what = msg.what;
            var data = msg.data;
            console.log(msg);

            if (!msg.what) {
                /* TODO: for backward compatibility, remove this branch in the future */
                what = msg.type;
                data = msg; // only used for 'offer' in the switch case below
                console.log("still using the old API?");
            }
            console.log("type=" + what);

            switch (what) {
                case "offer":
                    pc.setRemoteDescription(new RTCSessionDescription(JSON.parse(data)),
                        function onRemoteSdpSuccess() {
                            console.log('onRemoteSdpSucces()');
                            pc.createAnswer(function (sessionDescription) {
                                pc.setLocalDescription(sessionDescription);
                                var command = {
                                    command_id: "answer",
                                    data: JSON.stringify(sessionDescription)
                                };
                                ws.send(JSON.stringify(command));
                                console.log(command);

                            }, function (error) {
                                console.log("Failed to createAnswer: ", error);

                            }, mediaConstraints);
                        },
                        function onRemoteSdpError(event) {
                            console.log('Failed to set remote description (unsupported codec on this browser?): ', event);
                            stop();
                        }
                    );

                    var command = {
                        command_id: "generateIceCandidates"
                    };
                    //console.log(command);
                    ws.send(JSON.stringify(command));
                    break;

                case "answer":
                    break;

                case "message":
                    console.log(msg.data, msg);

                    if (msg.data == "ICE connection failed") {
                        Watchdog.getInstance().startStreamingFallback();
                    }
                    break;
                case "geticecandidate":
                case "iceCandidates":
                    var candidates = JSON.parse(msg.data);
                    for (var i = 0; candidates && i < candidates.length; i++) {
                        var elt = candidates[i];
                        let candidate = new RTCIceCandidate({
                            sdpMLineIndex: elt.sdpMLineIndex,
                            candidate: elt.candidate
                        });
                        pc.addIceCandidate(candidate,
                            function () {
                                console.log("IceCandidate added: " + JSON.stringify(candidate));
                            },
                            function (error) {
                                console.error("addIceCandidate error: " + error);
                            }
                        );
                    }
                    break;
            }
        };

        ws.onclose = function (evt) {
            if (pc) {
                pc.close();
                pc = null;
            }
        };

        ws.onerror = function (evt) {
            console.log("An error has occurred!");
            ws.close();
        };

    } else {
        alert("Sorry, this browser does not support WebSockets.");
    }
}

function stopWebrtc() {
    if (datachannel) {
        console.log("closing data channels");
        datachannel.close();
        datachannel = null;
    }

    onRemoteStreamRemoved();
    if (pc) {
        pc.close();
        pc = null;
    }
    if (ws) {
        ws.close();
        ws = null;
    }
}

function onIceCandidate(event) {
    if (event.candidate) {
        var candidate = {
            sdpMLineIndex: event.candidate.sdpMLineIndex,
            sdpMid: event.candidate.sdpMid,
            candidate: event.candidate.candidate
        };
        var command = {
            command_id: "addicecandidate",
            data: JSON.stringify(candidate)
        };
        console.log("Send command", command);
        ws.send(JSON.stringify(command));
    } else {
        console.log("End of candidates.");
    }
}

function onDataChannel(event) {
    console.log("onDataChannel()");
    datachannel = event.channel;

    event.channel.onopen = function () {
        console.log("Data Channel is open!");
    };

    event.channel.onerror = function (error) {
        console.log("Data Channel Error:", error);
    };

    event.channel.onmessage = function (event) {
        console.log("Got Data Channel Message:", event.data);
    };

    event.channel.onclose = function () {
        datachannel = null;
        console.log("The Data Channel is Closed");
    };
}

function onRemoteStreamRemoved(event) {
    $("#streamingView").remove();
}

function onRemoteStreamAdded(event) {
    //var url = URL.createObjectURL(event.stream);
    //url = url.replace("http://localhost:8080", "http://" + signalling_server_hostname + ":3001");
    console.log("Remote stream added:", event);
    var remoteVideoElement = document.getElementById('streamingView');
    //remoteVideoElement.src = url;
    remoteVideoElement.srcObject=event.stream;
    remoteVideoElement.play();
}

function isMobile() {
    if( navigator.userAgent.match(/Android/i)
        || navigator.userAgent.match(/webOS/i)
        || navigator.userAgent.match(/iPhone/i)
        || navigator.userAgent.match(/iPad/i)
        || navigator.userAgent.match(/iPod/i)
        || navigator.userAgent.match(/BlackBerry/i)
        || navigator.userAgent.match(/Windows Phone/i)
    ){
        return true;
    }
    else {
        return false;
    }
}