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
#ifndef EdgeList_H
#define EdgeList_H

#include <math.h>

typedef struct {	// a polygon edge 
    float x;		// x coordinate of edge's intersection with current scanline 
    float dx;		// change in x with respect to y 
    int i;			// edge number: edge i goes from pt[i] to pt[i+1] 
} Edge;

class EdgeList 
{
public:
	EdgeList(int maxEdges); 
	virtual ~EdgeList();

	void Delete(int i);
	void Insert( int x1, int y1, int x2, int y2, int i, int y);
	void Sort();
	inline void Clear() { mSize = 0; }
	inline int Size() { return mSize;}
	inline float Left(int j) { return ceilf(mEdges[j].x-.5f); }
	inline float Right(int j) { return floorf(mEdges[j+1].x-.5f); }
	void Increment(int j) {	mEdges[j].x += mEdges[j].dx; mEdges[j+1].x += mEdges[j+1].dx; }

protected:
	int	  mSize;		/* number of active edges */
	Edge *mEdges;		/* active edge list:edges crossing scanline y */

private:
	EdgeList( const EdgeList& source );
	EdgeList& operator=( const EdgeList& source );

};

#endif  
