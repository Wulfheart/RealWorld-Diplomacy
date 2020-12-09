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
#ifndef __LWindowsDefs__
#define __LWindowsDefs__

#include "MacTypes.h"

class ScrollableWindow;

class LWindow
{
public:
	static LWindow* GetNewLWindow(ScrollableWindow* parent, char* title);

	LWindow();
	~LWindow();
	void Close();
	void ClearList();
	int  AddLine( char* line );
	void SetLine( char* line, int index );
	void SelectLine( int index );
	void ClearSelections();
	void CopySelection();
	bool IsSelection();
	void Size( int w, int h );
	int  MaxHeight();
	void Draw();
/*CONVERT	void InContent(const EventRecord* event);
	void Activate(int activate);
	void Grow(EventRecord *event);
*/	
	bool IsWindow(HWND window);
	void GetRect(Rect* rect);
	void SetPosition( int x, int y, int w, int h );
	void Show(bool show);

protected:
	HWND mWindow;

	// utility variables/functions
	static bool mClassCreated;
	static LRESULT CALLBACK	LWindowProc( const HWND hWnd, const UINT uMsg, 
										 const WPARAM wParam, const LPARAM lParam );
	static int MaxLWindowHeight(HWND hWnd);
	static void SizeLWindow(HWND theWindow, int w, int h);
	static void ClearLWindowSelections(HWND theWindow);
};

typedef LWindow* LWindowPtr;

#endif