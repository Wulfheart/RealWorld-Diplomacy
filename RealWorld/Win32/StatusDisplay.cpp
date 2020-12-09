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
#include "MapDisplay.h"
#include "InfoDisplay.h"
#include "OrdersDisplay.h"
#include "Preferences.h"
#include "Util.h"
#include "StatusDisplay.h"
#include "ListUtil.h"
#include "Resource.h"
#include "WindowUtil.h"

#define FORWARD_WM_SETFONT(hwnd, hfont, fRedraw, fn) \
    (void)(fn)((hwnd), WM_SETFONT, (WPARAM)(HFONT)(hfont), (LPARAM)(BOOL)(fRedraw))
#define     SetWindowFont(hwnd, hfont, fRedraw) FORWARD_WM_SETFONT((hwnd), (hfont), (fRedraw), SNDMSG)

HWND		statusWindow;
HWND		centersList, dislodgeList;

//pascal void CentersListItem(WindowPtr theWindow, short itemNo);
//pascal void DislodgeListItem(WindowPtr theWindow, short itemNo);
BOOL CALLBACK statusProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK statusProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	int code;
	switch (iMsg)
	{
	case WM_INITDIALOG:
		centersList = GetDlgItem(hDlg, IDC_CENTERSLIST);
		SetWindowFont(centersList, GetStockObject(ANSI_FIXED_FONT), TRUE);
		dislodgeList = GetDlgItem(hDlg, IDC_DISLODGELIST);
		SetWindowFont(dislodgeList, GetStockObject(ANSI_FIXED_FONT), TRUE);
		return TRUE;

	case WM_CLOSE:
		ShowWindow( hDlg, SW_HIDE );
		break;

	case WM_COMMAND:
		code = HIWORD(wParam);
		if (code == LBN_SELCHANGE)
		{
			errorWindow->ClearSelections();
			ordersWindow->ClearSelections();
			ClearInfoSelection();
			if ((void*)lParam == centersList)
				ClearDislodgeSelection();
			else if ((void*)lParam == dislodgeList)
				ClearSupplySelection();
			// drop through
		}

	default:
		break;
//		return(DefDlgProc( hDlg, iMsg, wParam, lParam ));
	}
	return FALSE; 
}



/* bring up a new status window */
bool
NewStatusWindow(void)
{
	if (statusWindow != NULL)
		return true;

	HINSTANCE instance = (HINSTANCE) GetWindowLong(mapWindow->GetNative(), GWL_HINSTANCE);
	if ((statusWindow = CreateDialog(instance, MAKEINTRESOURCE(IDD_STATUS), mapWindow->GetNative(), (DLGPROC)statusProc)) == NULL)
	{
		gErrNo = errMem;
		return false;
	}
	
	if (prefs.mStatusWindowRect.bottom-prefs.mStatusWindowRect.top != 0)
	{
		int width = prefs.mStatusWindowRect.right - prefs.mStatusWindowRect.left;
		int height = prefs.mStatusWindowRect.bottom - prefs.mStatusWindowRect.top;
		int x = prefs.mStatusWindowRect.left;
		int y = prefs.mStatusWindowRect.top;
		SetWindowPos( statusWindow, 0, x, y, width, height, SWP_NOZORDER|SWP_NOSIZE );
	}

	return true;
} /* NewStatusWindow */


void
CleanCentersList(void)
{
	SendMessage(centersList, LB_RESETCONTENT, 0, 0);
}

void
CleanDislodgeList(void)
{
	SendMessage(dislodgeList, LB_RESETCONTENT, 0, 0);
}

void
AddCenterLine(char *line)
{
	(void) AddListLine(centersList, line, 65);
}

void
AddDislodgeLine(char *line)
{
	(void) AddListLine(dislodgeList, line, 65);
}

void
SetExpecting(char *line)
{

	HWND item = GetDlgItem(statusWindow, IDC_EXPECTING);				
	(void) SetWindowText(item, line);
}

void ClearStatusSelections()
{
	ClearSupplySelection();
	ClearDislodgeSelection();
}

void ClearSupplySelection()
{
	ClearListSelections(centersList);
}

void ClearDislodgeSelection()
{
	ClearListSelections(dislodgeList);
}

// close orders window 
void		
CloseStatusWindow(void)
{
/*CONVERT	if (centersList != NIL)
		LDispose(centersList);
	centersList = NIL;
	
	if (dislodgeList != NIL)
		LDispose(dislodgeList);
	dislodgeList = NIL;
		
	if (statusWindow == NIL)
		return;
		
	CloseDialog(statusWindow);
	statusWindow = NIL;
*/
}


bool IsStatusSelection()
{
	return (IsListSelection(centersList)
		|| IsListSelection(dislodgeList));
}

// copy selection to clipboard 
void
CopyStatusSelection()
{
	if (IsListSelection(centersList))
		CopyListSelection(centersList);
	else if (IsListSelection(dislodgeList))
		CopyListSelection(dislodgeList);
}

void 
GetStatusRect(Rect& rect)
{
	::GetWindowRect(statusWindow, &rect);
}

bool
IsStatusWindow(HWND window)
{
	return (window == statusWindow);
}

void
ShowStatusWindow()
{
	ShowWindow( statusWindow, SW_SHOW );
}

void
HideStatusWindow()
{
	ShowWindow( statusWindow, SW_HIDE );
}
