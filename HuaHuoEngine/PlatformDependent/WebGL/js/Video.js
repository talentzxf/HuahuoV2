var LibraryVideoWebGL = {
$videoInstances: [],

JS_Video_Create__proxy: 'sync',
JS_Video_Create__sig: 'ii',
JS_Video_Create: function(url)
{
	var str = Pointer_stringify(url);
	var video = document.createElement('video');
	video.style.display = 'none';
	video.src = str;

	// Managing the fact that the application has detached from the video object
	// so we can prune out callbacks arriving afterwards.
	video.detached = false;
	video.muted = true;

	// Enable CORS on the request fetching the video so the browser accepts
	// playing it.  This is needed since the data is fetched and used
	// programmatically - rendering into a canvas - and not displayed normally.
	video.crossOrigin = "anonymous";

	// Implementing looping ourselves instead of using the native 'loop'
	// property so we can get one 'ended' event per loop, which triggers the
	// wanted callback (and not just when the playback actually stops, as
	// HTML5's player does).
	video.looping = false;

	video.addEventListener("ended", function(evt)
	{
		if (video.looping && !video.detached)
		{
			video.play();
		}
	});

	return videoInstances.push(video) - 1;
},

JS_Video_UpdateToTexture__proxy: 'sync',
JS_Video_UpdateToTexture__sig: 'iii',
JS_Video_UpdateToTexture: function(video, tex)
{
	var v = videoInstances[video];

	// If the source video has not yet loaded (size is reported as 0), ignore uploading
	if (!(v.videoWidth > 0 && v.videoHeight > 0))
		return false;

	// If video is still going on the same video frame as before, ignore reuploading as well
	if (v.lastUpdateTextureTime === v.currentTime)
		return false;

	v.lastUpdateTextureTime = v.currentTime;

	GLctx.pixelStorei(GLctx.UNPACK_FLIP_Y_WEBGL, true);

	// It is not possible to get the source pixel aspect ratio of the video from
	// HTMLViewElement, which is problematic when we get anamorphic content. The videoWidth &
	// videoHeight properties report the frame size _after_ the pixel aspect ratio stretch has
	// been applied, but without this ratio ever being exposed. The caller has presumably
	// created the destination texture using the width/height advertized with the
	// post-pixel-aspect-ratio info (from JS_Video_Width and JS_Video_Height), which means it
	// may be incorrectly sized. As a workaround, we re-create the texture _without_
	// initializing its storage. The call to texImage2D ends up creating the appropriately-sized
	// storage. This may break the caller's assumption if the texture was created with properties
	// other than what is selected below. But for the specific (and currently dominant) case of
	// using Video.js with the VideoPlayer, this provides a workable solution.
	//
	// We do this texture re-creation every time we notice the videoWidth/Height has changed in
	// case the stream changes resolution.
	//
	// We could constantly call texImage2D instead of using texSubImage2D on subsequent calls,
	// but texSubImage2D has less overhead because it does not reallocate the storage.
	if (v.previousUploadedWidth != v.videoWidth || v.previousUploadedHeight != v.videoHeight)
	{
		GLctx.deleteTexture(GL.textures[tex]);
		var t = GLctx.createTexture();
		t.name = tex;
		GL.textures[tex] = t;
		GLctx.bindTexture(GLctx.TEXTURE_2D, t);
		GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_WRAP_S, GLctx.CLAMP_TO_EDGE);
		GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_WRAP_T, GLctx.CLAMP_TO_EDGE);
		GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MIN_FILTER, GLctx.LINEAR);
		GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, v);
		v.previousUploadedWidth = v.videoWidth;
		v.previousUploadedHeight = v.videoHeight;
	}
	else
	{
		GLctx.bindTexture(GLctx.TEXTURE_2D, GL.textures[tex]);
		GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, v);
	}

	GLctx.pixelStorei(GLctx.UNPACK_FLIP_Y_WEBGL, false);
	return true;
},

JS_Video_Destroy__proxy: 'sync',
JS_Video_Destroy__sig: 'vi',
JS_Video_Destroy: function(video)
{
	videoInstances[video].detached = true;
	videoInstances[video] = null;
	// GC will take care of the rest.
},

JS_Video_Play__proxy: 'sync',
JS_Video_Play__sig: 'vi',
JS_Video_Play: function(video)
{
	videoInstances[video].play();
},

JS_Video_Pause__proxy: 'sync',
JS_Video_Pause__sig: 'vi',
JS_Video_Pause: function(video)
{
	videoInstances[video].pause();
},

JS_Video_Seek__proxy: 'sync',
JS_Video_Seek__sig: 'vii',
JS_Video_Seek: function(video, time)
{
	videoInstances[video].currentTime = time;
},

JS_Video_SetLoop__proxy: 'sync',
JS_Video_SetLoop__sig: 'vii',
JS_Video_SetLoop: function(video, loop)
{
	// See note in JS_Video_Create for why we use .looping instead of .loop.
	videoInstances[video].looping = loop;
},

JS_Video_SetMute__proxy: 'sync',
JS_Video_SetMute__sig: 'vii',
JS_Video_SetMute: function(video, muted)
{
	videoInstances[video].muted = muted;
},

JS_Video_SetPlaybackRate__proxy: 'sync',
JS_Video_SetPlaybackRate__sig: 'vii',
JS_Video_SetPlaybackRate: function(video, rate)
{
	videoInstances[video].playbackRate = rate;
},

JS_Video_GetNumAudioTracks__proxy: 'sync',
JS_Video_GetNumAudioTracks__sig: 'ii',
JS_Video_GetNumAudioTracks: function(video)
{
	var tracks = videoInstances[video].audioTracks;
	// For browsers that don't support the audioTracks property, let's assume
	// there is one.
	return tracks ? tracks.length : 1;
},

JS_Video_GetAudioLanguageCode__proxy: 'sync',
JS_Video_GetAudioLanguageCode__sig: 'iii',
JS_Video_GetAudioLanguageCode: function(video, trackIndex)
{
	var tracks = videoInstances[video].audioTracks;
	if (!tracks)
		return "";
	var track = tracks[trackIndex];
	return track ? track.language : "";
},

JS_Video_EnableAudioTrack__proxy: 'sync',
JS_Video_EnableAudioTrack__sig: 'viii',
JS_Video_EnableAudioTrack: function(video, trackIndex, enabled)
{
	var tracks = videoInstances[video].audioTracks;
	if (!tracks)
		return;
	var track = tracks[trackIndex];
	if (track)
		track.enabled = enabled ? true : false;
},

JS_Video_SetVolume__proxy: 'sync',
JS_Video_SetVolume__sig: 'vii',
JS_Video_SetVolume: function(video, volume)
{
	videoInstances[video].volume = volume;
},

JS_Video_Height__proxy: 'sync',
JS_Video_Height__sig: 'ii',
JS_Video_Height: function(video)
{
	return videoInstances[video].videoHeight;
},

JS_Video_Width__proxy: 'sync',
JS_Video_Width__sig: 'ii',
JS_Video_Width: function(video)
{
	return videoInstances[video].videoWidth;
},

JS_Video_Time__proxy: 'sync',
JS_Video_Time__sig: 'ii',
JS_Video_Time: function(video)
{
	return videoInstances[video].currentTime;
},

JS_Video_Duration__proxy: 'sync',
JS_Video_Duration__sig: 'ii',
JS_Video_Duration: function(video)
{
	return videoInstances[video].duration;
},

JS_Video_IsReady__proxy: 'sync',
JS_Video_IsReady__sig: 'ii',
JS_Video_IsReady: function(video)
{
	// If the ready state is HAVE_ENOUGH_DATA or higher, we can start playing.
	if (!videoInstances[video].isReady &&
		videoInstances[video].readyState >= videoInstances[video].HAVE_ENOUGH_DATA)
		videoInstances[video].isReady = true;
	return videoInstances[video].isReady;
},

JS_Video_IsPlaying__proxy: 'sync',
JS_Video_IsPlaying__sig: 'ii',
JS_Video_IsPlaying: function(video)
{
	var element = videoInstances[video];
	return !element.paused && !element.ended;
},

JS_Video_SetErrorHandler__proxy: 'sync',
JS_Video_SetErrorHandler__sig: 'viii',
JS_Video_SetErrorHandler: function(video, ref, onerror)
{
	var instance = videoInstances[video];
	instance.onerror = function(evt)
	{
		if (!instance.detached)
		{
			dynCall('vii', onerror, [ref, evt.target.error.code]);
		}
	};
},

JS_Video_SetReadyHandler__proxy: 'sync',
JS_Video_SetReadyHandler__sig: 'viii',
JS_Video_SetReadyHandler: function(video, ref, onready)
{
	var instance = videoInstances[video];
	instance.addEventListener("canplay", function(evt)
	{
		if (!instance.detached)
		{
			dynCall('vi', onready, [ref]);
		}
	});
},

JS_Video_SetEndedHandler__proxy: 'sync',
JS_Video_SetEndedHandler__sig: 'viii',
JS_Video_SetEndedHandler: function(video, ref, onended)
{
	var instance = videoInstances[video];
	instance.addEventListener("ended", function(evt)
	{
		if (!instance.detached)
		{
			dynCall('vi', onended, [ref]);
		}
	});
},

JS_Video_SetSeekedOnceHandler__proxy: 'sync',
JS_Video_SetSeekedOnceHandler__sig: 'viii',
JS_Video_SetSeekedOnceHandler: function(video, ref, onseeked)
{
	var instance = videoInstances[video];
	instance.addEventListener("seeked", function listener(evt)
	{
		instance.removeEventListener("seeked", listener);
		if (!instance.detached)
		{
			dynCall('vi', onseeked, [ref]);
		}
	});
},

JS_Video_CanPlayFormat__proxy: 'sync',
JS_Video_CanPlayFormat__sig: 'ii',
JS_Video_CanPlayFormat: function(format)
{
	var str = Pointer_stringify(format);
	var video = document.createElement('video');
	return video.canPlayType(str) != '';
}

};
autoAddDeps(LibraryVideoWebGL, '$videoInstances');
mergeInto(LibraryManager.library, LibraryVideoWebGL);
