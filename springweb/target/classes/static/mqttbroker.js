const express = require('express');
const aedes = require('aedes')();
const mqttServer = require('net').createServer(aedes.handle);
const http = require('http');
const app = express();

// MQTT Broker
const portMQTT = 1883;
mqttServer.listen(portMQTT, function () {
  console.log('MQTT broker is running on port ' + portMQTT);
});

// Web Server
const portHTTP = 3000;
const httpServer = http.createServer(app);
httpServer.listen(portHTTP, function () {
  console.log('HTTP server is running on port ' + portHTTP);
});

// Attach MQTT server to HTTP server
aedes.attachHttpServer(httpServer);

aedes.on('subscribe', function (subscriptions, client) {
  console.log('MQTT client subscribed', subscriptions, client);
});

aedes.on('unsubscribe', function (subscriptions, client) {
  console.log('MQTT client unsubscribed', subscriptions, client);
});

aedes.on('client', function (client) {
  console.log('MQTT client connected', client.id);
});

aedes.on('clientDisconnect', function (client) {
  console.log('MQTT client disconnected', client.id);
});

aedes.on('publish', function (packet, client) {
  if (client) {
    console.log('MQTT client published', packet.payload.toString(), client.id);
  }
});
