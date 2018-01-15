
var IEPage = {
	port: null,
	windowId: null,
    tabId: null,
    hostName: 'com.vicon.valerus.app',
	currentWindowPos: {x:-1,y:-1},
	urlToSend: "http://47.21.44.216/", // default url
	initialize: function(){
		window.addEventListener("resize", 	 this.onResizeNativeMessaging);
		window.addEventListener("mousemove", this.onMoveNativeMessaging);
	
		urlToSend = decodeURIComponent(this.getQueryParams("url"));
		
		chrome.tabs.getCurrent(function(tab) {
            this.tabId = tab.id;
            this.windowId = tab.windowId;
			this.connect();
			this.initEvents();			
        }.bind(this));
	},
	stop: function(){
		message = {"text": "#STOP#"};
		this.port.postMessage(message);
		console.log("Sent message: " + JSON.stringify(message));
	},
	connect: function() {
	  console.log("Connecting to native messaging host " + this.hostName);
	  
	  this.port = chrome.runtime.connectNative(this.hostName);
	  this.port.onMessage.addListener(this.onNativeMessage);
	  this.port.onDisconnect.addListener(this.onDisconnected);
	  
	  this.initNativeMessaging();
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

		chrome.tabs.getCurrent(function (tab) {
			if (tab.active) {
				this.onActivated();
			}
		}.bind(this));
	},
	onActivated: function(){
		this.port.postMessage({text: '#SHOW#'});
	},
	onDeactivated: function(){
		this.port.postMessage({text: '#HIDE#'});
	},
	onDisconnected: function() {
		console.log("onDisconnected : " + chrome.runtime.lastError.message);
		this.port = null;
	},
	onNativeMessage: function(message) {
		console.log("Received message: " + JSON.stringify(message));
	},
	getQueryParams: function( name, url ) {
		if (!url) url = location.href;
		name = name.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
		var regexS = "[\\?&]"+name+"=([^&#]*)";
		var regex = new RegExp( regexS );
		var results = regex.exec( url );
		console.log('getQueryParams', results);
		return results == null ? null : results[1];
	},
	onMoveNativeMessaging: function(e) {
		var windowPos = this.getWindowPos();
		
		if( windowPos.x != this.currentWindowPos.x || this.currentWindowPos.y != this.currentWindowPos.y){
			this.currentWindowPos = windowPos;
			message = {	"text": "#ONMOVE#",
					"width"  : window.innerWidth,
					"height" : window.innerHeight,
					"x"		 : this.currentWindowPos.x,
					"y"		 : this.currentWindowPos.y			
			};
		
			this.port.postMessage(message);
			console.log("Sent message: " + JSON.stringify(message));
		}	
	},
	onResizeNativeMessaging: function(e) {
		message = {	"text": "#ONRESIZE#",
					"width"  : window.innerWidth,
					"height" : window.innerHeight,
					"x"		 : this.currentWindowPos.x,
					"y"		 : this.currentWindowPos.y 
		};
		this.port.postMessage(message);
		console.log("Sent message: " + JSON.stringify(message));
	},
	initNativeMessaging: function(e) {
		message = {	"text": "#INIT#",
					"width"  : window.innerWidth,
					"height" : window.innerHeight,
					"x"		 : this.currentWindowPos.x,
					"y"		 : this.currentWindowPos.y,
					"url"	 : urlToSend
		};

		if(this.port){
			this.port.postMessage(message);
		}

		console.log("Sent message: " + JSON.stringify(message));
	},
	getWindowPos: function() {
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
}

document.addEventListener('DOMContentLoaded', function () {	
	IEPage.initialize();
});

window.onbeforeunload = function() {
	IEPage.stop();
};
