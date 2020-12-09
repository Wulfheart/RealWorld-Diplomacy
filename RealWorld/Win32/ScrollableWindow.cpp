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
#include "ScrollableWindow.h"
#include "EventUtil.h"
#include "Global.h"
#include "Offscreen.h"
#include "Image.h"
#include "Resource.h"
#include "Util.h"
//#include "WindowUtil.h"

extern HINSTANCE gInstance;

ScrollableWindow::ScrollableWindow() : 
	mHwnd(NULL),
    mDC(NULL),
	mOffscreen(NULL), 
	mWindowDepth(0),
	mFirstTime(true), 
	mInitialized(false),
	mPaintCallback(NULL)
{
	memset(&mWindowRect, 0, sizeof(mWindowRect));
	mBorderX = 2*GetSystemMetrics(SM_CXSIZEFRAME);
	mBorderY = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) 
		+ 2*GetSystemMetrics(SM_CYSIZEFRAME);
}

ScrollableWindow::~ScrollableWindow()
{
    if (mInitialized)
    {
        ReleaseDC(mHwnd, mDC);
        mDC = NULL;
	    DestroyWindow(mHwnd);
	    mHwnd = NULL;
	    mInitialized = false;
    }
}

bool				
ScrollableWindow::Initialize( const char* title, int width, int height )
{
	if (mInitialized)
		return false;

	// Register the main application window class with Windows 95. 
	//			  This should be in creation of the main map display?
	WNDCLASSEX wc;
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC)WindowProc;       
	wc.cbClsExtra		= 0;                      
	wc.cbWndExtra		= 0;                      
	wc.hInstance		= gInstance;              
	wc.hIcon			= LoadIcon( gInstance, title ); 
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName		= title;
	wc.lpszClassName	= title;              
	wc.hIconSm			= (HICON) LoadImage(wc.hInstance, wc.lpszClassName,
							IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                    
	if ( !RegisterClassEx(&wc) ) return false;

	// create the map window
	mHwnd = CreateWindow( title,
					title,
					WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					width, height,
					NULL, NULL,
					gInstance, 
					NULL );

    // if can't allocate for some reason 
    if (mHwnd != NULL)
    {
        mDC = GetDC(mHwnd);
        SetPort(mDC);
	    mInitialized = TRUE;
    }

	return mInitialized;
}


void				
ScrollableWindow::Reset()
{
	unsigned int	width, height;
	int				x, y;
	Rect			windowRect;

	width = 0;	// doesn't matter what these are
	height = 0;
	OnSize(width, height);
	::GetWindowRect(mHwnd, &windowRect);
	
	// if the first time through, init window bounds 
	if (mFirstTime)
	{	
		mFirstTime = false;
		if (mWindowRect.bottom - mWindowRect.top == 0)
		{ 
			HDC hdc = ::GetDC((HWND)NULL);

			width = (unsigned int)::GetDeviceCaps(hdc, HORZRES);
			height = (unsigned int)::GetDeviceCaps(hdc, VERTRES) - 48;
			x = 1;
			y = 24;
			::ReleaseDC(NULL, hdc);

			mWindowRect.top = y;
			mWindowRect.left = x;
			mWindowRect.bottom = y+height;
			mWindowRect.right = x+width;
		}
		else
		{
//			width = mWindowRect.right - mWindowRect.left + 2*GetSystemMetrics(SM_CXSIZEFRAME);
//			height = mWindowRect.bottom - mWindowRect.top + GetSystemMetrics(SM_CYCAPTION) 
//					+ GetSystemMetrics(SM_CYMENU) + 2*GetSystemMetrics(SM_CYSIZEFRAME) - 24;  // -24 for start menu
			width = mWindowRect.right - mWindowRect.left;
			height = mWindowRect.bottom - mWindowRect.top; 
			x = mWindowRect.left;
			y = mWindowRect.top;
		}
	
		if (width > mOffscreen->GetWidth() + mBorderX)
			width = mOffscreen->GetWidth() + mBorderX;
		if (height > mOffscreen->GetHeight() + mBorderY)
			height = mOffscreen->GetHeight() + mBorderY;
		::SetWindowPos( mHwnd, 0, x, y, width, height, SWP_NOZORDER );
		
		ShowWindow(mHwnd, SW_SHOWDEFAULT);
	}
	// otherwise, need to reset window size 
	else
	{
		// check width and height
		HDC hdc = ::GetDC((HWND)NULL);
		width = (unsigned int)::GetDeviceCaps(hdc, HORZRES);
		height = (unsigned int)::GetDeviceCaps(hdc, VERTRES) - 48;
		::ReleaseDC(NULL, hdc);
		x = 1;
		y = 24;
//		width = windowRect.right - windowRect.left + 2*GetSystemMetrics(SM_CYSIZEFRAME);
//		height = windowRect.bottom - windowRect.top + GetSystemMetrics(SM_CYMENU) 
//				+ GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYSIZEFRAME) - 24; // -24 for start menu
		if (width > mOffscreen->GetWidth() + mBorderX)
			width = mOffscreen->GetWidth() + mBorderX;
		if (height > mOffscreen->GetHeight() + mBorderY)
			height = mOffscreen->GetHeight() + mBorderY;
//		::SetWindowPos( mHwnd, 0, 0, 0, width, height, SWP_NOZORDER|SWP_NOMOVE );
		::SetWindowPos( mHwnd, 0, x, y, width, height, SWP_NOZORDER );
		::GetWindowRect(mHwnd, &mWindowRect);

		// if by resizing it we moved it too far off the screen
		if ((int)width + mWindowRect.left < 10)
		{
			SetWindowPos(mHwnd, 0, 5, mWindowRect.top, width, height, SWP_NOZORDER|SWP_NOSIZE );			
			::GetWindowRect(mHwnd, &mWindowRect);
		}
        InvalidateRect(mHwnd, NULL, FALSE);
 	}
}

// native event handlers
UINT		
ScrollableWindow::OnSetPalette(HWND theWindow, bool background)
{
	UINT nColorsChanged = 0;
	if (mOffscreen->GetPalette() != NULL)
	{
		// All views but one should be a background palette.
		// Only on QUERYNEWPALETTE message the palette is on the
		// foreground.
		HDC hdc = ::GetDC(theWindow);
		HPALETTE oldPalette = SelectPalette(hdc, mOffscreen->GetPalette(), background);

		nColorsChanged = RealizePalette(hdc);
		if (nColorsChanged > 0)
		{
			InvalidateRect(theWindow, NULL, FALSE);
		}
		SelectPalette(hdc, oldPalette, TRUE);

		if (oldPalette != NULL)
		{
			nColorsChanged += RealizePalette(hdc);
			if (nColorsChanged > 0)
			{
			    InvalidateRect(theWindow, NULL, FALSE);
			//	pDoc->UpdateAllViews(NULL);
			}
		}

//		else
//		{
//			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
//		}
		::ReleaseDC(theWindow, hdc);
	}
	return nColorsChanged;
}

bool		
ScrollableWindow::OnUpdate()
{
//NEED THIS?	(void) OnSetPalette();
	Paint(NULL);
	return true;
}

bool		
ScrollableWindow::OnSize( int w, int h )
{
	RECT windowRect;
	int  maxx = mOffscreen->GetWidth() + mBorderX;
	int  maxy = mOffscreen->GetHeight() + mBorderY;

	// Get the size of the total window.
    // Note, that we do not take the size of the client area,
    // because that excludes the scrollbars and the scrollbars
    // is something we have to recalculate.
	::GetWindowRect(mHwnd, &windowRect);

	w = windowRect.right - windowRect.left;
	h = windowRect.bottom - windowRect.top;

	// if window wide enough
	if (maxx-w <= 0 && maxy-h <= 0)
		::SetScrollRange(mHwnd, SB_HORZ, 0, 0, TRUE);
	// window not wide nor tall enough
	else
		::SetScrollRange(mHwnd, SB_HORZ, 0, maxx - w + GetSystemMetrics(SM_CXVSCROLL), TRUE);

	// if window tall enough
	if (maxy-h <= 0 && maxx-w <= 0)
		::SetScrollRange(mHwnd, SB_VERT, 0, 0, TRUE);
	// window not tall nor wide enough
	else
		::SetScrollRange(mHwnd, SB_VERT, 0, maxy - h + GetSystemMetrics(SM_CYHSCROLL), TRUE);

	return true;
}

bool		
ScrollableWindow::OnGrow( LPMINMAXINFO minfo )
{
	int  maxx = mOffscreen->GetWidth() + mBorderX;
	int  maxy = mOffscreen->GetHeight() + mBorderY;

	if (minfo != NULL)
	{
		minfo->ptMaxSize.x = maxx;
		minfo->ptMaxSize.y = maxy;
		minfo->ptMinTrackSize.x = 200;
		minfo->ptMinTrackSize.y = 300;
		minfo->ptMaxTrackSize.x = maxx;
		minfo->ptMaxTrackSize.y = maxy;
	}

	return true;
}


bool		
ScrollableWindow::OnDrag( int h, int v )
{
	return true;
}

bool		
ScrollableWindow::OnActivate( bool activating )
{
    return true;
}

bool		
ScrollableWindow::OnInContent( int h, int v )
{
	return false;
}


void		
ScrollableWindow::Paint(Rect* rect)
{
	RECT portRect;
	::GetWindowRect(mHwnd, &portRect);
	ScreenToClient(mHwnd, (POINT *)&portRect);
	ScreenToClient(mHwnd, (POINT *)&portRect.right);

	PaintBody(rect);
	if (mPaintCallback)
		mPaintCallback(rect);

	ValidateRect(mHwnd, &portRect);	
}

void		
ScrollableWindow::PaintBody(Rect* rect)
{
	long hdiff, vdiff;
	RECT scrollRect, windowRect;
	HDC currentPort;
	GetPort(&currentPort);
	HPALETTE oldPalette = NULL;
	
	hdiff = GetScrollPos(mHwnd, SB_HORZ);
	if (hdiff < 0)
		hdiff = 0;
		
	vdiff = GetScrollPos(mHwnd, SB_VERT);
	if (vdiff < 0)
		vdiff = 0;

	if (rect == NULL)
	{
		::GetWindowRect(mHwnd, &windowRect);
		ScreenToClient(mHwnd, (POINT *)&windowRect);
		ScreenToClient(mHwnd, (POINT *)&windowRect.right);
	}
	else
	{
		windowRect = *rect;
		OffsetRect(&windowRect, -hdiff, -vdiff);
	}
	
	scrollRect = windowRect;

	OffsetRect(&scrollRect, hdiff, vdiff);
	PenNormal();

	mOffscreen->Lock();
	if (mOffscreen->GetPalette() != NULL)
	{
		oldPalette = SelectPalette(currentPort, mOffscreen->GetPalette(), TRUE);

		RealizePalette(currentPort);
	}
	
	BitBlt(currentPort, windowRect.left, windowRect.top, 
		windowRect.right-windowRect.left, windowRect.bottom-windowRect.top,
		mOffscreen->GetNative(), scrollRect.left, scrollRect.top, SRCCOPY);

	if (oldPalette != NULL)
	{
		SelectPalette(currentPort, oldPalette, TRUE);
		RealizePalette(currentPort);
	}

	mOffscreen->Unlock();
}

void
ScrollableWindow::Refresh( Rect* clipRect )
{
	if (clipRect != NULL)
		::InvalidateRect(mHwnd, clipRect, FALSE);
	else
	{
		::GetWindowRect(mHwnd, &mWindowRect);
		::InvalidateRect(mHwnd, &mWindowRect, FALSE);
	}

	BeginUpdate(mHwnd);
	
	Paint(clipRect);
	
	EndUpdate(mHwnd);
}


void				
ScrollableWindow::TrackVBar( int partCode, int value )
{
	RECT 	  windowRect, scrollRect;
	int oldValue;

	// going to assume that only front window can access this	
	::GetWindowRect(mHwnd, &windowRect);
	::ScreenToClient(mHwnd, (POINT *)&windowRect);
	::ScreenToClient(mHwnd, (POINT *)&windowRect.right);
	scrollRect = windowRect;
	
	oldValue = ::GetScrollPos(mHwnd, SB_VERT);
	switch (partCode) 
	{
		case SB_LINEUP:
			::SetScrollPos(mHwnd, SB_VERT, oldValue-5, TRUE);
			scrollRect.bottom = scrollRect.top+5;
			break;
			
		case SB_LINEDOWN:
			::SetScrollPos(mHwnd, SB_VERT, oldValue+5, TRUE);
			scrollRect.top = scrollRect.bottom-5;
			break;
			
		case SB_PAGEUP:
			::SetScrollPos(mHwnd, SB_VERT, oldValue-40, TRUE);
			scrollRect.bottom = scrollRect.top+40;
			break;
			
		case SB_PAGEDOWN:
			::SetScrollPos(mHwnd, SB_VERT, oldValue+40, TRUE);
			scrollRect.top = scrollRect.bottom-40;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			::SetScrollPos(mHwnd, SB_VERT, value, TRUE);
			// what to set scroll rect here?
			break;

		default:
			break;		
	}
	
	::InvalidateRect(mHwnd, &windowRect, FALSE);

    // Do not paint here, but wait on paint message. This
    // prevents flooding of scroll messages.
}


void				
ScrollableWindow::TrackHBar( int partCode, int value )
{
	RECT 	windowRect, scrollRect;
	int		oldValue;

	// going to assume that only front window can access this 	
	::GetWindowRect(mHwnd, &windowRect);
	::ScreenToClient(mHwnd, (POINT *)&windowRect);
	::ScreenToClient(mHwnd, (POINT *)&windowRect.right);
	scrollRect = windowRect;
	
	oldValue = ::GetScrollPos(mHwnd, SB_HORZ);
	switch (partCode) 
	{
		case SB_LINEUP:
			::SetScrollPos(mHwnd, SB_HORZ, oldValue-5, TRUE);
			scrollRect.bottom = scrollRect.top+5;
			break;
			
		case SB_LINEDOWN:
			::SetScrollPos(mHwnd, SB_HORZ, oldValue+5, TRUE);
			scrollRect.top = scrollRect.bottom-5;
			break;
			
		case SB_PAGEUP:
			::SetScrollPos(mHwnd, SB_HORZ, oldValue-40, TRUE);
			scrollRect.bottom = scrollRect.top+40;
			break;
			
		case SB_PAGEDOWN:
			::SetScrollPos(mHwnd, SB_HORZ, oldValue+40, TRUE);
			scrollRect.top = scrollRect.bottom-40;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			::SetScrollPos(mHwnd, SB_HORZ, value, TRUE);
			// what to set scroll rect here?
			break;

		default:
			break;		
	}
	
	::InvalidateRect(mHwnd, &windowRect, FALSE);

    // Do not paint here, but wait on paint message. This
    // prevents flooding of scroll messages.
}


bool				
ScrollableWindow::Show( bool show )
{
//	if (show)
//	{
//		SelectWindow(mWindow);
//		ShowWindow(mWindow);
//	}
//	else
//	{
//		HideWindow(mWindow);
//	} 
	return true;
}

void				
ScrollableWindow::SetTitle( char* title )
{
	char windowTitle[256];
	strcpy(windowTitle, kAppName);
	strcat(windowTitle, ": ");
	strcat(windowTitle, title);
	SetWindowText( mHwnd, windowTitle );
}

void				
ScrollableWindow::SetOffscreen( Offscreen* offscreen )
{
	mOffscreen = offscreen;
}

void 
ScrollableWindow::SetAsCurrent()
{
	SetPort(mDC);
}

void 
ScrollableWindow::ClipToScrollbars()
{
	// don't need to worry about this in Windows
}

void 
ScrollableWindow::ClipToFull()
{
	// don't need to worry about this in Windows
}


void 
ScrollableWindow::CopyToScrap()
{
	OpenClipboard( mHwnd );
	EmptyClipboard();
	(void) mOffscreen->CopyToScrap();
	CloseClipboard();
}

void
ScrollableWindow::SaveToFile(char* prompt, char* filename)
{
	char szFilter[] = "BMP Files (*.BMP)\0*.bmp\0"\
						"All Files (*.*)\0*.*\0\0";

	OPENFILENAME ofn3 = 
	{
		sizeof(OPENFILENAME),
		NULL,
		NULL,
		szFilter, 
		NULL,
		0,
		0,
		NULL,
		_MAX_PATH,
		NULL,
		_MAX_FNAME + _MAX_EXT,
		NULL,
		NULL,
		0,
		0,
		0,
		"bmp",
		0L,
		NULL,
		NULL
	};

	ofn3.hwndOwner = mHwnd;
	ofn3.lpstrFile = filename;
	ofn3.lpstrFileTitle = prompt;
	ofn3.Flags = OFN_OVERWRITEPROMPT;
	
	if (!GetSaveFileName(&ofn3))
		return;
		
	ImagePtr image = mOffscreen->CreateImage();
	if (!image->Save(ofn3.lpstrFile))
		ErrorString("Unable to save BMP");
	delete image;
}

void
ScrollableWindow::SaveToPngFile(char* prompt, char* filename)
{
	char szFilter[] = "PNG Files (*.PNG)\0*.png\0"\
						"All Files (*.*)\0*.*\0\0";

	OPENFILENAME ofn3 = 
	{
		sizeof(OPENFILENAME),
		NULL,
		NULL,
		szFilter, 
		NULL,
		0,
		0,
		NULL,
		_MAX_PATH,
		NULL,
		_MAX_FNAME + _MAX_EXT,
		NULL,
		NULL,
		0,
		0,
		0,
		"png",
		0L,
		NULL,
		NULL
	};

	ofn3.hwndOwner = mHwnd;
	ofn3.lpstrFile = filename;
	ofn3.lpstrFileTitle = prompt;
	ofn3.Flags = OFN_OVERWRITEPROMPT;
	
	if (!GetSaveFileName(&ofn3))
		return;
		
	ImagePtr image = mOffscreen->CreateImage();
	if (!image->Save(ofn3.lpstrFile))
		ErrorString("Unable to save BMP");
	delete image;
}


int
ScrollableWindow::GetHScrollHeight() const
{
//	int gunk = GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYSIZEFRAME);
	Rect portRect;

	::GetClientRect(mHwnd, &portRect);

/*	::GetWindowRect(mHwnd, &portRect);
	if (portRect.right - portRect.left < (int)mOffscreen->GetWidth() + mBorderX
		|| portRect.bottom - portRect.top < (int)mOffscreen->GetHeight() + mBorderY )
		gunk += GetSystemMetrics(SM_CYHSCROLL);
*/	
	return (portRect.bottom - portRect.top);// - gunk);
}


int	
ScrollableWindow::GetHorizVal() const
{
	return ::GetScrollPos(mHwnd, SB_HORZ);
}

int	
ScrollableWindow::GetVertVal() const
{
	return ::GetScrollPos(mHwnd, SB_VERT);
}
