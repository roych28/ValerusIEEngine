
function connect(){

	chrome.tabs.executeScript(null, {file: "main.js"});
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
