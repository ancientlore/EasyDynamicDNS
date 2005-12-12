// DnsUpdater.cpp
//

#include "stdafx.h"

using namespace std;

DnsUpdater::DnsUpdater()
{
	evLog = 0;
	waitArray[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (waitArray[0] == NULL) {
		throw _T("Could not create Event!");
	}
	waitObjects = 1;
	timeout = 1000 * 60 * 60 * 1;	// 1 hour
	if (WSAStartup(MAKEWORD(2,2), &wsadata) != 0)
		throw _T("Could not start winsock");
}

DnsUpdater::~DnsUpdater()
{
	CloseHandle(waitArray[0]);
	WSACleanup();
}

void DnsUpdater::Add(const DomainInfo& info)
{
	domainInfo.push_back(info);
}

bool DnsUpdater::RunOnce()
{
	bool rval = true;

	// get host name
	char hostname[256];
	*hostname = 0;
	const char *addr = "1.1.1.1";
	if (gethostname(hostname, 256) == 0) {
		struct hostent* host = gethostbyname(hostname);
		addr = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
	}
	// evLog->LogEvent(EVENTLOG_INFORMATION_TYPE, addr);
	
	// go through and post any files that exist currently, and set up change notification
	{
		DomainInfoVectIter iter = domainInfo.begin();
		while (iter != domainInfo.end()) {
			iter->UpdateDNS(addr, evLog);
			iter++;
		}
	}

	// wait for change notifications

	while (waitObjects > 0) {
		DWORD result = MsgWaitForMultipleObjectsEx(waitObjects, waitArray, timeout, 0, 0);
		if (result == WAIT_OBJECT_0) {
			// event was signalled, time to quit
			rval = false;
			break;	// exit loop
		}
		else if (result == WAIT_TIMEOUT) {
			DomainInfoVectIter iter = domainInfo.begin();
			while (iter != domainInfo.end()) {
				iter->UpdateDNS(addr, evLog);
				if (WaitForSingleObject(waitArray[0], 0) != WAIT_TIMEOUT)
					break;	// service stopping
				iter++;
			}
		}
		// else if (result > WAIT_OBJECT_0  &&  result < WAIT_OBJECT_0 + waitObjects) {
	}

	return rval;
}

void DnsUpdater::Run()
{
	while (RunOnce());
}

void DnsUpdater::Stop()
{
	SetEvent(waitArray[0]);
}

BOOL DomainInfo::UpdateDNS(const TCHAR* addr, EventLog* plog)
{
	CAtlHttpClient client;
	BOOL rval = FALSE;

    CAtlNavigateData navData;
	navData.SetMethod(ATL_HTTP_METHOD_GET);
	if (httpTimeout > 0)
		navData.SetSocketTimeout(httpTimeout);

	// set auth
	CBasicAuthObject auth;
	client.AddAuthObj(_T("basic"), &auth, this);

	// proxy
	if (proxy.length() > 0) {
		client.SetProxy(proxy.c_str(), (short) proxyPort);
	}

	// setup headers
	tstring headers;
	if (navData.GetExtraHeaders() != NULL)
		headers += navData.GetExtraHeaders();
	headers += _T("Connection: close");
	headers += _T("\r\n");
	navData.SetExtraHeaders(headers.c_str());

	// setup url
	TCHAR item[256];
	DWORD itemLen;
	tstring cUrl = easydnsurl;
	if (easydnsurl.length() == 0)
		cUrl = _T("http://members.easydns.com/dyn/dyndns.php");
	// hostname
	cUrl += _T("?hostname=");
	if (AtlEscapeUrl(hostname.c_str(), item, &itemLen, 256) == FALSE) {
		plog->LogEvent(EVENTLOG_ERROR_TYPE, "hostname is too long");
		return FALSE;
	}
	cUrl += item;
	// tld
	if (tld.length() > 0) {
		cUrl += _T("&tld=");
		if (AtlEscapeUrl(tld.c_str(), item, &itemLen, 256) == FALSE) {
			plog->LogEvent(EVENTLOG_ERROR_TYPE, "tld is too long");
			return FALSE;
		}
		cUrl += item;
	}
	// myip
	cUrl += _T("&myip=");
	tstring tmp = myip;
	if (tmp.length() == 0)
		tmp = addr;
	if (AtlEscapeUrl(tmp.c_str(), item, &itemLen, 256) == FALSE) {
		plog->LogEvent(EVENTLOG_ERROR_TYPE, "myip is too long");
		return FALSE;
	}
	cUrl += item;
	// mx
	if (mx.length() > 0) {
		cUrl += _T("&mx=");
		if (AtlEscapeUrl(mx.c_str(), item, &itemLen, 256) == FALSE) {
			plog->LogEvent(EVENTLOG_ERROR_TYPE, "mx is too long");
			return FALSE;
		}
		cUrl += item;
	}
	// backmx
	if (backmx.length() > 0) {
		cUrl += _T("&backmx=");
		if (AtlEscapeUrl(backmx.c_str(), item, &itemLen, 256) == FALSE) {
			plog->LogEvent(EVENTLOG_ERROR_TYPE, "backmx is too long");
			return FALSE;
		}
		cUrl += item;
	}
	// wildcard
	if (wildcard.length() > 0) {
		cUrl += _T("&wildcard=");
		if (AtlEscapeUrl(wildcard.c_str(), item, &itemLen, 256) == FALSE) {
			plog->LogEvent(EVENTLOG_ERROR_TYPE, "wildcard is too long");
			return FALSE;
		}
		cUrl += item;
	}

	// issue request
	tstring msg = _T("Result for ");
	msg += hostname;
	msg += _T(" - HTTP ");
	TCHAR s[128];
	if (client.Navigate(cUrl.c_str(), &navData) || true) {
		const BYTE* body = client.GetBody();
		wsprintf(s, _T("%d"), client.GetStatus());
		msg += s;
		msg += _T(" - ");
		WORD et = EVENTLOG_WARNING_TYPE;
		if (client.GetStatus() != 200)
			et = EVENTLOG_ERROR_TYPE;
		if (body != 0) {
			if (_tcsstr((const char*) body, "NOERROR") != 0)
				et = EVENTLOG_INFORMATION_TYPE;
			if (sizeof(TCHAR) > 1) {
				TCHAR* pData = new TCHAR[client.GetBodyLength() + 1];
				if (pData) {
					mbstowcs((wchar_t*) pData, (const char*) body, client.GetBodyLength());
					pData[client.GetBodyLength()] = 0;
					msg += pData;
					delete[] pData;
				}
			}
			else
				msg += (const char *) body;
		}
		plog->LogEvent(et, msg.c_str());
		if (client.GetStatus() == 200)
			rval = TRUE;
	}
	else {
		wsprintf(s, _T("%d"), client.GetStatus());
		msg += s;
		msg += _T(" - ");
		msg += _T("Navigate failed");
		plog->LogEvent(EVENTLOG_ERROR_TYPE, msg.c_str());
	}

	return rval;
}

HRESULT DomainInfo::GetPassword(LPTSTR szPwd, DWORD* dwBuffSize)
{
    _tcsncpy(szPwd, password.c_str(), *dwBuffSize);
	return S_OK;
}

HRESULT DomainInfo::GetUsername(LPTSTR szUid, DWORD* dwBuffSize)
{
    _tcsncpy(szUid, username.c_str(), *dwBuffSize);
	return S_OK;
}
HRESULT DomainInfo::GetDomain(LPTSTR szDomain, DWORD* dwBuffSize)
{
    _tcsncpy(szDomain, domain.c_str(), *dwBuffSize);
	return S_OK;
}
