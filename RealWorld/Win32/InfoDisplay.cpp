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
#include "StatusDisplay.h"
#include "Util.h"
#include "TextFile.h"
#include "ListUtil.h"
#include "Resource.h"
#include "WindowUtil.h"

#define FORWARD_WM_SETFONT(hwnd, hfont, fRedraw, fn) \
    (void)(fn)((hwnd), WM_SETFONT, (WPARAM)(HFONT)(hfont), (LPARAM)(BOOL)(fRedraw))
#define     SetWindowFont(hwnd, hfont, fRedraw) FORWARD_WM_SETFONT((hwnd), (hfont), (fRedraw), SNDMSG)

HWND		infoWindow;
HWND		textList;

BOOL CALLBACK infoProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK infoProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	int code;
	switch (iMsg)
	{
	case WM_INITDIALOG:
		textList = GetDlgItem(hDlg, IDC_TEXTLIST);
		SetWindowFont(textList, GetStockObject(ANSI_FIXED_FONT), TRUE);
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
			ClearStatusSelections();
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
NewInfoWindow(void)
{
	if (infoWindow != NULL)
		return true;

	HINSTANCE instance = (HINSTANCE) GetWindowLong(mapWindow->GetNative(), GWL_HINSTANCE);
	if ((infoWindow = CreateDialog(instance, MAKEINTRESOURCE(IDD_INFO), mapWindow->GetNative(), (DLGPROC)infoProc)) == NULL)
	{
		gErrNo = errMem;
		return false;
	}

	if (prefs.mInfoWindowRect.bottom-prefs.mInfoWindowRect.top != 0)
	{
		int width = prefs.mInfoWindowRect.right - prefs.mInfoWindowRect.left;
		int height = prefs.mInfoWindowRect.bottom - prefs.mInfoWindowRect.top;
		int x = prefs.mInfoWindowRect.left;
		int y = prefs.mInfoWindowRect.top;
		SetWindowPos( infoWindow, 0, x, y, width, height, SWP_NOZORDER );
	}

	return true;
} /* NewInfoWindow */


void
CleanInfoText(void)
{
	SendMessage(textList, LB_RESETCONTENT, 0, 0);
}

bool 
LoadInfoText(char* filename)
{
	FSSpec fsspec;
	TextFile file;
	char line[2048];
	MakeFSSpec(fsspec, filename);
	if (!file.Open(fsspec, false))
	{
		return false;
	}
	CleanInfoText();
	while (file.ReadLine(line, 2047))
	{
		AddInfoTextLine( line );
	}
	file.Close();
	
	return true;
}

void
AddInfoTextLine(char *line)
{
	(void) AddListLine(textList, line, 78, 0);
}


void ClearInfoSelection()
{
	ClearListSelections(textList);
}


// close orders window 
void		
CloseInfoWindow(void)
{
}


bool IsInfoSelection()
{
	return IsListSelection(textList);
}

// copy selection to clipboard 
void
CopyInfoSelection()
{
	if (IsListSelection(textList))
		CopyListSelection(textList);
}

void 
GetInfoRect(Rect& rect)
{
	::GetWindowRect(infoWindow, &rect);
}

bool
IsInfoWindow(HWND window)
{
	return (window == infoWindow);
}

void
ShowInfoWindow()
{
	ShowWindow( infoWindow, SW_SHOW );
}

void
HideInfoWindow()
{
	ShowWindow( infoWindow, SW_HIDE );
}
