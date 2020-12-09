//===============================================================================
// @ FileFinder.cpp
// ------------------------------------------------------------------------------
// Class for searching through directories.
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
// 00-Jan-06	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <string.h>
#include "FileFinder.h"
#include "Util.h"

//-------------------------------------------------------------------------------
//-- Defines, Constants ---------------------------------------------------------
//-------------------------------------------------------------------------------

//----------------------------------------------------------------------------
//-- Functions ---------------------------------------------------------------
//----------------------------------------------------------------------------

FileFinder::FileFinder()
{
	mHandle = NULL;
	memset(&mFindData, 0, sizeof(WIN32_FIND_DATA));
	mDone = true;
}


FileFinder::~FileFinder()
{
	FindClose(mHandle);
}


bool
FileFinder::Start(const char* directory)
{
	// get the path
	char searchPath[MAX_PATH];

	strncpy(searchPath, directory, MAX_PATH);
	strncat(searchPath, "*.*", MAX_PATH);

	// set up search info
	mHandle = FindFirstFile( searchPath, &mFindData );

	mDone = (mHandle == INVALID_HANDLE_VALUE);
	
	return !mDone;
}


bool
FileFinder::Next(char* filename)
{
	if (mDone || mHandle == INVALID_HANDLE_VALUE)
		return false;
	
	// get the file name
	strcpy(filename, mFindData.cFileName);
		
	// find next file
	if (!FindNextFile( mHandle, &mFindData ))
	{
		// We've run into an error using _findnext()
		mDone = true;
	}
	
	return true;
}
