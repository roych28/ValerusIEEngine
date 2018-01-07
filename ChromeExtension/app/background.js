// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
alert('1');

chrome.browserAction.onClicked.addListener(function callback(){
	alert('browserAction.onClicked');
	
	

      chrome.tabs.create({'url': 'main.html'}, function(tab) {

      });

	//window.addEventListener("resize", onResizeNativeMessaging);
	//window.addEventListener("mousemove", onMoveNativeMessaging);
 
	//connect();
});