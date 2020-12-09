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
#ifndef __OffscreenDefs__
#define __OffscreenDefs__

#include <windows.h>
#include "MacTypes.h"

class Image;
class ScrollableWindow;

class Offscreen
{
public:
	Offscreen();
	~Offscreen();
	bool Initialize( ScrollableWindow* window, Image* image );

	inline bool			IsInitialized() { return mInitialized; }

	inline unsigned int GetWidth() { return mWidth; }
	inline unsigned int GetHeight() { return mHeight; }
	inline bool			IsMonochrome() { return (mDepth < 4); }
	inline bool			IsGrayScale() { return (mGrayScale); }

	bool 				Update(const Rect& windowRect);
	
	void				Lock();
	void				Unlock();

	bool				SetAsCurrent();
	bool				RestorePrevious();
	bool				CopyToScrap();

	Image*				CreateImage();

	inline HDC			GetNative() { return mDC; }
	inline HPALETTE		GetPalette() { return mPalette; }

protected:
	bool		 mInitialized;
	HDC			 mDC;
	HBITMAP		 mBits;
	HPALETTE	 mPalette;
	HDC			 mOldDC;
	unsigned int mWidth;
	unsigned int mHeight;
	short		 mDepth;
	bool		 mGrayScale;
};

typedef Offscreen* OffscreenPtr;

#endif	// __OffscreenDefs__
