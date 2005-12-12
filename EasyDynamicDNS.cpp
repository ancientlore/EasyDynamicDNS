// EasyDynamicDNS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


BOOL Install()
{
	EventLog::RegisterEventSource();
	EventLog log;

	// open SCManager
	SC_HANDLE scMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scMgr == NULL) {
		log.LogLastError("Unable to open service control manager");
		_tprintf(_T("Unable to open service manager!\n"));
		return FALSE;
	}
	// get module path
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), modulePath, MAX_PATH);
	// create service
	SC_HANDLE svc = CreateService(scMgr, _T("EasyDynamicDNS"), _T("EasyDynamicDNS"),
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL, modulePath,
		NULL, NULL, NULL, NULL, NULL);
	if (svc == NULL) {
		log.LogLastError("Unable to install service");
		_tprintf(_T("Unable to install service!\n"));
		CloseServiceHandle(scMgr);
		return FALSE;
	}
	CloseServiceHandle(svc);

	// close SCManager
	CloseServiceHandle(scMgr);

	HKEY key;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\EasyDynamicDNS"), 0, KEY_READ | KEY_WRITE, &key);
	if (result == ERROR_SUCCESS) {
		const TCHAR* desc = _T("Updates dynamic DNS for easyDNS domains");
		RegSetValueEx(key, _T("Description"), 0, REG_EXPAND_SZ, (LPBYTE) desc, (DWORD) _tcslen(desc) + 1);
		DWORD val = 2;
		RegSetValueEx(key, _T("Start"), 0, REG_DWORD, (LPBYTE) &val, (DWORD) sizeof(DWORD));
		RegCloseKey(key);
	}

	log.LogEvent(EVENTLOG_INFORMATION_TYPE, "EasyDynamicDNS installed.");
	return TRUE;
}

BOOL Uninstall()
{
	EventLog log;

	// open SCManager
	SC_HANDLE scMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scMgr == NULL) {
		log.LogLastError("Unable to open service control manager");
		_tprintf(_T("Unable to open service manager!\n"));
		return FALSE;
	}

	// open service and delete
	SC_HANDLE svc = OpenService(scMgr, _T("EasyDynamicDNS"), DELETE);
	if (svc == NULL) {
		log.LogLastError("Unable to open service");
		_tprintf(_T("Unable to open service!\n"));
		CloseServiceHandle(scMgr);
		return FALSE;
	}
	if (DeleteService(svc) != TRUE) {
		log.LogLastError("Unable to delete service");
		_tprintf(_T("Unable to delete service!\n"));
		CloseServiceHandle(svc);
		CloseServiceHandle(scMgr);
		return FALSE;
	}
	CloseServiceHandle(svc);

	// close SCManager
	CloseServiceHandle(scMgr);

	log.LogEvent(EVENTLOG_INFORMATION_TYPE, "EasyDynamicDNS uninstalled.");
	EventLog::UnregisterEventSource();
	return TRUE;
}

BOOL Start()
{
	EventLog log;

	// open SCManager
	SC_HANDLE scMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scMgr == NULL) {
		log.LogLastError("Unable to open service control manager");
		_tprintf(_T("Unable to open service manager!\n"));
		return FALSE;
	}

	// open service and delete
	SC_HANDLE svc = OpenService(scMgr, _T("EasyDynamicDNS"), SERVICE_ALL_ACCESS);
	if (svc == NULL) {
		log.LogLastError("Unable to open service");
		_tprintf(_T("Unable to open service!\n"));
		CloseServiceHandle(scMgr);
		return FALSE;
	}
	if (StartService(svc, 0, NULL) != TRUE) {
		log.LogLastError("Unable to start service");
		_tprintf(_T("Unable to start service!\n"));
		CloseServiceHandle(svc);
		CloseServiceHandle(scMgr);
		return FALSE;
	}

	// Wait for it to start
	SERVICE_STATUS stat;
	if (!QueryServiceStatus(svc, &stat)){
		log.LogLastError("Unable to query service status");
		_tprintf(_T("Unable to query status!\n"));
		CloseServiceHandle(svc);
		CloseServiceHandle(scMgr);
		return FALSE;
	}

	int tries = 5;

	while (stat.dwCurrentState == SERVICE_START_PENDING  &&  tries > 0) 
	{ 
		tries--;
		Sleep(2000);
 
		if (!QueryServiceStatus(svc, &stat)) {
			log.LogLastError("Unable to query service status");
			_tprintf(_T("Unable to query status!\n"));
			CloseServiceHandle(svc);
			CloseServiceHandle(scMgr);
			return FALSE;
		}
	}			

	CloseServiceHandle(svc);

	// close SCManager
	CloseServiceHandle(scMgr);

	return TRUE;
}

BOOL Stop()
{
	EventLog log;

	// open SCManager
	SC_HANDLE scMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scMgr == NULL) {
		log.LogLastError("Unable to open service control manager");
		_tprintf(_T("Unable to open service manager!\n"));
		return FALSE;
	}

	// open service and delete
	SC_HANDLE svc = OpenService(scMgr, _T("EasyDynamicDNS"), SERVICE_ALL_ACCESS);
	if (svc == NULL) {
		log.LogLastError("Unable to open service");
		_tprintf(_T("Unable to open service!\n"));
		CloseServiceHandle(scMgr);
		return FALSE;
	}
	SERVICE_STATUS stat;
	if (ControlService(svc, SERVICE_CONTROL_STOP, &stat) != TRUE) {
		log.LogLastError("Unable to stop service");
		_tprintf(_T("Unable to stop service!\n"));
		CloseServiceHandle(svc);
		CloseServiceHandle(scMgr);
		return FALSE;
	}

	if (!QueryServiceStatus(svc, &stat)){
		log.LogLastError("Unable to query service status");
		_tprintf(_T("Unable to query status!\n"));
		CloseServiceHandle(svc);
		CloseServiceHandle(scMgr);
		return FALSE;
	}

	int tries = 5;

	while (stat.dwCurrentState == SERVICE_STOP_PENDING  &&  tries > 0) 
	{ 
		tries--;
		Sleep(2000);
 
		if (!QueryServiceStatus(svc, &stat)) {
			log.LogLastError("Unable to query service status");
			_tprintf(_T("Unable to query status!\n"));
			CloseServiceHandle(svc);
			CloseServiceHandle(scMgr);
			return FALSE;
		}
	}			

	CloseServiceHandle(svc);

	// close SCManager
	CloseServiceHandle(scMgr);

	return TRUE;
}

DWORD LoadTimeout(HKEY baseKey)
{
	DWORD disposition;
	DWORD type;
	DWORD tmpInt = 0;
	DWORD result = 1000 * 60 * 60 * 1;	// default 1 hour
	HKEY subkey;

	LONG res = RegOpenKeyEx(baseKey, _T("EasyConfig"), 0, KEY_READ, &subkey);
	if (res == ERROR_SUCCESS) {
		disposition = sizeof(DWORD);
		type = REG_DWORD;
		if (RegQueryValueEx(subkey, _T("Timeout"), NULL, &type, (LPBYTE) &tmpInt, &disposition) == ERROR_SUCCESS && type == REG_DWORD) {
			if (tmpInt > 0)
				result = tmpInt;
		}
		RegCloseKey(subkey);
	}

	return result;
}

BOOL SaveTimeout(HKEY baseKey, DWORD timeout)
{
	BOOL rval = FALSE;
	HKEY subkey;
	DWORD disposition;

	LONG result = RegCreateKeyEx(baseKey, _T("EasyConfig"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &subkey, &disposition);
	if (result == ERROR_SUCCESS) {
		if (RegSetValueEx(subkey, _T("Timeout"), 0, REG_DWORD, (LPBYTE) &timeout, (DWORD) sizeof(DWORD)) == ERROR_SUCCESS)
			rval = TRUE;
		RegCloseKey(subkey);
	}

	return rval;
}

BOOL LoadConfig(HKEY baseKey, DWORD id, DomainInfo& info)
{
	DWORD disposition;
	DWORD type;
	TCHAR tmpStr[MAX_PATH + 1];
	DWORD tmpInt;
	HKEY subkey;
	TCHAR x[128];
	BOOL rval = FALSE;

	wsprintf(x, _T("DomainInfo%03d"), id);
	LONG result = RegOpenKeyEx(baseKey, x, 0, KEY_READ, &subkey);
	if (result == ERROR_SUCCESS) {

		disposition = sizeof(DWORD);
		type = REG_DWORD;
		if (RegQueryValueEx(subkey, _T("Http Timeout"), NULL, &type, (LPBYTE) &tmpInt, &disposition) == ERROR_SUCCESS && type == REG_DWORD)
			info.httpTimeout = tmpInt;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("Host Name"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.hostname = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("Top Level Domain"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.tld = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("My IP Address"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.myip = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("MX"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.mx = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("Back MX"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.backmx = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("WildCard"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.wildcard = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("easyDNS URL"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.easydnsurl = tmpStr;

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("Username"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.username = tmpStr;

		disposition = MAX_PATH;
		type = REG_BINARY;
		DATA_BLOB in;
		DATA_BLOB out;
		out.cbData = 0;
		out.pbData = 0;
		in.cbData = 4000;
		in.pbData = (BYTE*) LocalAlloc(LPTR, 4000);
		if (RegQueryValueEx(subkey, _T("Password"), NULL, &type, (LPBYTE) in.pbData, &in.cbData) == ERROR_SUCCESS && type == REG_BINARY) {
			if (!CryptUnprotectData(&in, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &out))
				rval = FALSE;
			info.password = (TCHAR*) out.pbData;
		}
		if (in.pbData)
			LocalFree(in.pbData);
		if (out.pbData)
			LocalFree(out.pbData);

		disposition = MAX_PATH;
		type = REG_EXPAND_SZ;
		*tmpStr = 0;
		if (RegQueryValueEx(subkey, _T("Proxy"), NULL, &type, (LPBYTE) &tmpStr, &disposition) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
			info.proxy = tmpStr;

		disposition = sizeof(DWORD);
		type = REG_DWORD;
		if (RegQueryValueEx(subkey, _T("Proxy Port"), NULL, &type, (LPBYTE) &tmpInt, &disposition) == ERROR_SUCCESS && type == REG_DWORD)
			info.proxyPort = tmpInt;

		rval = TRUE;
		RegCloseKey(subkey);
	}
	return rval;
}

BOOL AddConfig(HKEY baseKey, DWORD id, const DomainInfo& info)
{
	DWORD disposition;
	HKEY subkey;
	TCHAR x[128];
	BOOL rval = TRUE;

	wsprintf(x, _T("DomainInfo%03d"), id);
	LONG result = RegCreateKeyEx(baseKey, x, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &subkey, &disposition);
	if (result == ERROR_SUCCESS) {
		if (RegSetValueEx(subkey, _T("Http Timeout"), 0, REG_DWORD, (LPBYTE) &info.httpTimeout, (DWORD) sizeof(DWORD)) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("Host Name"), 0, REG_EXPAND_SZ, (LPBYTE) info.hostname.c_str(), (DWORD) info.hostname.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("Top Level Domain"), 0, REG_EXPAND_SZ, (LPBYTE) info.tld.c_str(), (DWORD) info.tld.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("My IP Address"), 0, REG_EXPAND_SZ, (LPBYTE) info.myip.c_str(), (DWORD) info.myip.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("MX"), 0, REG_EXPAND_SZ, (LPBYTE) info.mx.c_str(), (DWORD) info.mx.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("Back MX"), 0, REG_EXPAND_SZ, (LPBYTE) info.backmx.c_str(), (DWORD) info.backmx.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("Wildcard"), 0, REG_EXPAND_SZ, (LPBYTE) info.wildcard.c_str(), (DWORD) info.wildcard.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("easyDNS URL"), 0, REG_EXPAND_SZ, (LPBYTE) info.easydnsurl.c_str(), (DWORD) info.easydnsurl.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("Username"), 0, REG_EXPAND_SZ, (LPBYTE) info.username.c_str(), (DWORD) info.username.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;

		DATA_BLOB in;
		DATA_BLOB out;
		in.cbData = (DWORD) info.password.length() + 1;
		in.pbData = (BYTE*) info.password.c_str();
		out.pbData = 0;
		out.cbData = 0;
		if (!CryptProtectData(&in, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &out))
			rval = FALSE;
		else if (RegSetValueEx(subkey, _T("Password"), 0, REG_BINARY, (LPBYTE) out.pbData, (DWORD) out.cbData) != ERROR_SUCCESS)
			rval = FALSE;
		if (out.pbData != 0)
			LocalFree(out.pbData);

		if (RegSetValueEx(subkey, _T("Proxy"), 0, REG_EXPAND_SZ, (LPBYTE) info.proxy.c_str(), (DWORD) info.proxy.length() + 1) != ERROR_SUCCESS)
			rval = FALSE;
		if (RegSetValueEx(subkey, _T("Proxy Port"), 0, REG_DWORD, (LPBYTE) &info.proxyPort, (DWORD) sizeof(DWORD)) != ERROR_SUCCESS)
			rval = FALSE;
		RegCloseKey(subkey);
	}
	else
		rval = FALSE;
	return rval;
}

BOOL Config(int argc, _TCHAR* argv[])
{
	EventLog log;
	HKEY key;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\EasyDynamicDNS"), 0, KEY_READ | KEY_WRITE, &key);
	if (result == ERROR_SUCCESS) {
		tstring command = _T("show");
		if (argc > 0)
			command = argv[0];
		if (_tcsicmp(command.c_str(), _T("show")) == 0) {
			if (argc > 1)
				_tprintf(_T("Syntax error; use \"/config help\" for more information.\n"));
			else {
				DWORD timeout = LoadTimeout(key);
				_tprintf(_T("Update dynamic DNS domains every %d minutes.\n\n"), timeout / (1000 * 60));
				int i = 0;
				BOOL found;
				do {
					DomainInfo info;
					found = LoadConfig(key, i, info);
					if (found == TRUE) {
						_tprintf(_T("Entry #%d\n"), i);
						_tprintf(_T("  Host Name:        %s\n"), info.hostname.c_str());
						_tprintf(_T("  Top Level Domain: %s\n"), info.tld.c_str());
						_tprintf(_T("  My IP Address:    %s\n"), (info.myip.length() > 0 ? info.myip.c_str() : _T("<my PC's address>")));
						_tprintf(_T("  MX:               %s\n"), info.mx.c_str());
						_tprintf(_T("  Back MX:          %s\n"), info.backmx.c_str());
						_tprintf(_T("  Wildcard:         %s\n"), info.wildcard.c_str());
						_tprintf(_T("  easyDNS URL:      %s\n"), (info.easydnsurl.length() > 0 ? info.easydnsurl.c_str() : _T("http://members.easydns.com/dyn/dyndns.php")));
						_tprintf(_T("  HTTP Timeout:     %d sec\n"), info.httpTimeout / 1000);
						_tprintf(_T("  Username:         %s\n"), info.username.c_str());
						_tprintf(_T("  Password:         ********\n"));
						_tprintf(_T("  Proxy:            %s\n"), info.proxy.c_str());
						_tprintf(_T("  Proxy Port:       %d\n"), info.proxyPort);
					}
					i++;
				}
				while (i < 100);
			}
		}
		else if (_tcsicmp(command.c_str(), _T("add")) == 0) {
			if (argc != 13)
				_tprintf(_T("Syntax error; use \"/config help\" for more information.\n"));
			else {
				int i = 0;
				BOOL found;
				do {
					DomainInfo info;
					found = LoadConfig(key, i, info);
					if (found != TRUE) {
						info.hostname = argv[1];
						info.tld = argv[2];
						info.myip = argv[3];
						info.mx = argv[4];
						info.backmx = argv[5];
						info.wildcard = argv[6];
						if (argv[7][0] != 0)
							info.easydnsurl = argv[7];
						info.httpTimeout = atoi(argv[8]) * 1000;
						info.username = argv[9];
						info.password = argv[10];
						//info.domain = argv[11];
						info.proxy = argv[11];
						info.proxyPort = atoi(argv[12]);
						if (AddConfig(key, i, info) != TRUE) {
							TCHAR x[50];
							wsprintf(x, _T("DomainInfo%03d"), i);
							_tprintf(_T("Error saving to registry\n"));
							RegDeleteKey(key, x);
						}
					}
					i++;
				}
				while (i < 100 && found == TRUE);
				if (found == TRUE)
					_tprintf(_T("There is not room for another entry.\n"));
			}
		}
		else if (_tcsicmp(command.c_str(), _T("prompt")) == 0) {
			if (argc != 1)
				_tprintf(_T("Syntax error; use \"/config help\" for more information.\n"));
			else {
				int i = 0;
				BOOL found;
				do {
					DomainInfo info;
					found = LoadConfig(key, i, info);
					if (found != TRUE) {
						TCHAR buf[256];
						_tprintf("Enter your setup as described below. This program does not do much validation;\n"
							"please follow the instructions carefully.\n\n");
						_tprintf("Host Name = the full hostname being updated (REQUIRED)\n");
						info.hostname = _getts(buf);
						_tprintf("Top Level Domain = the root domain of your hostname, for example if your\n"
							"hostname is \"example.co.uk\" you should set \"tld\" to \"co.uk\". This field\n"
							"can be omitted in cases of second level domains like example.com\n");
						info.tld = _getts(buf);
						_tprintf("My IP Address = the IP address of the client to be updated. Send \"0.0.0.0\"\n"
							"to set record to an offline state (sets record to \"offline.easydns.com\"). If\n"
							"you are behind a firewall or NAT set this to 1.1.1.1 and our system will detect\n"
							"your remote IP for you. Leave blank to use your PC's address.\n");
						info.myip = _getts(buf);
						_tprintf("MX = use this parameter as the MX handler for the domain being updated, it\n"
							"defaults to preference 5. (OPTIONAL)\n");
						info.mx = _getts(buf);
						_tprintf("Back MX = either \"YES\" or \"NO\", if \"YES\" we set smtp.easydns.com to\n"
							"be a backup mail spool for domain being updated at preference 100. (OPTIONAL)\n");
						info.backmx = _getts(buf);
						_tprintf("Wildcard = either \"ON\" or \"OFF\", if \"ON\" sets a wildcard host record for\n"
							"the domain being updated equal to the IP address specified in \"myip\" (OPTIONAL)\n");
						info.wildcard = _getts(buf);

						_tprintf("easyDNS URL = the url to send the update to; defaults to\n"
							"http://members.easydns.com/dyn/dyndns.php (OPTIONAL)\n");
						info.easydnsurl = _getts(buf);
						_tprintf("HTTP Timeout = Number of seconds to allow the request to complete. 0 for\n"
							"no timeout.\n");
						info.httpTimeout = atoi(_getts(buf)) * 1000;
						_tprintf("Username = easyDNS user name (REQUIRED)\n");
						info.username = _getts(buf);
						_tprintf("Password = easyDNS password (which is stored securely) (REQUIRED)\n");
						info.password = _getts(buf);
						_tprintf("Proxy = proxy server (leave blank if you don't have one)\n");
						info.proxy = _getts(buf);
						_tprintf("Proxy Port = proxy port; 0 if you don't have one\n");
						info.proxyPort = atoi(_getts(buf));

						if (AddConfig(key, i, info) != TRUE) {
							TCHAR x[50];
							wsprintf(x, _T("DomainInfo%03d"), i);
							_tprintf(_T("Error saving to registry\n"));
							RegDeleteKey(key, x);
						}
					}
					i++;
				}
				while (i < 100 && found == TRUE);
				if (found == TRUE)
					_tprintf(_T("There is not room for another entry.\n"));
			}
		}
		else if (_tcsicmp(command.c_str(), _T("delete")) == 0) {
			if (argc != 2)
				_tprintf(_T("Syntax error; use \"/config help\" for more information.\n"));
			else {
				int i = atoi(argv[1]);
				TCHAR x[50];
				wsprintf(x, _T("DomainInfo%03d"), i);
				if (RegDeleteKey(key, x) != ERROR_SUCCESS)
					_tprintf(_T("Item %d not found.\n"), i);
			}
		}
		else if (_tcsicmp(command.c_str(), _T("clear")) == 0) {
			if (argc != 1)
				_tprintf(_T("Syntax error; use \"/config help\" for more information.\n"));
			else {
				for (int i = 0; i < 100; i++) {
					TCHAR x[50];
					wsprintf(x, _T("DomainInfo%03d"), i);
					RegDeleteKey(key, x);
				}
			}
		}
		else if (_tcsicmp(command.c_str(), _T("timeout")) == 0) {
			if (argc != 2)
				_tprintf(_T("Syntax error; use \"/config help\" for more information.\n"));
			else {
				int i = atoi(argv[1]) * 1000 * 60;
				if (SaveTimeout(key, i) != TRUE)
					_tprintf(_T("Unable to set timeout.\n"));
			}
		}
		else {
			_tprintf(_T("Subcommands:\n"));
			_tprintf(_T("\tprompt: prompt for setup with help\n"));
			_tprintf(_T("\tadd hostname tld myip mx backmx wildcard easydnsurl httpTimeoutSec username password proxy proxyPort\n"));
			_tprintf(_T("\tdelete n: delete entry number n\n"));
			_tprintf(_T("\tclear: erase the entired configuration (be careful!)\n"));
			_tprintf(_T("\ttimeout n: set the timeout to n minutes\n"));
			_tprintf(_T("\thelp: show this text\n"));
			_tprintf(_T("\tshow: display the configuration\n"));
		}
		RegCloseKey(key);
	}
	else {
		log.LogLastError("Unable to open registry, please install first");
		_tprintf(_T("Unable to open registry\n"));
		return FALSE;
	}
	return TRUE;
}

BOOL Run()
{
	EventLog log;
	HKEY key;
	try {
		int cnt = 0;
		DnsUpdater updater;
		updater.evLog = &log;
		LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\EasyDynamicDNS"), 0, KEY_READ, &key);
		if (result == ERROR_SUCCESS) {
			DWORD timeout = LoadTimeout(key);
			updater.SetTimeout(timeout);
			for (int i = 0; i < 100; i++) {
				DomainInfo info;
				if (LoadConfig(key, i, info) == TRUE) {
					updater.Add(info);
					cnt++;
				}
			}
			RegCloseKey(key);
			if (cnt == 0)
				throw _T("No configuration information found.");
			updater.Run();
		}
		else
			throw _T("Unable to read configuration");			
	}
	catch (const TCHAR* msg) {
		_tprintf(_T("%s\n"), msg);
		log.LogEvent(EVENTLOG_ERROR_TYPE, msg);
		return FALSE;
	}
	return TRUE;
}

void Usage()
{
	_tprintf(_T("EasyDynamicDNS is used to provide dynamic DNS updates for easyDNS domains.\n\n"));
	_tprintf(_T("Usage:\n"));
	_tprintf(_T("\tEasyDynamicDNS /install\n\t\tInstalls the service\n"));
	_tprintf(_T("\tEasyDynamicDNS /uninstall\n\t\tRemoves the service\n"));
	_tprintf(_T("\tEasyDynamicDNS /start\n\t\tStarts the service\n"));
	_tprintf(_T("\tEasyDynamicDNS /stop\n\t\tStops the service\n"));
	_tprintf(_T("\tEasyDynamicDNS /run\n\t\tRuns in non-service mode\n"));
	_tprintf(_T("\tEasyDynamicDNS /config\n\t\tConfigures service (requires restart)\n"));
	_tprintf(_T("\tEasyDynamicDNS /help\n\t\tShows usage\n"));
}

static SERVICE_STATUS EasyDynamicDNSServiceStatus;
static SERVICE_STATUS_HANDLE EasyDynamicDNSServiceStatusHandle;
static DnsUpdater* myupdater = 0;
static EventLog* myLog = 0;

void EasyDynamicDNSCtrlHandler(DWORD Opcode) 
{ 
	switch(Opcode) 
	{ 
		case SERVICE_CONTROL_STOP: 
			if (myupdater != 0)
				myupdater->Stop();

			EasyDynamicDNSServiceStatus.dwWin32ExitCode = 0; 
			EasyDynamicDNSServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
			EasyDynamicDNSServiceStatus.dwCheckPoint    = 0; 
			EasyDynamicDNSServiceStatus.dwWaitHint      = 0; 
 
			if (!SetServiceStatus(EasyDynamicDNSServiceStatusHandle, &EasyDynamicDNSServiceStatus))
			{
				if (myLog != 0)
					myLog->LogLastError("Cannot set service status");
            } 
            return; 

		case SERVICE_CONTROL_INTERROGATE: 
			// Fall through to send current status. 
            break; 

		default: 
			if (myLog != 0)
				myLog->LogEvent(EVENTLOG_ERROR_TYPE, _T("Unknown op code"));
			break;
	} 
 
    // Send current status. 
    if (!SetServiceStatus(EasyDynamicDNSServiceStatusHandle, &EasyDynamicDNSServiceStatus)) 
    { 
		if (myLog != 0)
			myLog->LogLastError("Cannot set service status");
    } 

	return; 
} 

void EasyDynamicDNSStart(DWORD argc, LPTSTR *argv) 
{ 
	EventLog log;
	myLog = &log;

	log.LogEvent(EVENTLOG_INFORMATION_TYPE, "Service starting...");

	EasyDynamicDNSServiceStatus.dwServiceType        = SERVICE_WIN32; 
	EasyDynamicDNSServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
	EasyDynamicDNSServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;
	EasyDynamicDNSServiceStatus.dwWin32ExitCode      = 0; 
	EasyDynamicDNSServiceStatus.dwServiceSpecificExitCode = 0; 
	EasyDynamicDNSServiceStatus.dwCheckPoint         = 0; 
	EasyDynamicDNSServiceStatus.dwWaitHint           = 0; 

	EasyDynamicDNSServiceStatusHandle = RegisterServiceCtrlHandler(_T("EasyDynamicDNS"), (LPHANDLER_FUNCTION) EasyDynamicDNSCtrlHandler); 
    if (EasyDynamicDNSServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    {
		log.LogLastError(_T("Could not register service control handler"));
		myLog = 0;
		return; 
    } 
 
    // Initialization complete - report running status. 
    EasyDynamicDNSServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    EasyDynamicDNSServiceStatus.dwCheckPoint         = 0; 
    EasyDynamicDNSServiceStatus.dwWaitHint           = 0; 
 
    if (!SetServiceStatus(EasyDynamicDNSServiceStatusHandle, &EasyDynamicDNSServiceStatus)) 
    {
		log.LogLastError(_T("Could not set service status"));
    } 
 
	try {
		// get settings
		DWORD timeout = 1000 * 60 * 60 * 1;
		HKEY key;
		DnsUpdater updater;
		int cnt = 0;
		updater.evLog = &log;
		LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\EasyDynamicDNS"), 0, KEY_READ, &key);
		if (result == ERROR_SUCCESS) {
			DWORD timeout = LoadTimeout(key);
			updater.SetTimeout(timeout);
			for (int i = 0; i < 100; i++) {
				DomainInfo info;
				if (LoadConfig(key, i, info) == TRUE) {
					updater.Add(info);
					cnt++;
				}
			}
			RegCloseKey(key);
			if (cnt == 0)
				throw _T("No configuration information found.");
		}
		else
			throw _T("Unable to read configuration");			

		log.LogEvent(EVENTLOG_INFORMATION_TYPE, "Service started");

		updater.Run();

		log.LogEvent(EVENTLOG_INFORMATION_TYPE, "Service stopped");
	}
	catch (const TCHAR* msg) {
		log.LogEvent(EVENTLOG_ERROR_TYPE, msg);
	}

	myLog = 0;
	myupdater = 0;

    return; 
} 


int _tmain(int argc, _TCHAR* argv[])
{
	EventLog::SetSourceName(_T("EasyDynamicDNS"));

	if (argc >= 2) {
		TCHAR cmd[20];
		_tcsncpy(cmd, argv[1], 19);
		cmd[19] = 0;
		_tcslwr(cmd);
		if (_tcscmp(cmd, "/install") == 0) {
			if (Install() != TRUE)
				_tprintf(_T("Installation error.\n"));
			return 0;
		}
		else if (_tcscmp(cmd, "/uninstall") == 0) {
			if (Uninstall() != TRUE)
				_tprintf(_T("Remove error.\n"));
			return 0;
		}
		else if (_tcscmp(cmd, "/start") == 0) {
			if (Start() != TRUE)
				_tprintf(_T("Start error.\n"));
			return 0;
		}
		else if (_tcscmp(cmd, "/stop") == 0) {
			if (Stop() != TRUE)
				_tprintf(_T("Stop error.\n"));
			return 0;
		}
		else if (_tcscmp(cmd, "/config") == 0) {
			if (Config(argc - 2, &argv[2]) != TRUE)
				_tprintf(_T("Config error\n"));
			return 0;
		}
		else if (_tcscmp(cmd, "/run") == 0) {
			if (Run() != TRUE)
				_tprintf(_T("Run error\n"));
			return 0;
		}
		else if (_tcscmp(cmd, "/help") == 0) {
			Usage();
			return 0;
		}
		else {
			_tprintf ("Syntax error\n");
			Usage();
			return 1;
		}
	}

	// run service

	SERVICE_TABLE_ENTRY DispatchTable[] = 
	{
		{ _T("EasyDynamicDNS"), (LPSERVICE_MAIN_FUNCTION) EasyDynamicDNSStart },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(DispatchTable)) {
		EventLog log;
		log.LogLastError("Service run error");
	}

	return 0;
}


