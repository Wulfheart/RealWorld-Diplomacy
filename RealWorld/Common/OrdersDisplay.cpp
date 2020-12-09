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
#include "MacTypes.h"
#include "OrdersDisplay.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "Order.h"
#include "Parse.h"
#include "Util.h"
#include "Preferences.h"

LWindowPtr	ordersWindow, errorWindow;

/* bring up a new window with the orders in it */
bool
NewOrdersWindow()
{
/*	Str255				title;
	RgnHandle			ourRgn;
	Rect				tRect;
	short				fontNum;
	unsigned char       line[80];
	Point 				cell;
*/	long				width, height, x, y;

	if (ordersWindow == NULL)
	{
		/* if can't allocate for some reason */
		if ((ordersWindow = LWindow::GetNewLWindow(mapWindow, "Orders")) == NULL)
		{
			gErrNo = errMem;
			sprintf(gErrorString, "Couldn't open orders window -- out of memory?");
			return false;
		}
	}
	
	if (prefs.mOrdersWindowRect.bottom > 0 && prefs.mOrdersWindowRect.top > 0
		&& prefs.mOrdersWindowRect.bottom-prefs.mOrdersWindowRect.top >= 200)
	{
		width = 400;
		height = prefs.mOrdersWindowRect.bottom - prefs.mOrdersWindowRect.top;
		x = prefs.mOrdersWindowRect.left;
		y = prefs.mOrdersWindowRect.top;
		ordersWindow->SetPosition( x, y, width, height );
	}
	else
	{
		// get rectangle in global coordinates
		prefs.mOrdersWindowRect.right = prefs.mOrdersWindowRect.left + 400;
		ordersWindow->GetRect(&prefs.mOrdersWindowRect);
	}

	if (errorWindow == NULL)
	{
		/* if can't allocate for some reason */
		if ((errorWindow = LWindow::GetNewLWindow (mapWindow, "Errors")) == NULL)
		{
			gErrNo = errMem;
			sprintf(gErrorString, "Couldn't open errors window -- out of memory?");
			return false;
		}
	}
	
	if (prefs.mErrorsWindowRect.bottom > 0 && prefs.mErrorsWindowRect.top > 0
		&& prefs.mErrorsWindowRect.bottom-prefs.mErrorsWindowRect.top >= 200)
	{
		width = 400;
		height = prefs.mErrorsWindowRect.bottom - prefs.mErrorsWindowRect.top;
		x = prefs.mErrorsWindowRect.left;
		y = prefs.mErrorsWindowRect.top;
		errorWindow->SetPosition( x, y, width, height );
	}
	else
	{
		// get rectangle in global coordinates
		prefs.mOrdersWindowRect.right = prefs.mOrdersWindowRect.left + 400;
		errorWindow->GetRect(&prefs.mErrorsWindowRect);
	}				
		
	return true;
} /* NewOrdersWindow */


void CleanOrdersList(void)
{
	ordersWindow->ClearList();
}

void ClearErrorList(void)
{
	errorWindow->ClearList();
}

int
AddOrderLine(char *line)
{
	return ordersWindow->AddLine(line);			
}

int AddErrorLine(char *line)
{
	return errorWindow->AddLine(line);			
}

void
SetOrderLine(char *line, int index)
{
	ordersWindow->SetLine(line, index);			
}

void
SelectOrderLine(int index)
{
	ordersWindow->SelectLine(index);			
}

void
ClearOrderSelections()
{
	ordersWindow->ClearSelections();
}

void
ShowOrdersWindow()
{
	ordersWindow->Show(true);
}

void ShowErrorWindow()
{
	errorWindow->Show(true);
}

void
HideOrdersWindow()
{
	ordersWindow->Show(false);
}

void HideErrorWindow()
{
	errorWindow->Show(false);
}
