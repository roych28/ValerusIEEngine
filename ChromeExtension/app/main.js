var port = null;
var currentWindowPos = {x:-1,y:-1};
var urlToSend = "";

function getWindowPos()
{
	var size = {x:-1, y:-1}; 

	if(window.screenX){
		size.x = window.screenX;
		size.y = window.screenY;
	}
	else if(window.screenLeft){
		size.x = window.screenLeft;
		size.y = window.screenTop;
	}
	return size;
}

function sendNativeMessage(e) {
  message = {	"text": document.getElementById('input-text').value,
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y
	};
  port.postMessage(message);
  console.log("Sent message: " + JSON.stringify(message));
}

function initNativeMessaging(e) {
  message = {	"text": "#INIT#",
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y,
				"url"	 : urlToSend
	};
	
	if(port){
		port.postMessage(message);
	}
  
	console.log("Sent message: " + JSON.stringify(message));
}

function onResizeNativeMessaging(e) {
  message = {	"text": "#ONRESIZE#",
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y
	};
  port.postMessage(message);
  console.log("Sent message: " + JSON.stringify(message));
}

function onMoveNativeMessaging(e) {

	var windowPos = getWindowPos();
	
	if( windowPos.x != currentWindowPos.x || currentWindowPos.y != currentWindowPos.y){
		currentWindowPos = windowPos;
		message = {	"text": "#ONMOVE#",
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y			
		};
	
		port.postMessage(message);
		console.log("Sent message: " + JSON.stringify(message));
	}	
}

function onNativeMessage(message) {
  console.log("Received message: " + JSON.stringify(message));
}

function onDisconnected() {
  console.log("Failed to connect: " + chrome.runtime.lastError.message);
  port = null;
  //updateUiState();
}

function connect() {
  var hostName = "com.vicon.valerus.app";
  console.log("Connecting to native messaging host " + hostName);
  
  port = chrome.runtime.connectNative(hostName);
  port.onMessage.addListener(onNativeMessage);
  port.onDisconnect.addListener(onDisconnected);
  
  initNativeMessaging();
}

function getQueryParams( name, url ) {
    if (!url) url = location.href;
    name = name.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
    var regexS = "[\\?&]"+name+"=([^&#]*)";
    var regex = new RegExp( regexS );
    var results = regex.exec( url );
	console.log('getQueryParams', results);
    return results == null ? null : results[1];
}

var IEPage = {
	windowId: null,
    tabId: null,
    isAttached: false,
    realTitle: '',
    hostName: '',
	onActivated: function(){
		port.postMessage({text: '#SHOW#'});
	},
	onDeactivated: function(){
		port.postMessage({text: '#HIDE#'});
	},
	initEvents: function(){
		chrome.tabs.onActivated.addListener(function (activeInfo) {
			if (activeInfo.tabId == this.tabId) {
				this.onActivated();
			} else {
				if (activeInfo.windowId == this.windowId) {
					this.onDeactivated();
				}
			}
		}.bind(this));

		chrome.windows.onFocusChanged.addListener(function (windowId) {
			if (windowId == this.windowId) {
				chrome.tabs.getCurrent(function (tab) {
					if (tab.active) {
						this.onActivated();
					}
				}.bind(this));
			} else {
				this.onDeactivated();
			}
		}.bind(this));

		chrome.tabs.onAttached.addListener(function (tabId, attachInfo) {
			if (tabId == this.tabId) {
				this.windowId = attachInfo.newWindowId;
			}
		}.bind(this));

		// If this is the active tab, then activate the host window
		chrome.tabs.getCurrent(function (tab) {
			if (tab.active) {
				this.onActivated();
			}
		}.bind(this));
	},
	init: function(){
		
		urlToSend = decodeURIComponent(getQueryParams("url"));
		
		chrome.tabs.getCurrent(function(tab) {
            this.tabId = tab.id;
            this.windowId = tab.windowId;
			connect();
			this.initEvents();			
        }.bind(this));
	}
}

document.addEventListener('DOMContentLoaded', function () {
	window.addEventListener("resize", onResizeNativeMessaging);
	window.addEventListener("mousemove", onMoveNativeMessaging);
	
	IEPage.init();
});

window.onbeforeunload = function() {
  message = {"text": "#STOP#"};
  port.postMessage(message);
  console.log("Sent message: " + JSON.stringify(message));
};
