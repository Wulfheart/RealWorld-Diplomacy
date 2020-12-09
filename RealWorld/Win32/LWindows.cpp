//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// Modifications (C) 2005 by Dirk Brüggemann
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
#include <string.h>
#include "LWindows.h"
#include "Util.h"
#include "InfoDisplay.h"
#include "ListUtil.h"
#include "OrdersDisplay.h"
#include "Resource.h"
#include "ScrollableWindow.h"
#include "StatusDisplay.h"
#include "Global.h"

#define FORWARD_WM_SETFONT(hwnd, hfont, fRedraw, fn) \
    (void)(fn)((hwnd), WM_SETFONT, (WPARAM)(HFONT)(hfont), (LPARAM)(BOOL)(fRedraw))
#define     SetWindowFont(hwnd, hfont, fRedraw) FORWARD_WM_SETFONT((hwnd), (hfont), (fRedraw), SNDMSG)

const int kMinHeight = 100;
const int kInitHeight = 495;

bool LWindow::mClassCreated = false;

extern HINSTANCE gInstance;

/* create new list window */
LWindow*
LWindow::GetNewLWindow(ScrollableWindow* parent, char *title)
{
	HWND window;
	LWindow* lWindow;

	// Register the order/error window class with Windows 95. 
	//			  This should be in creation of the main map display?
	if (!mClassCreated)
	{
		WNDCLASSEX wc;
		wc.cbSize			= sizeof(WNDCLASSEX);
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)LWindowProc;       
		wc.cbClsExtra		= 0;                      
		wc.cbWndExtra		= 0;                      
		wc.hInstance		= gInstance;              
		wc.hIcon			= LoadIcon( gInstance, kAppName ); 
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= "ListWindow";              
		wc.hIconSm			= (HICON) LoadImage(wc.hInstance, wc.lpszClassName,
								IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                    
		if ( !RegisterClassEx(&wc) ) 
			return NULL;

		mClassCreated = true;
	}

	// create the window
	window = CreateWindow( "ListWindow",
					title,
					WS_OVERLAPPEDWINDOW,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					100, kInitHeight,
					parent->GetNative(), NULL, 
					gInstance, 
					NULL);

	// if can't allocate for some reason 
	if (window == NULL)
	{
		gErrNo = errMem;
		return NULL;
	}

	lWindow = new LWindow();
	lWindow->mWindow = window;

	return lWindow;

} /* GetNewLWindow */


//----------------------------------------------------------------------------
// @ LWindowProc()
// ---------------------------------------------------------------------------
// Standard windows function to handle list window events.
//----------------------------------------------------------------------------
LRESULT CALLBACK
LWindow::LWindowProc( const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam )
{
	static TEXTMETRIC tm;
	HWND		hwndList;
	HDC dc;
	RECT windRect;
	int code = 0;

	switch ( uMsg ) 
	{
	case WM_CREATE:
		// get our listbox
		GetWindowRect(hWnd, &windRect);
		hwndList = CreateWindow("listbox", NULL,
				WS_CHILDWINDOW | WS_VISIBLE | LBS_NOTIFY | LBS_EXTENDEDSEL | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
				0, 0,
				tm.tmAveCharWidth * LINEWIDTH + GetSystemMetrics(SM_CXVSCROLL), 
				windRect.bottom-windRect.top,
				hWnd, NULL, 
				(HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE), 
				NULL);
		SetWindowFont(hwndList, GetStockObject(ANSI_FIXED_FONT), TRUE);
		dc = GetDC(hwndList);
		SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
		GetTextMetrics(dc, &tm);
		ReleaseDC(hwndList, dc);

		// store the list handle into the userdata
		SetWindowLong(hWnd, GWL_USERDATA, (LPARAM) hwndList);

		// resize the window to fit the listbox
		SetWindowPos( hWnd, 0, 0, 0, 
			tm.tmAveCharWidth * LINEWIDTH + GetSystemMetrics(SM_CXVSCROLL), 
			windRect.bottom-windRect.top, SWP_NOZORDER|SWP_NOMOVE );

		break;

	case WM_CLOSE:
		ShowWindow( hWnd, SW_HIDE );
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO minfo = (LPMINMAXINFO) lParam;
			minfo->ptMaxSize.x = tm.tmAveCharWidth * LINEWIDTH + GetSystemMetrics(SM_CXVSCROLL);
			minfo->ptMaxSize.y = MaxLWindowHeight(hWnd);
			minfo->ptMinTrackSize.x = tm.tmAveCharWidth * LINEWIDTH + GetSystemMetrics(SM_CXVSCROLL);
			minfo->ptMinTrackSize.y = kMinHeight;
			minfo->ptMaxTrackSize.x = tm.tmAveCharWidth * LINEWIDTH + GetSystemMetrics(SM_CXVSCROLL);
			minfo->ptMaxTrackSize.y = MaxLWindowHeight(hWnd);
		}
		break;

	case WM_SIZE:
		SizeLWindow(hWnd, LOWORD(lParam), HIWORD(lParam));
		
//	case WM_PAINT:
//		HandlePaint(hWnd, wParam, lParam);
//		break;

	case WM_COMMAND:
		code = HIWORD(wParam);
		if (code == LBN_SELCHANGE)
		{
			if (hWnd == ordersWindow->mWindow)
				errorWindow->ClearSelections();
			else if (hWnd == errorWindow->mWindow)
				ordersWindow->ClearSelections();
			ClearStatusSelections();
			ClearInfoSelection();
			// drop through
		}

	default:
		return(DefWindowProc( hWnd, uMsg, wParam, lParam ));
   }
   return( FALSE );
}	// End of LWindowProc()


LWindow::LWindow()
{
	mWindow = NULL;
}

LWindow::~LWindow()
{
	Close();
}


/* close window */
void		
LWindow::Close()
{	
	if (mWindow == NULL)
		return;
		
	HWND list = (HWND) GetWindowLong(mWindow, GWL_USERDATA);
	if (list != NULL)
		DestroyWindow(list);
	DestroyWindow(mWindow);
	mWindow = NULL;
}

void
LWindow::ClearList()
{
	HWND list = (HWND) GetWindowLong(mWindow, GWL_USERDATA);
	SendMessage(list, LB_RESETCONTENT, 0, 0);
}

int
LWindow::AddLine(char *line)
{
	return AddListLine((HWND) GetWindowLong(mWindow, GWL_USERDATA), line);
}

void
LWindow::SetLine(char *line, int index)
{
	HWND list = (HWND) GetWindowLong(mWindow, GWL_USERDATA);
	SendMessage(list, WM_SETREDRAW, FALSE, 0);
	SendMessage(list, LB_DELETESTRING, index-1, 0);
	SendMessage(list, LB_INSERTSTRING, index-1, (LPARAM) line);
	SendMessage(list, WM_SETREDRAW, TRUE, 0);
}

void
LWindow::SelectLine(int index)
{
	ClearSelections();
	HWND list = (HWND) GetWindowLong(mWindow, GWL_USERDATA);
	SendMessage(list, LB_SETCURSEL, index-1, 0);
}

void 
LWindow::ClearSelections()
{
	ClearLWindowSelections(mWindow);
}

void 
LWindow::ClearLWindowSelections(HWND theWindow)
{
	HWND list = (HWND) GetWindowLong(theWindow, GWL_USERDATA);
	ClearListSelections(list);
}

void 
LWindow::CopySelection()
{
	HWND list = (HWND) GetWindowLong(mWindow, GWL_USERDATA);
	CopyListSelection(list);
}

bool LWindow::IsSelection()
{
	HWND list = (HWND) GetWindowLong(mWindow, GWL_USERDATA);
	return IsListSelection(list);
}

int
LWindow::MaxHeight()
{
	return MaxLWindowHeight(mWindow);	
}

int
LWindow::MaxLWindowHeight(HWND theWindow)
{
	HWND list = (HWND) GetWindowLong(theWindow, GWL_USERDATA);
	if (list == NULL)
		return 480;
	
	static TEXTMETRIC fStatus;
	HDC dc;

	dc = GetDC(list);
	SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(dc, &fStatus);
	ReleaseDC(list, dc);

	int numCells = SendMessage(list, LB_GETCOUNT, 0, 0);	
	int height = numCells*(fStatus.tmAscent + fStatus.tmDescent + fStatus.tmExternalLeading);

	height += GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYSIZEFRAME);
	if (height < kInitHeight)
		height = kInitHeight;

	return height;	
}

void
LWindow::Size(int w, int h)
{
	SizeLWindow(mWindow, w, h);
}

void
LWindow::SizeLWindow(HWND theWindow, int w, int h)
{
	// resize the listbox to fit the window
	HWND list = (HWND) GetWindowLong(theWindow, GWL_USERDATA);
	SetWindowPos( list, 0, 0, 0, w, h, SWP_NOZORDER|SWP_NOMOVE );
}

bool
LWindow::IsWindow( HWND theWindow )
{
	return (mWindow == theWindow);
}

void LWindow::GetRect( Rect* rect )
{
	GetWindowRect( mWindow, rect );
}

void LWindow::SetPosition( int x, int y, int w, int h )
{
	SetWindowPos( mWindow, 0, x, y, w, h, SWP_NOZORDER );
}

void LWindow::Show( bool show )
{
	if (show)
	{
		ShowWindow(mWindow, SW_SHOWDEFAULT);
		UpdateWindow(mWindow);				
	}
	else
	{
		ShowWindow(mWindow, SW_HIDE);
	}
}

