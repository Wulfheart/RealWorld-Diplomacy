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
#ifndef __HistoryDefs__
#define __HistoryDefs__

#include "Unit.h"
#include "Order.h"
#include "Game.h"
#include "MapData.h"

typedef char LongInfoString[2*MAX_SECT];
typedef char MediumInfoString[512];
typedef char ShortInfoString[256];

class TextFile;

#define ORDERS

// 
// maintains history database
//
class History
{
public:
	History();
	~History();
	void Clear();				// clear database
	void Reallocate();			// allocate more space
	void AppendCurrentState();	// append current game state to history
	void CopyCurrentState();	// set current history to current game state (for editing)
	bool Branch();				// delete all history after current point
	bool Previous();			// move backwards in time
	bool Next();				// move forwards in time
	inline bool AtBeginning();	// you're at the beginning of recorded history
	inline bool AtEnd();		// there is no tomorrow
	bool Read( TextFile& file );	// read in database
	bool Write( TextFile& file, bool rp ); 	// write out database
								// use old realpolitik format, if required
	bool WriteLastState( TextFile& file , bool rp ); 	
								// write out last state in game format
	bool WriteResults( TextFile& file, char* name ); // write out penultimate results
	bool WriteResultsCSV( TextFile& file, char* name ); // write out penultimate results
	
protected:
	bool SetAsCurrentState();	// replace current state with position in history
	void CopyStateToIndex(unsigned int index);

	unsigned int	mNumStored;
	unsigned int	mNumAllocated;
	unsigned int	mCurrent;
	LongInfoString* mUnits;
#ifdef ORDERS
	Orderlist**		mOrders;
#endif
	MediumInfoString* mSupplies;
	MediumInfoString* mLands;
	int*			  mAdjustments;
	ShortInfoString*  mBuilds;
	int*			  mDislodges;
	DislodgeList*	  mDslglist;
	int*			  mYear;
	int*			  mSeason;
};

inline bool
History::AtBeginning()
{
	return (mCurrent == 0);
}

inline bool
History::AtEnd()
{
	return (mNumStored == 0 || mCurrent == mNumStored-1);
}

extern History history;

#endif	// __HistoryDefs__
