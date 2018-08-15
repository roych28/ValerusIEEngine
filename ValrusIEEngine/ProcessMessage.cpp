#include "stdafx.h"
#include "ProcessMessage.h"
#include "json\json.h"

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
	parsedValues["height"] = root.get("height", "").asString();
	parsedValues["url"] = root.get("url", "http://47.21.44.216/").asString();
	parsedValues["windowTitle"] = root.get("windowTitle", "omfoabjiohaoglgimheiknfmmlfdhpke/main.html").asString();
}

