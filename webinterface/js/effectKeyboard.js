function effectKeyboard(div) {
	// Variables
	var width = 960;
	var height = 384;
	var labelHeight = 48;
	var labelFontSize = 24;
	var maxAmountOfNotes = 8;

	// Create keyboard effect
	command('ADD keyboard myKeyboard ' + maxAmountOfNotes);

	// Set div
	div
		.css({width: width + 'px', height: height + 'px', backgroundColor: '#47484B'})
		.append($('<div>').addClass('label').text('K E Y B O A R D'))
		.append($('<div>').addClass('keys'));

		// The cool part

	// Data stuff
	var noteData = new Array(maxAmountOfNotes*2); // [freq1, velo1, freq2, velo2, ..., freqN, veloN], where N = maxAmountOfNotes
	for(var i = 0;i < maxAmountOfNotes;i ++) {
		noteData[2*i] = 0.0;
		noteData[2*i+1] = 0.0;
	}

	var m = null; // The MIDIAccess object
	navigator.requestMIDIAccess().then(function (access) {
		m = access;

	  // Things you can do with the MIDIAccess object:
	  var inputs = m.inputs; // inputs = MIDIInputMaps, you can retrieve the inputs with iterators
	  var iteratorInputs = inputs.values(); // returns an iterator that loops over all inputs
	  var input = iteratorInputs.next().value; // get the first input

	  input.onmidimessage = function (event) {
			if(event.data[0] == 0x90 || event.data[0] == 0x80) { // Note On
				var midiNote = event.data[1];
				var velocity = event.data[0] == 0x80 ? 0 : event.data[2];

				// Determine frequency and update noteData
				var frequency = midiNoteToFrequency(midiNote) / 2.0;
				for(var i = 0;i < maxAmountOfNotes - 1;i ++) {
					if(noteData[2*i] == frequency) break;
				}

				noteData.splice(2*i, 2);
				noteData.unshift(frequency, velocity / 127.0);

				// Send new data to interface
				command('UPDATE myKeyboard tones ' + noteData.join(','));
				// div.find('.info').append("Played " + midiNoteToName(event.data[1]) + " at velocity " + event.data[2] + "<br/>");

				updateVisual(div, midiNote, velocity / 127.0);
			}

			div.find('.info').html(noteData.join('</br>'));
		};
	}, function (error) {
		log.print('Something went wrong in requesting MIDI access: ' + error.code);
	});

	// Set css
	div.find('.label').css({
		position: 'absolute',
		top: '0px',
		lineHeight: labelHeight + 'px',
		width: width + 'px',
		textAlign: 'center',
		fontSize: labelFontSize + 'px',
		color: '#909090'
	});

	// Create keys
	var keys = div.find('.keys').css({
		position: 'relative',
		top: (labelHeight + 128) + 'px',
		left: '37px'
	});
	var x = 0;
	for(var i = 0;i < 88;i ++) {
		switch(i % 12) {
			// White key
			case 0: case 2: case 3: case 5: case 7: case 8: case 10:
				keys.append($('<div>').addClass('key').css({
					position: 'absolute',
					left: x + 'px',
					top: '0px',
					width: '16px',
					height: '128px',
					backgroundColor: 'rgb(249, 249, 249)'
				}));
				x += 17;
				break;
			// Black key
			default:
				keys.append($('<div>').addClass('key').css({
					position: 'absolute',
					left: (x - 6) + 'px',
					top: '0px',
					width: '12px',
					height: '96px',
					backgroundColor: 'rgb(64, 64, 64)',
					zIndex: 1
				}));
			break;
		}
	}
}

function midiNoteToFrequency(midiNote) {
	// Using that an A4 = 440Hz has midiNote = 69
	return 440.0 * Math.pow(2.0, (midiNote - 69) / 12.0);
}

function midiNoteToName(midiNote) {
	var arr = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
	return arr[midiNote % 12];
}

function updateVisual(div, midiNote, x) {
	var key = midiNote - 21;
	var keyArray = div.find('.key');
	var c;
	switch(key % 12) {
		// White key
		case 0: case 2: case 3: case 5: case 7: case 8: case 10:
		c = x * 160 + (1 - x) * 249;
		break;
		default:
		c = (1 - x) * 64;
		break;
	}
	c = Math.round(c);
	keyArray.eq(key).css({
		backgroundColor: 'rgb(' +c+','+c+','+c+ ')'
	});
}
