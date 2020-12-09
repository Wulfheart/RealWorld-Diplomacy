//===============================================================================
// @ PieceIcon.cpp
// ------------------------------------------------------------------------------
// Routines for loading icons for pieces
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
// 00-Jan-19	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "PieceIcon.h"
#include <string.h>
#include "Global.h"
#include "FileFinder.h"

char PieceIcon::mPath[256] = "";

//----------------------------------------------------------------------------
//-- Functions ---------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// @ PieceIcon::PieceIcon()
//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
PieceIcon::PieceIcon(char* name) :
	mInitialized(false)
{
	char path[256];
	strcpy(path, mPath);
	strcat(path, "\\");
	strcat(path, name);
	strcat(path, ".ico");
	mIcon = (HICON) LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	if (mIcon == NULL)
	{
		return;
	}
	ICONINFO info;
	GetIconInfo( mIcon, &info );
	mWidth = 2*info.xHotspot;
	mHeight = 2*info.yHotspot;
	mInitialized = true;
	
} 	// End of PieceIcon::PieceIcon()

//----------------------------------------------------------------------------
// @ PieceIcon::PieceIcon()
//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
PieceIcon::~PieceIcon() 
{
	DestroyIcon(mIcon);
	mInitialized = false;
	
} 	// End of PieceIcon::PieceIcon()


//----------------------------------------------------------------------------
// @ PieceIcon::Draw()
//----------------------------------------------------------------------------
// Draw plain old icon
//----------------------------------------------------------------------------
void
PieceIcon::Draw(const Rect &iconRect, bool bw, bool defsize)
{
	// need to check for bw
	HDC thePort;
	
	GetPort(&thePort);

	if (defsize)
	{
		DrawIconEx(thePort, iconRect.left, iconRect.top, mIcon, 0, 0,
				0, NULL, DI_NORMAL|DI_DEFAULTSIZE);
	}
	else
	{
		DrawIconEx(thePort, iconRect.left, iconRect.top, mIcon, 0, 0,
				0, NULL, DI_NORMAL);
	}
		
} 	// End of PieceIcon::Draw()


//----------------------------------------------------------------------------
// @ PieceIcon::DrawGreyed()
//----------------------------------------------------------------------------
// Draw greyed icon
//----------------------------------------------------------------------------
void
PieceIcon::DrawGreyed(const Rect &iconRect, bool bw, bool defsize)
{
	// need to check for bw

	HDC thePort;
	HDC hdcMem;
	HBRUSH oldBrush;
	ICONINFO info;
    HBITMAP oldBitmap;
	int nWidth;
	int nHeight;
	if (defsize)
	{
		nWidth = 32;
		nHeight = 32;
	}
	else
	{
		nWidth = mWidth;
		nHeight = mHeight;
	}
	GetPort(&thePort);
	hdcMem = CreateCompatibleDC( thePort );
	oldBrush = (HBRUSH) SelectObject(thePort, CreateSolidBrush(0x7F7F7F));
	GetIconInfo( mIcon, &info );
	oldBitmap = (HBITMAP) SelectObject( hdcMem, info.hbmMask );
	BitBlt( thePort, iconRect.left, iconRect.top, nWidth, nHeight,
			hdcMem, 0, 0, SRCAND );
	SelectObject( hdcMem, info.hbmColor );
	BitBlt( thePort, iconRect.left, iconRect.top, nWidth, nHeight,
			hdcMem, 0, 0, 0x620786 );
    SelectObject( hdcMem, oldBitmap);
    DeleteObject(info.hbmMask);
    DeleteObject(info.hbmColor);
	DeleteObject(SelectObject(thePort, oldBrush));
	DeleteDC( hdcMem );

} 	// End of PieceIcon::DrawGreyed()


//----------------------------------------------------------------------------
// @ PieceIcon::InitIconResources()
//----------------------------------------------------------------------------
// Initialize icon path
//----------------------------------------------------------------------------
bool
PieceIcon::InitIconResources( char* path )
{
	FileFinder finder;
	if (!finder.Start(path))
		return false;

	strcpy(mPath, path);

	return true;
	
} 	// End of PieceIcon::InitIconResources()
	
	
//----------------------------------------------------------------------------
// @ PieceIcon::ReleaseIconResources()
//----------------------------------------------------------------------------
// Release icon path
//----------------------------------------------------------------------------
void
PieceIcon::ReleaseIconResources()
{
	strcpy(mPath, "");

} 	// End of PieceIcon::ReleaseIconResources()
