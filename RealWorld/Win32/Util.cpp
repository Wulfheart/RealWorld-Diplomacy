//===============================================================================
// @ Util.cpp
// ------------------------------------------------------------------------------
// Useful native utility routines.
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

#include <string.h>
#include <wtypes.h>
#include <stdio.h>
#include "Util.h"
#include "Global.h"
#include "MapDisplay.h"
#include "Strings.h"

int		gErrNo = errNoError;		// fatal error, if any
char	gErrorString[256] = "";
SFTypeList csvFilter = "CSV Files (*.CSV)\0*.csv\0"	\
						"All Files (*.*)\0*.*\0\0";
SFTypeList textFilter = "Text Files (*.TXT)\0*.txt\0"	\
						"All Files (*.*)\0*.*\0\0";
SFTypeList gameFilter = "RealWorld Files (*.DPW)\0*.dpw\0"\
						"Realpolitik Files (*.DPY)\0*.dpy\0"\
						"All Files (*.*)\0*.*\0\0";
SFTypeList saveFilter = "RealWorld Files (*.DPW)\0*.dpw\0"\
						"All Files (*.*)\0*.*\0\0";
SFTypeList oldsFilter = "Realpolitik Files (*.DPY)\0*.dpy\0"\
						"All Files (*.*)\0*.*\0\0";

static OPENFILENAME textofn = 
{
	sizeof(OPENFILENAME),
	NULL,
	NULL,
	textFilter, 
	NULL,
	0,
	0,
	NULL,
	_MAX_PATH,
	NULL,
	_MAX_FNAME + _MAX_EXT,
	NULL,
	NULL,
	0,
	0,
	0,
	"txt",
	0L,
	NULL,
	NULL
};

static OPENFILENAME dpyofn = 
{
	sizeof(OPENFILENAME),
	NULL,
	NULL,
	gameFilter, 
	NULL,
	0,
	0,
	NULL,
	_MAX_PATH,
	NULL,
	_MAX_FNAME + _MAX_EXT,
	NULL,
	NULL,
	0,
	0,
	0,
	"dpy,dpw",
	0L,
	NULL,
	NULL
};

static OPENFILENAME dpwofn = 
{
	sizeof(OPENFILENAME),
	NULL,
	NULL,
	saveFilter, 
	NULL,
	0,
	0,
	NULL,
	_MAX_PATH,
	NULL,
	_MAX_FNAME + _MAX_EXT,
	NULL,
	NULL,
	0,
	0,
	0,
	"dpw",
	0L,
	NULL,
	NULL
};

static OPENFILENAME sdpyofn = 
{
	sizeof(OPENFILENAME),
	NULL,
	NULL,
	oldsFilter, 
	NULL,
	0,
	0,
	NULL,
	_MAX_PATH,
	NULL,
	_MAX_FNAME + _MAX_EXT,
	NULL,
	NULL,
	0,
	0,
	0,
	"dpy",
	0L,
	NULL,
	NULL
};

//
// bring up an alert with an error message based on number
//
void ErrorAlert(int errNumber, char* string) 
{
	char	theMessage[256];
	
	GetIndCString(theMessage, kErrorStringsID, errNumber);
	if (string != NULL)
	{
		strcat(theMessage, ": ");
		strcat(theMessage, string);
	}
	ErrorString(theMessage);	
}

/* bring up an alert with an error message, then quit */
void ErrorString(char *string) 
{
	MessageBox( NULL, string, kAppName, MB_ICONEXCLAMATION | MB_OK );
}


void ShowWaitCursor()
{
	HCURSOR		watch;
	
	watch = LoadCursor(NULL, IDC_WAIT);
	(void) SetCursor(watch);	
}

void ShowArrowCursor()
{
	HCURSOR		arrow;
	
	arrow = LoadCursor(NULL, IDC_ARROW);
	(void) SetCursor(arrow);	
}


bool GetFile(FSSpec& filename, char* prompt, SFTypeList filter)
{
	OPENFILENAME *ofn;
	if (filter == gameFilter)
	{
		ofn = &dpyofn;
	}
	else if (filter == saveFilter)
	{
		ofn = &dpwofn;
	}
	else if (filter == oldsFilter)
	{
		ofn = &sdpyofn;
	}
	else 
	{
		ofn = &textofn;
	}
	ofn->hwndOwner = mapWindow->GetNative();
	ofn->lpstrFile = filename;
	ofn->lpstrFile[0] = '\0';
	ofn->lpstrFileTitle = prompt;
	ofn->Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
		
	return (GetOpenFileName(ofn) != 0);		
}

bool PutFile(FSSpec& filename, char *prompt, char* defaultName, SFTypeList filter)
{
	OPENFILENAME *ofn;
	if (filter == gameFilter)
	{
		ofn = &dpyofn;
	}
	else if (filter == saveFilter)
	{
		ofn = &dpwofn;
	}
	else if (filter == oldsFilter)
	{
		ofn = &sdpyofn;
	}
	else
	{
		ofn = &textofn;
	}

	ofn->hwndOwner = mapWindow->GetNative();
	ofn->lpstrFile = filename;
	ofn->lpstrFile[0] = '\0';
	ofn->lpstrFileTitle = prompt;
	ofn->Flags = OFN_OVERWRITEPROMPT;
	
	return (GetSaveFileName(ofn) != 0);
}

void GetBasename(char* basename, const FSSpec& filename)
{
	const char* lastslash = strrchr(filename, '\\');
	if (lastslash == NULL)
		strcpy(basename, filename);
	else
	{
		lastslash++;
		strcpy(basename, lastslash);
	}
}

bool CopyScrapToFile(FSSpec& tempName)
{
	HANDLE tHandle;
	
	if (!OpenClipboard(mapWindow->GetNative()))
	{
		ErrorAlert(errScrapNoTEXT, NULL);
		return false;
	}

	if ((tHandle = GetClipboardData(CF_TEXT)) == NULL)
	{
		ErrorAlert(errScrapNoTEXT, NULL);
		return false;   
	}
	
	// get the data
	char* data = (char*) GlobalLock(tHandle);
	if ( strlen(data) == 0 )
	{
		GlobalUnlock(tHandle);
		CloseClipboard();
		return false;
	}

	// create a temporary file
	::strcpy( tempName, "DIP" );
	HMMIO result = ::mmioOpen( tempName, NULL, MMIO_GETTEMP );
	if ((HMMIO)MMIOERR_FILENOTFOUND == result)
	{
		GlobalUnlock(tHandle);
		CloseClipboard();
		ErrorAlert(errTmpFileCreat, NULL);
		return false;
	}

	// write data to it
	FILE* tempFile = fopen(tempName, "w");
	if (tempFile == NULL)
	{
		GlobalUnlock(tHandle);
		CloseClipboard();
		ErrorAlert(errTmpFileOpen, NULL);
		return false;
	}
	if (fwrite(data, strlen(data), 1, tempFile) == 0)
	{
		GlobalUnlock(tHandle);
		CloseClipboard();
		ErrorAlert(errTmpFileWrite, NULL);
		return false;
	}
	fclose(tempFile);
	
	// free up some memory
	GlobalUnlock(tHandle);
	CloseClipboard();
	
	return true;
}

void DeleteTempFile(const FSSpec& tempName)
{
	(void)mmioOpen( (char*)tempName, NULL, MMIO_DELETE );
}

bool GetInstallDirectory(char* path)
{
	HKEY			key;					// Handle of the key we're inspecting
	unsigned char	message[ 256 ];			// Buffer for holding the value
	unsigned long	messageLength = 256;
	HRESULT			result;
	DWORD			type;

	// Open the key
	result = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
							   "Software\\RealWorld",
							   0,
							   KEY_QUERY_VALUE,
							   &key );
	if ( ERROR_SUCCESS == result )
	{
		result = ::RegQueryValueEx( key, 
			"InstallDir", 
			NULL, 
			&type, 
			(unsigned char*)message, 
			&messageLength );
		// Clean up memory
		(void) ::RegCloseKey( key );
		if ( ERROR_SUCCESS == result )
		{
			strcpy(path, (char *)message);
		}
 	}
	// if no key found
	if ( ERROR_SUCCESS != result )
	{
		result = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
							"Software\\Realpolitik",
							0,
							KEY_QUERY_VALUE,
							&key );
 		if ( ERROR_SUCCESS == result )
		{
			result = ::RegQueryValueEx( key, 
				"InstallDir", 
				NULL, 
				&type, 
				(unsigned char*)message, 
				&messageLength );
			// Clean up memory
			(void) ::RegCloseKey( key );
			if ( ERROR_SUCCESS == result )
			{
				strcpy(path, (char *)message);
				strncat(path, "\\", MAX_PATH);
			}
 		}
		if ( ERROR_SUCCESS != result )
		{
			// get path from module or current directory
			extern HINSTANCE gInstance;
			GetModuleFileName(gInstance,path,MAX_PATH);
			char *path_sep=strrchr(path,'\\');
			if (path_sep)
				path_sep[1]='\0';
			else 
			{
				GetCurrentDirectory(MAX_PATH, path);
				strncat(path, "\\", MAX_PATH);
			}
 			return false;
	
		}
	}
	return true;
}

bool GetDataDirectory(char* path)
{
	HKEY			key;					// Handle of the key we're inspecting
	unsigned char	message[ 256 ];			// Buffer for holding the value
	unsigned long	messageLength = 256;
	HRESULT			result;
	DWORD			type;

	// Open the key
	result = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
							   "Software\\RealWorld",
							   0,
							   KEY_QUERY_VALUE,
							   &key );
	if ( ERROR_SUCCESS == result )
	{
		result = ::RegQueryValueEx( key, 
			"DataDir", 
			NULL, 
			&type, 
			(unsigned char*)message, 
			&messageLength );
		// Clean up memory
		(void) ::RegCloseKey( key );
		if ( ERROR_SUCCESS == result )
		{
			strcpy(path, (char *)message);
		//	strncat(path, "\\", MAX_PATH);
		}
 	}

	// if no key found
	if ( ERROR_SUCCESS != result )
 	{
		return GetInstallDirectory(path);
	}

	return true;
}

void SetCountryRGBColor( short colorID )
{
	RGBColor grayColor = {0x7FFF, 0x7FFF, 0x7FFF}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF};
	RGBColor color;
	
	Index2Color(colorID, &color);
	if (color.red == 0xFFFF && color.green == 0xFFFF && color.blue == 0xFFFF)
		RGBBackColor(&grayColor); 
	else
		RGBBackColor(&whiteColor);
	RGBForeColor(&color); 
}

void SetShiftedCountryRGBColor( short colorID )
{
	RGBColor grayColor = {0x7FFF, 0x7FFF, 0x7FFF}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF};
	RGBColor color;
	RGBColor shiftedColor;
	
	Index2Color(colorID, &color);
	shiftedColor.blue = color.blue + 0x7F00;
	shiftedColor.green = color.green + 0x7F00;
	shiftedColor.red = color.red + 0x7F00;

	if (shiftedColor.red == 0xFFFF && shiftedColor.green == 0xFFFF && shiftedColor.blue == 0xFFFF)
		RGBBackColor(&grayColor); 
	else
		RGBBackColor(&whiteColor);
	RGBForeColor(&shiftedColor); 
}