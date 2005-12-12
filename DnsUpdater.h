// DnsUpdater.h
//
#pragma once

class DomainInfo : public IAuthInfo {
public:
	// for query string
	tstring hostname;
	tstring tld;
	tstring myip;
	tstring mx;
	tstring backmx;
	tstring wildcard;
	// provider info
	tstring easydnsurl;
	DWORD httpTimeout;
	// auth
	tstring username;
	tstring password;
	tstring domain;
	// proxy
	tstring proxy;
	DWORD proxyPort;

	DomainInfo()
	{
		easydnsurl = _T("http://members.easydns.com/dyn/dyndns.php");
		domain = _T("");
		myip = _T("1.1.1.1");
		httpTimeout = 0;
		proxyPort = 0;
	}

	DomainInfo(const DomainInfo& info)
	{
		// query string
		hostname = info.hostname;
		tld = info.tld;
		myip = info.myip;
		mx = info.mx;
		backmx = info.backmx;
		wildcard = info.wildcard;
		// provider info
		easydnsurl = info.easydnsurl;
		httpTimeout = info.httpTimeout;
		// auth
		username = info.username;
		password = info.password;
		domain = info.domain;
		// proxy
		proxy = info.proxy;
		proxyPort = info.proxyPort;
	}

	DomainInfo& operator = (const DomainInfo& info)
	{
		// query string
		hostname = info.hostname;
		tld = info.tld;
		myip = info.myip;
		mx = info.mx;
		backmx = info.backmx;
		wildcard = info.wildcard;
		// provider info
		easydnsurl = info.easydnsurl;
		httpTimeout = info.httpTimeout;
		// auth
		username = info.username;
		password = info.password;
		domain = info.domain;
		// proxy
		proxy = info.proxy;
		proxyPort = info.proxyPort;
		return *this;
	}

	BOOL UpdateDNS(const TCHAR* addr, EventLog* plog);

    HRESULT GetPassword(LPTSTR szPwd, DWORD* dwBuffSize);
    HRESULT GetUsername(LPTSTR szUid, DWORD* dwBuffSize);
    HRESULT GetDomain(LPTSTR szDomain, DWORD* dwBuffSize);
};

typedef std::vector<DomainInfo> DomainInfoVect;
typedef std::vector<DomainInfo>::iterator DomainInfoVectIter;
const int FP_MAXWAIT = MAXIMUM_WAIT_OBJECTS - 1;

class DnsUpdater {
private:
	DomainInfoVect domainInfo;

	// wait array
	HANDLE waitArray[1];
	DWORD waitObjects;
	DWORD timeout;

	WSADATA wsadata;

public:
	EventLog* evLog;

	void CloseHandles(bool includeEvent = false);

	DnsUpdater();
	~DnsUpdater();

	// config
	void Add(const DomainInfo& info);
	void SetTimeout(DWORD timeoutMs) { timeout = timeoutMs; }

	// run
	bool RunOnce();
	void Run();
	void Stop();
};

