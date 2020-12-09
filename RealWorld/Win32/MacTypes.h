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
#ifndef _MACTYPES_
#define _MACTYPES_

#include <windows.h>
#include <windowsx.h>

//---------------------------------------------------------------------
// Defines, typedefs, etc.
//---------------------------------------------------------------------
typedef char FSSpec[256];
typedef RECT Rect;

typedef struct
{
	short h, v;
} Point;

typedef char *SFTypeList;

typedef struct
{
	int red,green,blue;
} RGBColor;

enum Pattern {  kRandom = 0, kThinDiag, kHash, kGray, kQuilt, kSparse, kDiag, kThinHoriz, 
				kHoriz, kVert, kStripe, kFlood, kBlack, kWhite
			  };
			  
typedef unsigned char Bitmap[8];

struct Poly
{
   int polySize;
   RECT polyBBox;
   POINT polyPoints[128];
};
typedef Poly *PolyPtr;
typedef PolyPtr PolyHandle;

enum textFace { normal, bold, italic };

// quickdraw global type
typedef struct
{
	Pattern black, gray, white;
	Pattern currentPattern;
	short penType;
	short brushType;
	bool hatched;
	COLORREF foregroundColor;
	COLORREF backgroundColor;
	short penWidth;
	short penHeight;
	HBRUSH foregroundBrush;
	short fontType;
	short fontSize;
	textFace face;
	int currentx, currenty;
} QDType;


//---------------------------------------------------------------------
// External variables
//---------------------------------------------------------------------

extern QDType qd;
extern Bitmap bitmaps[14];

//---------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------

inline void ExitToShell() { ExitProcess(0); }
inline unsigned char* CtoPstr(char* s) { return (unsigned char*) s; }
inline char* PtoCstr(unsigned char* s) { return (char*) s; }
void SysBeep(short duration);

// port management
HDC SetPort(HDC port);
void GetPort(HDC* port);

// window updates
// these need to be paired together to work properly
#define BeginUpdate(window) \
	PAINTSTRUCT BeginUpdate_ps;	\
    HDC BeginUpdate_oldPort; \
	BeginUpdate_oldPort = SetPort(BeginPaint(window, &BeginUpdate_ps))

// The SetPort must be before the EndPaint, because the SetPort
// will also cleanup the old port and the EndPaint destructs the
// old port.
#define EndUpdate(window) SetPort(BeginUpdate_oldPort); \
    EndPaint(window, &BeginUpdate_ps)

// Quickdraw calls
void PenPat(Pattern* type);
void PenNormal();
void PenSize(short w, short h);

void GetIndPattern(Pattern* cntPattern, short resourceID, short patID);

void RGBBackColor(RGBColor* color);
void RGBForeColor(RGBColor* color);
void Index2Color(short id, RGBColor* color);

void MoveTo(int h, int v);
void LineTo(int h, int v);

inline void Pt2Rect(Point pt1, Point pt2, Rect* theRect) 
{ 
	if (pt1.v < pt2.v)
	{
		theRect->top = pt1.v;
		theRect->bottom = pt2.v;
	}
	else
	{
		theRect->top = pt2.v;
		theRect->bottom = pt1.v;
	}

	if (pt1.h < pt2.h)
	{
		theRect->left = pt1.h;
		theRect->right = pt2.h;
	}
	else
	{
		theRect->left = pt2.h;
		theRect->right = pt1.h;
	}
}

PolyHandle OpenPoly();
void ClosePoly();
void PaintPoly(PolyHandle poly);
void KillPoly(PolyHandle poly);
void OffsetPoly(PolyHandle arrow, int dx, int dy);

void PaintOval(Rect *theRect);
void FillOval(Rect *theRect, Pattern* patType);
void FrameOval(Rect *theRect);
void EraseOval(Rect *theRect);
void FrameArc(Rect *circleRect, short angle1, short angle2);

void EraseRect(Rect *theRect);
void PaintRect(Rect *theRect);
void FrameRect(Rect *theRect);
//#define InsetRect(theRect, hinset, vinset)	InflateRect(theRect, -(hinset), -(vinset))

// font stuff
void GetFNum(unsigned char *fontname, short *fontNum);
void TextFont(short fontNum);
void TextSize(short fontSize);
void TextFace(textFace tf);
void DrawString(unsigned char *string);
void DrawChar(char c);
void NumToString(short i, unsigned char *string);

#endif