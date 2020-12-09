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
#include "EventUtil.h"
#include "MenuUtil.h"
#include "MapDisplay.h"
#include "MapContent.h"
#include "OrderPalette.h"
#include "Order.h"
#include "MacTypes.h"
#include "Game.h"
#include "Global.h"
#include "Variants.h"

/* local prototypes */
static void HandlePaint( const HWND theWindow, const WPARAM wParam, const LPARAM lParam );
static void DoInContent (HWND theWindow, const WPARAM wParam, const LPARAM lParam );
static void HandleKeyDown( HWND theWindow, const WPARAM wParam, const LPARAM lParam );

//----------------------------------------------------------------------------
// @ WindowProc()
// ---------------------------------------------------------------------------
// Standard windows function to handle window events.
//----------------------------------------------------------------------------
LRESULT CALLBACK
WindowProc( const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam )
{
	switch ( uMsg ) 
	{
	case WM_CLOSE:
		gDone = QuitGame();
		break;
		
	case WM_PAINT:
		HandlePaint(hWnd, wParam, lParam);
		break;

	case WM_INITMENU:
		AdjustMenus(hWnd, (HMENU)wParam);
		break;

	case WM_GETMINMAXINFO:
		if (mapWindow->IsWindow(hWnd))
			mapWindow->OnGrow((LPMINMAXINFO) lParam);
		break;

	case WM_SIZE:
		if (mapWindow->IsWindow(hWnd))
		{
			SizeMapWindow( mapWindow, LOWORD(lParam), HIWORD(lParam) );
			return(DefWindowProc( hWnd, uMsg, wParam, lParam ));
		}
		break;

	case WM_VSCROLL:
		if (mapWindow->IsWindow(hWnd))
		{
			mapWindow->TrackVBar( LOWORD(wParam), HIWORD(wParam) );
		}
		break;

	case WM_HSCROLL:
		if (mapWindow->IsWindow(hWnd))
		{
			mapWindow->TrackHBar( LOWORD(wParam), HIWORD(wParam) );
		}
		break;

	case WM_COMMAND:
		DoMenuCommand(wParam);
		break;

	case WM_QUERYNEWPALETTE:
		return mapWindow->OnSetPalette(hWnd, FALSE);

	case WM_PALETTECHANGED:
		if (!mapWindow->IsWindow((HWND) wParam))
			return mapWindow->OnSetPalette(hWnd, TRUE);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		DoInContent( hWnd, wParam, lParam );
		break;

	case WM_KEYDOWN:
		HandleKeyDown( hWnd, wParam, lParam );

	default:
		return(DefWindowProc( hWnd, uMsg, wParam, lParam ));
   }
   return( FALSE );
}	// End of WindowProc()


/* handle paint events */
void HandlePaint( const HWND theWindow, const WPARAM wParam, const LPARAM lParam )
{
	BeginUpdate(theWindow);
		if (mapWindow->IsWindow(theWindow))
			mapWindow->OnUpdate();
	EndUpdate(theWindow);
	
} /* HandlePaint */


void DoInContent (HWND theWindow, const WPARAM wParam, const LPARAM lParam )
{
	Point local, screen;
	local.h = LOWORD(lParam);
	local.v = HIWORD(lParam);
	POINT localPoint = {local.h, local.v};
	ClientToScreen(theWindow, &localPoint);
	screen.h = (short) localPoint.x;
	screen.v = (short) localPoint.y;
	bool shiftclick;

	
	if (mapWindow->IsWindow(theWindow)) 
	{
		shiftclick = (wParam == 5);
		MapContent(local, screen, shiftclick);
	}
}

/* handle a key down event */
void 
HandleKeyDown( HWND theWindow, const WPARAM wParam, const LPARAM lParam )
{
	if (mapWindow->IsWindow(theWindow) && !gCommit)
	{
    	switch (wParam)
		{
       	case VK_DELETE:   // Delete 
        	ClearSelectedUnit();
 			break;
			
		case VK_LEFT:   // left arrow 
			orderPalette.DoShiftPalette(true);
			break;
			
		case VK_RIGHT:   // right arrow 
			orderPalette.DoShiftPalette(false);
			break;

		case 'M':
			if (dislodges)
				(void) orderPalette.SetOrderMode(O_RETR);
			else
				(void) orderPalette.SetOrderMode(O_MOVE);
			break;
		case 'S':
			(void) orderPalette.SetOrderMode(O_SUPP);
			break;
		case 'C':
			(void) orderPalette.SetOrderMode(O_CONV);
			break;
		case 'T':
			if (gAllowsTrafo)
				(void) orderPalette.SetOrderMode(O_TRAFO);
			break;
		case 'F':
			(void) orderPalette.SetOrderMode(O_FLEET);
			break;
		case 'A':
			(void) orderPalette.SetOrderMode(O_ARMY);
			break;
		case 'W':
			if (gHasWings)
				(void) orderPalette.SetOrderMode(O_WING);
			break;
		case 'D':
		case 'X':
			(void) orderPalette.SetOrderMode(O_REMOVE);
			break;
		case 'R':
			if (dislodges)
				(void) orderPalette.SetOrderMode(O_RETR);
			else
				(void) orderPalette.SetOrderMode(O_MOVE);
			break;
		default:
			break;
		}					
	}
}

/*

static void 
HandleKeyDown (EventRecord *event)
{

	char 			c;
	
	c = event->message & charCodeMask;		// get character from event record
	
	if (event->modifiers & cmdKey) 
	{
		if (c == '.')
		{
			ResetSelectedUnit();
		}
		else
		{ 
			if (!AdjustMenus())
			{
				ErrorAlert(gErrNo);
				gDone = true;
				return;
			}
			DoMenuCommand(MenuKey (c));		// do menu commands
			HiliteMenu (0);
		} 
	}
	else 
	{
	}
} // HandleKeyDown */


