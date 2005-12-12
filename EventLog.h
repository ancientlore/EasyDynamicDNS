#pragma once

class EventLog
{
	static TCHAR sourceName[256];
	HANDLE hEventSource;
public:
	EventLog();
	~EventLog();

	void LogEvent(WORD type, const TCHAR* text);
	void LogLastError(const TCHAR* text);

	static BOOL RegisterEventSource();
	static BOOL UnregisterEventSource();
	static void SetSourceName(TCHAR* name);
};
