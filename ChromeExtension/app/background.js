chrome.runtime.onMessage.addListener(
  function(request, sender, sendResponse) {
    if (request.command == "connect")
	
		var url = "main.html?url="
        + encodeURIComponent(request.url);
	
		chrome.tabs.create({'url': url}, function(tab) {
			console.log("Valerus tab loaded");
		});
  });