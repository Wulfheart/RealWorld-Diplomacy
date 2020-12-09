//===============================================================================
// @ MenuUtil.cpp
// ------------------------------------------------------------------------------
// Menu functions.   
//
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
//
// Change history:
//
// 00-Jan-02	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <afxres.h>
#include "MenuUtil.h"
#include "Game.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "OrdersDisplay.h"
#include "Resource.h"
#include "Variants.h"
#include "Util.h"
#include "MapContent.h"
#include "LWindows.h"
#include "ReadOrders.h"
#include "InfoDisplay.h"
#include "StatusDisplay.h"
#include "EditMap.h"
#include "Global.h"
#include "Strings.h"
#include "Print.h"
#include "History.h"
#include "WindowUtil.h"
#include "Preferences.h"

void SetItem(HMENU menu, int item, char* string);

void DoMenuCommand( const WPARAM wParam )
{
	char string[256];
	int id = LOWORD(wParam);
	switch (id) 
	{
	case ID_FILE_NEW:
		if (!NewGame() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true; 
		}
		break;
		
	case ID_FILE_OPEN:
		if (!OpenGame() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			if (gErrNo != errBadGameData)
				gDone = true; 
			if (!NewGame() && gErrNo != errNoError)
				gDone = true;
		}
		break;

	case ID_FILE_CLOSE:
//		DoCloseWindow(FrontWindow());
		break;
		
	case ID_FILE_SAVE:
		if (!SaveGame() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true;
		}
		break;
		
	case ID_FILE_SAVEAS:
		if (!SaveGameAs() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true; 
		} 
		break;

	case ID_FILE_SAVEASRP:
		if (!SaveGameAsRP() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true; 
		} 
		break;

	case ID_FILE_SAVEBMP:
		SaveMapDisplay();
		break;

	case ID_FILE_SAVEREPORT:
		SaveReport();
		break;
		
	case ID_FILE_REVERT:
		if (!RevertGame() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true; 
		}
		break;
		
	case ID_FILE_PAGESETUP:
//		PageSetup();
		break;
		
	case ID_FILE_PRINT:
		if (!PrintLoop() && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
		//	gDone = true; 
		}					
		break;

	case ID_FILE_QUIT:
		gDone = QuitGame();
		break;

			
	case ID_EDIT_UNDO:
		if (gCommit)
			UndoResolve();
		else if (LastOrderSet())
			UndoLastOrder();
		break;
					
	case ID_EDIT_COPY:
		if (ordersWindow->IsSelection())
			ordersWindow->CopySelection();
		else if (IsStatusSelection())
			CopyStatusSelection();
		else if (IsInfoSelection())
			CopyInfoSelection();
		else 
			CopyMapToScrap();
		break;
		
	case ID_EDIT_PASTE:
		PasteOrders();
		break;
		
	case ID_EDIT_CLEAR:
		ClearSelectedUnit();
		ClearOrderSelections();
		ClearStatusSelections();
		ClearInfoSelection();
		break;
							
	case ID_MAP_SUPPLYCENTERS:
		prefs.mShowSupplies = !prefs.mShowSupplies;
		if (!CheckMenuItemByIndex(kMapMenuID, kSupplySectorsItem, prefs.mShowSupplies))
			return;
		DrawNewMap();
		UpdateMapDisplay(NULL);
		break;
	
	case ID_MAP_OTHERLAND:
		prefs.mShowOtherLand = !prefs.mShowOtherLand;
		if (!CheckMenuItemByIndex(kMapMenuID, kOtherLandItem, prefs.mShowOtherLand))
			return;
		DrawNewMap();
		UpdateMapDisplay(NULL);
		break;

	case ID_MAP_UNITS:
		prefs.mShowUnits = !prefs.mShowUnits;
		if (!CheckMenuItemByIndex(kMapMenuID, kUnitsItem, prefs.mShowUnits))
			return;
		DrawNewMap();
		UpdateMapDisplay(NULL);
		break;
	
	case ID_MAP_ORDERS:
		prefs.mShowOrders = !prefs.mShowOrders;
		if (!CheckMenuItemByIndex(kMapMenuID, kOrdersItem, prefs.mShowOrders))
			return;
		DrawNewMap();
		UpdateMapDisplay(NULL);
		break;
		
	case ID_MAP_NAMES:
		prefs.mShowNames = !prefs.mShowNames;
		if (!CheckMenuItemByIndex(kMapMenuID, kNamesItem, prefs.mShowNames))
			return;
		DrawNewMap();
		UpdateMapDisplay(NULL);
		break;
		
	case ID_MAP_EDITMAP:
		ToggleEditMode();
		if (!CheckMenuItemByIndex(kMapMenuID, kEditMapItem, EditModeOn()))
			return;
		break;
		
	case ID_MAP_EDITSEASON:
		EditSeason();
		break;

	case ID_MAP_EDITRULES:
		EditRules();
		if (IsGameChanged())
		{
			DrawNewMap();
			UpdateMapDisplay(NULL);
			CleanOrdersList();
			OutputOrders();
			OutputStatus();
		}
		break;

	case ID_ORDERS_OVERWRITEWHENLOADING:
		gIgnoreUnits = !gIgnoreUnits;
		CheckIgnoreUnitsItem(gIgnoreUnits);
		break;
	
	case ID_ORDERS_LOADORDERS:
		if (!LoadOrders(NULL) && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true; 
		}	 			
		break;
		
	case ID_ORDERS_RESOLVE:
		ResolveOrders();
		break;
		
	case ID_ORDERS_COMMIT:
		CommitOrders();
		break;
	
	case ID_ORDERS_ADDUNITTOORDER:
		prefs.mAddUnitToOrder = !prefs.mAddUnitToOrder;
		gAddUnitToOrder = prefs.mAddUnitToOrder;
		CheckAddUnitToOrderItem(gAddUnitToOrder);
		break;

	case ID_ORDERS_IGNORECONVOYROUTES:
		prefs.mIgnoreConvoyRoutes = !prefs.mIgnoreConvoyRoutes;
		gIgnoreConvoyRoutes = prefs.mIgnoreConvoyRoutes;
		CheckIgnoreConvoyRoutesItem(gIgnoreConvoyRoutes);
		break;
	
	case ID_HISTORY_PREVIOUS:
		if (!PreviousSeason())
		{
			ErrorAlert(gErrNo, NULL);
			gDone = true;
		}
		break;
		
	case ID_HISTORY_NEXT:
		if (!NextSeason())
		{
			ErrorAlert(gErrNo, NULL);
			gDone = true;
		}
		break;
		
	case ID_HISTORY_BRANCH:
		GetIndCString(string, kSpecialStringsID, kBranchStringID);
		if (CheckAction(string))
		{
			(void) history.Branch();
 			CleanOrdersList();
 			OutputOrders();
		}
		DrawNewMap();
		UpdateMapDisplay(NULL);
		break;

	case ID_WINDOWS_ORDERS:
		ShowOrdersWindow();
		break;

	case ID_WINDOWS_STATUS:
		ShowStatusWindow();
		break;

	case ID_WINDOWS_INFO:
		ShowInfoWindow();
		break;

	case ID_HELP_ABOUTREALPOLITIK:
		DoAboutBox();
		break;
	
	case ID_HELP_ACKNOWLEDGEMENTS:
		DoAcknowledgements();
		break;
		 			
	default:
		// must be a variant menu item
		int menuItem = wParam-ID_VARIANTS_STANDARD;
		if (!NewVariant(menuItem) && gErrNo != errNoError)
		{
			ErrorAlert(gErrNo, gErrorString);
			gDone = true; 
		}
		break;	

	}

}	/* DoMenuCommand */



void AdjustMenus(HWND window, HMENU menu)
{
	short s;
	bool d;
	char string[256];

	if (GetMenu(window) != menu)
	{
		return;
	}
	
	if (gGameRunning)
	{
		EnableMenuItem(menu, ID_FILE_PRINT, MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(menu, ID_FILE_SAVEAS, MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(menu, ID_FILE_SAVEASRP, MF_BYCOMMAND|((gHasWings || gIsHanse) ? MF_GRAYED : MF_ENABLED));					
		EnableMenuItem(menu, ID_FILE_SAVE, MF_BYCOMMAND|(IsGameChanged() ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(menu, ID_FILE_REVERT, MF_BYCOMMAND|(IsGameChanged() ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(menu, ID_FILE_SAVEREPORT, MF_BYCOMMAND|(!EditModeOn() ? MF_ENABLED : MF_GRAYED));
		if (gCommit)
		{
			GetIndCString(string, kSpecialStringsID, kSaveResultsID);
			SetItem(menu, ID_FILE_SAVEREPORT, string);
		}
		else
		{
			GetIndCString(string, kSpecialStringsID, kSaveStatusID);
			SetItem(menu, ID_FILE_SAVEREPORT, string);
		}
	}
	else
	{
		EnableMenuItem(menu, ID_FILE_PRINT, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_FILE_SAVEAS, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_FILE_SAVEASRP, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_FILE_SAVE, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_FILE_REVERT, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_FILE_SAVEREPORT, MF_BYCOMMAND|MF_GRAYED);
	}

	// enable Edit stuff if DA is using it, otherwise forget it 
	if (gCommit)
	{
		EnableMenuItem(menu, ID_EDIT_UNDO, MF_ENABLED);
		GetIndCString(string, kSpecialStringsID, kUndoResolveID);
		SetItem(menu, ID_EDIT_UNDO, string);
	}
	else if (LastOrderSet())
	{
		EnableMenuItem(menu, ID_EDIT_UNDO, MF_BYCOMMAND|MF_ENABLED);
		GetIndCString(string, kSpecialStringsID, kUndoOrderID);
		SetItem(menu, ID_EDIT_UNDO, string);
	}			
	else
	{
		GetIndCString(string, kSpecialStringsID, kUndoID);
		SetItem(menu, ID_EDIT_UNDO, string);
		EnableMenuItem(menu, ID_EDIT_UNDO, MF_BYCOMMAND|MF_GRAYED);
	}

	EnableMenuItem(menu, ID_EDIT_CUT, MF_BYCOMMAND|MF_GRAYED);

	if (EditModeOn())
		EnableMenuItem(menu, ID_EDIT_COPY, MF_BYCOMMAND|MF_GRAYED);
	else 
	if (ordersWindow->IsSelection() || errorWindow->IsSelection())
		EnableMenuItem(menu, ID_EDIT_COPY, MF_BYCOMMAND|MF_ENABLED);
	else if (IsStatusSelection())
		EnableMenuItem(menu, ID_EDIT_COPY, MF_BYCOMMAND|MF_ENABLED);
	else if (IsInfoSelection())
		EnableMenuItem(menu, ID_EDIT_COPY, MF_BYCOMMAND|MF_ENABLED);
	else if (mapWindow->IsWindow(window))
		EnableMenuItem(menu, ID_EDIT_COPY, MF_BYCOMMAND|MF_ENABLED);
	else
		EnableMenuItem(menu, ID_EDIT_COPY, MF_BYCOMMAND|MF_GRAYED);

	GetSelectedSector(&s, &d);
	if (EditModeOn())
		EnableMenuItem(menu, ID_EDIT_CLEAR, MF_BYCOMMAND|MF_GRAYED);
	else //if (s != 0 && IsMapWindow(window))
		EnableMenuItem(menu, ID_EDIT_CLEAR, MF_BYCOMMAND|MF_ENABLED);
//	else
//		EnableMenuItem(menu, ID_EDIT_CLEAR, MF_BYCOMMAND|MF_GRAYED);

	if ((ordersWindow->IsWindow(window) || mapWindow->IsWindow(window))
		&& IsClipboardFormatAvailable(CF_TEXT) && !EditModeOn())
		EnableMenuItem(menu, ID_EDIT_PASTE, MF_BYCOMMAND|MF_ENABLED);
	else
		EnableMenuItem(menu, ID_EDIT_PASTE, MF_BYCOMMAND|MF_GRAYED);
	
	// Map menu
	EnableMenuItem(menu, ID_MAP_SUPPLYCENTERS, MF_BYCOMMAND|(gGameRunning ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(menu, ID_MAP_UNITS, MF_BYCOMMAND|(gGameRunning ? MF_ENABLED : MF_GRAYED));

	if (dislodges || adjustments || gCommit || !history.AtEnd())
		EnableMenuItem(menu, ID_MAP_EDITMAP, MF_BYCOMMAND|MF_GRAYED);
	else
		EnableMenuItem(menu, ID_MAP_EDITMAP, MF_BYCOMMAND|MF_ENABLED);
	if (EditModeOn())
	{
		EnableMenuItem(menu, ID_MAP_EDITSEASON, MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(menu, ID_MAP_EDITRULES, MF_BYCOMMAND|MF_ENABLED);
	}
	else
	{
		EnableMenuItem(menu, ID_MAP_EDITSEASON, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_MAP_EDITRULES, MF_BYCOMMAND|MF_GRAYED);
	}

	// Orders menu 
	if (gGameRunning && !EditModeOn() && history.AtEnd())
		EnableMenuItem(menu, ID_ORDERS_LOADORDERS, MF_BYCOMMAND|MF_ENABLED);
	else
		EnableMenuItem(menu, ID_ORDERS_LOADORDERS, MF_BYCOMMAND|MF_GRAYED);
		
	if (EditModeOn() || !history.AtEnd())
		EnableMenuItem(menu, ID_ORDERS_OVERWRITEWHENLOADING, MF_BYCOMMAND|MF_GRAYED);
	else
		EnableMenuItem(menu, ID_ORDERS_OVERWRITEWHENLOADING, MF_BYCOMMAND|MF_ENABLED);
		
	if (gResolve && !EditModeOn() && history.AtEnd())
		EnableMenuItem(menu, ID_ORDERS_RESOLVE, MF_BYCOMMAND|MF_ENABLED);
	else
		EnableMenuItem(menu, ID_ORDERS_RESOLVE, MF_BYCOMMAND|MF_GRAYED);
		
	if (gDemo)
		EnableMenuItem(menu, ID_ORDERS_RESOLVE, MF_BYCOMMAND|MF_GRAYED);
	
	if (gCommit && !EditModeOn() && history.AtEnd())
		EnableMenuItem(menu, ID_ORDERS_COMMIT, MF_BYCOMMAND|MF_ENABLED);
	else
		EnableMenuItem(menu, ID_ORDERS_COMMIT, MF_BYCOMMAND|MF_GRAYED);

	if (EditModeOn() || !history.AtEnd())
		EnableMenuItem(menu, ID_ORDERS_ADDUNITTOORDER, MF_BYCOMMAND|MF_GRAYED);
	else
		EnableMenuItem(menu, ID_ORDERS_ADDUNITTOORDER, MF_BYCOMMAND|MF_ENABLED);

	if (history.AtBeginning() || EditModeOn())
		EnableMenuItem(menu, ID_HISTORY_PREVIOUS, MF_BYCOMMAND|MF_GRAYED);
	else
		EnableMenuItem(menu, ID_HISTORY_PREVIOUS, MF_BYCOMMAND|MF_ENABLED);

	if (history.AtEnd() || EditModeOn())
	{
		EnableMenuItem(menu, ID_HISTORY_NEXT, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(menu, ID_HISTORY_BRANCH, MF_BYCOMMAND|MF_GRAYED);
	}
	else
	{
		EnableMenuItem(menu, ID_HISTORY_NEXT, MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(menu, ID_HISTORY_BRANCH, MF_BYCOMMAND|MF_ENABLED);
	}

	EnableMenuItem(menu, ID_WINDOWS_ORDERS, MF_BYCOMMAND|(gGameRunning ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(menu, ID_WINDOWS_STATUS, MF_BYCOMMAND|(gGameRunning ? MF_ENABLED : MF_GRAYED));
}

void AddToVariantMenu(char* string)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	HMENU variantMenu = GetSubMenu(mainMenu, kVariantMenuID);
	int count = GetMenuItemCount(variantMenu);
	if (count != 0)
		(void) AppendMenu(variantMenu, MF_STRING, ID_VARIANTS_STANDARD+count, string);
}

void
CheckVariantItem(short index, bool checked)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	CheckMenuItem(mainMenu, ID_VARIANTS_STANDARD+index, checked ? MF_CHECKED : MF_UNCHECKED);
}

void
CheckIgnoreUnitsItem(bool checked)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	CheckMenuItem(mainMenu, ID_ORDERS_OVERWRITEWHENLOADING, checked ? MF_CHECKED : MF_UNCHECKED);
}

void
CheckAddUnitToOrderItem(bool checked)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	CheckMenuItem(mainMenu, ID_ORDERS_ADDUNITTOORDER, checked ? MF_CHECKED : MF_UNCHECKED);
}

void
CheckIgnoreConvoyRoutesItem(bool checked)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	CheckMenuItem(mainMenu, ID_ORDERS_IGNORECONVOYROUTES, checked ? MF_CHECKED : MF_UNCHECKED);
}

void
CheckEditMapItem(bool checked)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	CheckMenuItem(mainMenu, ID_MAP_EDITMAP, checked ? MF_CHECKED : MF_UNCHECKED);
}

bool CheckMenuItemByIndex(int menuID, int itemID, bool value)
{
	HMENU mainMenu = GetMenu(mapWindow->GetNative());
	int count = GetMenuItemCount(mainMenu);
	HMENU subMenu = GetSubMenu(mainMenu, menuID);
	CheckMenuItem(subMenu, itemID, value ? MF_CHECKED : MF_UNCHECKED);

	return true;
}

