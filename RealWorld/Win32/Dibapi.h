//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// Based on code by Microsoft Corporation
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


#ifndef _INC_DIBAPI
#define _INC_DIBAPI

#include <windows.h>
#include <stdio.h>

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Function prototypes */
BOOL      WINAPI  PaintDIB (HDC, LPRECT, HDIB, LPRECT, HPALETTE pPal);
BOOL      WINAPI  CreateDIBPalette(HDIB hDIB, HPALETTE& cPal);
LPSTR     WINAPI  FindDIBBits (LPSTR lpbi);
DWORD     WINAPI  DIBWidth (LPSTR lpDIB);
DWORD     WINAPI  DIBHeight (LPSTR lpDIB);
WORD      WINAPI  PaletteSize (LPSTR lpbi);
WORD      WINAPI  DIBNumColors (LPSTR lpbi);
HGLOBAL   WINAPI  CopyHandle (HGLOBAL h);
HANDLE	DDBToDIB( HBITMAP bitmap, DWORD dwCompression, HPALETTE hPal );

BOOL      WINAPI  SaveDIB (HDIB hDib, char* filename);
HDIB      WINAPI  ReadDIBFile(char* filename);

// failure code 
extern int gDibError;

#define	DIB_ERR_NONE		0
#define	DIB_ERR_NOTFOUND	1
#define DIB_ERR_NOTBMP		2
#define DIB_ERR_NOT8BIT		3
#define DIB_ERR_INVALIDFILE	4	

#endif //!_INC_DIBAPI
