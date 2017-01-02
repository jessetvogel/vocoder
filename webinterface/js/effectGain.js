function effectGain(div) {
	// Create pitch effect
	command('ADD gain myGain');
	
	// Variables
	var width = 256;
	var height = 384;
	var slideRadius = 32;
	var slideHeight = 256;
	var stripWidth = 8;
	var fontSize = 16;
	var labelHeight = 48;
	var labelFontSize = 24;
	
	// Set div
	div
		.css({width: width + 'px', height: height + 'px', backgroundColor: '#47484B'})
		.append($('<div>').addClass('label').text('G A I N'))
		.append($('<div>').addClass('slideContainer')
			.append($('<div>').addClass('strip'))
			.append($('<div>').addClass('slide')) // slider for pitch
			.append($('<span>').addClass('labelTop').text('+12dB')).append($('<span>').addClass('labelBottom').text('-12dB'))
		);
	
	div.find('.slide').on('touchmove', function(event) {
		var touch = event.targetTouches[0];
		var y = touch.pageY - $(this).parent().offset().top;
		var minY = slideRadius;
		var maxY = slideRadius + slideHeight;
		if(y < minY) y = minY;
		if(y > maxY) y = maxY;
		$(this).css({top: (y - slideRadius) + 'px'});
		var value = (maxY - y) / (maxY - minY) * 2 - 1;
		command('UPDATE myGain gain ' + Math.pow(4.0, value));
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
	
	div.find('.slideContainer').css({
		position: 'relative',
		top: labelHeight + 'px',
		width: width + 'px',
		height: (slideHeight + slideRadius*2) + 'px'
	});
	
	div.find('.slide').css({
		position: 'absolute',
		top: (slideHeight/2) + 'px',
		left: (width/2 - slideRadius) + 'px',
		width: (slideRadius*2) + 'px',
		height: (slideRadius*2) + 'px',
		backgroundColor: '#F9F9F9',
		borderRadius: slideRadius + 'px'
	});
	
	div.find('.strip').css({
		position: 'absolute',
		top: slideRadius + 'px',
		left: ((width - stripWidth)/2) + 'px',
		width: stripWidth + 'px',
		height: slideHeight + 'px',
		backgroundColor: '#000'
	});
	
	div.find('.labelTop').css({
		position: 'absolute',
		top: (slideRadius - fontSize/2) + 'px',
		left: (width/2 + slideRadius*1.5) + 'px'
	});
	
	div.find('.labelBottom').css({
		position: 'absolute',
		top: (slideRadius + slideHeight - fontSize/2) + 'px',
		left: (width/2 + slideRadius*1.5) + 'px'
	});
}