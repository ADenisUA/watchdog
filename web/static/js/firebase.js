/**
 * Created by davdeev on 3/15/17.
 */

// Initialize Firebase
var config = {
    apiKey: "AIzaSyAYLjw4OaDc6SBjvR74559OUbylMNBLza0",
    authDomain: "watchdog-d1d6f.firebaseapp.com",
    databaseURL: "https://watchdog-d1d6f.firebaseio.com",
    storageBucket: "watchdog-d1d6f.appspot.com",
    messagingSenderId: "853420978869"
};

firebase.initializeApp(config);

// Retrieve Firebase Messaging object.
const messaging = firebase.messaging();

messaging.requestPermission()
    .then(function() {
        console.log('Notification permission granted.');
        // Get Instance ID token. Initially this makes a network call, once retrieved
        // subsequent calls to getToken will return from cache.
        messaging.getToken()
            .then(function(currentToken) {
                if (currentToken) {
                    sendTokenToServer(currentToken);
                } else {
                    // Show permission request.
                    console.log('No Instance ID token available. Request permission to generate one.');
                }
            })
            .catch(function(err) {
                console.log('An error occurred while retrieving token. ', err);
            });
    })
    .catch(function(err) {
        console.log('Unable to get permission to notify.', err);
    });

// Callback fired if Instance ID token is updated.
messaging.onTokenRefresh(function() {
    messaging.getToken()
        .then(function(refreshedToken) {
            console.log('Token refreshed.');
            // Send Instance ID token to app server.
            sendTokenToServer(refreshedToken);
            // ...
        })
        .catch(function(err) {
            console.log('Unable to retrieve refreshed token ', err);
        });
});

function sendTokenToServer(currentToken) {
    console.log("sendTokenToServer", currentToken);
    Watchdog.getInstance().getApi().register(currentToken);
}


// messaging.onMessage(function(payload) {
//     console.log("Message received. ", payload);
// });