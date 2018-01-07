
function connect(){
	chrome.runtime.sendMessage({
		command: "connect",
		url:	 document.getElementById("urlInput").value
    },
    function(response) {
		
    });
}

document.addEventListener('DOMContentLoaded', function () {
  document.getElementById('connect-button').addEventListener(
      'click', connect);
});
/*
function hello() {
  chrome.tabs.executeScript({
    file: 'alert.js'
  }); 
}

document.getElementById('clickme').addEventListener('click', hello);
*/
