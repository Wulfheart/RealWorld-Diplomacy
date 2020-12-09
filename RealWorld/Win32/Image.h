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
#ifndef __ImageDefs__
#define __ImageDefs__

#include "DIBApi.h"
#include "MacTypes.h"

class Image
{
public:
	Image();
	~Image();
	bool				Initialize( char* filename );
	bool				Initialize( HDIB dib );

	inline bool			IsInitialized() { return mInitialized; }

	inline unsigned int GetWidth() { return mWidth; }
	inline unsigned int GetHeight() { return mHeight; }
	void				GetRect( Rect& rect );

	bool				Draw( Rect& rect );
	bool				Save( char* filename );

	inline HDIB			GetNative() { return mBitmap; }
	inline HPALETTE		GetPalette() { return mPalette; }

protected:
	bool		 mInitialized;
	HDIB		 mBitmap;
	HPALETTE	 mPalette;
	Rect		 mRect;
	unsigned int mWidth;
	unsigned int mHeight;
};

typedef Image* ImagePtr;

#endif	// __ImageDefs__