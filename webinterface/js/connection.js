var connection = null;

var hostLocation = 'ws://192.168.2.40:1337';
var hostProtocol = 'vocoder-protocol';

function connect() {
    connection = new WebSocket(hostLocation, hostProtocol);
	log.print('Trying to connect...', log.color.yellow);
    connection.onclose = function(){
		log.print('Connection closed or failed to connect. Attempt to reconnect in 5 sec.', log.color.red);
        setTimeout(connect, 5000);
    };
	connection.onopen = function () {
		log.print('Connection established.', log.color.green);	
	};
	connection.onmessage = function (e) {
		handleCommand(e.data);
	};
}

function command(c) {
	try {
		connection.send(c + '\n');
	} catch(e) {
		log.print('Unable to send command \'' + c + '\'');
	}
}