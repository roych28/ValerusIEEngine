#include <windows.h>
#include <string>

enum {
	MSG_ERROR_1,
	MSG_WARNING_1,
	MSG_INFO_1
};

void install_event_log_source(const std::wstring& a_name);
void log_event_log_message(const std::wstring& a_msg,
	const WORD         a_type,
	const std::wstring& a_name);
void uninstall_event_log_source(const std::wstring& a_name);

const std::wstring event_log_source_name(L"ValerusIEEngine");