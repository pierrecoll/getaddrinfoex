// getaddrinfoex.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifndef UNICODE
#define UNICODE
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <objbase.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

// Need to link with Ole32.lib to print GUID
#pragma comment(lib, "ole32.lib")

int __cdecl wmain(int argc, wchar_t ** argv)
{

	//-----------------------------------------
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;

	DWORD dwRetval;

	int i = 1;

	DWORD dwNamespace = NS_ALL;
	LPGUID lpNspid = NULL;

	ADDRINFOEX *result = NULL;
	ADDRINFOEX *ptr = NULL;
	ADDRINFOEX hints;

	// LPSOCKADDR sockaddr_ip;
	struct sockaddr_in *sockaddr_ipv4;
	struct sockaddr_in6 *sockaddr_ipv6;

	// DWORD ipbufferlength = 46;
	wchar_t ipstringbuffer[46];

	// variables needed to print namespace provider GUID
	int iRet = 0;
	WCHAR GuidString[40] = { 0 };

	// Validate the parameters
	if (argc != 4) 
	{
		wprintf(L"Version 1.01 26/09/2017 pierrelc@microsoft.com\n");
		wprintf(L"usage: getaddrinfoex <hostname> <servicename> <namespace>\n");
		wprintf(L"getaddrinfoex provides protocol-independent translation\n");
		wprintf(L"   from a host name to an IP address\n");
		wprintf(L"getaddrinfoex example usage\n");
		wprintf(L"   getaddrinfoex www.contoso.com http 12\n");
		wprintf(L"   looks up the www.contoso.com in the NS_DNS namespace\n");
		wprintf(L"Open  \\%%WINDIR%%\\system32\\drivers\\etc\\services for servicename values\n");
		wprintf(L"See https://msdn.microsoft.com/en-us/library/ms738518(v=vs.85).aspx for namespace values\n");		

		wprintf(L"\tNS_ALL   \t 0\t All installed and active namespaces.\n\
	NS_DNS     \t 12\t The domain name system(DNS) namespace.\n\
	NS_NETBT   \t 13\t The NetBIOS over TCP / IP(NETBT) namespace.\n\
	NS_WINS    \t 14\t The Windows Internet Naming Service(NS_WINS) namespace.\n\
	NS_NLA     \t 15\t The network location awareness(NLA) namespace (XP and later).\n\
	NS_BTH     \t 16\t The Bluetooth namespace (Vista and later).\n\
	NS_NTDS    \t 32\t The Windows NT Directory Services(NS_NTDS) namespace.\n\
	NS_EMAIL   \t 37\t The email namespace (Vista and later).\n\
	NS_PNRPNAME\t 38\t The peer - to - peer namespace for a specific peer name (Vista and later).\n\
	NS_PNRPCLOUD\t 39\t The peer - to - peer namespace for a collection of peer names (Vista and later).\n");

		return 1;
	}
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		wprintf(L"WSAStartup failed: %d\n", iResult);
		return 1;
	}
	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	dwNamespace = (DWORD)_wtoi(argv[3]);

	wprintf(L"Calling GetAddrInfoEx with following parameters:\n");
	wprintf(L"\tName = %ws\n", argv[1]);
	wprintf(L"\tServiceName (or port) = %ws\n", argv[2]);
	wprintf(L"\tNamespace = %s (", argv[3]);
	switch (dwNamespace) {
	case NS_ALL:
		wprintf(L"(NS_ALL)\n");
		break;
	case NS_DNS:
		wprintf(L"(NS_DNS)\n");
		break;
	case NS_NETBT:
		wprintf(L"NS_NETBT");
		break;
	case NS_WINS:
		wprintf(L"NS_WINS");
		break;
	case NS_NLA:
		wprintf(L"NS_NLA");
		break;
	case NS_BTH:
		wprintf(L"NS_BTH");
		break;
	case NS_NTDS:
		wprintf(L"NS_NTDS");
		break;
	case NS_EMAIL:
		wprintf(L"NS_EMAIL");
		break;
	case NS_PNRPNAME:
		wprintf(L"NS_PNRPNAME");
		break;
	case NS_PNRPCLOUD:
		wprintf(L"NS_PNRPCLOUD");
		break;
	default:
		wprintf(L"Other");
		break;
	}
	wprintf(L")\n\n");

	//--------------------------------
	// Call getaddrinfoex(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval =
		GetAddrInfoEx(argv[1], argv[2], dwNamespace, lpNspid, &hints, &result,
			NULL, NULL, NULL, NULL);
	if (dwRetval != 0) {
		wprintf(L"GetAddrInfoEx failed with error: %d\n", dwRetval);
		WSACleanup();
		return 1;
	}

	wprintf(L"GetAddrInfoEx returned success\n");

	// Retrieve each address and print out the hex bytes
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		wprintf(L"GetAddrInfoEx response %d\n", i++);
		wprintf(L"\tFlags: 0x%x\n", ptr->ai_flags);
		wprintf(L"\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			wprintf(L"Unspecified\n");
			break;
		case AF_INET:
			wprintf(L"AF_INET (IPv4)\n");
			// the InetNtop function is available on Windows Vista and later
			sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			wprintf(L"\tIPv4 address %ws\n",
				InetNtop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer,
					46));

			// We could also use the WSAAddressToString function
			// sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			// ipbufferlength = 46;
			// iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
			//    ipstringbuffer, &ipbufferlength );
			// if (iRetval)
			//    wprintf(L"WSAAddressToString failed with %u\n", WSAGetLastError() );
			// else    
			//    wprintf(L"\tIPv4 address %ws\n", ipstringbuffer);
			break;
		case AF_INET6:
			wprintf(L"AF_INET6 (IPv6)\n");
			// the InetNtop function is available on Windows Vista and later
			sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			wprintf(L"\tIPv6 address %ws\n",
				InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr,
					ipstringbuffer, 46));

			// We could also use WSAAddressToString which also returns the scope ID
			// sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			// ipbufferlength = 46;
			//iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
			//    ipstringbuffer, &ipbufferlength );
			//if (iRetval)
			//    wprintf(L"WSAAddressToString failed with %u\n", WSAGetLastError() );
			//else    
			//    wprintf(L"\tIPv6 address %ws\n", ipstringbuffer);
			break;
		default:
			wprintf(L"Other %ld\n", ptr->ai_family);
			break;
		}
		wprintf(L"\tSocket type: ");
		switch (ptr->ai_socktype) {
		case 0:
			wprintf(L"Unspecified\n");
			break;
		case SOCK_STREAM:
			wprintf(L"SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			wprintf(L"SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			wprintf(L"SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			wprintf(L"SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			wprintf(L"SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			wprintf(L"Other %ld\n", ptr->ai_socktype);
			break;
		}
		wprintf(L"\tProtocol: ");
		switch (ptr->ai_protocol) {
		case 0:
			wprintf(L"Unspecified\n");
			break;
		case IPPROTO_TCP:
			wprintf(L"IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			wprintf(L"IPPROTO_UDP (UDP) \n");
			break;
		default:
			wprintf(L"Other %ld\n", ptr->ai_protocol);
			break;
		}
		wprintf(L"\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
		wprintf(L"\tCanonical name: %s\n", ptr->ai_canonname);

		if (ptr->ai_blob == NULL)
			wprintf(L"\tBlob: (null)\n");
		else
			wprintf(L"\tLength of the blob: %u\n",
			(DWORD)ptr->ai_bloblen);

		if (ptr->ai_provider == NULL)
			wprintf(L"\tNamespace provider GUID: (null)\n");
		else {
			iRet =
				StringFromGUID2(*(ptr->ai_provider), (LPOLESTR)& GuidString,
					39);
			// For c rather than C++ source code, the above line needs to be
			// iRet = StringFromGUID2(&ptr.ai_provider, (LPOLESTR) &GuidString, 39); 
			if (iRet == 0)
				wprintf(L"StringFromGUID2 failed\n");
			else {
				wprintf(L"\tNamespace provider: %ws\n", GuidString);
			}
		}
	}

	FreeAddrInfoEx(result);
	WSACleanup();

	return 0;
}

