var LibraryAudioWebGL = {
$WEBAudio: {
	audioInstances: [],
	audioContext: {},
	audioWebEnabled: 0,
	},

JS_Sound_Init__proxy: 'sync',
JS_Sound_Init__sig: 'v',
JS_Sound_Init: function()
{
	try {
		window.AudioContext = window.AudioContext||window.webkitAudioContext;
		WEBAudio.audioContext = new AudioContext();
		
		var tryToResumeAudioContext = function() {
			if (WEBAudio.audioContext.state === 'suspended')
				WEBAudio.audioContext.resume();
			else
				Module.clearInterval(resumeInterval);
		};
		var resumeInterval = Module.setInterval(tryToResumeAudioContext, 400);
		
		WEBAudio.audioWebEnabled = 1;
	}
	catch(e) {
		alert('Web Audio API is not supported in this browser');
	}
},

JS_Sound_ReleaseInstance__proxy: 'async',
JS_Sound_ReleaseInstance__sig: 'vi',
JS_Sound_ReleaseInstance: function(instance)
{
	WEBAudio.audioInstances[instance] = null;
},

JS_Sound_Load_PCM__proxy: 'sync',
JS_Sound_Load_PCM__sig: 'iiiii',
JS_Sound_Load_PCM: function(channels, length, sampleRate, ptr)
{
	if (WEBAudio.audioWebEnabled == 0)
		return 0;

	var sound = {
		buffer: WEBAudio.audioContext.createBuffer(channels, length, sampleRate), 
		error: false
	};
	for (var i = 0; i < channels; i++)
	{
		var offs = (ptr>>2) + length * i;
		var buffer = sound.buffer;
		var copyToChannel = buffer['copyToChannel'] || function (source, channelNumber, startInChannel) 
		{
			// Shim for copyToChannel on browsers which don't support it like Safari.
			var clipped = source.subarray(0, Math.min(source.length, this.length - (startInChannel | 0)));
			this.getChannelData(channelNumber | 0).set(clipped, startInChannel | 0);
		};
		copyToChannel.apply(buffer, [HEAPF32.subarray(offs, offs + length),i, 0]);
	}
	var instance = WEBAudio.audioInstances.push(sound) - 1;
	return instance;
},

JS_Sound_Load__proxy: 'sync',
JS_Sound_Load__sig: 'iii',
JS_Sound_Load: function(ptr, length)
{
	if (WEBAudio.audioWebEnabled == 0)
		return 0;

	var sound = {
		buffer: null, 
		error: false
	};
	var instance = WEBAudio.audioInstances.push(sound) - 1;
#if USE_PTHREADS
	// AudioContext.decodeAudioData() does not currently allow taking in a view to a
	// SharedArrayBuffer, so make a copy of the data over to a regular ArrayBuffer instead.
	// See https://github.com/WebAudio/web-audio-api/issues/1850
	var audioData = new ArrayBuffer(length);
	new Uint8Array(audioData).set(HEAPU8.subarray(ptr, ptr+length));
#else
	var audioData = HEAPU8.buffer.slice(ptr, ptr+length);
#endif
	WEBAudio.audioContext.decodeAudioData(
		audioData,
		function(buffer) 
		{
			sound.buffer = buffer;
		}, 
		function () 
		{
			sound.error = true;
			console.log ("Decode error.");
		}
    );
	return instance;
},

JS_Sound_Create_Channel__proxy: 'sync',
JS_Sound_Create_Channel__sig: 'vii',
JS_Sound_Create_Channel: function (callback, userData)
{
	if (WEBAudio.audioWebEnabled == 0)
		return; 

	var channel = {
		gain: WEBAudio.audioContext.createGain(),
		panner: WEBAudio.audioContext.createPanner(),
		threeD: false,
		playBuffer: function(delay, buffer, offset)
		{			
			this.source.buffer = buffer;
			var chan = this;
			this.source.onended = function()
			{
				if (callback)
					dynCall('vi', callback, [userData]);

				// recreate channel for future use.
				chan.setup();
			};
			this.source.start(delay, offset);
		},
		setup: function()
		{
			this.source = WEBAudio.audioContext.createBufferSource();
			this.setupPanning();
		},
		setupPanning: function()
		{
			if(this.threeD)
			{
				this.source.disconnect();
				this.source.connect(this.panner);
				this.panner.connect(this.gain);		
			}
			else
			{
				this.panner.disconnect();
				this.source.connect(this.gain);
			}
		}
	};
	channel.panner.rolloffFactor = 0; // We calculate rolloff ourselves.
	channel.gain.connect ( WEBAudio.audioContext.destination);				
	channel.setup();
	return WEBAudio.audioInstances.push(channel) - 1;
},

JS_Sound_Play__proxy: 'sync',
JS_Sound_Play__sig: 'viiii',
JS_Sound_Play: function (bufferInstance, channelInstance, offset, delay)
{
	// stop sound which is playing in the channel currently.
	_JS_Sound_Stop (channelInstance, 0);

	if (WEBAudio.audioWebEnabled == 0)
		return;

	var sound = WEBAudio.audioInstances[bufferInstance];
	var channel = WEBAudio.audioInstances[channelInstance];

	if (sound.buffer) {
		try {
			channel.playBuffer (WEBAudio.audioContext.currentTime + delay, sound.buffer, offset);
		}
		catch(e) {
			// Need to catch exception, otherwise execution will stop on Safari if audio output is missing/broken
			console.error("playBuffer error. Exception: " + e);
		}
	}
	else
		console.log ("Trying to play sound which is not loaded.")
},

JS_Sound_SetLoop__proxy: 'sync',
JS_Sound_SetLoop__sig: 'vii',
JS_Sound_SetLoop: function (channelInstance, loop)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	WEBAudio.audioInstances[channelInstance].source.loop = loop;
},

JS_Sound_SetLoopPoints__proxy: 'sync',
JS_Sound_SetLoopPoints__sig: 'vidd',
JS_Sound_SetLoopPoints: function (channelInstance, loopStart, loopEnd)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;
	var channel = WEBAudio.audioInstances[channelInstance];
	channel.source.loopStart = loopStart;
	channel.source.loopEnd = loopEnd;
},

JS_Sound_Set3D__proxy: 'sync',
JS_Sound_Set3D__sig: 'vii',
JS_Sound_Set3D: function (channelInstance, threeD)
{
	var channel = WEBAudio.audioInstances[channelInstance];
	if (channel.threeD != threeD)
	{
		channel.threeD = threeD;
		channel.setupPanning();
	}
},

JS_Sound_Stop__proxy: 'sync',
JS_Sound_Stop__sig: 'vid',
JS_Sound_Stop: function (channelInstance, delay)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	var channel = WEBAudio.audioInstances[channelInstance];
	
	// stop sound currently playing.
	if (channel.source.buffer)
	{
		try {
			channel.source.stop(WEBAudio.audioContext.currentTime + delay);
		} catch (e) {
			// when stop() is used more than once for the same source in Safari it causes the following exception:
			// InvalidStateError: DOM Exception 11: An attempt was made to use an object that is not, or is no longer, usable.
			channel.source.disconnect();
		}

		if (delay == 0)
		{
			// disable callback for this channel when manually stopped.
			channel.source.onended = function(){};

			// recreate channel for future use.
			channel.setup();
		}
	}
},

JS_Sound_SetPosition__proxy: 'sync',
JS_Sound_SetPosition__sig: 'viddd',
JS_Sound_SetPosition: function (channelInstance, x, y, z)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	WEBAudio.audioInstances[channelInstance].panner.setPosition(x, y, z);
},

JS_Sound_SetVolume__proxy: 'sync',
JS_Sound_SetVolume__sig: 'vid',
JS_Sound_SetVolume: function (channelInstance, v)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	try {
		WEBAudio.audioInstances[channelInstance].gain.gain.setValueAtTime(v, WEBAudio.audioContext.currentTime);
	} catch(e) {
		console.error('Invalid audio volume ' + v + ' specified to WebAudio backend!');
	}
},

JS_Sound_SetPitch__proxy: 'sync',
JS_Sound_SetPitch__sig: 'vid',
JS_Sound_SetPitch: function (channelInstance, v)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	try {
		WEBAudio.audioInstances[channelInstance].source.playbackRate.setValueAtTime(v, WEBAudio.audioContext.currentTime);
	} catch(e) {
		console.error('Invalid audio pitch ' + v + ' specified to WebAudio backend!');
	}
},

JS_Sound_SetListenerPosition__proxy: 'sync',
JS_Sound_SetListenerPosition__sig: 'vddd',
JS_Sound_SetListenerPosition: function (x, y, z)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	if(WEBAudio.audioContext.listener.positionX) {
		WEBAudio.audioContext.listener.positionX.setValueAtTime(x, WEBAudio.audioContext.currentTime);
	 	WEBAudio.audioContext.listener.positionY.setValueAtTime(y, WEBAudio.audioContext.currentTime);
	 	WEBAudio.audioContext.listener.positionZ.setValueAtTime(z, WEBAudio.audioContext.currentTime);
	}
	else {
		// use setPosition if new properties are not supported
	 	WEBAudio.audioContext.listener.setPosition(x, y, z);
	}
},

JS_Sound_SetListenerOrientation__proxy: 'sync',
JS_Sound_SetListenerOrientation__sig: 'vdddddd',
JS_Sound_SetListenerOrientation: function (x, y, z, xUp, yUp, zUp)
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	// Web Audio uses a RHS coordinate system, Unity uses LHS, causing orientations to be flipped.
	// So we pass a negative direction here to compensate, otherwise channels will be flipped.
	if(WEBAudio.audioContext.listener.forwardX) {
		WEBAudio.audioContext.listener.forwardX.setValueAtTime(-x, WEBAudio.audioContext.currentTime);
		WEBAudio.audioContext.listener.forwardY.setValueAtTime(-y, WEBAudio.audioContext.currentTime);
		WEBAudio.audioContext.listener.forwardZ.setValueAtTime(-z, WEBAudio.audioContext.currentTime);
		WEBAudio.audioContext.listener.upX.setValueAtTime(xUp, WEBAudio.audioContext.currentTime);
		WEBAudio.audioContext.listener.upY.setValueAtTime(yUp, WEBAudio.audioContext.currentTime);
		WEBAudio.audioContext.listener.upZ.setValueAtTime(zUp, WEBAudio.audioContext.currentTime);
	}
	else {
		// use setOrientation if new properties are not supported
		WEBAudio.audioContext.listener.setOrientation(-x, -y, -z, xUp, yUp, zUp);
	}
},

JS_Sound_GetLoadState__proxy: 'sync',
JS_Sound_GetLoadState__sig: 'ii',
JS_Sound_GetLoadState: function (bufferInstance)
{
	if (WEBAudio.audioWebEnabled == 0)
		return 2;

	var sound = WEBAudio.audioInstances[bufferInstance];
	if (sound.error)
		return 2;
	if (sound.buffer)
		return 0;
	return 1;
},

JS_Sound_ResumeIfNeeded__proxy: 'sync',
JS_Sound_ResumeIfNeeded__sig: 'v',
JS_Sound_ResumeIfNeeded: function ()
{
	if (WEBAudio.audioWebEnabled == 0)
		return;

	if (WEBAudio.audioContext.state === 'suspended')
		WEBAudio.audioContext.resume();

},

JS_Sound_GetLength__proxy: 'sync',
JS_Sound_GetLength__sig: 'ii',
JS_Sound_GetLength: function (bufferInstance)
{
	if (WEBAudio.audioWebEnabled == 0)
		return 0;

	var sound = WEBAudio.audioInstances[bufferInstance];

	// Fakemod assumes sample rate is 44100, though that's not necessarily the case,
	// depending on OS, if the audio file was not imported by our pipeline.
	// Therefore we need to recalculate the length based on the actual samplerate.
	var sampleRateRatio = 44100 / sound.buffer.sampleRate;
	return sound.buffer.length * sampleRateRatio;
}

};

autoAddDeps(LibraryAudioWebGL, '$WEBAudio');
mergeInto(LibraryManager.library, LibraryAudioWebGL);
