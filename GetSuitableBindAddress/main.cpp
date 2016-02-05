// GetSuitableBindAddress.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NetUtil.h"
#include <Windows.h>
#include <conio.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"IPHLPAPI.lib")

int main()
{
	WCHAR SelectedDesc[128] = {};
	WCHAR SelectedIP[32] = {};
	WCHAR GateWayIP[32] = {};

	if (GetSuitableBindAddress(SelectedDesc,_countof(SelectedDesc),SelectedIP,_countof(SelectedIP),GateWayIP,_countof(GateWayIP)))
	{
		wprintf_s(L"Selected IP : %s, %s , Gateway : %s\n",SelectedIP,SelectedDesc,GateWayIP);
	}
	else
	{
		wprintf_s(L"Filaed to GetSuitableBindAddress()\n");
	}
	_getch();
    return 0;
}

