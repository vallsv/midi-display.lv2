function (event) {

	function handle_event (symbol, value) {
		if (symbol.includes("midi_msg_")) {
			var filter = '[mod-role=' + symbol + ']';
			var node = event.icon.find(filter)
			if (value < 0) {
				value = "";
			} else {
				value = Math.round(value).toString();
				value = value.padStart(3, '0');
			}
			node.text(value);
		}
	}

	if (event.type == 'start') {
		var ports = event.ports;
		for (var p in ports) {
			handle_event (ports[p].symbol, ports[p].value);
		}
	}
	else if (event.type == 'change') {
		handle_event (event.symbol, event.value);
	}
}
