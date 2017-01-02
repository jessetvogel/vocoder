var WebSocketServer = require('websocket').server;
var http = require('http');
var net = require('net');

// Define options, i.e. ports, adressess etc.
var portWebsocket = 1337;
var portHTTP = 8080;
var portInterface = 7000;
var hostInterface = '127.0.0.1';
var protocolWebsocket = 'vocoder-protocol';

var clientsWebsocket = [];
var connectionInterface = new net.Socket();

// Create a HTTP server for websocket server
var server = http.createServer(function(request, response) {
    response.writeHead(404);
    response.end();
}).listen(portWebsocket, function() {
    console.log(tag() + 'Server is listening on port ' + portWebsocket + '.');
});

// Create a websocket server
serverWebSocket = new WebSocketServer({
    httpServer: server,
    autoAcceptConnections: false
});

serverWebSocket.on('request', function(request) {
	// Make sure we only accept requests from an allowed origin
    if(!requestAllowed(request.origin)) {
      request.reject();
      console.log(tag() + 'Connection from origin ' + request.origin + ' rejected.');
      return;
    }

	// Try establishing a connection
	try {
		var connection = request.accept(protocolWebsocket, request.origin);
		console.log(tag() + 'Connection accepted from origin ' + request.origin + '.');

		connection.on('message', function(message) {
			// So far only support UTF8
			if(message.type === 'utf8') {
				sendToInterface(message.utf8Data);
			}
		});

		connection.on('close', function(reasonCode, description) {
			console.log(tag() + 'Peer ' + connection.remoteAddress + ' disconnected.');
		});
	} catch(err) {
		console.log(tag() + err);
	}
});

connectionInterface.on('error', function(e) {
	// If the connection was refused, try again in 5 seconds
	if(e.code == 'ECONNREFUSED') {
		console.log(tag() + 'Failed to connect interface. Retrying in 5 seconds.');
		setTimeout(function() {connectionInterface.connect(portInterface, hostInterface);}, 5000);
	}
});

connectionInterface.on('data', function(data) {
	// Whenever data is received, pass it on to all clients
	sendToAllClientsWebsocket(data);
});

connectionInterface.on('close', function() {
	// Whenever the connection is closed, notify
	console.log(tag() + 'Connection closed');
});

connectionInterface.connect(portInterface, hostInterface, function() {
	// Whenever the connection is established, notify
	console.log(tag() + 'Connected to ' + hostInterface + ':' + portInterface + '.');
});

function sendToAllClientsWebsocket(data) {
	// Send the data to all clients
	for(var i = 0;i < clientsWebsocket.length;i ++) {
		clientsWebsocket[i]
	}
}

function sendToInterface(data) {
	// Write data to interface
	connectionInterface.write(data);
}

// Some useful functions
function tag() {return '[' + ("" + (new Date())).slice(16, 24) + '] ';}

function requestAllowed(origin) {
  // Check whether the request is allowed
  return true; // TODO: make criterion, maybe if source is local?
}