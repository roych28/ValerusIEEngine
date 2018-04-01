
var timer = null;
var installTabId;
function loadTab(){
    chrome.storage.sync.get({
        url: 'http://47.21.44.216/'
    }, function(items) {
        console.log("chrome open valerus tab url : " + items.url);

        var url = "main.html?url="
            + encodeURIComponent(items.url);

        if(installTabId){
            chrome.tabs.remove(installTabId);
        }

        chrome.windows.create({'url': url}, function (tab) {
            console.log("Valerus tab loaded", tab.id);
        });
    });
}

function stop(port){
    message = {"text": "#STOP#"};
    if(port){
        port.postMessage(message);
        console.log("Sent message: " + JSON.stringify(message));
    }
}

function isNativeHostInstalled(){
    var port = chrome.runtime.connectNative('com.vicon.valerus.app');
    if(port){
        port.onMessage.addListener(function(msg){
            port.disconnect();
            port = null;
            clearTimeout(timer);
            loadTab();
        });

        port.onDisconnect.addListener(function(){
            port = null;
            if(!installTabId){
                chrome.tabs.create({'url': "installNativeHost.html"}, function (tab) {
                    installTabId = tab.id;
                    console.log("installNativeHost Valerus tab loaded", tab.id);
                });
            }

            timer = setTimeout(isNativeHostInstalled, 1000);
        });

        window.setTimeout(function() {
            stop(port);
        }, 1000);

    }
}

chrome.browserAction.onClicked.addListener(
	function(){
        installTabId = null;
	    isNativeHostInstalled(function success(){

        }, function error() {

        });
});