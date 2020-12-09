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
#include "Offscreen.h"
#include "Image.h"
#include "DIBAPI.h"
#include "MacTypes.h"
#include "ScrollableWindow.h"

Offscreen::Offscreen() : mWidth(0), mHeight(0), mInitialized(false)
{
	mDC = NULL;
	mPalette = NULL;
	mBits = NULL;
	mOldDC = NULL;
	mGrayScale = false;
	mDepth = 8;
}


Offscreen::~Offscreen()
{
	if (mDC != NULL)
	{
		DeleteDC(mDC);
		DeleteObject(mBits);
	}
	mDC = NULL;
	mBits = NULL;

	if (mPalette != NULL)
	{
		DeleteObject(mPalette);
	}
	mPalette = NULL;
	mInitialized = false;
}


bool Offscreen::Initialize( ScrollableWindow* window, Image* image )
{
	if (mInitialized)
		return false;

	// get the palette
	mPalette = image->GetPalette();

	HDC hdc = GetDC(window->GetNative());
	
	// create the offscreen buffers
	mDC = CreateCompatibleDC(hdc);
	mBits = CreateCompatibleBitmap(hdc, image->GetWidth(), image->GetHeight());
	SelectObject(mDC, mBits);
	SelectPalette(mDC, mPalette, FALSE);

	ReleaseDC(window->GetNative(), hdc);
	
	mWidth = image->GetWidth();
	mHeight = image->GetHeight();

	// assuming that all goes well
	mInitialized = true;

	return mInitialized;
}


bool				
Offscreen::SetAsCurrent()
{
	if (!mInitialized)
		return false;

    mOldDC = SetPort(mDC);
	return true;
}


bool				
Offscreen::RestorePrevious()
{
	if (!mInitialized)
		return false;

	SetPort(mOldDC);
	return true;
}


void				
Offscreen::Lock()
{
}

void				
Offscreen::Unlock()
{
}

// native event handlers
bool		
Offscreen::Update(const Rect& windowRect)
{	
	return false;
}

bool				
Offscreen::CopyToScrap()
{
	if (!mInitialized)
		return false;

	SetClipboardData( CF_BITMAP, mBits );
	return true;
}


Image*				
Offscreen::CreateImage()
{
	HDIB dib = (HDIB) DDBToDIB( mBits, BI_RGB, mPalette ); 
	Image* image = new Image();
	if (image != NULL)
		image->Initialize(dib);
	
	return image;
}

