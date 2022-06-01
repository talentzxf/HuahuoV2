var LibraryFileSystemWebGL = {

$fs: {
	numPendingSync : 0,
	syncInternal   : 1000, // ms
	syncInProgress : false,
	sync : function(onlyPendingSync)
	{
		if (onlyPendingSync) {
			if (fs.numPendingSync == 0)
				return;
		}
		else if (fs.syncInProgress) {
			// this is to avoid indexedDB memory leak when FS.syncfs is executed before the previous one completed.
			fs.numPendingSync++;
			return;
		}

		fs.syncInProgress = true;
		FS.syncfs(false, (function(err) {
			fs.syncInProgress = false;
		}));
		fs.numPendingSync = 0;
	},
},

JS_FileSystem_Sync__proxy: 'sync',
JS_FileSystem_Sync__sig: 'v',
JS_FileSystem_Sync: function()
{
	fs.sync(false);
},

JS_FileSystem_Initialize__proxy: 'sync',
JS_FileSystem_Initialize__sig: 'v',
JS_FileSystem_Initialize: function()
{
	Module.setInterval(function(){
		fs.sync(true);
	}, fs.syncInternal);
},

JS_FileSystem_Terminate__proxy: 'sync',
JS_FileSystem_Terminate__sig: 'v',
JS_FileSystem_Terminate: function()
{
	// do nothing. Note that the JS Interval will be cleared on Quit
}

};

autoAddDeps(LibraryFileSystemWebGL, '$fs');
mergeInto(LibraryManager.library, LibraryFileSystemWebGL);
