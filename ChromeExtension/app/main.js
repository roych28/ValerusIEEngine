
var IEPage = {
	port: null,
	windowId: null,
    tabId: null,
    hostName: 'com.vicon.valerus.app',
	currentWindowPos: {x:-1,y:-1},
    realWindowTitle: "",
    nextTabId: 0,
	urlToSend: "http://47.21.44.216/", // default url
	initialize: function(){
		window.addEventListener("resize", 	 this.onResizeNativeMessaging.bind(this));
		//window.addEventListener("mousemove", this.onMoveNativeMessaging.bind(this));
	
		urlToSend = decodeURIComponent(this.getQueryParams("url"));
		
		chrome.tabs.getCurrent(function(tab) {
            this.tabId = tab.id;
            this.windowId = tab.windowId;
			this.connect();
        }.bind(this));
	},
	stop: function(){
		message = {"text": "#STOP#"};
		if(this.port){
			this.port.postMessage(message);
			console.log("Sent message: " + JSON.stringify(message));
		}
		
	},
	connect: function() {
	  	console.log("Connecting to native messaging host " + this.hostName);
	  
	  	this.port = chrome.runtime.connectNative(this.hostName);
	  	this.port.onMessage.addListener(this.onNativeMessage.bind(this));
	  	this.port.onDisconnect.addListener(this.onDisconnected.bind(this));

        this.connectNativeMessaging();
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
			/*if (windowId == this.windowId) {
				chrome.tabs.getCurrent(function (tab) {
					if (tab.active) {
						this.onActivated();
					}
				}.bind(this));
			} else {
				this.onDeactivated();
			}*/
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
		if(this.port){
			this.port.postMessage({text: '#SHOW#'});
		}
	},
	onDeactivated: function(){
		if(this.port){
			this.port.postMessage({text: '#HIDE#'});
		}
	},
	onDisconnected: function() {
		console.log("onDisconnected : " + chrome.runtime.lastError.message);
		this.port = null;
	},
	onNativeMessage: function(message) {
        console.log("Received message: " + JSON.stringify(message));
		switch(message.type){
			case"#CONNECTED#":
				this.restoreWindowTitle();
                this.initNativeMessaging();
                this.initEvents();
                break;
			case "#NEW_WINDOW_OPEN#":
                var url = "main.html?url="
                    + encodeURIComponent(message.text);
                chrome.windows.create({
                    // Just use the full URL if you need to open an external page
                    url: chrome.runtime.getURL(url)
                });
				break;
		}
	},
    getMessage: function(type){
        var message = {	"text": type,
            "width"  : this.getInnerWidth(),
            "height" : this.getInnerHeight(),
            "x"		 : this.currentWindowPos.x,
            "y"		 : this.currentWindowPos.y,
            "url"	 : urlToSend
        };

        return message;
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
    getInnerWidth: function() {
        return Math.floor(window.innerWidth * window.devicePixelRatio);
    },
    getInnerHeight: function() {
        return Math.floor(window.innerHeight * window.devicePixelRatio);
    },
	/*onMoveNativeMessaging: function(e) {
		var windowPos = this.getWindowPos();
		
		if( windowPos.x != this.currentWindowPos.x || this.currentWindowPos.y != this.currentWindowPos.y){
			this.currentWindowPos = windowPos;
            var message = this.getMessage('#ONMOVE#');

			if(this.port){
				this.port.postMessage(message);
			}

			console.log("Sent message: " + JSON.stringify(message));
		}	
	},*/
	onResizeNativeMessaging: function(e) {
        var message = this.getMessage('#ONRESIZE#');
		if(this.port){
			this.port.postMessage(message);
			console.log("Sent message: " + JSON.stringify(message));
		}
	},
    initNativeMessaging: function() {
        var message = this.getMessage('#INIT#');

        if(this.port){
            this.port.postMessage(message);
        }

        console.log("Sent message: " + JSON.stringify(message));
    },
    connectNativeMessaging: function() {
        if (document.title.indexOf('valerusTab-') == -1) {
            this.realWindowTitle = document.title;
            document.title = 'valerusTab-' + this.getNextTabId();
        }

        var message = this.getMessage('#CONNECTED#');
        message.windowTitle = document.title;

        if(this.port){
            console.log("Sent message: " + JSON.stringify(message));
            this.port.postMessage(message);
        }
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
	},
    getNextTabId: function() {
        this.nextTabId++;
        return (Math.random().toString(36).substr(2,5)) + this.nextTabId;
    },
	restoreWindowTitle: function(){
        document.title = this.realWindowTitle;
	}
}

document.addEventListener('DOMContentLoaded', function () {	
	IEPage.initialize();
});

window.onbeforeunload = function() {
	IEPage.stop();
};
