$(document).ready(function () {
	logInit();
	connect();
	
	$('.fullscreenButton').click(function() {
		fullscreen($('.view').get(0));
	});
	
	$('.addPitch').click(function () {
		addEffect(effectPitch);
//		addEffect(effectGain);
		addEffect(effectEqualizer);
	});
});

function addEffect(f) {
	var effectDiv = $('<div>').addClass('effectBlock');
	$('.workspace').append(effectDiv);
	f(effectDiv);
}
