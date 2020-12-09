//===============================================================================
// @ WindowUtil.cpp
// ------------------------------------------------------------------------------
// Useful routines for manipulating windows
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
// 99-Dec-22	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "WindowUtil.h"
#include <string.h>
#include <stdio.h>
#include "Global.h"
#include "Resource.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "Util.h"
#include "Game.h"
#include "Variants.h"

/* prototypes */
bool LoadCountryIcons();
void removewings(UnitList *lp);

//-------------------------------------------------------------------------------
//-- Static Variables -----------------------------------------------------------
//-------------------------------------------------------------------------------

static HWND mLoadingDialog = NULL;
extern HINSTANCE gInstance;

/* local defines */
#define kGameOverOK 1
#define kGameOverUserItem 3

//-------------------------------------------------------------------------------
//-- Functions ------------------------------------------------------------------
//-------------------------------------------------------------------------------
BOOL CALLBACK testProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam); 

void
DoAboutBox()
{
	static char messageString[] = 
"\tRealWorld version 2.0.6.\n\n\
\tCopyright 2001-2008,\n\
The Realpolitik Development Team\n\
Additions by Dirk Brüggemann \n\n\
For support on the original Realpolitik, join:\n\
RPForum@yahoogroups.com\n\
http://realpolitik.sourceforge.net\n\n\
RealWorld comes with ABSOLUTELY NO\n\
WARRANTY.  This is free software, and\n\
you are welcome to redistribute it under\n\
certain conditions. See license.txt for details.";

	MessageBox( mapWindow->GetNative(), messageString, "About...", MB_OK );
/*CONVERT
	DialogPtr theDialog;
	Boolean done = 0;
	EventRecord  	event;
	
	if ((theDialog = GetNewDialog(kAboutDialogID, (Ptr)NULL, (GrafPtr)-1)) == NULL)
		return;

	CenterDialog(theDialog);
	ShowWindow(theDialog);
	DrawDialog(theDialog);
	
	while (true) 
	{
		if (GetNextEvent(mDownMask, &event))
			break;
	}

	if (theDialog != NULL)
		DisposeDialog(theDialog);
*/
}

void DoAcknowledgements()
{
	static char messageString[] = 
"Diplomacy is a trademark of Hasbro, Inc., all rights reserved.\n\n\
The Realpolitik Development Team is: \n\
\tJames M. Van Verth\n\
\tLucas B. Kruijswijk\n\
\tBen Hines\n\
\tSimon Haines\n\
\tRonnie van 't Westeinde\n\n\
Electronic versions of Classical, Chromatic, SailHo and Hundred\n\
by Ben Hines.  Ancient Med by Don Hessong. \n\
All other electronic versions © 2001, James M. Van Verth.\n\
Addition of wings © 2005, Dirk Brüggemann.\n\n\
Addition of optional rules © 2007, Dirk Brüggemann.\n\n\
Many thanks to all those who have contributed to Realpolitik,\n\
in particular Andy J. Williams, Jamie Dreier and Don Hessong.\n\
Additional thanks to Jörg Weiß for beta testing,\n\
and to Stefan Krekeler for many useful suggestions \n\
and the promotion of RW.";

	MessageBox( mapWindow->GetNative(), messageString, 
		"Acknowledgements...", MB_OK );
/*	DialogPtr theDialog;
	Boolean done = 0;
	EventRecord  	event;
	
	if ((theDialog = GetNewDialog(kAckDialogID, (Ptr)NULL, (GrafPtr)-1)) == NULL)
		return;
	
	CenterDialog(theDialog);
	ShowWindow(theDialog);
	DrawDialog(theDialog);
	
	while (true) 
	{
		if (GetNextEvent(mDownMask, &event))
			break;
	}

	if (theDialog != NULL)
		DisposeDialog(theDialog);
*/
}


BOOL CALLBACK testProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	if (iMsg == WM_INITDIALOG)
		return TRUE;
	return FALSE; 
}

void
ShowLoadingDialog()
{
	mLoadingDialog = CreateDialog(gInstance, MAKEINTRESOURCE(IDD_LOADING), mapWindow->GetNative(), (DLGPROC)testProc);
	ShowWindow(mLoadingDialog, SW_SHOW);
}

void 
HideLoadingDialog()
{
	if (mLoadingDialog != NULL)
		DestroyWindow(mLoadingDialog);
}

/*
 * Displays if the game has been modified	
 * @param isModified  true if the game has unsaved changes	
 */	
void	
SetSaveStatus(bool isModified)	
{	
 	    // what is a Win32-like way to show the modified status?	
}

/*
 * CheckSave() - opens dialog box and asks user if really wants to save
 *               returns kSaveYes if yes,
 *                       kSaveNo  if no,
 *                       kSaveCancel if cancelled
 */
short
CheckSave(void)
{
	short  theItem = MessageBox(mapWindow->GetNative(), "Save current game?", "Save (y/n)?", MB_YESNOCANCEL);
	if (theItem == IDCANCEL) 
		return kSaveCancel;		
	else if (theItem == IDYES) 
		return kSaveYes;
	else
		return kSaveNo;
}


/*
 * CheckAction() - opens dialog box and asks user if really wants to do that action
 *                 returns true if yes, false if no
 */
bool
CheckAction(char* message)
{
	short  theItem = MessageBox(mapWindow->GetNative(), message, "Warning", MB_YESNO);
	return (theItem == IDYES);
}

/*
 * CheckBlockAction() - opens dialog box and asks user if he wants to block Moscow
 *                      returns true if yes, false if no
 */
bool
CheckBlockAction(char* message, char* title)
{
	short  theItem = MessageBox(mapWindow->GetNative(), message, title, MB_YESNO);
	return (theItem == IDYES);
}


//countries[winner].name
//slistsize(supplies[winner])
void
WinDialog(char *message, char* name, int numSupplies)
{
	char string[256];
	sprintf(string, "%s has won with %d supply centers\n%s", name, numSupplies, message);
	MessageBox( NULL, string, kAppName, MB_ICONEXCLAMATION | MB_OK );
}


BOOL CALLBACK editSeasonProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	int code;
	char yearString[32];
	switch (iMsg)
	{
	case WM_INITDIALOG:
		sprintf(yearString, "%d", year);
		Edit_SetText(GetDlgItem(hDlg, IDC_YEAR), yearString);
		Button_SetCheck(GetDlgItem(hDlg, IDC_SPRING), (season == S_SPRING)?BST_CHECKED:BST_UNCHECKED);
		Button_SetCheck(GetDlgItem(hDlg, IDC_FALL), (season == S_FALL)?BST_CHECKED:BST_UNCHECKED);
		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;

	case WM_COMMAND:
		code = LOWORD(wParam);
		switch (code)
		{
		case IDOK:
			EndDialog(hDlg, 0);
			Edit_GetText(GetDlgItem(hDlg, IDC_YEAR), yearString, 31);
			sscanf(yearString, "%d", &year);
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_SPRING)) == BST_CHECKED)
				season = S_SPRING;
			else
				season = S_FALL;
			SetGameChanged(true);
			UpdateMapPalette();
			break;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	default:
		break;
//		return(DefDlgProc( hDlg, iMsg, wParam, lParam ));
	}
	return FALSE; 
}

BOOL CALLBACK editRulesProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	int code;
	switch (iMsg)
	{
	case WM_INITDIALOG:
		SetCanals(hDlg);
		SetButtons(hDlg);
		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;

	case WM_COMMAND:
		code = LOWORD(wParam);
		switch (code)
		{
		case IDOK:
			EndDialog(hDlg, 0);
			if (!gHasWings && Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_WINGS)) == BST_CHECKED)
			{
				gHasWings = true;
				if (!LoadCountryIcons())
				{
					gHasWings = false;
				}
			}
			else if (gHasWings && Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_WINGS)) == BST_UNCHECKED)
			{
				static char messageString[] = "Disabling Wings will remove any existing Wings from the game. Are you sure you want to do this?";
				if (MessageBox(mapWindow->GetNative(), messageString, "Removal Confirmation", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				{
					removewings();
					gHasWings = false;				    
				}
			}
			gAllowsTrafo = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_TRAFO)) == BST_CHECKED);
			gHasEmergencyRule = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_ERM)) == BST_CHECKED);
			gSzykman = (Button_GetCheck(GetDlgItem(hDlg, IDC_PARADOX_SZYKMAN)) == BST_CHECKED);
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_BM_STANDARD)) == BST_CHECKED)
			{
				gBuildMode = VBM_STANDARD;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_BM_ABERRATION)) == BST_CHECKED)
			{
				gBuildMode = VBM_ABERRATION;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_BM_CHAOS)) == BST_CHECKED)
			{
				gBuildMode = VBM_CHAOS;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_BM_HANSE)) == BST_CHECKED)
			{
				gBuildMode = VBM_HANSE;
			}
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_STAY)) == BST_CHECKED)
			{
				gCanalPermissionMode = VCP_STAY;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_HOLD)) == BST_CHECKED)
			{
				gCanalPermissionMode = VCP_HOLD;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_ANY)) == BST_CHECKED)
			{
				gCanalPermissionMode = VCP_ANYUNIT;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_OWNER)) == BST_CHECKED)
			{
				gCanalPermissionMode = VCP_OWNER;
			}
			else if (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_NONE)) == BST_CHECKED)
			{
				gCanalPermissionMode = VCP_NONE;
			}
			gAllowsCanalWingUsage = (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_WING)) == BST_CHECKED);
			gAllowsCanalConvoys = (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_CONVOY)) == BST_CHECKED);
			gAllowsCanalSupports = (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_SUPPORT)) == BST_CHECKED);
			gAllowsCanalRetreats = (Button_GetCheck(GetDlgItem(hDlg, IDC_CANAL_RETREAT)) == BST_CHECKED);
			SetGameChanged(true);
			break;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;

		case IDC_BUTTON_DEFAULT:
			EvaluateFlags(variantInfo->flags);
			SetButtons(hDlg);
			break;
		}
		break;

	default:
		break;
//		return(DefDlgProc( hDlg, iMsg, wParam, lParam ));
	}
	return FALSE; 
}


void 
EditSeason( void )
{
	DialogBox(gInstance, MAKEINTRESOURCE(IDD_EDITSEASON), mapWindow->GetNative(), editSeasonProc);

}

void 
EditRules( void )
{
	DialogBox(gInstance, MAKEINTRESOURCE(IDD_EDITRULES), mapWindow->GetNative(), editRulesProc);

}

void
SetButtons( HWND hDlg )
{
	Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_WINGS), gHasWings?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_ERM), gHasEmergencyRule?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_TRAFO), gAllowsTrafo?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_PARADOX_STANDARD), gSzykman?BST_UNCHECKED:BST_CHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_PARADOX_SZYKMAN), gSzykman?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_BM_STANDARD), (gBuildMode == VBM_STANDARD)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_BM_ABERRATION), (gBuildMode == VBM_ABERRATION)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_BM_CHAOS), (gBuildMode == VBM_CHAOS)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_BM_HANSE), (gBuildMode == VBM_HANSE)?BST_CHECKED:BST_UNCHECKED);
	// Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_STAY), (gCanalPermissionMode == VCP_STAY)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_HOLD), (gCanalPermissionMode == VCP_HOLD)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_ANY), (gCanalPermissionMode == VCP_ANYUNIT)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_OWNER), (gCanalPermissionMode == VCP_OWNER)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_NONE), (gCanalPermissionMode == VCP_NONE)?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_WING), gAllowsCanalWingUsage?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_CONVOY), gAllowsCanalConvoys?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_SUPPORT), gAllowsCanalSupports?BST_CHECKED:BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_CANAL_RETREAT), gAllowsCanalRetreats?BST_CHECKED:BST_UNCHECKED);
}

void
SetCanals( HWND hDlg )
{
	Static_Enable(GetDlgItem(hDlg, IDC_CANALBOX),(canalcount() > 0));
	Static_Enable(GetDlgItem(hDlg, IDC_CANALPERMISSION),(canalcount() > 0));
	Static_Enable(GetDlgItem(hDlg, IDC_PASSAGEPERMISSION),(canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_HOLD), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_ANY), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_OWNER), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_NONE), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_WING), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_CONVOY), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_SUPPORT), (canalcount() > 0));
	Button_Enable(GetDlgItem(hDlg, IDC_CANAL_RETREAT), (canalcount() > 0));
}

/*
void SetWindowBounds(WindowPtr theWindow, Rect newBounds)
{
	short top;
	short left;
	short height;
	short width;
	
	short topInset;
	short leftInset;
	short bottomInset;
	short rightInset;
	
	Rect oldBounds;
	
	RgnHandle contRgn;
	RgnHandle structRgn;
	
	contRgn = ((WindowPeek) theWindow)->contRgn;
	structRgn = ((WindowPeek) theWindow)->strucRgn;
	
	oldBounds = (**structRgn).rgnBBox;
	
	if (!EqualRect(&oldBounds, &newBounds))
	{
		topInset = (**contRgn).rgnBBox.top - (**structRgn).rgnBBox.top;
		leftInset = (**contRgn).rgnBBox.left - (**structRgn).rgnBBox.left;
		bottomInset = (**contRgn).rgnBBox.bottom - (**structRgn).rgnBBox.bottom;
		rightInset = (**contRgn).rgnBBox.right - (**structRgn).rgnBBox.right;
	
		top = newBounds.top + topInset;
		left = newBounds.left + leftInset;
		height = newBounds.bottom - top - bottomInset;
		width = newBounds.right - left - rightInset;
		
		HideWindow(theWindow);
		MoveWindow(theWindow, left, top, FALSE);
		SizeWindow(theWindow, width, height, TRUE);
		SelectWindow(theWindow);
		ShowWindow(theWindow);
	}
}


Rect GetWindowContentRect(WindowPtr window)
{
	WindowPtr oldPort;
	Rect contentRect;
	
	GetPort(&oldPort);
	SetPort(window);
	contentRect = window->portRect;
	LocalToGlobalRect(&contentRect);
	SetPort(oldPort);
	
	return contentRect;
}


Rect GetWindowStructureRect(WindowPtr window)
{
	const short kOffscreenLocation = 0x4000;
	
	GrafPtr oldPort;
	Rect structureRect;
	Point windowLoc;
	
	if (((WindowPeek)window)->visible)
		structureRect = (*(((WindowPeek)window)->strucRgn))->rgnBBox;
	else
	{
		GetPort(&oldPort);
		SetPort(window);
		windowLoc = GetGlobalTopLeft(window);
		MoveWindow(window, windowLoc.h, kOffscreenLocation, FALSE);
		ShowHide(window, TRUE);
		structureREct = (*(((WindowPeek)window)->strucRgn))->rgnBBox;
		ShowHide(window, FALSE);
		MoveWindow(window, windowLoc.h, windowLoc.v, FALSE);
		OffsetRect(&structureRect, 0, windowLoc.v - kOffscreenLocation);
		SetPort(&oldPort);
	}
	return structureRect;
}
*/		
