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
#include "MacTypes.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "MapContent.h"
#include "EditMap.h"
#include "Game.h"
#include "History.h"
#include "Unit.h"
#include "Order.h"
#include "OrdersDisplay.h"
#include "ReadOrders.h"
#include "EditMenu.h"
#include "Util.h"
#include "Strings.h"
#include <string.h>

static bool editing = false;


bool EditModeOn(void)
{
	return editing;
}


void EnterEditMode(void)
{
	ResetSelectedUnit();
	ClearClicks();
	editing = true;
	
	if (strlen(gamename) > 0)
	{
		char maptitle[256];
		char editstring[256];
		strcpy(maptitle, gamename);
		strcat(maptitle, ": ");
		GetIndCString(editstring, kSpecialStringsID, kEditingID);
		strcat(maptitle, editstring);
		SetMapTitle(maptitle);
	}
	orderPalette.Activate(mapWindow, false);
}


void ExitEditMode(void)
{
	editing = false;
	history.CopyCurrentState();
	if (strlen(gamename) > 0)
		SetMapTitle(gamename);
	orderPalette.Activate(mapWindow, true);
}


bool ToggleEditMode(void)
{
	ResetSelectedUnit();
	ClearClicks();
	editing = !editing;
	if (editing)
	{
		if (strlen(gamename) > 0)
		{
			char maptitle[256];
			char editstring[256];
			strcpy(maptitle, gamename);
			strcat(maptitle, ": ");
			GetIndCString(editstring, kSpecialStringsID, kEditingID);
			strcat(maptitle, editstring);
			SetMapTitle(maptitle);
		}
	}
	else
	{
		history.CopyCurrentState();
		if (strlen(gamename) > 0)
			SetMapTitle(gamename);
	}
	orderPalette.Activate(mapWindow, !editing);

	return editing;
}


void DoEditClick(Point where, short part)
{
	EditMenu *menu;
	long result;

	BlinkSector(part, C_NONE);
	menu = new EditMenu(part);
	result = menu->PopUpMenuSelect(where.h, where.v);
		
	if (result)
	{
		ShowWaitCursor();
		menu->HandleMapEdit(part, result);
		DrawNewMap();
		UpdateMapDisplay(NULL);
		CleanOrdersList();
		OutputOrders();
		OutputStatus();
		SetGameChanged(true);
		ShowArrowCursor();
	}

	delete menu;	
}

