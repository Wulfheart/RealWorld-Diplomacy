//===============================================================================
// @ MenuUtil.h
// ------------------------------------------------------------------------------
// Interface to menu functions 
//
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
//
// Change history:
//
// 00-Jan-02	JMV	First version
//
//===============================================================================

#ifndef __AppMenuDefs__
#define __AppMenuDefs__
#include <windows.h>

//-------------------------------------------------------------------------------
//-- Constants ------------------------------------------------------------------
//-------------------------------------------------------------------------------

#define kFileMenuID		0
#define kEditMenuID		1
#define kMapMenuID		2
#define kOrdersMenuID	3
#define kHistoryMenuID	4
#define kVariantMenuID	5

#define kIgnoreUnitsItem	ID_ORDERS_OVERWRITEWHENLOADING
#define kAddUnitToOrderItem ID_ORDERS_ADDUNITTOORDER

#define kOrdersItem			ID_MAP_ORDERS
#define kUnitsItem			ID_MAP_UNITS
#define kSupplySectorsItem	ID_MAP_SUPPLYCENTERS
#define kOtherLandItem	    ID_MAP_OTHERLAND
#define kNamesItem			ID_MAP_NAMES
#define kEditMapItem		ID_MAP_EDITMAP

//-------------------------------------------------------------------------------
//-- Prototypes -----------------------------------------------------------------
//-------------------------------------------------------------------------------

void DoMenuCommand(const WPARAM wParam);
void AdjustMenus(HWND window, HMENU menu);
void AddToVariantMenu(char* string);
void CheckVariantItem(short index, bool checked);
void CheckIgnoreUnitsItem(bool checked);
void CheckAddUnitToOrderItem(bool checked);
void CheckIgnoreConvoyRoutesItem(bool checked);
void CheckEditMapItem(bool checked);
bool CheckMenuItemByIndex(int menuID, int itemID, bool value);

inline HMENU GetMenu( short resourceID )
{
	extern HINSTANCE gInstance;
	HMENU menu = LoadMenu( gInstance, MAKEINTRESOURCE(resourceID) );
	menu = GetSubMenu( menu, 0 );
	return menu;
}

inline void SetItem(HMENU menu, int item, char* string)
{
	ModifyMenu(menu, item, MF_BYCOMMAND | MF_STRING, item, string);
}

#endif