var LibraryWebRequestWebGL = {
	$wr: {
		requestInstances: {},
		nextRequestId: 1
	},

	JS_WebRequest_Create__proxy: 'sync',
	JS_WebRequest_Create__sig: 'iii',
	JS_WebRequest_Create: function(url, method)
	{
		var _url = Pointer_stringify(url);
		var _method = Pointer_stringify(method);
		var http = Module.companyName && Module.productName && Module.XMLHttpRequest ? new Module.XMLHttpRequest({
			companyName: Module.companyName,
			productName: Module.productName,
			cacheControl: Module.cacheControl(_url),
		}) : new XMLHttpRequest();
		http.open(_method, _url, true);
		http.responseType = 'arraybuffer';
		wr.requestInstances[wr.nextRequestId] = http;
		return wr.nextRequestId++;
	},

	JS_WebRequest_SetTimeout__proxy: 'sync',
	JS_WebRequest_SetTimeout__sig: 'vii',
	JS_WebRequest_SetTimeout: function (request, timeout)
	{
		wr.requestInstances[request].timeout = timeout;
	},

	JS_WebRequest_SetRequestHeader__proxy: 'sync',
	JS_WebRequest_SetRequestHeader__sig: 'viii',
	JS_WebRequest_SetRequestHeader: function (request, header, value)
	{
		var _header = Pointer_stringify(header);
		var _value = Pointer_stringify(value);
		wr.requestInstances[request].setRequestHeader(_header, _value);
	},

	JS_WebRequest_SetResponseHandler__proxy: 'sync',
	JS_WebRequest_SetResponseHandler__sig: 'viii',
	JS_WebRequest_SetResponseHandler: function (request, arg, onresponse)
	{
		var http = wr.requestInstances[request];
		// LOAD
		http.onload = function http_onload(e) {
			if (onresponse)
			{
				var kWebRequestOK = 0;
				var byteArray = new Uint8Array(http.response);
				// 200 is successful http request, 0 is returned by non-http requests (file:).
				if (byteArray.length != 0)
				{
					var buffer = _malloc(byteArray.length);
					HEAPU8.set(byteArray, buffer);
					dynCall('viiiiii', onresponse, [arg, http.status, buffer, byteArray.length, 0, kWebRequestOK]);
				}
				else
				{
					dynCall('viiiiii', onresponse, [arg, http.status, 0, 0, 0, kWebRequestOK]);
				}
			}
		};

		function HandleError(err, code)
		{
			if (onresponse)
			{
				var len = lengthBytesUTF8(err) + 1;
				var buffer = _malloc(len);
				stringToUTF8(err, buffer, len);
				dynCall('viiiiii', onresponse, [arg, http.status, 0, 0, buffer, code]);
				_free(buffer);
			}
		}

		// ERROR
		http.onerror = function http_onerror(e) {
			var kWebErrorUnknown = 2;
			HandleError ("Unknown error.", kWebErrorUnknown);
		};

		http.ontimeout = function http_onerror(e) {
			var kWebErrorTimeout = 14;
			HandleError ("Connection timed out.", kWebErrorTimeout);
		};

		http.onabort = function http_onerror(e) {
			var kWebErrorAborted = 17;
			HandleError ("Aborted.", kWebErrorAborted);
		};
	},

	JS_WebRequest_SetProgressHandler__proxy: 'sync',
	JS_WebRequest_SetProgressHandler__sig: 'viii',
	JS_WebRequest_SetProgressHandler: function (request, arg, onprogress)
	{
		var http = wr.requestInstances[request];

		http.onprogress = function http_onprogress(e) {
			if (onprogress)
			{
				if (e.lengthComputable)
					dynCall('viii', onprogress, [arg, e.loaded, e.total]);
			}
		};
	},

	JS_WebRequest_Send__proxy: 'sync',
	JS_WebRequest_Send__sig: 'viii',
	JS_WebRequest_Send: function (request, ptr, length)
	{
		var http = wr.requestInstances[request];

		try {
			if (length > 0) {
				var postData = HEAPU8.subarray(ptr, ptr+length);
#if USE_PTHREADS
				// In multithreaded builds, HEAPU8 views into a SharedArrayBuffer,
				// but currently XMLHttpRequest does not allow send()ing data from
				// a SharedArrayBuffer sources. Therefore copy the data from a SAB
				// to an ArrayBuffer for the API call.
				// See https://github.com/whatwg/xhr/issues/245
				postData = new Uint8Array(postData);
#endif
				http.send(postData);
			}
			else
				http.send();
		}
		catch(e) {
			console.error(e.name + ": " + e.message);
		}
	},

	JS_WebRequest_GetResponseHeaders__proxy: 'sync',
	JS_WebRequest_GetResponseHeaders__sig: 'iiii',
	JS_WebRequest_GetResponseHeaders: function(request, buffer, bufferSize)
	{
		var headers = wr.requestInstances[request].getAllResponseHeaders();
		if (buffer)
			stringToUTF8(headers, buffer, bufferSize);
		return lengthBytesUTF8(headers);
	},

	JS_WebRequest_GetStatusLine__proxy: 'sync',
	JS_WebRequest_GetStatusLine__sig: 'iiii',
	JS_WebRequest_GetStatusLine: function(request, buffer, bufferSize)
	{
		var status = wr.requestInstances[request].status + " " + wr.requestInstances[request].statusText;
		if (buffer)
			stringToUTF8(status, buffer, bufferSize);
		return lengthBytesUTF8(status);
	},

	JS_WebRequest_Abort__proxy: 'sync',
	JS_WebRequest_Abort__sig: 'vi',
	JS_WebRequest_Abort: function (request)
	{
		wr.requestInstances[request].abort();
	},

	JS_WebRequest_Release__proxy: 'sync',
	JS_WebRequest_Release__sig: 'vi',
	JS_WebRequest_Release: function (request)
	{
		var http = wr.requestInstances[request];

		http.onload = null;
		http.onerror = null;
		http.ontimeout = null;
		http.onabort = null;
		delete http;

		wr.requestInstances[request] = null;
	}
};

autoAddDeps(LibraryWebRequestWebGL, '$wr');
mergeInto(LibraryManager.library, LibraryWebRequestWebGL);
