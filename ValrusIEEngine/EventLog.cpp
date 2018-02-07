#include "stdafx.h"
#include "EventLog.h"

void install_event_log_source(const std::wstring& a_name)
{
	const std::wstring key_path(L"SYSTEM\\CurrentControlSet\\Services\\"
		L"EventLog\\Application\\" + a_name);

	HKEY key;

	DWORD last_error = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		key_path.c_str(),
		0,
		0,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		0,
		&key,
		0);

	if (ERROR_SUCCESS == last_error)
	{
		BYTE exe_path[] = "C:\\Users\\roych\\source\\repos\\vicon\\Debug\\ValrusIEEngine.exe";
		DWORD last_error;
		const DWORD types_supported = EVENTLOG_ERROR_TYPE |
			EVENTLOG_WARNING_TYPE |
			EVENTLOG_INFORMATION_TYPE;

		last_error = RegSetValueEx(key,
			L"EventMessageFile",
			0,
			REG_SZ,
			exe_path,
			sizeof(exe_path));

		if (ERROR_SUCCESS == last_error)
		{
			last_error = RegSetValueEx(key,
				L"TypesSupported",
				0,
				REG_DWORD,
				(LPBYTE)&types_supported,
				sizeof(types_supported));
		}

		if (ERROR_SUCCESS != last_error)
		{
			//std::cerr << L"Failed to install source values: "
			//	<< last_error << "\n";
		}

		RegCloseKey(key);
	}
	else
	{
		//std::cerr << "Failed to install source: " << last_error << "\n";
	}
}

void log_event_log_message(const std::wstring& a_msg,
	const WORD         a_type,
	const std::wstring& a_name)
{
	DWORD event_id;

	switch (a_type)
	{
	case EVENTLOG_ERROR_TYPE:
		event_id = MSG_ERROR_1;
		break;
	case EVENTLOG_WARNING_TYPE:
		event_id = MSG_WARNING_1;
		break;
	case EVENTLOG_INFORMATION_TYPE:
		event_id = MSG_INFO_1;
		break;
	default:
		//std::cerr << "Unrecognised type: " << a_type << "\n";
		event_id = MSG_INFO_1;
		break;
	}

	HANDLE h_event_log = RegisterEventSource(0, a_name.c_str());

	if (0 == h_event_log)
	{
		//std::cerr << "Failed open source '" << a_name << "': " <<
		//	GetLastError() << "\n";
	}
	else
	{
		LPCTSTR message = a_msg.c_str();

		if (FALSE == ReportEvent(h_event_log,
			a_type,
			0,
			event_id,
			0,
			1,
			0,
			&message,
			0))
		{
			//std::cerr << "Failed to write message: " <<
			//	GetLastError() << "\n";
		}

		DeregisterEventSource(h_event_log);
	}
}

void uninstall_event_log_source(const std::wstring& a_name)
{
	const std::wstring key_path(L"SYSTEM\\CurrentControlSet\\Services\\"
		L"EventLog\\Application\\" + a_name);

	DWORD last_error = RegDeleteKey(HKEY_LOCAL_MACHINE,
		key_path.c_str());

	if (ERROR_SUCCESS != last_error)
	{
		//std::cerr << "Failed to uninstall source: " << last_error << "\n";
	}
}

/*int main(int a_argc, char** a_argv)
{
	const std::string event_log_source_name("my-test-event-log-source");

	install_event_log_source(event_log_source_name);

	log_event_log_message("hello, information",
		EVENTLOG_INFORMATION_TYPE,
		event_log_source_name);

	log_event_log_message("hello, error",
		EVENTLOG_ERROR_TYPE,
		event_log_source_name);

	log_event_log_message("hello, warning",
		EVENTLOG_WARNING_TYPE,
		event_log_source_name);

	// Uninstall when your application is being uninstalled.
	//uninstall_event_log_source(event_log_source_name);

	return 0;
}*/