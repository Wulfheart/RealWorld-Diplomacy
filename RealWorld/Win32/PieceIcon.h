//===============================================================================
// @ PieceIcon.h
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

#ifndef __PieceIconDefs__
#define __PieceIconDefs__

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "MacTypes.h"

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

class PieceIcon
{
public:
	PieceIcon(char *name);
	~PieceIcon();

	void Draw(const Rect &iconRect, bool bw, bool defsize);
	void DrawGreyed(const Rect &iconRect, bool bw, bool defsize);
	
	inline bool IsInitialized() { return mInitialized; }
	inline unsigned int GetHeight() { return mHeight; }
	inline unsigned int GetWidth() { return mWidth; }

	static bool InitIconResources( char* path );
	static void ReleaseIconResources();
	
protected:
	bool		mInitialized;
	HICON 		mIcon;
	static char		mPath[256];
	unsigned int    mHeight;
	unsigned int    mWidth;
	
private:
	PieceIcon(const PieceIcon& icon);
	PieceIcon& operator=(const PieceIcon& icon);
};

#endif
