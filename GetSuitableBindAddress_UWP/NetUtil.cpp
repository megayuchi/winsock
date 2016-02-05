#include "pch.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include "NetUtil.h"

using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;



BOOL GetSuitableAddress(WCHAR* wchOutIP,DWORD dwMaxBufferLen)
{
	BOOL	bResult = FALSE;
	IVectorView<HostName^>^ hostList = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();

	ConnectionProfile^ profile = NetworkInformation::GetInternetConnectionProfile();
	Guid profile_adpater_id = profile->NetworkAdapter->NetworkAdapterId;

	DWORD	dwTotalHostCount = hostList->Size;
	DWORD	dwRealHostCount = 0;

	for each (HostName^ host in hostList)
	{
		const WCHAR* wchDisplayName = host->DisplayName->Data();
		const WCHAR* wchRawName = host->RawName->Data();
		const WCHAR* wchCanonicalName = host->CanonicalName->Data();

		IPInformation^ info = host->IPInformation;
		if (!info)
			continue;

		NetworkAdapter^ adapter = info->NetworkAdapter;
		unsigned int ianaType = adapter->IanaInterfaceType;
		
		switch (ianaType)
		{
		case 6:		// Ethernet network interface => 6
		case 71:	// IEEE 802.11 wireless network interface. => 71
			break;
		default:
			continue;	// goto next;
		}

		
		dwRealHostCount++;

		Guid adapter_id = adapter->NetworkAdapterId;

		//NetworkTypes networkType = adapter->NetworkItem->GetNetworkTypes();
		if (profile_adpater_id == adapter_id)
		{
			wcscpy_s(wchOutIP,dwMaxBufferLen,wchCanonicalName);
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}