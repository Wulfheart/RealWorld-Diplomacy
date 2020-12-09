//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// Modifications (C) 2005 by Dirk Brüggemann
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Clarified Artistic License.
// In addition, to meet copyright restrictions you must also own at 
// least one copy of any of the following board games: 
// Diplomacy, Deluxe Diplomacy, Colonial Diplomacy or Machiavelli.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// Clarified Artistic License for more details.
//
// You should have received a copy of the Clarified Artistic License
// along with this program; if not, write to the Open Source Initiative
// at www.opensource.org
//---------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
// Preferences.cpp
//------------------------------------------------------------------------------------------
// Load/Save Preferences
//------------------------------------------------------------------------------------------

#include "Preferences.h"
#include "Global.h"
#include "MapDisplay.h"
#include "InfoDisplay.h"
#include "OrdersDisplay.h"
#include "StatusDisplay.h"
#include "WindowUtil.h"

bool Preferences::mLoaded = false;

Rect Preferences::mMapWindowRect = {0,0,0,0};
Rect Preferences::mOrdersWindowRect = {0,0,0,0};
Rect Preferences::mErrorsWindowRect = {0,0,0,0};
Rect Preferences::mStatusWindowRect = {0,0,0,0};
Rect Preferences::mInfoWindowRect = {0,0,0,0};
	
bool Preferences::mShowSupplies = true;
bool Preferences::mShowUnits = true;
bool Preferences::mShowOrders = true;
bool Preferences::mShowNames = false;
bool Preferences::mShowOtherLand = false;
bool Preferences::mAddUnitToOrder = false;
bool Preferences::mIgnoreConvoyRoutes = true;

char Preferences::mLastVariant[128] = "Standard";

// value names
const char* kMapRect			= "MapRect";
const char* kOrdersRect			= "OrdersRect";
const char* kErrorsRect			= "ErrorsMapRect";
const char* kStatusRect			= "StatusRect";
const char* kInfoRect			= "InfoRect";

const char* kShowSupplies		= "ShowSupplies";
const char* kShowUnits			= "ShowUnits";
const char* kShowOrders			= "ShowOrders";
const char* kShowNames			= "ShowNames";
const char* kShowOtherLand		= "ShowOtherLand";
const char* kAddUnitToOrder		= "AddUnitToOrder";
const char* kIgnoreConvoyRoutes = "IgnoreConvoyRoutes";

const char* kLastVariant	= "LastVariant";

Preferences prefs;

Preferences::Preferences()
{
}

Preferences::~Preferences()
{
}

bool Preferences::Load()
{
	HKEY			key;					// Handle of the key we're inspecting
	HRESULT			result;
	DWORD			type;
	DWORD			size;
	DWORD			intValue;
	// Open the key
	result = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
							   "Software\\RealWorld",
							   0,
							   KEY_QUERY_VALUE,
							   &key );
	if ( ERROR_SUCCESS != result )
	{
		return false;
	}

	// read in the preferences
	size = sizeof(Rect);
	result = RegQueryValueEx(key, kMapRect, 0, &type, (unsigned char*)&mMapWindowRect, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	size = sizeof(Rect);
	result = RegQueryValueEx(key, kOrdersRect, 0, &type, (unsigned char*)&mOrdersWindowRect, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	size = sizeof(Rect);
	result = RegQueryValueEx(key, kErrorsRect, 0, &type, (unsigned char*)&mErrorsWindowRect, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	size = sizeof(Rect);
	result = RegQueryValueEx(key, kStatusRect, 0, &type, (unsigned char*)&mStatusWindowRect, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	size = sizeof(Rect);
	result = RegQueryValueEx(key, kInfoRect, 0, &type, (unsigned char*)&mInfoWindowRect, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kShowSupplies, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mShowSupplies = (intValue != 0);

	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kShowUnits, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mShowUnits = (intValue != 0);

	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kShowOrders, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mShowOrders = (intValue != 0);

	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kShowNames, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mShowNames = (intValue != 0);

	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kShowOtherLand, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mShowOtherLand = (intValue != 0);

	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kAddUnitToOrder, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mAddUnitToOrder = (intValue != 0);
	
	size = sizeof(DWORD);
	result = RegQueryValueEx(key, kIgnoreConvoyRoutes, 0, &type, (unsigned char*)&intValue, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}
	mIgnoreConvoyRoutes = (intValue != 0);

	size = sizeof(mLastVariant);
	result = RegQueryValueEx(key, kLastVariant, 0, &type, (unsigned char*)mLastVariant, &size);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	

	// Clean up memory
	(void) ::RegCloseKey( key );
		
	return true;
}

bool Preferences::Save()
{
	// find the registry key, if not there, create it
	HKEY			key;					// Handle of the key we're inspecting
	unsigned long	messageLength = 256;
	DWORD			disposition;
	HRESULT			result;
	DWORD			intValue;
	// Open the key (create it if it doesn't already exist!)
	result = ::RegCreateKeyEx( HKEY_LOCAL_MACHINE,
							   "Software\\RealWorld",
							   0,
							   (char*)kAppName,
							   REG_OPTION_NON_VOLATILE,
							   KEY_WRITE,
							   NULL,
							   &key,
							   &disposition );
	if ( ERROR_SUCCESS != result )
	{
		return false;
	}

	// get values
	GetStatusRect(mStatusWindowRect);
	GetInfoRect(mInfoWindowRect);
	
	// write out the preferences
	if (mapWindow)
	{	
 	    mapWindow->GetWindowRect(mMapWindowRect);
		result = RegSetValueEx(key, kMapRect, 0, REG_BINARY, (unsigned char*)&mMapWindowRect, sizeof(Rect));
		if (result != ERROR_SUCCESS)
		{
			(void) ::RegCloseKey( key );
			return false;
		}
	}
	if (ordersWindow) 
	{	
 	    ordersWindow->GetRect(&mOrdersWindowRect);
		result = RegSetValueEx(key, kOrdersRect, 0, REG_BINARY, (unsigned char*)&mOrdersWindowRect, sizeof(Rect));
		if (result != ERROR_SUCCESS)
		{
			(void) ::RegCloseKey( key );
			return false;
		}
	}

	if (errorWindow) 
	{	
 	    errorWindow->GetRect(&mErrorsWindowRect);
		result = RegSetValueEx(key, kErrorsRect, 0, REG_BINARY, (unsigned char*)&mErrorsWindowRect, sizeof(Rect));
		if (result != ERROR_SUCCESS)
		{
			(void) ::RegCloseKey( key );
			return false;
		}
	}

	result = RegSetValueEx(key, kStatusRect, 0, REG_BINARY, (unsigned char*)&mStatusWindowRect, sizeof(Rect));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	result = RegSetValueEx(key, kInfoRect, 0, REG_BINARY, (unsigned char*)&mInfoWindowRect, sizeof(Rect));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mShowSupplies?1:0;
	result = RegSetValueEx(key, kShowSupplies, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mShowUnits?1:0;
	result = RegSetValueEx(key, kShowUnits, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mShowOrders?1:0;
	result = RegSetValueEx(key, kShowOrders, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mShowNames?1:0;
	result = RegSetValueEx(key, kShowNames, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mShowOtherLand?1:0;
	result = RegSetValueEx(key, kShowOtherLand, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mAddUnitToOrder?1:0;
	result = RegSetValueEx(key, kAddUnitToOrder, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	intValue = mIgnoreConvoyRoutes?1:0;
	result = RegSetValueEx(key, kIgnoreConvoyRoutes, 0, REG_DWORD, (unsigned char*)&intValue, sizeof(intValue));
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	result = RegSetValueEx(key, kLastVariant, 0, REG_SZ, (unsigned char*)mLastVariant, strlen(mLastVariant)+1);
	if (result != ERROR_SUCCESS)
	{
		(void) ::RegCloseKey( key );
		return false;
	}

	// Clean up memory
	(void) ::RegCloseKey( key );
		
	return true;
}

