$( document ).ready(function() {
    Watchdog.getInstance().stopStreaming();

    // checkTURNServer({
    //     url: 'turn:findreview.info',
    //     credential:"test",
    //     username: "test"
    // }).then(function(bool){
    //     console.log('is TURN server active? ', bool? 'yes':'no');
    // }).catch(console.error.bind(console));
});

// function checkTURNServer(turnConfig, timeout){
//
//     return new Promise(function(resolve, reject){
//
//         setTimeout(function(){
//             console.log("timeout");
//             if(promiseResolved) return;
//             resolve(false);
//             promiseResolved = true;
//         }, timeout || 10000);
//
//         var promiseResolved = false
//             , myPeerConnection = window.RTCPeerConnection || window.mozRTCPeerConnection || window.webkitRTCPeerConnection   //compatibility for firefox and chrome
//             , pc = new myPeerConnection({iceServers:[turnConfig]})
//             , noop = function(){};
//         pc.createDataChannel("");    //create a bogus data channel
//         pc.createOffer(function(sdp){
//             if(sdp.sdp.indexOf('typ relay') > -1){ // sometimes sdp contains the ice candidates...
//                 promiseResolved = true;
//                 resolve(true);
//             }
//             pc.setLocalDescription(sdp, noop, noop);
//         }, noop);    // create offer and set local description
//         pc.onicecandidate = function(ice){  //listen for candidate events
//             if(promiseResolved || !ice || !ice.candidate || !ice.candidate.candidate || !(ice.candidate.candidate.indexOf('typ relay')>-1))  return;
//             promiseResolved = true;
//             resolve(true);
//         };
//     });
// }