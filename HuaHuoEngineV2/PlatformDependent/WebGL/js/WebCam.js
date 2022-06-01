var LibraryWebCamWebGL = {
	$webcam: {
		canvas: null
	},

	JS_WebCam_IsSupported__proxy: 'sync',
	JS_WebCam_IsSupported__sig: 'i',
	JS_WebCam_IsSupported: function()
	{
		var getMedia  = navigator.getUserMedia ||
						navigator.webkitGetUserMedia ||
						navigator.mozGetUserMedia ||
						navigator.msGetUserMedia;
		return getMedia != null;
	},

	JS_WebCamVideo_GetNumDevices__proxy: 'sync',
	JS_WebCamVideo_GetNumDevices__sig: 'i',
	JS_WebCamVideo_GetNumDevices: function()
	{
		return MediaDevices.length;
	},

	JS_WebCamVideo_GetDeviceName__proxy: 'sync',
	JS_WebCamVideo_GetDeviceName__sig: 'iiii',
	JS_WebCamVideo_GetDeviceName: function(deviceId, buffer, bufferSize)
	{
		if (buffer)
			stringToUTF8(MediaDevices[deviceId].deviceName, buffer, bufferSize);
		return lengthBytesUTF8(MediaDevices[deviceId].deviceName);
	},

	JS_WebCamVideo_GetNativeWidth__proxy: 'sync',
	JS_WebCamVideo_GetNativeWidth__sig: 'ii',
	JS_WebCamVideo_GetNativeWidth: function(deviceId)
	{
		return MediaDevices[deviceId].video ? MediaDevices[deviceId].video.videoWidth : 0;
	},

	JS_WebCamVideo_GetNativeHeight__proxy: 'sync',
	JS_WebCamVideo_GetNativeHeight__sig: 'ii',
	JS_WebCamVideo_GetNativeHeight: function(deviceId)
	{
		return MediaDevices[deviceId].video ? MediaDevices[deviceId].video.videoHeight : 0;
	},

	JS_WebCamVideo_Start__proxy: 'sync',
	JS_WebCamVideo_Start__sig: 'vi',
	JS_WebCamVideo_Start: function(deviceId)
	{
		if (MediaDevices[deviceId].video)
		{
			MediaDevices[deviceId].refCount++;
			return;
		}

		if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) 
		{
			navigator.getMedia  = navigator.getUserMedia ||
			                          navigator.webkitGetUserMedia ||
			                          navigator.mozGetUserMedia ||
			                          navigator.msGetUserMedia;
		}
		else
		{
			navigator.getMedia = function (constraints, success, error)
			{
				navigator.mediaDevices.getUserMedia(constraints).then(success).catch(error);
			};	
		}


		if (!navigator.getMedia)
		{
			console.log("WebCam is not supported. Try a different browser.");
			return;
		}

		if (!webcam.canvas)
		{
			canvas = document.createElement('canvas');
			canvas.style.display = "none";

			var context2d = canvas.getContext('2d');
			if (!context2d)
			{
				console.log("context2d is null");
				return;
			}

			document.body.appendChild(canvas);
			webcam.canvas = canvas;
		}

		var video = document.createElement('video');

		navigator.getMedia(
		{ 
			video: true,
			audio: false 
		},
		// success function
		function(stream) {
			video.srcObject = stream;

			webcam.canvas.appendChild(video);

			video.play();
			MediaDevices[deviceId].video = video;
			MediaDevices[deviceId].stream = stream;
			MediaDevices[deviceId].refCount++;
		},
		// fallback function
		function(err) {
			console.log("An error occurred! " + err);
		}
		);
	},

	JS_WebCamVideo_CanPlay__proxy: 'sync',
	JS_WebCamVideo_CanPlay__sig: 'ii',
	JS_WebCamVideo_CanPlay: function(deviceId)
	{
		return MediaDevices[deviceId].video && MediaDevices[deviceId].video.videoWidth > 0 && MediaDevices[deviceId].video.videoHeight > 0;
	},


	JS_WebCamVideo_Stop__proxy: 'sync',
	JS_WebCamVideo_Stop__sig: 'vi',
	JS_WebCamVideo_Stop: function(deviceId)
	{
		if (!MediaDevices[deviceId].video)
		{
			console.error("WebCam not initialized.");
			return;
		}

		if (--MediaDevices[deviceId].refCount == 0)
		{
			webcam.canvas.removeChild(MediaDevices[deviceId].video);
			MediaDevices[deviceId].video = null;

			// stop all tracks to release camera so it can be used by other apps.
	        MediaDevices[deviceId].stream.getVideoTracks().forEach(function(track) {
	        	if (track.stop)
		           track.stop();
	        });
		}
	},

	JS_WebCamVideo_GrabFrame__proxy: 'sync',
	JS_WebCamVideo_GrabFrame__sig: 'viiii',
	JS_WebCamVideo_GrabFrame: function (deviceId, buffer, destWidth, destHeight)
	{
		if (!MediaDevices[deviceId].video)
		{
			console.error("WebCam not initialized.");
			return;
		}
	
		var context = webcam.canvas.getContext('2d');
		if (context)
		{
			canvas.width = destWidth;
			canvas.height = destHeight;
			context.drawImage(MediaDevices[deviceId].video, 0, 0, MediaDevices[deviceId].video.videoWidth, MediaDevices[deviceId].video.videoHeight, 0, 0, destWidth, destHeight);

			var imageData = context.getImageData(0, 0, destWidth, destHeight);

			writeArrayToMemory(imageData.data, buffer);
		}
		else
		{
			 console.log("2d Context is null");
		}
	}
};

autoAddDeps(LibraryWebCamWebGL, '$webcam');
mergeInto(LibraryManager.library, LibraryWebCamWebGL);
