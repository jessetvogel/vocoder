function effectEqualizer(div) {
	// Variables
	var width = 512;
	var height = 384;
	var amountOfBars = 8;
	var frequencyMin = 40;
	var frequencyMax = 8000;
	var labelHeight = 48;
	var labelFontSize = 24;
	var eqWidth = width - 64;
	var eqHeight = height - 32 - labelHeight;
	
	// Create pitch effect
	command('ADD equalizer myEqualizer ' + frequencyMin + ',' + frequencyMax + ',' + amountOfBars);
	
	// Data stuff
	var eqContainer = $('<div>').addClass('eqContainer');
	var eqBars = [];
	var eqValues = [];
	for(var i = 0;i < amountOfBars;i ++) {
		eqBars[i] = $('<div>').addClass('eqBar').css({left: (eqWidth / amountOfBars * i) + 'px', height: (eqHeight / 2) + 'px'}).append($('<div>').addClass('eqDot'));
		eqValues[i] = 1.0;
		eqContainer.append(eqBars[i]);
	}
	
	// Set div
	div
		.css({width: width + 'px', height: height + 'px', backgroundColor: '#47484B'})
		.append($('<div>').addClass('label').text('E Q U A L I Z E R'))
		.append(eqContainer);
	
	div.find('.eqContainer').on('touchmove', function(event) {
		var touch = event.targetTouches[0];
		var x = touch.pageX - $(this).offset().left;
		var y = touch.pageY - $(this).offset().top;
		var thisWidth = $(this).width();
		var thisHeight = $(this).height();
		var minY = 0;
		var maxY = thisHeight;
		if(y < minY) y = minY;
		if(y > maxY) y = maxY;
		if(x < 0) x = 0;
		if(x > thisWidth) x = thisWidth;
		var eqBar = Math.floor(x / thisWidth * amountOfBars);
		var value = (maxY - y) / (maxY - minY) * 2 - 1;
		eqValues[eqBar] = Math.pow(4.0, value);
		$(this).find('.eqBar').eq(eqBar).height(thisHeight - y);
		
		command('UPDATE myEqualizer values ' + eqValues.join(','));
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
	
	div.find('.eqContainer').css({
		position: 'relative',
		top: labelHeight + 'px',
		left: ((width - eqWidth)/2) + 'px',
		width: eqWidth + 'px',
		height: eqHeight + 'px',
		backgroundColor: 'rgba(53, 186, 243, 0.50)',
		backgroundImage: 'linear-gradient(to right, rgba(255, 255, 255, 0.5) 1px, transparent 1px), linear-gradient(to bottom, rgba(255, 255, 255, 0.5) 1px, transparent 1px)',
		backgroundPosition: (eqWidth / amountOfBars / 4) + 'px ' + (eqWidth / amountOfBars / 4 - 2) + 'px',
		backgroundSize: (eqWidth / amountOfBars / 2) + 'px ' + (eqWidth / amountOfBars / 2) + 'px'
	});
	
	div.find('.eqBar').css({
		position: 'absolute',
		bottom: '0px',
		width: (eqWidth / amountOfBars) + 'px',
		backgroundColor: 'rgba(135, 219, 255, 0.5)'
	});
	
	div.find('.eqDot').css({
		position: 'relative',
		bottom: '4px',
		left: (eqWidth / amountOfBars / 2 - 6) + 'px',
		width: '8px',
		height: '8px',
		borderRadius: '8px',
		border: '2px solid #47484B',
//		backgroundColor: 'rgba(135, 219, 255, 1.0)'
		backgroundColor: 'white',
		opacity: 1.0
	});
}