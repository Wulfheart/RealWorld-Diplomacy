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
#ifndef __RgnDefs__
#define __RgnDefs__

#include <stdio.h>
#include "MacTypes.h"

typedef struct	// a scan line
{	
    short x, y;		// coordinates of start of scanline
    int length;		// length of scanline
} ScanLine;

class TextFile;

//
// A rgn is a collection of scanlines (can't use Region, already defined by Mac)
//
class Rgn 
{
public:
	Rgn(); 
	~Rgn();

	void AddScanLine(short x, short y, int length);
	void Draw(const Pattern& pattern);
	void Draw();

	bool IsPointInside(short x, short y);
	void ScanPoly(Point* points, int numPoints);

	void Offset( short x, short y );
	void GetRect( Rect& box );

	int Read(FILE* fp);
	int Read(TextFile& fp);
	int Write(FILE* fp);

	friend Rgn* Union(const Rgn* first, const Rgn* second);

protected:
	void AppendScanLine(short x, short y, int length);
	void DeleteScanLine(int index);
	bool MergeScanLine(ScanLine& line, short x, int length);

	Rect	mRect;
	int		mNumLines;
	ScanLine* mScanLines;
	HRGN	mNative;

private:
	Rgn( const Rgn& source );
	Rgn& operator=( const Rgn& source );

};

#endif  