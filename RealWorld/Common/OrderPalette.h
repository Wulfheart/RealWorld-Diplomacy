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
#ifndef __OrderPaletteDefs__
#define __OrderPaletteDefs__

#include "MacTypes.h"

class ScrollableWindow;

class OrderPalette
{
public:
	OrderPalette();
	~OrderPalette();

	void Draw();
	void Reset();
	void SetRect(const ScrollableWindow* theWindow);
	void Activate(ScrollableWindow* theWindow, bool activating);
	bool InPalette(Point where);
	short DoContent(const Point& where);
	short DoShiftPalette(bool left);

	inline short GetOrderMode() { return mOrderMode; }
	bool SetOrderMode( short mode );
	inline Rect& GetRect() { return mRect; }
	inline bool IsActive() { return mActive; }

protected:
	void DrawSeason();

	void DrawMovePalette();
	void DrawGreyMovePalette();
	void DrawAdjustPalette();
	void DrawGreyAdjustPalette();
	void DrawDislodgePalette();
	void DrawGreyDislodgePalette();

	void DoDislodgePalette(Point where);
	void DoAdjustPalette(Point where);
	void DoMovePalette(Point where);

	short		mOrderMode;
	bool	    mActive;
	Rect		mRect;
};

#endif	// __OrderPaletteDefs__
