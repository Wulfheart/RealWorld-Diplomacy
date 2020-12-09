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
#ifndef __ScrollableWindowDefs__
#define __ScrollableWindowDefs__

#include <windows.h>
#include "MacTypes.h"

class Offscreen;

class ScrollableWindow
{
public:
	ScrollableWindow();
	~ScrollableWindow();
	bool				Initialize( const char* title, int width, int height );

	inline bool			IsInitialized() { return mInitialized; }

	// native event handlers
	UINT		OnSetPalette( HWND theWindow, bool background );
	bool		OnUpdate();
	bool		OnSize( int w, int h );
	bool		OnGrow( LPMINMAXINFO event );
	bool		OnDrag( int h, int v );
	bool		OnActivate( bool activating );
	bool		OnInContent( int h, int v );

	// special routines
	void		Reset();

	// draw routines
	void				Paint(Rect* rect);
	void				PaintBody(Rect* rect);
	inline void			SetPaintCallback( void (*callback) (Rect* rect) )
		{ mPaintCallback = callback; }
	void				Refresh(Rect* rect);

	// scroll bar tracking
	void				TrackVBar( int part, int value );
	void				TrackHBar( int part, int value );

	inline bool			IsWindow( HWND window ) { return (mHwnd == window); }

	bool				Show( bool show );
	void				SetTitle( char* title );

	inline HWND			GetNative() { return mHwnd; }
	inline Offscreen*	GetOffscreen() { return mOffscreen; }
	void				SetOffscreen( Offscreen* offscreen );

	int					GetHorizVal() const;
	int					GetVertVal() const;
	
	void				ClipToScrollbars();
	void				ClipToFull();

	inline void			SetWindowRect( const Rect& rect ) { mWindowRect = rect; }
	inline void			GetWindowRect( Rect& rect ) { ::GetWindowRect(mHwnd, &rect); }

	void				SetAsCurrent();
	void				CopyToScrap();
	void				SaveToFile(char* title, char* defaultName);
	void				SaveToPngFile(char* title, char* defaultName);

	int					GetHScrollHeight() const;

protected:
	bool		 mInitialized;
	HWND		 mHwnd;
    HDC          mDC;
	Offscreen*	 mOffscreen;
	Rect		 mWindowRect;
	short		 mWindowDepth;
	bool		 mFirstTime;
	int			 mBorderX;
	int			 mBorderY;
	void		 (*mPaintCallback) (Rect* rect);
};

#endif	// __ScrollableWindowDefs__