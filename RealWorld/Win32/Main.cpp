//===============================================================================
// @ Main.cpp
// ------------------------------------------------------------------------------
// Main file for Realpolitik.   This is where the game is created and we hook into 
// MacOS.
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

#include <windows.h>
#include "Game.h"
#include "Initialize.h"
#include "Preferences.h"
#include "Resource.h"
#include "Util.h"
#include "MapDisplay.h"
#include <crtdbg.h>

//-------------------------------------------------------------------------------
//-- Defines, Constants ---------------------------------------------------------
//-------------------------------------------------------------------------------

// special Win32 stuff
static HACCEL		gAccel = NULL;
HINSTANCE 			gInstance = NULL;

// other globals
bool				gDone = false;			// true when we're done
bool				gInBackground = false;  // true if we're running in background
bool				gDemo = false;  		// true if we're a demo version

const char* 		kAppName = "RealWorld";
const char* 		kOldAppName = "Realpolitik";
const char* 		kDataDirectory = "Variant Files\\";
const char*			kBitmapExtension = ".bmp";
const char*			kSeparator = "\\";
const char*			kIconPath = "Icon Files";

//----------------------------------------------------------------------------
//-- Functions ---------------------------------------------------------------
//----------------------------------------------------------------------------

bool InitApplication(HINSTANCE instance, LPSTR lpCmdLine, int cmdShow);
void MainEventLoop(void);
int CleanUpAndExit(void);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (InitApplication(hInstance, lpCmdLine, nCmdShow))
		MainEventLoop();
	return CleanUpAndExit();
}


// Initialize the whole shooting match
bool InitApplication(HINSTANCE instance, LPSTR lpCmdLine, int cmdShow)
{
	// Get current flag
int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

// Turn on leak-checking bit
tmpFlag |= _CRTDBG_LEAK_CHECK_DF || _CRTDBG_CHECK_ALWAYS_DF;

// Turn off CRT block checking bit
//tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

// Set flag to the new value
_CrtSetDbgFlag( tmpFlag );

	gInstance = instance;
	
	// initialize common game code
 	if (!InitializeCommon())
 		return false;
 	
	char arguments[256];
	memset(arguments, 0, 256);
 	// drag and drop support
 	if (strlen(lpCmdLine) > 0)
 	{
		if (lpCmdLine[0] == '"')
		{
			// get rid of the double quotes
			strncpy(arguments, lpCmdLine+1, strlen(lpCmdLine)-2);
//			ErrorString(arguments);
 			if (!OpenFinderGame(arguments))
 			{
 				ErrorAlert(gErrNo, gErrorString);
 				ExitProcess(0);
 				gDone = true;
 				return false;
 			}
		}
		else
		{
			strcpy(arguments, lpCmdLine);
//			ErrorString(arguments);
 			if (!OpenFinderGame(arguments))
 			{
 				ErrorAlert(gErrNo, gErrorString);
 				ExitProcess(0);
 				gDone = true;
 				return false;
 			}
		}
	}
	else
 	{
 		if (!InitNewGame())
 		{
			ErrorAlert(gErrNo, lpCmdLine);
			ExitProcess(0);
			gDone = true;
			return false;
 		}
 	}
	
 	gAccel = LoadAccelerators(instance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
 	
	return true;
}



//
// This is where all the action is -- wait for events and deal with them
// appropriately.
//
void MainEventLoop(void)
{
	MSG msg;

	// Run the main event loop
	while( !gDone )
	{
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if (msg.message == WM_QUIT)
			{
				gDone = true;
				break;
			}
			if (!TranslateAccelerator( mapWindow->GetNative(), gAccel, &msg ))
			{
//				if ( statusWindow == NULL || !IsDialogMessage(statusWindow, &msg) )
				{

					TranslateMessage( &msg );
					DispatchMessage( &msg );
					_ASSERTE(_CrtCheckMemory());
				}
			}
		}
	}
	
}


// clean up some stuff (if necessary) before we go 
int 
CleanUpAndExit()
{
	prefs.Save();
	return 0;
}

