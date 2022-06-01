var LibraryLoggingWebGL = {

JS_Log_Dump: function(ptr, type)
{
	var str = Pointer_stringify(ptr);
	if (typeof dump == 'function')
		dump (str);
	switch (type)
	{
		case 0: //LogType_Error
		case 1: //LogType_Assert
		case 4: //LogType_Exception
			console.error (str);
			return;

		case 2: //LogType_Warning
			console.warn (str);
			return;

		case 3: //LogType_Log
		case 5: //LogType_Debug
			console.log (str);
			return;			

		default:
			console.error ("Unknown console message type!")
			console.error (str);
	}
},

JS_Log_StackTrace: function(buffer, bufferSize)
{
	var trace = stackTrace();
	if (buffer)
		stringToUTF8(trace, buffer, bufferSize);
	return lengthBytesUTF8(trace);	
}
};

mergeInto(LibraryManager.library, LibraryLoggingWebGL);
