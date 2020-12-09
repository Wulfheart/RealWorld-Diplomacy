//===============================================================================
// @ Util.h
// ------------------------------------------------------------------------------
// Native utility routines.
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
// 00-Jan-02	JMV	First version
//
//===============================================================================
#ifndef __UtilDefs__
#define __UtilDefs__

#include "MacTypes.h"

//
// Error handling 
//
#define errNoError		0
#define errBadROMs 		1
#define errBadSystem	2
#define errOldSystem	3
#define errNoMenuBar	4
#define errBadDialog  	5
#define errBadGameData	6
#define errBadMapData	7
#define errBadCountryData 8
#define errBadPict		9
#define errMem			10
#define errFile			11
#define errBadString	12
#define errBadMenu		13
#define errPrint        14
#define errScrapNoTEXT	15
#define errTmpFileVol	16
#define errTmpFileCreat	17
#define errTmpFileOpen	18
#define errTmpFileWrite	19
#define errBadPoly		20
#define errBadVariant	21
#define errBadHistory	22
#define errBadInfo      23

extern int gErrNo;
extern char gErrorString[];

void ErrorAlert(int errNumber, char* string);
void ErrorString(char *string);

inline void sleep(long msecs) { Sleep(msecs/2); }

// High word/low word macros
#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define LoWrd(aLong)	((aLong) & 0xFFFF)

// stdfile replacements
//bool FSeof(short refNum);
//char *FSgets(char *s, int size, short refNum);

bool GetInstallDirectory(char* path);
bool GetDataDirectory(char* path);

// watch cursor -- shouldn't be here, really
void ShowWaitCursor();
void ShowArrowCursor();

// standard routines for getting open and save filenames
bool GetFile(FSSpec& filename, char* prompt, SFTypeList filter);
bool PutFile(FSSpec& filename, char* prompt, char* defaultFilename, SFTypeList filter);
// filters
extern SFTypeList csvFilter;
extern SFTypeList textFilter;
extern SFTypeList gameFilter;
extern SFTypeList saveFilter;
extern SFTypeList oldsFilter;
void GetBasename(char* basename, const FSSpec& filename);
inline void CopyFSSpec(char* destination, const FSSpec& source) { strcpy(destination, (char*)source); }
inline void ClearFSSpec(char* filename) { memset(filename, 0, strlen(filename)); }
inline bool EmptyFSSpec(const FSSpec& filename) { return (strlen(filename) == 0); }
inline void MakeFSSpec(char* fsspec, char* path) 
{ 
	strcpy(fsspec, path);
}

bool CopyScrapToFile(FSSpec& filename);
void DeleteTempFile(const FSSpec& filename);

inline Rect Union(Rect& rect1, const Rect& rect2)
{
	Rect newRect;
	UnionRect(&newRect, &rect1, &rect2);
	return newRect;
}

void SetCountryRGBColor( short colorID );

void SetShiftedCountryRGBColor( short colorID );

#endif