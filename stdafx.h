// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <atlbase.h>

// TODO: reference additional headers your program requires here
#include <winsock2.h>
#include <atlhttp.h>
#include <atlmime.h>

typedef std::basic_string<TCHAR> tstring;
#include <vector>

#include "eventlog.h"
#include "DnsUpdater.h"

