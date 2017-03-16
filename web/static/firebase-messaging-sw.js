
self.addEventListener('push', function(event) {
    console.log('Received a push message', event.data.json());

    var body = "";

    if (event.data && event.data.json() && event.data.json().data && event.data.json().data.payload) {

        try {
            var payload = JSON.parse(event.data.json().data.payload);
            for (var i in payload) {
                body += payload[i].name;
                body += "=";
                body += payload[i].value;
                body += "."
            }
        } catch (e) {
            body += "Unable to parse event";
        }
    }

    body += " Open Watchdog?";
    var title = 'Watchdog notification';

    var icon = '/img/icon192.png';
    var image = '/stream/snapshot.jpeg';
    var tag = 'tag';

    event.waitUntil(
        self.registration.showNotification(title, {
            "body": body,
            "icon": icon,
            "image": image,
            "vibrate": [300, 100, 400],
            "tag": tag,
            "actions": [
                { "action": "yes", "title": "Yes"},
                { "action": "no", "title": "No"}
            ]
        })
    );
});

self.addEventListener('install', function(event) {
    console.log('Install', event);
});

self.addEventListener('notificationclick', function(event) {
    console.log('On notification click: ', event);

    switch (event.action) {
        case "yes":
            // This looks to see if the current is already open and
            // focuses if it is
            event.waitUntil(
                clients.matchAll({
                    type: "window"
                })
                    .then(function(clientList) {
                        for (var i = 0; i < clientList.length; i++) {
                            var client = clientList[i];
                            if (client.url == '/' && 'focus' in client)
                                return client.focus();
                        }
                        if (clients.openWindow) {
                            return clients.openWindow('/');
                        }
                    })
            );
            break;
        case "no":
            break;
    }

    event.notification.close();
});