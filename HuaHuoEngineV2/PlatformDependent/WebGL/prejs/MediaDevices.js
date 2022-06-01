var MediaDevices = [];

if (typeof ENVIRONMENT_IS_PTHREAD === "undefined" || !ENVIRONMENT_IS_PTHREAD) {
  Module['preRun'].push(function () {

		var enumerateMediaDevices = function ()
		{

			var getMedia  = navigator.getUserMedia ||
							navigator.webkitGetUserMedia ||
							navigator.mozGetUserMedia ||
							navigator.msGetUserMedia;

			if (!getMedia) 
				return;

			function addDevice(label) 
			{
				label = label ? label : ("device #" + MediaDevices.length);

				var device = 
				{
					deviceName: label,
					refCount: 0,
					video: null
				};

				MediaDevices.push(device);
			}

			// try MediaDevices.enumerateDevices, if available
			if (!navigator.mediaDevices || !navigator.mediaDevices.enumerateDevices) 
			{
				if (typeof MediaStreamTrack == 'undefined' ||
					typeof MediaStreamTrack.getSources == 'undefined') 
				{
					console.log("Media Devices cannot be enumerated on this browser.");
					return;
				}

				function gotSources(sourceInfos) 
				{

					for (var i = 0; i !== sourceInfos.length; ++i) 
					{
						var sourceInfo = sourceInfos[i];
						if (sourceInfo.kind === 'video') 
							addDevice(sourceInfo.label);
					}
				}

				// MediaStreamTrack.getSources asynchronously returns a list of objects that identify devices
				// and for privacy purposes the .label properties are not filled in unless the user has consented to
				// device access through getUserMedia.
				MediaStreamTrack.getSources(gotSources);
			}

			// List cameras and microphones.
			navigator.mediaDevices.enumerateDevices().then(function(devices) 
			{
				devices.forEach(function(device) 
				{
				  	// device: kind, label, deviceId
					if (device.kind == 'videoinput')
						addDevice(device.label);
				});
			})
			.catch(function(err) 
			{
				console.log(err.name + ": " + error.message);
			});
		};
		enumerateMediaDevices();
  });
}
