﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include "NetUtil.h"

using namespace GetSuitableBindAddress_UWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}


void GetSuitableBindAddress_UWP::MainPage::R​etrieve_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	WCHAR	wchSelectedIP[32] = {};
	
	if (GetSuitableAddress(wchSelectedIP,_countof(wchSelectedIP)))
	{
		SelectedIP->Text = ref new Platform::String(wchSelectedIP);
	}
	else
	{
		SelectedIP->Text = L"Failed to GetSuitableAddress()";
	}

}
