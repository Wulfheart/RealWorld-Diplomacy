//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
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
#include <string.h>
#include "EditMenu.h"
#include "MenuUtil.h"
#include "MapData.h"
#include "Resource.h"
#include "Strings.h"
#include "MapDisplay.h"
#include "Util.h"
#include "Variants.h"

EditMenu::EditMenu(short part)
{
	char string[256];
	short i;
	HMENU menuHandle, mh2, countryMenu1, countryMenu2, countryMenu3, countryMenu4, countryMenu5, countryMenu6, countryMenu7;

	if (isbicoastal(map+part))
	{
		if (hasthirdcoast(map+part))
		{
			mh2 = GetMenu( kEditUnitMenu3ID );
		}
		else
		{
			mh2 = GetMenu( kEditUnitMenu2ID );
			GetIndCString(string, kSpecialStringsID, kFleetID);
			strcat(string, scoast[map[part].coast2]);
			SetItem(mh2, kFleet2Item, string);
		}
	}
	else 
	{
		mh2 = GetMenu(kEditUnitMenu1ID);
	}
	if (!island(map+part))
		EnableMenuItem(mh2, kArmyItem, MF_BYCOMMAND|MF_GRAYED);
	if (!issea(map+part))
		EnableMenuItem(mh2, kFleetItem, MF_BYCOMMAND|MF_GRAYED);
	if (!gHasWings)
		RemoveMenu(mh2, kWingItem, MF_BYCOMMAND);

	countryMenu1 = CreateMenu();
	countryMenu2 = CreateMenu();
	countryMenu3 = CreateMenu();
	countryMenu4 = CreateMenu();
	countryMenu5 = CreateMenu();
	if (issupp(map+part))
	{
		countryMenu6 = CreateMenu();
		(void) AppendMenu(countryMenu6, MF_STRING, kCountryMenu6Item, "None");
	}
	else if (island(map+part))
	{
		countryMenu7 = CreateMenu();
		(void) AppendMenu(countryMenu7, MF_STRING, kCountryMenu7Item, "None");
	}
	for (i = 1; i <= NUM_COUN; i++)
	{
		(void) AppendMenu(countryMenu1, MF_STRING, kCountryMenu1Item+i, countries[i].name);
		(void) AppendMenu(countryMenu2, MF_STRING, kCountryMenu2Item+i, countries[i].name);
		(void) AppendMenu(countryMenu3, MF_STRING, kCountryMenu3Item+i, countries[i].name);
		(void) AppendMenu(countryMenu4, MF_STRING, kCountryMenu4Item+i, countries[i].name);
		if (gHasWings)
			(void) AppendMenu(countryMenu5, MF_STRING, kCountryMenu5Item+i, countries[i].name);
		if (issupp(map+part))
		{
			(void) AppendMenu(countryMenu6, MF_STRING, kCountryMenu6Item+i+1, countries[i].name);
		}
		else if (island(map+part))
		{
			(void) AppendMenu(countryMenu7, MF_STRING, kCountryMenu7Item+i+1, countries[i].name);
		}
	}
	if (island(map+part))
		ModifyMenu( mh2, kArmyItem, MF_POPUP, (UINT)countryMenu1, "Army" );
	if (isbicoastal(map+part))
	{
		GetIndCString(string, kSpecialStringsID, kFleetID);
		strcat(string, scoast[map[part].coast1]);
		ModifyMenu(mh2, kFleetItem, MF_POPUP, (UINT)countryMenu2, string );
		GetIndCString(string, kSpecialStringsID, kFleetID);
		strcat(string, scoast[map[part].coast2]);
		ModifyMenu(mh2, kFleet2Item, MF_POPUP, (UINT)countryMenu3, string );
		if (hasthirdcoast(map+part))
		{
			GetIndCString(string, kSpecialStringsID, kFleetID);
			strcat(string, scoast[map[part].coast3]);
			ModifyMenu(mh2, kFleet3Item, MF_POPUP, (UINT)countryMenu4, string );
		}
	}
	else if (issea(map+part))
	{
		ModifyMenu(mh2, kFleetItem, MF_POPUP, (UINT)countryMenu2, "Fleet" );
	}
	if (gHasWings) 
		ModifyMenu(mh2, kWingItem, MF_POPUP, (UINT)countryMenu5, "Wing" );

	if (issupp(map+part))
	{
		menuHandle = GetMenu(kEditMapMenuID);
		ModifyMenu( menuHandle, kEditUnitItem, MF_POPUP, (UINT)mh2, "Unit" );
		ModifyMenu( menuHandle, kEditSupplyItem, MF_POPUP, (UINT)countryMenu6, "Supply" );
		mMenu = menuHandle;
	}
	else if (island(map+part))
	{
		menuHandle = GetMenu(kEditMapMenuID);
		ModifyMenu( menuHandle, kEditUnitItem, MF_POPUP, (UINT)mh2, "Unit" );
		ModifyMenu( menuHandle, kEditSupplyItem, MF_POPUP, (UINT)countryMenu7, "Owner" );
		mMenu = menuHandle;
	}
	else
	{
		mMenu = mh2;
	}


}

EditMenu::~EditMenu()
{
	#pragma message("memory leak?")
/*
	DeleteMenu(kCountryMenu1ID);
	DisposeMenu(countryMenu1);
	DeleteMenu(kCountryMenu2ID);
	DisposeMenu(countryMenu2);
	
	if (isbicoastal(map+part))
	{
		DeleteMenu(kEditUnitMenu2ID);
		DeleteMenu(kCountryMenu3ID);
		DisposeMenu(countryMenu3);
	}
	else 
		DeleteMenu(kEditUnitMenu1ID);
*/
	DestroyMenu(mMenu);
/*	
	if (issupp(map+part))
	{
		DeleteMenu(kCountryMenu4ID);
		DisposeMenu(countryMenu4);
		DeleteMenu(kEditMapMenuID);
		ReleaseResource((Handle)menuHandle);
	}
*/
}


long EditMenu::PopUpMenuSelect(short x, short y)
{
	int result = TrackPopupMenu(mMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NONOTIFY, x, y, 0, mapWindow->GetNative(), NULL);

	return result;
}

void EditMenu::HandleMapEdit(short sector, long result)
{
	short menuItem;

	// none item for unit
	if (result == kNoneItem)
	{
		deleteallunit(units, sector);
		deleteallorder(orders, sector);
	}
	// army list
	else if (result >= kCountryMenu1Item && result < kCountryMenu2Item)
	{
		menuItem = (short) result-kCountryMenu1Item;
		deleteallunit(units, sector);
		deleteallorder(orders, sector);
		addunit(&units[menuItem], U_ARM, sector, C_NONE, menuItem);
		appendnmr(menuItem, U_ARM, sector, C_NONE);		
	}
	// first fleet list
	else if (result >= kCountryMenu2Item && result < kCountryMenu3Item)
	{
		menuItem = result-kCountryMenu2Item;
		deleteallunit(units, sector);
		deleteallorder(orders, sector);
		addunit(&units[menuItem], U_FLT, sector,map[sector].coast1, menuItem);
		appendnmr(menuItem, U_FLT, sector, map[sector].coast1);		
	}
	// second fleet list
	else if (result >= kCountryMenu3Item && result < kCountryMenu4Item)
	{
		menuItem = result-kCountryMenu3Item;
		if (isbicoastal(map+sector))
		{
			deleteallunit(units, sector);
			deleteallorder(orders, sector);
			addunit(&units[menuItem], U_FLT, sector, map[sector].coast2, menuItem);
			appendnmr(menuItem, U_FLT, sector, map[sector].coast2);		
		}
	}
	// third fleet list
	else if (result >= kCountryMenu4Item && result < kCountryMenu5Item)
	{
		menuItem = result-kCountryMenu4Item;
		if (hasthirdcoast(map+sector))
		{
			deleteallunit(units, sector);
			deleteallorder(orders, sector);
			addunit(&units[menuItem], U_FLT, sector, map[sector].coast3, menuItem);
			appendnmr(menuItem, U_FLT, sector, map[sector].coast3);		
		}
	}
	// wing list
	else if (result >= kCountryMenu5Item && result < kCountryMenu6Item)
	{
		menuItem = result-kCountryMenu5Item;
		deleteallunit(units, sector);
		deleteallorder(orders, sector);
		addunit(&units[menuItem], U_WNG, sector, C_NONE, menuItem);
		appendnmr(menuItem, U_WNG, sector, C_NONE);		
	}
	// supply list
	else if (result >= kCountryMenu6Item && result < kCountryMenu7Item)
	{
		menuItem = result-kCountryMenu6Item;
		deleteallsupply(supplies, sector);
		// set supply for this sector to this country
		if (result != kCountryMenu6Item)
			addsupply(&supplies[menuItem-1], sector, menuItem-1);
		else
			addsupply(&supplies[0], sector, 0);
	}
	// non-supply land list
	else if (result >= kCountryMenu7Item)
	{
		menuItem = result-kCountryMenu7Item;
		deleteallsupply(lands, sector);
		// set ownership for this sector to this country
		if (result != kCountryMenu7Item)
			addsupply(&lands[menuItem-1], sector, menuItem-1);
		else
			addsupply(&lands[0], sector, 0);
	}
}
