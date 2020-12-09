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
// This is based on code by Paul Heckbert from Graphics Gems I, page 87.

#include <string.h>
#include <stdlib.h>
#include "EdgeList.h"

int CompareEdges(const void *u, const void *v);

// compare edges for qsort
int CompareEdges(const void *u, const void *v) 
{ 
	return ((Edge*)u)->x <= ((Edge*)v)->x ? -1 : 1; 
}

EdgeList::EdgeList(int maxEdges) :
	mEdges(NULL)
{
	mEdges = new Edge[(unsigned int) maxEdges];
	mSize = 0;
}


EdgeList::~EdgeList()
{
	if (mEdges != NULL)
		delete[] mEdges;
}

//
// Delete edge i from list
//
void EdgeList::Delete(int i)		
{
    int j;
	if(mEdges == NULL)
		return;

    for (j=0; j< mSize && mEdges[j].i != i; j++)
    {
    } 

    if (j >= mSize) 
		return;	// edge not in list
    mSize--;
    memcpy( mEdges + j, mEdges + j + 1, ((unsigned int)(mSize-j)) * sizeof(mEdges[0]));

}

//
// Add edge i to end of active list
//
void EdgeList::Insert(int x1, int y1, int x2, int y2, int i, int y)		
{
    float dx;
	if (mEdges == NULL)
		return;
 
    if (y1 < y2) 
	{
		// initialize x position at intersection of edge with scanline y
		mEdges[mSize].dx = dx = (float)(x2 - x1)/(float)(y2 - y1);
		mEdges[mSize].x = dx*(y+0.5f - y1) + x1;
		mEdges[mSize].i = i;
	}
    else
	{
		// initialize x position at intersection of edge with scanline y
		mEdges[mSize].dx = dx = (float)(x1 - x2)/(float)(y1 - y2);
		mEdges[mSize].x = dx*(y+0.5f - y2) + x2;
		mEdges[mSize].i = i;
	}
    mSize++;
}

//
// Delete edge i from list
//
void EdgeList::Sort()		
{
	if (mEdges == NULL)
		return;

	qsort(mEdges, (unsigned int) mSize, sizeof(Edge), CompareEdges);
}

