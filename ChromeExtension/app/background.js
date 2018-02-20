//chrome.runtime.onMessage.addListener(
chrome.browserAction.onClicked.addListener(
	function(){
        chrome.storage.sync.get({
            url: 'http://47.21.44.216/'
        }, function(items) {
            console.log("chrome open valerus tab url : " + items.url);

            var url = "main.html?url="
                + encodeURIComponent(items.url);

            chrome.tabs.create({'url': url}, function (tab) {
                console.log("Valerus tab loaded", tab.id);
            });
        });
});