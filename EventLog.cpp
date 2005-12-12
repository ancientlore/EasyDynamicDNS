#include "StdAfx.h"
#include "eventlog.h"
#include "EasyDynamicDNSMsg.h"
#include <stdarg.h>

TCHAR EventLog::sourceName[256] = _T("EasyDynamicDNS");

EventLog::EventLog()
{
	hEventSource = ::RegisterEventSource(NULL, sourceName);
}

EventLog::~EventLog()
{
	if (hEventSource != NULL) {
		::DeregisterEventSource(hEventSource);
		hEventSource = NULL;
	}
}

BOOL EventLog::RegisterEventSource()
{
	TCHAR modulePath[MAX_PATH];
	HKEY key;
	DWORD disposition;
	std::basic_string<TCHAR> s = _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
	s += sourceName;
	LONG result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, s.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &key, &disposition);
	if (result != ERROR_SUCCESS)
		return FALSE;
	// event types support
	disposition = 7;
	// get module path
	GetModuleFileName(GetModuleHandle(NULL), modulePath, MAX_PATH);
	RegSetValueEx(key, _T("EventMessageFile"), 0, REG_EXPAND_SZ, (LPBYTE) modulePath, (DWORD) _tcslen(modulePath) + 1);
	RegSetValueEx(key, _T("TypesSupported"), 0, REG_DWORD, (LPBYTE) &disposition, sizeof(DWORD));
	RegCloseKey(key);
	return TRUE;
}

BOOL EventLog::UnregisterEventSource()
{
	HKEY key;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"), 0, KEY_READ | KEY_WRITE, &key);
	if (result != ERROR_SUCCESS)
		return FALSE;
	RegDeleteKey(key, sourceName);
	RegCloseKey(key);
	return TRUE;
}

void EventLog::SetSourceName(TCHAR* name)
{
	_tcsncpy(sourceName, name, 255);
	sourceName[255] = 0;
}

void EventLog::LogEvent(WORD type, const TCHAR* text)
{
	DWORD dwEventID;
	LPCTSTR lpszStrings[1];

	lpszStrings[0] = text;
	if (*(lpszStrings[0]) == _T('\0'))
		lpszStrings[0] = _T("(Null)");

	switch (type) {
	case EVENTLOG_ERROR_TYPE:
		dwEventID = MSG_EASYDYNAMICDNS_ERROR;
		break;
	case EVENTLOG_WARNING_TYPE:
		dwEventID = MSG_EASYDYNAMICDNS_WARNING;
		break;
	case EVENTLOG_INFORMATION_TYPE:
		dwEventID = MSG_EASYDYNAMICDNS_INFO;
		break;
	case EVENTLOG_SUCCESS:
		dwEventID = MSG_EASYDYNAMICDNS_SUCCESS;
		break;
	default:
		type = EVENTLOG_ERROR_TYPE;
		dwEventID = MSG_EASYDYNAMICDNS_ERROR;
		break;
	}

	if (hEventSource != NULL && hEventSource != INVALID_HANDLE_VALUE)
		ReportEvent(hEventSource, type, 0, dwEventID, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
}

void EventLog::LogLastError(const TCHAR* text)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	LPCTSTR lpszStrings[2];

	lpszStrings[0] = text;
	if (*(lpszStrings[0]) == _T('\0'))
		lpszStrings[0] = _T("(Null)");
	lpszStrings[1] = (LPTSTR) lpMsgBuf;

	if (hEventSource != NULL && hEventSource != INVALID_HANDLE_VALUE)
		ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, MSG_EASYDYNAMICDNS_ERROR_INFO, NULL, 2, 0, (LPCTSTR*) &lpszStrings[0], NULL);

	// Free the buffer.
	LocalFree(lpMsgBuf);
}
