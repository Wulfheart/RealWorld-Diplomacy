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
#include "Image.h"
#include "Util.h"

static char* errorStrings[] =
{
	"",
	": can't find file %s",
	": %s is not a BMP",
	": %s is not in 8-bit format",
	": %s not valid BMP"
};

Image::Image() : mWidth(0), mHeight(0), mInitialized(false)
{
	mBitmap = NULL;
	mPalette = NULL;
	mRect.top = 0;
	mRect.bottom = 0;
	mRect.left = 0;
	mRect.right = 0;
}


Image::~Image()
{
	if (mBitmap != NULL)
		::GlobalFree((HGLOBAL) mBitmap);
	mBitmap = NULL;

	if (mPalette != NULL)
	{
		DeleteObject(mPalette);
	}
	mPalette = NULL;

	mInitialized = false;
}


bool Image::Initialize(char* filename)
{
	if (mInitialized)
		return false;

	mBitmap = ReadDIBFile(filename);
	if (mBitmap == NULL)
	{
		if (gDibError < 5)
			sprintf(gErrorString, errorStrings[gDibError], filename);
		else
			strcpy(gErrorString, filename);
		gErrNo = errBadPict;
		return false;
	}

	// get the palette
	if (!CreateDIBPalette(mBitmap, mPalette))
	{
		::GlobalFree((HGLOBAL) mBitmap);
		strcpy(gErrorString, filename);
		gErrNo = errBadPict;
		return false;
	}

	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) mBitmap);

	mWidth = DIBWidth(lpDIB);
	mHeight = DIBHeight(lpDIB);
	mRect.right = mWidth;
	mRect.bottom = mHeight;

	::GlobalUnlock((HGLOBAL) mBitmap);

	mInitialized = true;

	return true;
}


bool Image::Initialize( HDIB dib )
{
	if (mInitialized)
		return false;

	mBitmap = dib;
	if (mBitmap != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) mBitmap);

		mWidth = DIBWidth(lpDIB);
		mHeight = DIBHeight(lpDIB);
		mRect.right = mWidth;
		mRect.bottom = mHeight;

		::GlobalUnlock((HGLOBAL) mBitmap);

		mInitialized = true;
	}

	return mInitialized;
}


void Image::GetRect( Rect& rect )
{
	rect.left = 0;
	rect.top = 0;
	rect.right = GetWidth();
	rect.bottom = GetHeight();
}


bool 
Image::Save( char* filename )
{
	if (!mInitialized)
		return false;

	if (SaveDIB(mBitmap, filename))
		return true;
	else
		return false;
}


bool
Image::Draw( Rect& rect )
{
	if (!mInitialized)
		return false;
	HDC port;
	GetPort(&port);

	PaintDIB(port, &rect, mBitmap, &mRect, mPalette);
	return true;
}


