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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "History.h"
#include "Game.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "Order.h"
#include "OrderPalette.h"
#include "OrdersDisplay.h"
#include "Parse.h"
#include "ReadOrders.h"
#include "Strings.h"
#include "TextFile.h"
#include "Util.h"

//---------------------------------------------------------------------------
// Global variables
//---------------------------------------------------------------------------
History history;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

History::History()
{
	mNumAllocated = 5;
	mNumStored = 0;
	mCurrent = 0;
	mUnits = new LongInfoString[mNumAllocated];
	mSupplies = new MediumInfoString[mNumAllocated];
	mLands = new MediumInfoString[mNumAllocated];
	mOrders = new Orderlist*[mNumAllocated];	
	mAdjustments = new int[mNumAllocated];
	mBuilds = new ShortInfoString[mNumAllocated];
	mDislodges = new int[mNumAllocated];
	mDslglist = new DislodgeList[mNumAllocated];
	for (unsigned int i = 0; i < mNumAllocated; ++i)
	{
		mOrders[i] = NULL;
	}
	mYear = new int[mNumAllocated];
	mSeason = new int[mNumAllocated];
}

History::~History()
{
	Clear();
	delete [] mUnits;
	delete [] mSupplies;
	delete [] mLands;
	delete [] mOrders;
	delete [] mAdjustments;
	delete [] mBuilds;
	delete [] mDislodges;
	delete [] mDslglist;
	delete [] mYear;
	delete [] mSeason;
}

void
History::Clear()
{
	// delete previous dislodge and order data
	for (unsigned int i = 0; i < mNumStored; i++)
	{
		mDslglist[i].Clean();		
		if (mOrders[i] != NULL)
		{
			for (int c = 0; c <= NUM_COUN; c++)
				cleanorders( mOrders[i][c] );
			delete [] mOrders[i];
			mOrders[i] = NULL;
		}
	}
	// mark as empty
	mNumStored = 0;
	mCurrent = 0;
}

bool
History::Previous()
{
	if (AtBeginning())
		return false;
	// handle some special cases if at the end
	if (AtEnd())
	{
		// copy the current orders
		CopyCurrentState();
		// set current orders to be unresolved
		for (int c = 0; c <= NUM_COUN; c++)
		{
			for (Order order = startorder(mOrders[mCurrent][c]); isorder(order); nextorder(order))
			{
				setofails(order, R_NONE);
			}
		}
	}
	// move to previous phase
	mCurrent--;
	orderPalette.Activate( mapWindow, false );
	return SetAsCurrentState();
}

bool
History::Next()
{
	if (AtEnd())
		return false;
	mCurrent++;
	orderPalette.Activate( mapWindow, AtEnd() );
	return SetAsCurrentState();
}


// delete all history after current point
bool 
History::Branch()
{
	if (AtEnd())
		return true;
		
	if (mNumStored == 1)
		return false;
	
	// clean up dislodge and order lists
	for (unsigned int i = mCurrent+1; i < mNumStored; i++)
	{
		mDslglist[i].Clean();
		
		for (int c = 0; c <= NUM_COUN; c++)
			cleanorders( mOrders[i][c] );
		delete [] mOrders[i];
		mOrders[i] = NULL;
	}
	// set current orders to be unresolved
	for (int c = 0; c <= NUM_COUN; c++)
	{
		for (Order order = startorder(mOrders[mCurrent][c]); isorder(order); nextorder(order))
		{
     		char string[256];
			setofails(order, R_NONE);
	       	order2string(string, order, false, false);
		   	SetOrderLine(string, order->index);
		}
	}
		
	mNumStored = mCurrent+1;
	
	orderPalette.Activate( mapWindow, AtEnd() );
	gResolve = true;
	gCommit = false;
	
	// if in autumn or fall phase w/o retreats, copy supply data from previous phase
	if ( mNumStored > 1 && (mSeason[mCurrent] == S_AUTUMN || mSeason[mCurrent] == S_FALL) )
	{
		strcpy(mSupplies[mCurrent], mSupplies[mCurrent-1]);
		strcpy(mLands[mCurrent], mLands[mCurrent-1]);
	}	
		
	return SetAsCurrentState();
}


void 
History::CopyStateToIndex(unsigned int index)
{
	// copy the int data
	mSeason[index] = season;
	mYear[index] = year;
	mAdjustments[index] = adjustments;
	
	// copy the unit and supplies data
	memset(mUnits[index], 0, sizeof(LongInfoString));
	memset(mSupplies[index], 0, sizeof(MediumInfoString));
	memset(mLands[index], 0, sizeof(MediumInfoString));
	
	// go through all the spaces
	int currentUnit = 0;
	int currentSupply = 0;
	int currentLand = 0;
	int i;
	for (i = 1; i <= NUM_SECT; ++i)
	{
		mUnits[index][currentUnit] = '-';
		if (issupp(map+i))
			mSupplies[index][currentSupply] = '-';
		else if (island(map+i))
			mLands[index][currentLand] = '-';
		// go through all the countries
		for (int j = 1; j <= NUM_COUN; ++j)
		{	
			// if country owns unit in this sector
			Unit unit;
			if ((unit = getunit(units[j], U_NON, i, C_NONE)) != NULL)
			{
				mUnits[index][currentUnit] = countries[j].abbreviation;
				if (utype(unit) == U_ARMY)
				{
					currentUnit++;
					mUnits[index][currentUnit] = 'a';
				}
				else if (utype(unit) == U_WING)
				{
					// use 'p' instead of 'w' since 'w' means 'west coast'
					currentUnit++;
					mUnits[index][currentUnit] = 'p';
				}
				else if (isbicoastal(map+i) && utype(unit) == U_FLEET)
				{
					currentUnit++;
					mUnits[index][currentUnit] = coast2char(ucoast(unit));
				}
			}
			// if country owns supply center for this sector
			if (issupp(map+i) && inslist(supplies[j], i))
			{
				mSupplies[index][currentSupply] = countries[j].abbreviation;
				if (isblocked(getsupply(supplies[j],i)))
				{
					mSupplies[index][++currentSupply] = 'b';
				}
			}
			if (island(map+i) && inslist(lands[j], i))
			{
				mLands[index][currentLand] = countries[j].abbreviation;
			}
		}
		currentUnit++;
		if (issupp(map+i))
			currentSupply++;
		else if (island(map+i))
			currentLand++;
	}
	
	// go through all the countries
	memset(mBuilds[index], 0, sizeof(ShortInfoString));
	char* buildString = mBuilds[index];
	mOrders[index] = new Orderlist[NUM_COUN+1];
	for (i = 0; i <= NUM_COUN; ++i)
	{
		initolist( mOrders[index][i] );
	}
	for (i = 1; i <= NUM_COUN; ++i)
	{
		// copy build
		sprintf(buildString, "%d ", countries[i].builds);
		buildString += strlen(buildString);
		// copy orders, too
		copyolist( mOrders[index][i], orders[i] );
	}
	
	// copy the dislodges data
	if (dislodges > 0 && (season == S_SUMMER || season == S_AUTUMN))
	{
		mDislodges[index] = dislodges;
		mDslglist[index] = dslglist;
	}
	else
	{
		mDislodges[index] = 0;
		mDslglist[index].Clean();
	}	
}


void 
History::AppendCurrentState()
{
	// if overfull, reallocate
	if (mNumStored == mNumAllocated)
	{
		Reallocate();
	}
	
	CopyStateToIndex(mNumStored);
		
	// increment storage
	mCurrent = mNumStored;
	mNumStored++;
}

// set current history to current game state (for editing)
void 
History::CopyCurrentState()
{
	// delete the current orders
	for (int c = 0; c <= NUM_COUN; c++)
		cleanorders( mOrders[mCurrent][c] );
	delete [] mOrders[mCurrent];
	mOrders[mCurrent] = NULL;
	
	CopyStateToIndex( mCurrent );
}	

bool 
History::SetAsCurrentState()
{
	/* free up pointers */
	int i;
	for (i = 0; i <= NUM_COUN; i++)
	{
		if (units != NULL)
			cleanunits(&units[i]);
		if (supplies != NULL)
			cleansupplies(&supplies[i]);
		if (lands != NULL)
			cleansupplies(&lands[i]);
		if (orders != NULL)
			cleanorders(orders[i]);
		countries[i].builds = 0;
	}
    
    dslglist.Clean();
    dislodges = 0;

	// set season and year
	season = mSeason[mCurrent];
	year = mYear[mCurrent];
	
	// set units and supplies
	// go through all the spaces
	int currentUnit = 0;
	int currentSupply = 0;
	int currentLand = 0;
	for (i = 1; i <= NUM_SECT; ++i)
	{
		// add the units
		short country = char2country(mUnits[mCurrent][currentUnit]);
		if (country != C_NONE)
		{
			// if army
			if (mUnits[mCurrent][currentUnit+1] == 'a')
			{
				if (!addunit(&units[country], U_ARM, i, C_NONE, country))
				{
					cleanunits(&units[country]);
					return false;
				}
				currentUnit++;
			}
			// if wing (use 'p' instead of 'w' since 'w' means 'west coast')
			else if (mUnits[mCurrent][currentUnit+1] == 'p')
			{
				if (!addunit(&units[country], U_WNG, i, C_NONE, country))
				{
					cleanunits(&units[country]);
					return false;
				}
				currentUnit++;
			}
			else
			{
				CoastType coast = C_NONE; 
				if (isbicoastal(map+i))
				{
					coast = char2coast(mUnits[mCurrent][currentUnit+1]);
					currentUnit++;
				}
				if (!addunit(&units[country], U_FLT, i, coast, country))
				{
					cleanunits(&units[country]);
					return false;
				}
			}
		}	
		currentUnit++;
		
		// add the supplies
		if (issupp(map+i))
		{
			country = char2country(mSupplies[mCurrent][currentSupply]);
			if (!addsupply(&supplies[country], i, country))
			{
				cleansupplies(&supplies[country]);
				return false;
			}
			// if blocked
			if (mSupplies[mCurrent][currentSupply+1] == 'b')
			{
				blocksupply(&supplies[country], i);
				currentSupply++;
			}
			currentSupply++;
		}
		else if (island(map+i))
		{
			country = char2country(mLands[mCurrent][currentLand]);
			if (!addsupply(&lands[country], i, country))
			{
				cleansupplies(&lands[country]);
				return false;
			}
			currentLand++;
		}
	}	
	
	// set adjustments & orders   
	adjustments = mAdjustments[mCurrent];
	char* buildString = mBuilds[mCurrent];
	for (i = 1; i <= NUM_COUN; ++i)
	{
		countries[i].builds = atoi(buildString);
		buildString = skipword(buildString);
		copyolist( orders[i], mOrders[mCurrent][i] );
	}
	
	// set dislodges
	dislodges = mDislodges[mCurrent];
	dslglist = mDslglist[mCurrent];
	
	return true;
}


void 
History::Reallocate()
{
	mNumAllocated += 5;
	unsigned int i;
	LongInfoString* newLongInfoString = new LongInfoString[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		strcpy(newLongInfoString[i], mUnits[i]);
	}
	delete [] mUnits;
	mUnits = newLongInfoString;
	
	MediumInfoString* newInfoString = new MediumInfoString[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		strcpy(newInfoString[i], mSupplies[i]);
	}
	delete [] mSupplies;
	mSupplies = newInfoString;

	MediumInfoString* newLandString = new MediumInfoString[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		strcpy(newLandString[i], mLands[i]);
	}
	delete [] mLands;
	mLands = newLandString;

	int* newInt = new int[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		newInt[i] = mAdjustments[i];
	}
	delete [] mAdjustments;
	mAdjustments = newInt;
		
	ShortInfoString* newShortInfoString = new ShortInfoString[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		strcpy(newShortInfoString[i], mBuilds[i]);
	}
	delete [] mBuilds;
	mBuilds = newShortInfoString;

	Orderlist** newOrders = new Orderlist*[mNumAllocated];
	for (i = 0; i < mNumAllocated; ++i)
	{
		newOrders[i] = NULL;
	}
	for (i = 0; i < mNumStored; ++i)
	{
		newOrders[i] = mOrders[i];		
		mOrders[i] = NULL;
	}
	delete [] mOrders;
	mOrders = newOrders;
	
	newInt = new int[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		newInt[i] = mDislodges[i];
	}
	delete [] mDislodges;
	mDislodges = newInt;

	DislodgeList* newDslglist = new DislodgeList[mNumAllocated];
	for (i = 0; i < mNumAllocated; ++i)
	{
		newDslglist[i].Clean();
	}
	for (i = 0; i < mNumStored; ++i)
	{
		newDslglist[i] = mDslglist[i];
	}
	delete [] mDslglist;
	mDslglist = newDslglist;
		
	newInt = new int[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		newInt[i] = mYear[i];
	}
	delete [] mYear;
	mYear = newInt;
		
	int* newSeason = new int[mNumAllocated];
	for (i = 0; i < mNumStored; ++i)
	{
		newSeason[i] = mSeason[i];
	}
	delete [] mSeason;
	mSeason = newSeason;
}

bool
History::Read( TextFile& file )
{
	Clear();
	
	char inString[256];
	int size;
	bool version2 = false;
	bool withlandlist = false;
	if (!file.ReadLine(inString, 255))
		return false;
	if (*inString == 'v')
	{
		version2 = true;
		if (inString[1] == '3')
			withlandlist = true;
		if (!file.ReadLine(inString, 255))
			return false;
	}
	if (sscanf(inString, "%d", &size) != 1)
	{
		return false;
	}
	if (size <= 0)
	{
		// skip the -1, if any
		(void) file.ReadLine(inString, 255);
		return true;
	}
	
	// make enough room for everything
	while (mNumAllocated < (unsigned int)size)
		Reallocate();
	mNumStored = size;
	
	// copy data
	short season, year;
	if (!file.ReadLine(inString, 255) 
		|| sscanf(inString, "%hd %hd", &season, &year) != 2)
		return false;
	for (unsigned int i = 0; i < mNumStored; ++i)
	{
		mSeason[i] = season;
		mYear[i] = year;
		if (!file.ReadLine(mUnits[i], 2*MAX_SECT - 1))
			return false;
		if (!file.ReadLine(mSupplies[i], 511))
			return false;
		if (withlandlist)
		{
			if (!file.ReadLine(mLands[i], 511))
			return false;
		}
		else 
		{
			memset(mLands[i], 0, sizeof(MediumInfoString));
		}
		if (!file.ReadLine(inString, 255) || sscanf(inString, "%d", &mAdjustments[i]) != 1)
			return false;
		if (!file.ReadLine(mBuilds[i], 255))
			return false;
		if (!file.ReadLine(inString, 255) || sscanf(inString, "%d", &mDislodges[i]) != 1)
			return false;
		mDslglist[i].Clean();
		for (int j = 0; j < mDislodges[i]; ++j)
		{
			if (!file.ReadLine(inString, 255))
				return false;
			Dislodge* dislodge = new Dislodge();
			if (!string2dislodge(dislodge, inString) || !mDslglist[i].Add( dislodge ))
			{
				delete dislodge;
				return false;
			}
		}
		// allocate space for orders
		mOrders[i] = new Orderlist[NUM_COUN+1];
		for (int c = 0; c <= NUM_COUN; ++c)
		{
			initolist( mOrders[i][c] );
		}
		// read orders
		if (version2)
		{
			while (true)
			{
				Order order;
				if (!file.ReadLine(inString, 255))
					return false;
				// make sure we're not at the next phase
				if (sscanf(inString, "%hd %hd", &season, &year) == 2)
					break;
				// make sure we're not at the end! (trailing -1)
				if (*inString == '-')
					break;
				// it's an order, parse it
				order = neworder();
				// if can't parse order and is not retreat
				if (!string2order(order, inString) 
					&& ofails(order) != R_RETREAT)
				{
					// fail
					deleteorder(order);
					return false;
				}	
				// add to list
				appendorder(mOrders[i][ocountry(order)], order);	
			}
		}
		else if (i < mNumStored-1)
		{
			if (!file.ReadLine(inString, 255) 
				|| sscanf(inString, "%hd %hd", &season, &year) != 2)
				return false;
		}
	}
	
	return true;
}

bool
History::Write( TextFile& file, bool rp )
{
	char outString[256];
	Order order;
	long count;
	
	if (rp)
	{
		sprintf(outString, "v2");
	}
	else
	{
		sprintf(outString, "v3");
	}
	if (!file.WriteLine(outString))
		return false;
	sprintf(outString, "%d", mNumStored-1);
	if (!file.WriteLine(outString))
		return false;
	for (unsigned int i = 0; i < mNumStored-1; ++i)
	{
		sprintf(outString, "%d %d", mSeason[i], mYear[i]);
		if (!file.WriteLine(outString))
			return false;
		if (!file.WriteLine(mUnits[i]))
			return false;
		if (!file.WriteLine(mSupplies[i]))
			return false;
		if (!rp)
		{
			if (!file.WriteLine(mLands[i]))
				return false;
		}
		sprintf(outString, "%d", mAdjustments[i]);
		if (!file.WriteLine(outString))
			return false;
		if (!file.WriteLine(mBuilds[i]))
			return false;
		sprintf(outString, "%d", mDislodges[i]);
		if (!file.WriteLine(outString))
			return false;
		for (Dislodge* current = mDslglist[i].Start(); 
			 current != mDslglist[i].End();
			 current = mDslglist[i].Next(current) )
		{
			dislodge2string(outString, current);
			if (!file.WriteLine(outString))
				return false;
		}
		for (int c = 1; c <= NUM_COUN; c++)
		{
			for (order = startorder(mOrders[i][c]); isorder(order); nextorder(order))
			{
	       		sorder2string(outString, order, false, true);
//	       		strcat(text, "\n");
	       		count = strlen(outString);
		   		if (count > 0 && !file.WriteLine(outString))
		   		{
					return false;
				}
			}	
		}
	}
	sprintf(outString, "-1");
	if (!file.WriteLine(outString))
		return false;
	
	return true;
}

bool
History::WriteLastState( TextFile& outfile, bool rp )
{
	char string[256];

	GetIndCString(string, kSeasonStringsID, mSeason[mNumStored-1]);
	if (!sprintf(string, "%s %d", string, mYear[mNumStored-1]) || !outfile.WriteLine(string))
	{	
		gErrNo = errFile;
		return false;
	}
	
	if (!sprintf(string, "%d", mAdjustments[mNumStored-1]) || !outfile.WriteLine(string))
	{	
		gErrNo = errFile;
		return false;
	}
	
	if (!sprintf(string, "%d", NUM_COUN) || !outfile.WriteLine(string))
	{	
		gErrNo = errFile;
		return false;
	}
	
	char* buildString = mBuilds[mNumStored-1];
	for (int c = 1; c <= NUM_COUN; c++)
	{
		// write out builds
		if (!sprintf(string, "%d", atoi(buildString)) || !outfile.WriteLine(string))
		{	
			gErrNo = errFile;
			return false;
		}
		buildString = skipword(buildString);

		// write out units and supplies, and other land (in RW-mode) 
		char unitString[1024] = {0};
		char supplyString[1024] = {0};
		char landString[1024] = {0};
		char *us = unitString;
		char *ss = supplyString;
		char *ls = landString;
		int currentUnit = 0;
		int currentSupply = 0;
		int currentLand = 0;
		
		for (int i = 1; i <= NUM_SECT; ++i)
		{
			// add the units
			short country = char2country(mUnits[mNumStored-1][currentUnit]);
			if (country != C_NONE)
			{
				CoastType coast = C_NONE; 
				UnitType unit = U_ARM;
				// if army
				if (mUnits[mNumStored-1][currentUnit+1] == 'a')
				{
					currentUnit++;
				}
				// if wing (use 'p' instead of 'w' since 'w' means 'west coast')
				else if (mUnits[mNumStored-1][currentUnit+1] == 'p')
				{
					unit = U_WNG;
					currentUnit++;
				}
				else
				{
					unit = U_FLT;
					if (isbicoastal(map+i))
					{
						coast = char2coast(mUnits[mNumStored-1][currentUnit+1]);
						currentUnit++;
					}
				}
				if (country == c)
				{
					// add the info to the string
					strcpy(us, sunit2string(unit));
					us += strlen(us);
					*us = ' ';
					us++;
					strcpy(us, sreg2string(i));
					us += strlen(us);
					strcpy(us, scoast2string(coast));
					us += strlen(us);
					*us++ = ' ';
					*us = '\0';
				}
			}	
			currentUnit++;
			
			// add the supplies
			if (issupp(map+i))
			{
				country = char2country(mSupplies[mNumStored-1][currentSupply]);
				if (country == c)
				{
					strcpy(ss, sreg2string(i));
					ss += strlen(ss);
					*ss++ = ' ';
					if (mSupplies[mNumStored-1][currentSupply+1]=='b')
					{	
						strcat(ss, "(* blocked *)");
						ss += strlen(ss);
						currentSupply++;
					}
					*ss = '\0';
				}
				currentSupply++;
			}
			
			// add other land sectors
			
			else if (!rp && island(map+i))
			{
				country = char2country(mLands[mNumStored-1][currentLand]);
				if (country == c)
				{
					strcpy(ls, sreg2string(i));
					ls += strlen(ls);
					*ls++ = ' ';
					*ls = '\0';
				}
				currentLand++;
			}
			
		}	
	

		// write out units and supplies (whee!)
		if (!outfile.WriteLine(supplyString))
		{
			gErrNo = errFile;
			return false;
		}
		
		if (!rp)
		{
			if (!outfile.WriteLine(landString))
			{
				gErrNo = errFile;
				return false;
			}
		}

		if (!outfile.WriteLine(unitString))
		{
			gErrNo = errFile;
			return false;
		}		
	}
	
	// dislodges 
	if (!sprintf(string, "%d", mDislodges[mNumStored-1]) || !outfile.WriteLine(string))
	{	
		gErrNo = errFile;
		return false;
	}
	
	for (Dislodge* current = mDslglist[mNumStored-1].Start(); 
		 current != mDslglist[mNumStored-1].End();
		 current = mDslglist[mNumStored-1].Next(current) )
	{
		dislodge2string(string, current);
		if (!outfile.WriteLine(string))
			return false;
	}		
	
	return true;
}

bool
History::WriteResults( TextFile& file, char* name )
{
	// only valid if we're not at the end
	if (mCurrent+1 >= mNumStored)
		return false;

	// simulate next phase
	short numunits[MAX_COUN], numsupplies[MAX_COUN], numlands[MAX_COUN],
		  numbuilds[MAX_COUN];
	UnitList newunits[MAX_COUN+1];
	SupplyList newsupplies[MAX_COUN+1];
	SupplyList newlands[MAX_COUN+1];
	DislodgeList newdslglist;
	short newdislodges = 0;
	short newadjustments = 0;
	sim_next_phase(newunits, newsupplies, newlands, numunits, numsupplies, numlands, newdislodges, newdslglist, 
			   newadjustments, numbuilds, (season == S_SPRING || season == S_FALL));
	newdislodges = mDislodges[mCurrent+1];
	newadjustments = mAdjustments[mCurrent+1];
	bool retVal = ::WriteResults( file, season, year, name,
						   orders, newunits, newsupplies, 
						   numunits, numsupplies, 
						   newdislodges, mDslglist[mCurrent+1], 
						   newadjustments, numbuilds );
	newdslglist.Clean();
	for (int c = 0; c <= NUM_COUN; ++c)
	{
		// delete units and supplies
		cleanunits(&newunits[c]);
		cleansupplies(&newsupplies[c]);
		cleansupplies(&newlands[c]);
	}	

	return retVal;
}

bool
History::WriteResultsCSV( TextFile& file, char* name )
{
	// only valid if we're not at the end
	if (mCurrent+1 >= mNumStored)
		return false;

	// simulate next phase
	short numunits[MAX_COUN], numsupplies[MAX_COUN], numlands[MAX_COUN],
		  numbuilds[MAX_COUN];
	UnitList newunits[MAX_COUN+1];
	SupplyList newsupplies[MAX_COUN+1];
	SupplyList newlands[MAX_COUN+1];
	DislodgeList newdslglist;
	short newdislodges = 0;
	short newadjustments = 0;
	sim_next_phase(newunits, newsupplies, newlands, numunits, numsupplies, numlands, newdislodges, newdslglist, 
			   newadjustments, numbuilds, (season == S_SPRING || season == S_FALL));
	newdislodges = mDislodges[mCurrent+1];
	newadjustments = mAdjustments[mCurrent+1];
	bool retVal = ::WriteResultsCSV( file, season, year, name,
						   orders, newunits, newsupplies, 
						   numunits, numsupplies, 
						   newdislodges, mDslglist[mCurrent+1], 
						   newadjustments, numbuilds );
	newdslglist.Clean();
	for (int c = 0; c <= NUM_COUN; ++c)
	{
		// delete units and supplies
		cleanunits(&newunits[c]);
		cleansupplies(&newsupplies[c]);
		cleansupplies(&newlands[c]);
	}	

	return retVal;
}
