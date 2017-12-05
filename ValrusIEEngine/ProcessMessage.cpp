// ////////////////////////////////////////////////////////////////////////////
// Carestream Health RESTRICTED INFORMATION - for internal use only
// ////////////////////////////////////////////////////////////////////////////
// 
// File:    ProcessMessage.cpp
// Author:  Forrest Feng
// Date:    2016.02.18
// Remark:  Process message received from chromium with JsonCpp librar.
//
// Copyright 2016, All Rights Reserved.
// 
// ////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ProcessMessage.h"
#include "json\json.h"

// Edit this file to handle more request message from chromium browser

void ProcessMessage(const char* jsonMsg, std::map<std::string,std::string> &parsedValues)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(jsonMsg, root);
	if (!parsingSuccessful)
	{
		return;
	}

	parsedValues["command"] = root.get("text", "").asString();
	parsedValues["width"]	= root.get("width", "").asString();
	parsedValues["height"]	= root.get("height", "").asString();
	/*if (request == "logoff_windows")
	{
		//ExitWindowsEx(EWX_LOGOFF, 0);
	}*/
}

