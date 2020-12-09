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
#include "MacTypes.h"
#include <math.h>
#include <stdio.h>

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------

#define RGB_MAKE(r, g, b)       ((COLORREF) ( 0x02000000 | ((b) << 16) | ((g) << 8) | (r)))

HDC gCurrentPort = NULL;

RGBColor colors[] = 
{
	{0x0001,0x0001,0x0001},	// (0) black
	{0,0,0xFFFF},			// (1) blue
	{0x99FF,0x66FF,0x33FF},	// (2) brown
	{0,0xFFFF,0xFFFF},		// (3) cyan
	{0,0x7FFF,0},			// (4) forest
	{0,0xFFFF,0},			// (5) green
	{0xFFFF,0,0xFFFF},		// (6) magenta
	{0x8000,0x0, 0x8000},	// (7) purple
	{0xFFFF,0,0},			// (8) red
	{0xFFFF,0xFFFF,0xFFFF}, // (9) white
	{0xFFFF,0xFFFF,0},		// (10) yellow
	{0x7FFF,0,0},			// (11) Crimson
	{0,0,0x7FFF},			// (12) Navy
	{0,0x7FFF,0x7FFF},		// (13) Teal
	{0x7FFF,0x7FFF,0},		// (14) Olive
	{0x7FFF,0x7FFF,0x7FFF}, // (15) Charcoal
	{0xFFFF,0x66FF,0x33FF},	// (16) Orange
	{0xCCFF,0x99FF,0x66FF}, // (17) Tan
	{0xFFFF,0x0000,0xD5FF}, // (18) Australian
	{0x5AFF,0xB4FF,0x0000}, // (19) Brazilian
	{0x55FF,0xFFFF,0x0000}, // (20) Canadian
    {0x80FF,0x80FF,0xFFFF},	// (21) Chinese
    {0x0000,0x2BFF,0xD5FF},	// (22) Colombian
	{0xFFFF,0xD5FF,0xD5FF},	// (23) Egyptian
	{0xAAFF,0xAAFF,0x55FF},	// (24) English
	{0x0000,0xAAFF,0xFFFF}, // (25) French
	{0x55FF,0x55FF,0x55FF}, // (26) German
	{0x0000,0x80FF,0x0000}, // (27) Indian
	{0x5FFF,0x35FF,0x5FFF}, // (28) Indochina
	{0xD5FF,0x80FF,0x0000}, // (29) Indonesia
	{0x0000,0x55FF,0x55FF}, // (30) Italy
	{0xD5FF,0xD5FF,0x80FF}, // (31) Ivorian
	{0xFFFF,0xFFFF,0x0000}, // (32) Japanese
	{0x80FF,0x0000,0x0000}, // (33) Mexican
	{0x55FF,0x0000,0xAAFF}, // (34) Mongolian
	{0xAAFF,0xAAFF,0xAAFF}, // (35) Persian
	{0xAAFF,0x2BFF,0x0000}, // (36) Russian
	{0xAAFF,0x55FF,0x55FF}, // (37) African
	{0xFFFF,0x55FF,0x55FF}, // (38) Turkish
	{0x0000,0xAAFF,0xAAFF}, // (39) Islandish
	{0xD5FF,0x55FF,0x0000}, // (40) American
	{0x22FF,0x22FF,0x22FF}  // (41) Blocked Gray
};

Pattern patterns[] = 
{
	kRandom, kThinDiag, kHash, kGray, kQuilt, kSparse, 
	kDiag, kThinHoriz, kHoriz, kVert, kStripe, kFlood
};

Bitmap bitmaps[14] = {
	{0x80, 0x10, 0x02, 0x20, 0x01, 0x08, 0x40, 0x04},
	{0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88, 0x11},
	{0x88, 0x44, 0x0, 0x0, 0x88, 0x44, 0x0, 0x0},
	{0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22},
	{0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00},
	{0x0, 0x02, 0x0, 0x0, 0x0, 0x20, 0x0, 0x0},
	{0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80},
	{ 0x0, 0xAA, 0x0,0x0,0x0, 0x55, 0x0, 0x0 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE, 0x00, 0x00 },
	{ 0x80, 0x08, 0x80, 0x08, 0x80, 0x08, 0x80, 0x08 },
	{ 0x00, 0xAA, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
};

struct
{
	char *name;
	short type;
} fonts[] = 
{
	{ "Monaco", FF_MODERN|FIXED_PITCH },
	{ "Geneva", FF_SWISS|VARIABLE_PITCH },
	{ "Times", FF_ROMAN|VARIABLE_PITCH }
};


QDType qd = { kBlack, kGray, kWhite, kBlack, PS_SOLID, BS_SOLID, false, 0, 0xFFFFFF, 1, 1, 
			NULL, FF_SWISS|VARIABLE_PITCH, 12, normal, 0, 0 };

static PolyHandle currentPoly = NULL;

void RGBForeColor(COLORREF& colorref);

//---------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------


HDC SetPort(HDC port)
{
    HDC oldPort = gCurrentPort;

    gCurrentPort = port;
    if (oldPort != NULL)
    {
        // Clean up pens, brushes and fonts of the old port.
        DeleteObject(SelectObject(oldPort, GetStockObject(BLACK_PEN)));
        DeleteObject(SelectObject(oldPort, GetStockObject(BLACK_BRUSH)));
        DeleteObject(SelectObject(oldPort, GetStockObject(SYSTEM_FONT)));
    }
    return oldPort;
}


void GetPort(HDC* port)
{
    *port = gCurrentPort;
}



void PenNormal()
{
	qd.penType = PS_SOLID;
	qd.brushType = BS_SOLID;
	qd.hatched = false;
	qd.foregroundColor = 0;
	qd.backgroundColor = 0xFFFFFF;
	qd.penWidth = 1;
	qd.penHeight = 1;
	DeleteObject(SelectObject(gCurrentPort, GetStockObject(BLACK_PEN)));
	qd.foregroundBrush = CreateSolidBrush(qd.foregroundColor);
	DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
	SetBkMode(gCurrentPort, OPAQUE);
}

void PenSize(short w, short h)
{
	qd.penWidth = w;
	qd.penHeight = h;
	DeleteObject(SelectObject(gCurrentPort, CreatePen(qd.penType, w, qd.foregroundColor)));
}

void PenPat(Pattern* type)
{
	switch (*type)
	{
	case kBlack:
	default:
		qd.penType = PS_SOLID;
		qd.brushType = BS_SOLID;
		qd.hatched = false;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_SOLID, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateSolidBrush(qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kGray:
		qd.penType = PS_DOT;
		qd.brushType = BS_SOLID;
		qd.hatched = false;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateSolidBrush(0x777777);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kWhite:
		qd.penType = PS_SOLID;
		qd.brushType = BS_SOLID;
		qd.hatched = false;
		qd.foregroundColor = 0xFFFFFF;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_SOLID, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateSolidBrush(qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;
#if FALSE
	case kHorizontal:
		qd.penType = PS_DOT;
		qd.brushType = HS_HORIZONTAL;
		qd.hatched = true;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateHatchBrush(qd.brushType, qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kVertical:
		qd.penType = PS_DOT;
		qd.brushType = HS_VERTICAL;
		qd.hatched = true;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateHatchBrush(qd.brushType, qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kCross:
		qd.penType = PS_DOT;
		qd.brushType = HS_CROSS;
		qd.hatched = true;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateHatchBrush(qd.brushType, qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kBDiagonal:
		qd.penType = PS_DOT;
		qd.brushType = HS_BDIAGONAL;
		qd.hatched = true;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateHatchBrush(qd.brushType, qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kFDiagonal:
		qd.penType = PS_DOT;
		qd.brushType = HS_FDIAGONAL;
		qd.hatched = true;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateHatchBrush(qd.brushType, qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	case kDiagCross:
		qd.penType = PS_DOT;
		qd.brushType = HS_DIAGCROSS;
		qd.hatched = true;
		DeleteObject(SelectObject(gCurrentPort, CreatePen(PS_DOT, qd.penWidth, qd.foregroundColor)));
		qd.foregroundBrush = CreateHatchBrush(qd.brushType, qd.foregroundColor);
		DeleteObject(SelectObject(gCurrentPort, qd.foregroundBrush)); 
		break;

	default:
		break;
#endif
	}
	qd.currentPattern = *type;
}

void RGBForeColor(RGBColor* color)
{
	COLORREF colorref = 0xffffff & (RGB_MAKE(color->red >> 8, color->green >> 8, color->blue >> 8));
	RGBForeColor(colorref); 
}

void RGBForeColor(COLORREF& colorref)
{
	SetTextColor(gCurrentPort, colorref);
	qd.foregroundColor = colorref;
	DeleteObject(SelectObject(gCurrentPort, CreatePen(qd.penType, qd.penWidth, qd.foregroundColor)));
	if (!qd.hatched)
		DeleteObject(SelectObject(gCurrentPort, CreateSolidBrush(qd.foregroundColor))); 
	else
		DeleteObject(SelectObject(gCurrentPort, CreateHatchBrush(qd.brushType, qd.foregroundColor))); 
}

void RGBBackColor(RGBColor* color)
{
	COLORREF colorref = RGB_MAKE(color->red >> 8, color->green >> 8, color->blue >> 8);
	SetBkColor(gCurrentPort, colorref);	
	qd.backgroundColor = colorref;
}


PolyHandle OpenPoly()
{
	if (currentPoly != NULL)
		return NULL;

	currentPoly = new Poly;
	currentPoly->polySize = 0;

	return currentPoly;
}

void ClosePoly()
{
	currentPoly = NULL;
}

void PaintPoly(PolyHandle poly)
{
	Polygon(gCurrentPort, poly->polyPoints, poly->polySize);
}

void KillPoly(PolyHandle poly)
{
	delete poly;
}

void OffsetPoly(PolyHandle poly, int dx, int dy)
{
	for (int i = 0; i < poly->polySize; i++)
	{
		poly->polyPoints[i].x += dx;
		poly->polyPoints[i].y += dy;
	}
}

void MoveTo(int h, int v)
{
	if (currentPoly != NULL)
	{
		currentPoly->polyPoints[0].x = h;
		currentPoly->polyPoints[0].y = v;
		currentPoly->polySize = 1;
		return;
	}

	qd.currentx = h;
	qd.currenty = v;
	MoveToEx(gCurrentPort, h, v, NULL);

}

void LineTo(int h, int v)
{
	if (currentPoly != NULL)
	{
		currentPoly->polyPoints[currentPoly->polySize].x = h;
		currentPoly->polyPoints[currentPoly->polySize].y = v;
		currentPoly->polySize++;
		return;
	}

	qd.currentx = h;
	qd.currenty = v;
	LineTo(gCurrentPort, h, v);

}

void PaintOval(Rect *theRect)
{
	Ellipse(gCurrentPort, theRect->left, theRect->top, theRect->right, theRect->bottom);
}

void FillOval(Rect *theRect, Pattern* patType)
{
	Pattern oldPattern = qd.currentPattern;
	COLORREF foreground = qd.foregroundColor;
	PenPat( patType );
	Ellipse(gCurrentPort, theRect->left, theRect->top, theRect->right, theRect->bottom);
	PenPat( &oldPattern );
	RGBForeColor(foreground);
}

void FrameOval(Rect *theRect)
{
	HBRUSH oldBrush = (HBRUSH) SelectObject(gCurrentPort, GetStockBrush(HOLLOW_BRUSH));

	Ellipse(gCurrentPort, theRect->left, theRect->top, theRect->right, theRect->bottom);

	DeleteObject(SelectObject(gCurrentPort, oldBrush));
}

void EraseOval(Rect *theRect)
{
	Pattern oldPattern = qd.currentPattern;
	COLORREF foreground = qd.foregroundColor;
	PenPat( &qd.black );
	RGBForeColor( qd.backgroundColor );
	Ellipse(gCurrentPort, theRect->left, theRect->top, theRect->right, theRect->bottom);
	PenPat( &oldPattern );
	RGBForeColor(foreground);	
}

// this only works for the counterclockwise case
void FrameArc(Rect *theRect, short angle1, short angle2)
{
	float fangle1 = fabsf((float)angle1)*3.1415926f/180.0f;
	float fangle2 = fabsf((float)angle2)*3.1415926f/180.0f;
	float cos1 = cosf(fangle1)*100.0f + 0.5f;
	float sin1 = sinf(fangle1)*100.0f + 0.5f;
	float cos2 = cosf(fangle2)*100.0f + 0.5f;
	float sin2 = sinf(fangle2)*100.0f + 0.5f;
	Point center = { (theRect->right + theRect->left)/2, (theRect->bottom + theRect->top)/2 };

	(void) Arc( gCurrentPort, theRect->left, theRect->top, theRect->right, theRect->bottom,
		center.h + (int) cos2, center.v + (int) sin2, center.h + (int) cos1, center.v + (int) sin1 );
	// hack to get the 'C' drawing right
	MoveTo((theRect->right+theRect->left)/2, theRect->bottom-1);
	LineTo((theRect->right+theRect->left)/2+1, theRect->bottom-1);
}

void FrameRect(Rect *theRect)
{
	MoveTo(theRect->left, theRect->top);
	LineTo(theRect->left, theRect->bottom);
	LineTo(theRect->right, theRect->bottom);
	LineTo(theRect->right, theRect->top);
	LineTo(theRect->left, theRect->top);
//	FrameRect(gCurrentPort, theRect, qd.foregroundBrush);
}

void PaintRect(Rect *theRect)
{
	FillRect(gCurrentPort, theRect, qd.foregroundBrush);
}

void EraseRect(Rect *theRect)
{
	HBRUSH brush = CreateSolidBrush(qd.backgroundColor);
	FillRect(gCurrentPort, theRect, brush);
	DeleteObject(brush);
}

void GetFNum(unsigned char *fontname, short *fontNum)
{
	for (int i = 0; i < 3; i++)
	{
		if (!strcmp(fonts[i].name, (char*)fontname))
		{
			*fontNum = fonts[i].type;
			return;
		}
	}
	*fontNum = FF_DONTCARE|DEFAULT_PITCH;
}

static void SetFont()
{
	LOGFONT fontstuff = {0};
	fontstuff.lfHeight = qd.fontSize+((qd.fontType == italic)?10:3);
	fontstuff.lfWidth = 0;
	fontstuff.lfEscapement = 0;
	fontstuff.lfOrientation = 0;
	if (qd.fontType == bold)
		fontstuff.lfWeight = FW_BOLD;
//	else if (qd.fontType == italic)
//		fontstuff.lfWeight = FW_LIGHT;
	else
		fontstuff.lfWeight = FW_NORMAL;
	fontstuff.lfItalic = (qd.fontType == italic)?1:0;
	fontstuff.lfUnderline = 0;
	fontstuff.lfStrikeOut = 0;
	fontstuff.lfCharSet = 0;
	fontstuff.lfOutPrecision = OUT_DEFAULT_PRECIS;
	fontstuff.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	fontstuff.lfQuality = DEFAULT_QUALITY;
	fontstuff.lfPitchAndFamily = (unsigned char)qd.fontType;
	fontstuff.lfFaceName[0] = 0;

	HFONT hfont = CreateFontIndirect(&fontstuff);
    DeleteObject(SelectObject(gCurrentPort, hfont));
}

void TextFont(short fontNum)
{
    qd.fontType = fontNum;
	SetFont();
}

void TextSize(short fontSize)
{
    qd.fontSize = fontSize;
    SetFont();
}

void TextFace(textFace tf)
{
    qd.fontType = tf;
	SetFont();
}

void Index2Color(short id, RGBColor* color)
{
	*color = colors[id];
}


void GetIndPattern(Pattern* cntPattern, short resourceID, short patID)
{
	*cntPattern = patterns[patID];

}


void SysBeep(short duration)
{
	MessageBeep(MB_ICONEXCLAMATION);
}


void DrawString(unsigned char *string)
{
	SetBkMode(gCurrentPort, TRANSPARENT);
	
	TEXTMETRIC tm;
	GetTextMetrics(gCurrentPort, &tm);
	
	TextOut(gCurrentPort, qd.currentx+1, qd.currenty-tm.tmAscent, (char*)string, strlen((char*)string));

	SetBkMode(gCurrentPort, OPAQUE);
}

void DrawChar(char c)
{
	SetBkMode(gCurrentPort, TRANSPARENT);

	TEXTMETRIC tm;
	GetTextMetrics(gCurrentPort, &tm);
	
	TextOut(gCurrentPort, qd.currentx, qd.currenty-tm.tmAscent, &c, 1);

	SetBkMode(gCurrentPort, OPAQUE);
}


void NumToString(short i, unsigned char *string)
{
	sprintf((char*)string, "%d", i);
}

