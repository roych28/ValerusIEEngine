// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var port = null;
var currentWindowPos = {x:-1,y:-1};

var getKeys = function(obj){
   var keys = [];
   for(var key in obj){
      keys.push(key);
   }
   return keys;
}

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

function appendMessage(text) {
  document.getElementById('response').innerHTML += "<p>" + text + "</p>";
}

function updateUiState() {
  if (port) {
    document.getElementById('connect-button').style.display = 'none';
    //document.getElementById('input-text').style.display = 'block';
    document.getElementById('send-message-button').style.display = 'block';
  } else {
    document.getElementById('connect-button').style.display = 'block';
    //document.getElementById('input-text').style.display = 'none';
    document.getElementById('send-message-button').style.display = 'none';
  }
}

function sendNativeMessage(e) {
  message = {	"text": document.getElementById('input-text').value,
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y
	};
  port.postMessage(message);
  appendMessage("Sent message: <b>" + JSON.stringify(message) + "</b>");
}

function initNativeMessaging(e) {
  message = {	"text": "#INIT#",
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y
	};
	
	if(port){
		port.postMessage(message);
	}
  
  //appendMessage("Sent message: <b>" + JSON.stringify(message) + "</b>");
}

function onResizeNativeMessaging(e) {
  message = {	"text": "#ONRESIZE#",
				"width"  : window.innerWidth,
				"height" : window.innerHeight,
				"x"		 : currentWindowPos.x,
				"y"		 : currentWindowPos.y
	};
  port.postMessage(message);
  //appendMessage("Sent message: <b>" + JSON.stringify(message) + "</b>");
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
  appendMessage("Received message: <b>" + JSON.stringify(message) + "</b>");
}

function onDisconnected() {
  appendMessage("Failed to connect: " + chrome.runtime.lastError.message);
  port = null;
  updateUiState();
}

function connect() {
  var hostName = "com.google.chrome.example";
  appendMessage("Connecting to native messaging host <b>" + hostName + "</b>")
  port = chrome.runtime.connectNative(hostName);
  port.onMessage.addListener(onNativeMessage);
  port.onDisconnect.addListener(onDisconnected);
  updateUiState();
}

document.addEventListener('DOMContentLoaded', function () {
  document.getElementById('connect-button').addEventListener(
      'click', connect);
  document.getElementById('send-message-button').addEventListener(
      'click', initNativeMessaging);
  window.addEventListener("resize", onResizeNativeMessaging);
  window.addEventListener("mousemove", onMoveNativeMessaging);
 
  updateUiState();
});

window.onbeforeunload = function() {
  message = {"text": "#STOP#"};
  port.postMessage(message);
  appendMessage("Sent message: <b>" + JSON.stringify(message) + "</b>");
};
