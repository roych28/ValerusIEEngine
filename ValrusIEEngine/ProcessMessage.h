#include "stdafx.h"

// The message has the size header trimed, only contains the message boday.
void ProcessMessage(const char* jsonMsg, std::map<std::string, std::string> &parsedValues);