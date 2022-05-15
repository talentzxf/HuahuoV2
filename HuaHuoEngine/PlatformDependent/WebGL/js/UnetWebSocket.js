var LibraryUnetWebSockets = {
	$UNETWebSocketsInstances: {
		hosts: new Array(16),
		hostsContainingMessages: [],
		pingDataArray: null,
		HostStates: 
		{
			Created: 0,
			Opening: 1,
			Connected: 2,
			Closing: 3, 
			Closed: 4
		}, 
		EventTypes:
		{
			DataEvent: 0,
			ConnectEvent: 1,
			DisconnectEvent: 2,
			Nothing: 3
		}
	},
	JS_UNETWebSockets_Init__proxy: 'sync',
	JS_UNETWebSockets_Init__sig: 'v',
	JS_UNETWebSockets_Init: function()
	{
		UNETWebSocketsInstances.pingDataArray = new ArrayBuffer(1);
		var arr = new Uint8Array(UNETWebSocketsInstances.pingDataArray);
		arr[0] = 255;
	},
	JS_UNETWebSockets_HostsContainingMessagesPush__proxy: 'sync',
	JS_UNETWebSockets_HostsContainingMessagesPush__sig: 'vi',
	JS_UNETWebSockets_HostsContainingMessagesPush: function(socket)
	{
		if(socket.inQueue == false)
		{
			UNETWebSocketsInstances.hostsContainingMessages.push(socket);
			socket.inQueue = true;
		}
	},
	
	JS_UNETWebSockets_HostsContainingMessagesCleanHost__proxy: 'sync',
	JS_UNETWebSockets_HostsContainingMessagesCleanHost__sig: 'vi',
    JS_UNETWebSockets_HostsContainingMessagesCleanHost: function (hostId) {
	    for (i = 0; i < UNETWebSocketsInstances.hostsContainingMessages.length; i++) {
	        if(UNETWebSocketsInstances.hostsContainingMessages[i] != null) {
	            if (UNETWebSocketsInstances.hostsContainingMessages[i].id == hostId)
	                UNETWebSocketsInstances.hostsContainingMessages[i] = null;
	        }
	    }
	    var socket = UNETWebSocketsInstances.hostsContainingMessages[0];
	    if (socket == null)
	        return;
	    if (socket.messages.length == 0) {
	        socket.inQueue = false;
	    }
	    else {
	        UNETWebSocketsInstances.hostsContainingMessages.push(socket);
	    }
	    UNETWebSocketsInstances.hostsContainingMessages.shift();
	},

	JS_UNETWebSockets_SocketCreate__proxy: 'sync',
	JS_UNETWebSockets_SocketCreate__sig: 'vii',
	JS_UNETWebSockets_SocketCreate: function(hostId, url)
	{		
		var str = Pointer_stringify(url);
		var timerID = 0;
		function keepAlive(socket) { 
				var now = Date.now();
				var ab = new ArrayBuffer(1);
				var pData = new Uint8Array(ab);
				pData[0] = 255;
				if( now - socket.lastSentTime > socket.pingTimeout )
				{
					socket.socket.send (UNETWebSocketsInstances.pingDataArray);
					socket.lastSentTime = now;
				}	
		}
		function cancelKeepAlive(socket) {  
			if (socket.timerID) {  
				Module.clearInterval(socket.timerID);  
				socket.timerID = 0;
			}
		}
		
		var socket = {
			socket: new WebSocket(str, ['unitygame']),
			buffer: new Uint8Array(0),
			error: null,
			id: hostId,
			state: UNETWebSocketsInstances.HostStates.Created,
			inQueue: false,
			timerID: 0,
			pingTimeout: 0,
			lastSentTime: Date.now(),
			messages: []
		}
		socket.socket.onopen = function() {
			socket.state = UNETWebSocketsInstances.HostStates.Opening;
			_JS_UNETWebSockets_HostsContainingMessagesPush(socket);
			socket.timerID = Module.setInterval( function() { keepAlive(socket);}, socket.pingTimeout );
		};
		socket.socket.onmessage = function (e) {			
			if (e.data instanceof Blob)
			{
				var reader = new FileReader();
				reader.addEventListener("loadend", function() {
					var array = new Uint8Array(reader.result);
					_JS_UNETWebSockets_HostsContainingMessagesPush(socket);
					if(array.length == 1 && array[0] == 255)   //ping come in just drop it
					{
						return;
					}
					socket.messages.push(array);
				});
				reader.readAsArrayBuffer(e.data);
			}
		};
		socket.socket.onclose = function (e) {
			cancelKeepAlive(socket);
			if(socket.state == UNETWebSocketsInstances.HostStates.Closed)
			{
				return;
			}

			socket.state = UNETWebSocketsInstances.HostStates.Closing;
			_JS_UNETWebSockets_HostsContainingMessagesPush(socket);
		};
		socket.socket.onerror = function (e) {
			console.log("Error: " + e.data + " socket will be closed");
			socket.state = UNETWebSocketsInstances.HostStates.Closing;
			_JS_UNETWebSockets_HostsContainingMessagesPush(socket);
		};
		socket.pingTimeout = UNETWebSocketsInstances.hosts[socket.id].pingTimeout;
		UNETWebSocketsInstances.hosts[socket.id] = socket;
	},
//--
	JS_UNETWebSockets_SocketClose__proxy: 'sync',
	JS_UNETWebSockets_SocketClose__sig: 'vi',
	JS_UNETWebSockets_SocketClose: function (hostId)
	{
		var socket = UNETWebSocketsInstances.hosts[hostId];
		if(socket.socket != null)
			socket.socket.close();
	},
	JS_UNETWebSockets_SocketSend__proxy: 'sync',
	JS_UNETWebSockets_SocketSend__sig: 'viii',
	JS_UNETWebSockets_SocketSend: function (hostId, ptr, length)
	{
		var socket = UNETWebSocketsInstances.hosts[hostId];
		if(socket == 0 || socket.socket.readyState != 1 || socket.state != UNETWebSocketsInstances.HostStates.Connected)
			return;
		socket.socket.send (HEAPU8.buffer.slice(ptr, ptr+length));
		socket.lastSentTime = Date.now();
	}, 
	JS_UNETWebSockets_SocketRecvEvntType__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntType__sig: 'i',
	JS_UNETWebSockets_SocketRecvEvntType: function()
	{		
		if (UNETWebSocketsInstances.hostsContainingMessages.length == 0)
			return UNETWebSocketsInstances.EventTypes.Nothing;

		while( UNETWebSocketsInstances.hostsContainingMessages.length != 0 )
		{
		    if (UNETWebSocketsInstances.hostsContainingMessages[0] == null)
		        UNETWebSocketsInstances.hostsContainingMessages.shift();
			else if( UNETWebSocketsInstances.hostsContainingMessages[0].state == UNETWebSocketsInstances.HostStates.Closed )
				UNETWebSocketsInstances.hostsContainingMessages.shift();
			else if( UNETWebSocketsInstances.hostsContainingMessages[0].state == UNETWebSocketsInstances.HostStates.Opening ) 
				break;
			else if( UNETWebSocketsInstances.hostsContainingMessages[0].state == UNETWebSocketsInstances.HostStates.Closing )	
				break;
			else if (UNETWebSocketsInstances.hostsContainingMessages[0].messages.length == 0) {
		        UNETWebSocketsInstances.hostsContainingMessages[0].inQueue = false;
		        UNETWebSocketsInstances.hostsContainingMessages.shift();
		    } else
		        break;
		}
			
		if (UNETWebSocketsInstances.hostsContainingMessages.length == 0)
			return UNETWebSocketsInstances.EventTypes.Nothing;
		else if (UNETWebSocketsInstances.hostsContainingMessages[0].state == UNETWebSocketsInstances.HostStates.Opening)
	        return UNETWebSocketsInstances.EventTypes.ConnectEvent;
	    else if (UNETWebSocketsInstances.hostsContainingMessages[0].state == UNETWebSocketsInstances.HostStates.Closing && UNETWebSocketsInstances.hostsContainingMessages[0].messages.length == 0)
	        return UNETWebSocketsInstances.EventTypes.DisconnectEvent;
	    else
	        return UNETWebSocketsInstances.EventTypes.DataEvent;
	},
	JS_UNETWebSockets_SocketRecvEvntHost__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntHost__sig: 'i',
	JS_UNETWebSockets_SocketRecvEvntHost: function()
	{		
		return UNETWebSocketsInstances.hostsContainingMessages[0].id;
	},
	JS_UNETWebSockets_SocketRecvEvntBuffLength__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntBuffLength__sig: 'i',
	JS_UNETWebSockets_SocketRecvEvntBuffLength: function()
	{		
		return UNETWebSocketsInstances.hostsContainingMessages[0].messages[0].length;
	},
	JS_UNETWebSockets_SocketRecvEvntBuff__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntBuff__sig: 'vii',
	JS_UNETWebSockets_SocketRecvEvntBuff: function (ptr, length)
	{
		HEAPU8.set(UNETWebSocketsInstances.hostsContainingMessages[0].messages[0], ptr);
	},
	JS_UNETWebSockets_SocketCleanEvnt__proxy: 'sync',
	JS_UNETWebSockets_SocketCleanEvnt__sig: 'v',
	JS_UNETWebSockets_SocketCleanEvnt: function ()
	{
	    var host = UNETWebSocketsInstances.hostsContainingMessages.shift();
	    host.inQueue = false;
		if(host.state == UNETWebSocketsInstances.HostStates.Opening)
		{
			host.state = UNETWebSocketsInstances.HostStates.Connected;		
			if( host.messages.length != 0 )			
			    _JS_UNETWebSockets_HostsContainingMessagesPush(host);
		}
		else if(host.state == UNETWebSocketsInstances.HostStates.Closing)
		{
			if( host.messages.length == 0 )
				UNETWebSocketsInstances.hosts[host.id] = null;
			else
			{
				host.messages.shift();
				if( host.messages.length != 0 )
				    _JS_UNETWebSockets_HostsContainingMessagesPush(host);
			}
		}
		else
		{
			host.messages.shift();
			if( host.messages.length != 0 )
			    _JS_UNETWebSockets_HostsContainingMessagesPush(host);
		}
	},
	
	//from host part===================
	JS_UNETWebSockets_SocketRecvEvntTypeFromHost__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntTypeFromHost__sig: 'ii',
	JS_UNETWebSockets_SocketRecvEvntTypeFromHost: function(hostId) {
	    var evnt = UNETWebSocketsInstances.EventTypes.Nothing;
		if (UNETWebSocketsInstances.hosts[hostId].state == UNETWebSocketsInstances.HostStates.Opening)
		    evnt = UNETWebSocketsInstances.EventTypes.ConnectEvent;
		else if(UNETWebSocketsInstances.hosts[hostId].messages.length != 0)
		    evnt = UNETWebSocketsInstances.EventTypes.DataEvent;
		else if(UNETWebSocketsInstances.hosts[hostId].state == UNETWebSocketsInstances.HostStates.Closing)
		    evnt = UNETWebSocketsInstances.EventTypes.DisconnectEvent;
	    return evnt;
	},
	JS_UNETWebSockets_SocketRecvEvntBuffLengthFromHost__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntBuffLengthFromHost__sig: 'ii',
	JS_UNETWebSockets_SocketRecvEvntBuffLengthFromHost: function(hostId)
	{		
		return UNETWebSocketsInstances.hosts[hostId].messages[0].length;
	},
	JS_UNETWebSockets_SocketRecvEvntBuffFromHost__proxy: 'sync',
	JS_UNETWebSockets_SocketRecvEvntBuffFromHost__sig: 'viii',
	JS_UNETWebSockets_SocketRecvEvntBuffFromHost: function (hostId, ptr, length)
	{
		HEAPU8.set(UNETWebSocketsInstances.hosts[hostId].messages[0], ptr);
	},
	JS_UNETWebSockets_SocketCleanEvntFromHost__proxy: 'sync',
	JS_UNETWebSockets_SocketCleanEvntFromHost__sig: 'vi',
	JS_UNETWebSockets_SocketCleanEvntFromHost: function (hostId)
	{
		if (UNETWebSocketsInstances.hosts[hostId].state == UNETWebSocketsInstances.HostStates.Opening)
			UNETWebSocketsInstances.hosts[hostId].state = UNETWebSocketsInstances.HostStates.Connected;
		else if(UNETWebSocketsInstances.hosts[hostId].messages.length != 0)
			UNETWebSocketsInstances.hosts[hostId].messages.shift();
		else if(UNETWebSocketsInstances.hosts[hostId].state == UNETWebSocketsInstances.HostStates.Closing)
		{
			UNETWebSocketsInstances.hosts[hostId].state = UNETWebSocketsInstances.HostStates.Closed;
			UNETWebSocketsInstances.hosts[hostId] = null;
		    _JS_UNETWebSockets_HostsContainingMessagesCleanHost(hostId);
		}
	},
	//end========================
	JS_UNETWebSockets_AddHost__proxy: 'sync',
	JS_UNETWebSockets_AddHost__sig: 'ii',
	JS_UNETWebSockets_AddHost: function (pingTimeoutParam)
	{
		var placeHolderSocket = {
			socket: null,
			buffer: new Uint8Array(0),
			error: null,
			id: -1,
			state: UNETWebSocketsInstances.HostStates.Closed,
			pingTimeout: pingTimeoutParam,
			messages: []
		}
		for (i = 0; i < UNETWebSocketsInstances.hosts.length; i++) 
		{ 
			if(UNETWebSocketsInstances.hosts[i] == null)
			{
				placeHolderSocket.id = i;
				UNETWebSocketsInstances.hosts[i] = placeHolderSocket;
				return i;
			}
		}
		return -1;
	},	
	JS_UNETWebSockets_IsHostCorrect__proxy: 'sync',
	JS_UNETWebSockets_IsHostCorrect__sig: 'ii',
	JS_UNETWebSockets_IsHostCorrect: function (i)
	{
		if(i < UNETWebSocketsInstances.hosts.length && UNETWebSocketsInstances.hosts[i] != null && UNETWebSocketsInstances.hosts[i].socket != null)
		{
			return true;
		}
		return false;
	},
	JS_UNETWebSockets_IsHostReadyToConnect__proxy: 'sync',
	JS_UNETWebSockets_IsHostReadyToConnect__sig: 'ii',
	JS_UNETWebSockets_IsHostReadyToConnect: function (i)
	{
		if(i < UNETWebSocketsInstances.hosts.length && UNETWebSocketsInstances.hosts[i]!= null && UNETWebSocketsInstances.hosts[i].socket == null)
		{
			return true;
		}
		return false;
	},
	JS_UNETWebSockets_SocketStop__proxy: 'sync',
	JS_UNETWebSockets_SocketStop__sig: 'v',
	JS_UNETWebSockets_SocketStop: function ()
	{
		for (i = 0; i < UNETWebSocketsInstances.hosts.length; i++) 
		{ 
			if(UNETWebSocketsInstances.hosts[i] != null && UNETWebSocketsInstances.hosts[i].socket != null)
			{
				var socket = UNETWebSocketsInstances.hosts[i];
				socket.socket.close();
				UNETWebSocketsInstances.hosts[i] = null;
			}
		}
		UNETWebSocketsInstances.hosts = new Array(UNETWebSocketsInstances.hosts.length);
		UNETWebSocketsInstances.hostsContainingMessages = new Array();
	}
}

autoAddDeps(LibraryUnetWebSockets, '$UNETWebSocketsInstances');
autoAddDeps(LibraryUnetWebSockets, 'JS_UNETWebSockets_HostsContainingMessagesPush');
autoAddDeps(LibraryUnetWebSockets, 'JS_UNETWebSockets_HostsContainingMessagesCleanHost');
mergeInto(LibraryManager.library, LibraryUnetWebSockets);