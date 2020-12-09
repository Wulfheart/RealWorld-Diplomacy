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
#include "Rgn.h"
#include "EdgeList.h"
#include "TextFile.h"

#define UnionRgn(hrgnResult, hrgnA, hrgnB)	\
	CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_OR)
#define GetStockBrush(i) ((HBRUSH)GetStockObject(i))
#define RGB_MAKE(r, g, b)       ((COLORREF) (((b) << 16) | ((g) << 8) | (r)))

Rgn::Rgn()
{
	mRect.left = 1000;
	mRect.top = 1000;
	mRect.right = 0;
	mRect.bottom = 0;
	mNumLines = 0;
	mScanLines = NULL;
	mNative = CreateRectRgn(0, 0, 0, 0);
}

Rgn::~Rgn()
{
	delete [] mScanLines;
	DeleteObject(mNative);
}

void 
Rgn::AddScanLine(short x, short y, int length)
{
	bool overlap = false;
	// go through all scanlines
	for (int i = 0; i < mNumLines; i++)
	{
		// on same level as existing scanline
		if (mScanLines[i].y == y)
		{
			overlap = MergeScanLine( mScanLines[i], x, length );
			// if added, may need to merge other scanlines
			if (overlap)
			{
				// go through all remaining scanlines
				for (int j = i+1; j < mNumLines; j++)
				{
					// if on same level as existing scanline
					if (mScanLines[j].y == y)
					{
						// and if overlapped, delete it
						if (MergeScanLine( mScanLines[i], mScanLines[j].x, mScanLines[j].length ))
						{
							DeleteScanLine( j );
							--j;
						}
					}
				}
				break;
			}
		}
	}

	// if no special case found, just add it
	if ( !overlap )
		AppendScanLine(x, y, length);

	// expand bounding rectangle
	if (x < mRect.left)
	{
		mRect.left = x;
	}
	else if (x+length > mRect.right)
	{
		mRect.right = x+length;
	}
	if (y < mRect.top)
	{
		mRect.top = y;
	}
	else if (y > mRect.bottom)
	{
		mRect.bottom = y;
	}

	HRGN scanLine = CreateRectRgn( x, y, x+length, y+1 );
	UnionRgn( mNative, scanLine, mNative );
	DeleteObject( scanLine );
}

bool 
Rgn::MergeScanLine(ScanLine& line, short x, int length)
{
	bool overlap = false;
	short xl = line.x;
	short xr = xl + line.length;
	// if overlaps to the left
	if (x < xl && x+length >= xl)
	{
		// add to left
		overlap = true;
		line.x = x;
		line.length += (xl-x);
	}
	// if overlaps to the right
	if (x <= xr && x+length > xr)
	{
		// add to right
		overlap = true;
		line.length += (x+length-xr);
	}

	return overlap;
}


void
Rgn::DeleteScanLine( int index )
{
	// if outside range, ignore
	if (index < 0 || index >= mNumLines)
		return;

	// move last line to desired slot
	if (index < mNumLines-1)
	{
		mScanLines[index] = mScanLines[mNumLines-1];
	}
	--mNumLines;
}

void
Rgn::AppendScanLine(short x, short y, int length)
{
	ScanLine* newScanLines = new ScanLine[mNumLines+1];
	for (int i = 0; i < mNumLines; i++)
	{
		newScanLines[i] = mScanLines[i];
	}
	newScanLines[mNumLines].x = x;
	newScanLines[mNumLines].y = y;
	newScanLines[mNumLines].length = length;
	delete [] mScanLines;
	mScanLines = newScanLines;
	++mNumLines;
}	

void 
Rgn::Draw( const Pattern& pattern )
{
	HDC hdc;
	GetPort(&hdc);

	if (pattern == kFlood)
	{
		HBRUSH brush = CreateSolidBrush(qd.foregroundColor);
		FillRgn(hdc, mNative, brush);
		DeleteObject(brush);
		return;
	}

	// go through all scanlines
	for (int i = 0; i < mNumLines; i++)
	{
		int y = mScanLines[i].y;
		int xl = mScanLines[i].x;
		int xr = xl + mScanLines[i].length;
		for (; xl < xr; ++xl)
		{
			if ( xl >= mRect.left && xl <= mRect.right 
				&& ((0x1 << (xl%8)) & bitmaps[pattern][y%8]) )
			{
				MoveTo(xl, y); LineTo(xl+1, y+1);
//				SetPixel( hdc, xl, y, qd.foregroundColor );
			}
		}
	}
}

void 
Rgn::Draw()
{
	HDC hdc;
	GetPort(&hdc);
	FillRgn(hdc, mNative, GetStockBrush(BLACK_BRUSH));
}

bool 
Rgn::IsPointInside(short x, short y)
{
	if ( x < mRect.left || x > mRect.right ||
		 y < mRect.top || y > mRect.bottom )
		 return false;

	// go through all scanlines
	for (int i = 0; i < mNumLines; i++)
	{
		if (mScanLines[i].y == y)
		{
			if (x >= mScanLines[i].x && x < mScanLines[i].x + mScanLines[i].length)
				return true;
		}
	}

	return false;
}

Rgn* 
Union(const Rgn* first, const Rgn* second)
{
	Rgn* newRgn = new Rgn();
	int i;
	
	// add scan lines for first thing
	for (i = 0; i < first->mNumLines; i++)
	{
		newRgn->AddScanLine(first->mScanLines[i].x, first->mScanLines[i].y, 
			first->mScanLines[i].length );
	}
	// add scan lines for second thing
	for (i = 0; i < second->mNumLines; i++)
	{
		newRgn->AddScanLine(second->mScanLines[i].x, second->mScanLines[i].y, 
			second->mScanLines[i].length );
	}

	return newRgn;
}

void 
Rgn::ScanPoly(Point* points, int numPoints)
{
	EdgeList	 activeEdges(numPoints);							// currently active edges
    int k, yb, yt, y, i, j, xl, xr;
	Point pt;
	short* indices;

	indices = new short[numPoints];
	for (i = 0; i < numPoints; i++)
	{
		indices[i] = i;
	}
	// sort
	for (i = 0; i < numPoints; i++)
	{
		int min = i;
		for (int j = i+1; j < numPoints; j++)
		{
			if (points[indices[j]].v < points[indices[min]].v)
			{
				min = j;	
			}
		}
		int temp = indices[min];
		indices[min] = indices[i];
		indices[i] = temp;
	}

	k = 0;				/* indices[k] is next vertex to process */
    yb = (int) ceil(points[indices[0]].v-.5);	/* ymin of polygon */
    yt = (int)(points[indices[numPoints-1]].v-.5f);	/* ymax of polygon */
	activeEdges.Clear();

  	// step through scanlines 
	for (y = yb; y <= yt; y++) 
	{		
		/* scanline y is at y+.5 in continuous coordinates */

		/* check vertices between previous scanline and current one, if any */
		for (; k < numPoints && points[indices[k]].v <= y+.5; k++) 
		{
			/* to simplify, if pt.y=y+.5, pretend it's above */
			/* invariant: y-.5 < pt[i].y <= y+.5 */
			i = indices[k];	
			/*
			 * insert or delete edges before and after vertex i (i-1 to i,
			 * and i to i+1) from active list if they cross scanline y
			 */
			j = i>0 ? i-1 : numPoints-1;	/* vertex previous to i */
			pt = points[j];
			if (pt.v <= y-0.5f)	/* old edge, remove from active list */
				activeEdges.Delete(j);
			else if (pt.v > y+0.5f)	/* new edge, add to active list */
				activeEdges.Insert(pt.h, pt.v, points[i].h, points[i].v, j, y);

			j = i < numPoints-1 ? i+1 : 0;	/* vertex next after i */
			pt = points[j];
			if (pt.v <= y-0.5f)	/* old edge, remove from active list */
				activeEdges.Delete(i);
			else if (pt.v > y+0.5f)	/* new edge, add to active list */
				activeEdges.Insert(points[i].h, points[i].v, pt.h, pt.v, i, y);
		}

		/* sort active edge list by active[j].x */
		activeEdges.Sort();

		/* draw horizontal segments for scanline y */
		int m = activeEdges.Size();
		for (j = 0; j < m-1; j += 2) 
		{	
			/* draw horizontal segments */
			/* span 'tween j & j+1 is inside, span tween j+1 & j+2 is outside */
			xl = (int) activeEdges.Left(j)+1;
			xr = (int) activeEdges.Right(j);
			
			AddScanLine( xl, y, xr-xl+1 );
					
			activeEdges.Increment(j);
		}

	}
}

void 
Rgn::Offset(short x, short y)
{
	for (int i = 0; i < mNumLines; i++)
	{
		mScanLines[i].x += x;
		mScanLines[i].y += y;
	}

	mRect.top += y;
	mRect.bottom += y;
	mRect.left += x;
	mRect.right += x;

	OffsetRgn( mNative, x, y );
}


void 
Rgn::GetRect(Rect& box)
{
	GetRgnBox( mNative, &box );
}

int 
Rgn::Read(FILE* fp)
{
	if (mScanLines != NULL)
		delete [] mScanLines;
	mScanLines = NULL;
	
	int numLines = 0, x, y, length;
	fscanf(fp, "%d\n", &numLines);
	mNumLines = numLines;
	mScanLines = new ScanLine[mNumLines];
	for (int i = 0; i < numLines; i++)
	{
		fscanf(fp, "%d %d %d\n", &x, &y, &length);
		mScanLines[i].x = x;
		mScanLines[i].y = y;
		mScanLines[i].length = length;
		
		// expand bounding rectangle
		if (x < mRect.left)
		{
			mRect.left = x;
		}
		else if (x+length > mRect.right)
		{
			mRect.right = x+length;
		}
		if (y < mRect.top)
		{
			mRect.top = y;
		}
		else if (y > mRect.bottom)
		{
			mRect.bottom = y;
		}

		HRGN scanLine = CreateRectRgn( x, y, x+length, y+1 );
		UnionRgn( mNative, scanLine, mNative );
		DeleteObject( scanLine );
	}	
	
	return 0;
}

int 
Rgn::Read(TextFile& fp)
{
	if (mScanLines != NULL)
		delete [] mScanLines;
	mScanLines = NULL;
	
	int numLines = 0, x, y, length;
	char line[256];
	fp.ReadLine(line, 255);
	sscanf(line, "%d\n", &numLines);
	mNumLines = numLines;
	mScanLines = new ScanLine[mNumLines];
	for (int i = 0; i < numLines; i++)
	{
		fp.ReadLine(line, 255);
		sscanf(line, "%d %d %d\n", &x, &y, &length);
		mScanLines[i].x = x;
		mScanLines[i].y = y;
		mScanLines[i].length = length;
		
		// expand bounding rectangle
		if (x < mRect.left)
		{
			mRect.left = x;
		}
		else if (x+length > mRect.right)
		{
			mRect.right = x+length;
		}
		if (y < mRect.top)
		{
			mRect.top = y;
		}
		else if (y > mRect.bottom)
		{
			mRect.bottom = y;
		}

		HRGN scanLine = CreateRectRgn( x, y, x+length, y+1 );
		UnionRgn( mNative, scanLine, mNative );
		DeleteObject( scanLine );
	}	
	
	return 0;
}

int 
Rgn::Write(FILE* fp)
{
	fprintf(fp, "%d\n", mNumLines);
	for (int i = 0; i < mNumLines; i++)
	{
		if (mScanLines[i].length > 0)
			fprintf(fp, "%d %d %d\n", mScanLines[i].x, mScanLines[i].y, mScanLines[i].length);
	}	
	
	return 0;
}

