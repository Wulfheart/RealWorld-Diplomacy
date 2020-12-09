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
//===============================================================================
// @ Initialize.cpp
// ------------------------------------------------------------------------------
// Initialization routines
//
// Change history:
//
// 00-Jan-02	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <string.h>
#include "Initialize.h"
#include "Game.h"
#include "Global.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "MenuUtil.h"
#include "Neighbor.h"
#include "InfoDisplay.h"
#include "OrdersDisplay.h"
#include "Preferences.h"
#include "Resource.h"
#include "StatusDisplay.h"
#include "Util.h"
#include "Variants.h"

//----------------------------------------------------------------------------
//-- Functions ---------------------------------------------------------------
//----------------------------------------------------------------------------

//
// Initialize common data
//
bool 
InitializeCommon()
{
	// load preferences
	(void) prefs.Load();

	// pre-allocate memory
 	allocneighbor();
 	if (!AllocMapData())
 	{
 		ErrorAlert(gErrNo, gErrorString);
 		return false;
 	}

	// create map window
 	if (!AllocMapWindow())
 	{
 		ErrorAlert(gErrNo, gErrorString);
 		return false;
	} 	
	
	// initialize variants
	if (InitVariants() <= 0)
	{
//		char foo[1024];
//		GetCurrentDirectory(1024, foo);
//		ErrorString(foo);

 		ErrorString("No variant files found -- in wrong directory?"); 
 		return false;
	}
	gAddUnitToOrder = prefs.mAddUnitToOrder;
	CheckIgnoreUnitsItem(gIgnoreUnits);
	CheckAddUnitToOrderItem(gAddUnitToOrder);
	CheckMenuItemByIndex(kMapMenuID, kSupplySectorsItem, prefs.mShowSupplies);
	CheckMenuItemByIndex(kMapMenuID, kOtherLandItem, prefs.mShowOtherLand);
	CheckMenuItemByIndex(kMapMenuID, kUnitsItem, prefs.mShowUnits);
	CheckMenuItemByIndex(kMapMenuID, kOrdersItem, prefs.mShowOrders);
	CheckMenuItemByIndex(kMapMenuID, kNamesItem, prefs.mShowNames);
	
	if (!NewOrdersWindow())
 	{
 		ErrorAlert(gErrNo, gErrorString);
 		ExitToShell();
 		gDone = true;
 		return false;
 	}
	if (!NewStatusWindow())
 	{
 		ErrorAlert(gErrNo, gErrorString);
 		ExitToShell();
 		gDone = true;
 		return false;
 	}		
	if (!NewInfoWindow())
 	{
 		ErrorAlert(gErrNo, gErrorString);
 		ExitToShell();
 		gDone = true;
 		return false;
 	}		
		
	return true;
}


bool
InitNewGame()
{
	if (!SetVariantByName(prefs.mLastVariant))
		SetVariant(0);
 	
 	ShowWaitCursor();

	if (!InitMapDisplay())
 	{
 		ErrorAlert(gErrNo, gErrorString);
 		
 		// try with standard variant
		if (!ChangeVariant(0))
 		{
 			ShowArrowCursor();
 			ErrorAlert(gErrNo, gErrorString);
 			return false;
 		}
 	} 

	if (!gGameRunning)
	{
 		if (!NewGame())
 		{
 			ErrorAlert(gErrNo, gErrorString);
 			
 			// try with standard variant before giving up
 			if (!ChangeVariant(0) || !NewGame())
 			{
 				ShowArrowCursor();
 				ErrorAlert(gErrNo, gErrorString);
 				return false;
 			}
 		}
 	}

 	ShowArrowCursor();
 	
 	return true;
 }
