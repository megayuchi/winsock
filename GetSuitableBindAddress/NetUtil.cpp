#include "stdafx.h"



#include "NetUtil.h"

BOOL GetSuitableBindAddress(WCHAR* wchOutDescription,DWORD dwDescription_BufferCount,WCHAR* wchOutUnicastIP,DWORD dwUnicastIP_BufferCount,WCHAR* wchOutGatewayIP,DWORD dwGatewayIP_BufferCount)
{
	BOOL	bResult = FALSE;

	struct NET_ADAPTER_DESC
	{
		ULONG		Index;
		PIP_ADAPTER_ADDRESSES	pAdapter;
		BOOL		bGatewayPresent;
		SOCKADDR_IN	UnicastAddress;
		SOCKADDR_IN	GatewayAddress;
		SOCKADDR_IN	DNSServerAddress;
		int			Weight;
	};

	memset(wchOutDescription,0,sizeof(WCHAR)*dwDescription_BufferCount);
	memset(wchOutUnicastIP,0,dwUnicastIP_BufferCount);
	memset(wchOutGatewayIP,0,dwGatewayIP_BufferCount);
	
	const	DWORD	MAX_NET_ADAPTER_NUM = 16;

	NET_ADAPTER_DESC*	pAdapterList = new NET_ADAPTER_DESC[MAX_NET_ADAPTER_NUM];
	memset(pAdapterList,0,sizeof(NET_ADAPTER_DESC)*MAX_NET_ADAPTER_NUM);
	
	// Set the flags to pass to GetAdaptersAddresses
	ULONG flags = GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;// | GAA_FLAG_SKIP_DNS_SERVER;

	// default to unspecified address family (both)
	
	// process only IPV4
	ULONG	family = AF_INET;
	
	
	ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);

	// Make an initial call to GetAdaptersAddresses to get the 
	// size needed into the outBufLen variable
	if (GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		free(pAddresses);
		pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
	}

	

	DWORD dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

	WCHAR	wchTxt[256] = {0};
	if (NO_ERROR != dwRetVal) 
	{
		swprintf_s(wchTxt,L"Call to GetAdaptersAddresses failed with error: %u\n",dwRetVal);
		OutputDebugString(wchTxt);

		if (ERROR_NO_DATA == dwRetVal)
		{
			swprintf_s(wchTxt,L"\tNo addresses were found for the requested parameters\n");
			OutputDebugString(wchTxt);
		}
		else
		{
			LPVOID lpMsgBuf = NULL;
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   // Default language
				(LPTSTR) & lpMsgBuf, 0, NULL)) 
			{
				swprintf_s(wchTxt,L"\tError: %S", (char*)lpMsgBuf);
				LocalFree(lpMsgBuf);
			}
		}
		goto lb_exit;
	}
	
	
	DWORD	dwAdapterCount = 0;
	PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
	while (pCurrAddresses) 
	{
#ifdef _DEBUG
		swprintf_s(wchTxt,L"%s\n",pCurrAddresses->Description);
		OutputDebugString(wchTxt);
		swprintf_s(wchTxt,L"\tIfIndex (IPv4 interface): %u , Adapter name: %s\n", pCurrAddresses->IfIndex,pCurrAddresses->Description);
		OutputDebugString(wchTxt);
#endif
		char	szUnicastIP[32] = {0};
		char	szGatewayIP[32] = {0};
		
		PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
		if (!pUnicast)
			goto lb_next_adapter;

		if (IfOperStatusUp != pCurrAddresses->OperStatus)
			goto lb_next_adapter;

		if (0 == pCurrAddresses->PhysicalAddressLength) 
			goto lb_next_adapter;

		if (dwAdapterCount >= MAX_NET_ADAPTER_NUM)
		{
			OutputDebugString(L"dwAdapterCount >= MAX_NET_ADAPTER_NUM\n");
			__debugbreak();
		}
		NET_ADAPTER_DESC*	pDesc = pAdapterList + dwAdapterCount;
		dwAdapterCount++;

		pDesc->pAdapter = pCurrAddresses;
		pDesc->Index = pCurrAddresses->IfIndex;
		pDesc->UnicastAddress = *(SOCKADDR_IN*)pUnicast->Address.lpSockaddr;
		pDesc->Weight = 0;

		PIP_ADAPTER_GATEWAY_ADDRESS pGateway = pCurrAddresses->FirstGatewayAddress;
		if (pGateway)
		{
			pDesc->bGatewayPresent = TRUE;
			pDesc->Weight += 10;
			pDesc->GatewayAddress = *(SOCKADDR_IN*)pGateway->Address.lpSockaddr;
		}

		PIP_ADAPTER_DNS_SERVER_ADDRESS pDNSServer = pCurrAddresses->FirstDnsServerAddress;
		if (pDNSServer)
		{
			pDesc->DNSServerAddress = *(SOCKADDR_IN*)pDNSServer->Address.lpSockaddr;
			pDesc->Weight += 5;
		}
			
lb_next_adapter:
		pCurrAddresses = pCurrAddresses->Next;
	}
	
	if (!dwAdapterCount)
		goto lb_exit;


	int		MaxWeight = -1;
	NET_ADAPTER_DESC*	pSelectedDesc = NULL;
	for (DWORD i=0; i<dwAdapterCount; i++)
	{
		if (pAdapterList[i].Weight > MaxWeight)
		{
			MaxWeight = pAdapterList[i].Weight;
			pSelectedDesc = pAdapterList+i;
		}
	}
	
	SOCKADDR_IN* ipv4unicast = (SOCKADDR_IN*)&pSelectedDesc->UnicastAddress;
	InetNtopW (AF_INET,(in_addr*)&ipv4unicast->sin_addr,wchOutUnicastIP,dwUnicastIP_BufferCount);

	if (pSelectedDesc->bGatewayPresent)
	{
		SOCKADDR_IN* ipv4gateway = (SOCKADDR_IN*)&pSelectedDesc->GatewayAddress;
		InetNtopW (AF_INET,(in_addr*)&ipv4gateway->sin_addr,wchOutGatewayIP,dwGatewayIP_BufferCount);
	}
	
	
	wcscpy_s(wchOutDescription,dwDescription_BufferCount,pSelectedDesc->pAdapter->Description);
	
	bResult = TRUE;


lb_exit:
	if (pAddresses)
	{
		free(pAddresses);
		pAddresses = NULL;
	}
	if (pAdapterList)
	{
		delete [] pAdapterList;
		pAdapterList = NULL;
	}

	return bResult;
	
}