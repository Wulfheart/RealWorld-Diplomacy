//===============================================================================
// @ WindowUtil.h
// ------------------------------------------------------------------------------
// Interface for useful window utility functions
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
// 99 Aug 25	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Prototypes -----------------------------------------------------------------
//-------------------------------------------------------------------------------

#ifndef __WindowUtilDefs__
#define __WindowUtilDefs__

#include "MacTypes.h"

#define kSaveYes 	1
#define kSaveNo  	2
#define kSaveCancel	3
#define kSaveUserItem 5

/* undefined routine for setting default item */
//pascal OSErr SetDialogDefaultItem (DialogPtr theDialog, short newItem) = { 0x303C, 0x0304, 0xAA68 };

/*CONVERTbool IsAppWindow(WindowPtr theWindow);
bool IsDAWindow(WindowPtr theWindow);
bool IsDialogWindow(WindowPtr theWindow);

void DoCloseWindow(WindowPtr theWindow);

pascal void DefaultOutline(const DialogPtr theWindow, const short itemNo);
*/
void LocalToGlobalRect(Rect *aRect);
void DoAboutBox();
void DoAcknowledgements();

void ShowLoadingDialog();
void HideLoadingDialog();

void SetSaveStatus(bool isModified);
short CheckSave();
bool CheckAction(char* message);
bool CheckBlockAction(char* message, char* title);

void WinDialog(char *message, char* name, int numSupplies);

void EditSeason();
void EditRules();

void SetButtons(HWND hDlg);
void SetCanals(HWND hDlg);

#endif	// __WindowUtilDefs__