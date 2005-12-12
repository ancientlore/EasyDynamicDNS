// EasyDynamicDNS Messages
// Copyright (C) 2005 Michael D. Lore
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: MSG_EASYDYNAMICDNS_ERROR
//
// MessageText:
//
//  Error: %1
//
#define MSG_EASYDYNAMICDNS_ERROR         ((DWORD)0xC0000003L)

//
// MessageId: MSG_EASYDYNAMICDNS_WARNING
//
// MessageText:
//
//  Warning: %1
//
#define MSG_EASYDYNAMICDNS_WARNING       ((DWORD)0x80000002L)

//
// MessageId: MSG_EASYDYNAMICDNS_INFO
//
// MessageText:
//
//  Info: %1
//
#define MSG_EASYDYNAMICDNS_INFO          ((DWORD)0x40000001L)

//
// MessageId: MSG_EASYDYNAMICDNS_SUCCESS
//
// MessageText:
//
//  Success: %1
//
#define MSG_EASYDYNAMICDNS_SUCCESS       ((DWORD)0x00000000L)

//
// MessageId: MSG_EASYDYNAMICDNS_ERROR_INFO
//
// MessageText:
//
//  Error: %1
//  %2
//
#define MSG_EASYDYNAMICDNS_ERROR_INFO    ((DWORD)0xC0000004L)

