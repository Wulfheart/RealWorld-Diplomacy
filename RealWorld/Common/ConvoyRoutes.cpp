#include "ConvoyRoutes.h"

//---------------------------------------------------------------------------
// Global variables
//---------------------------------------------------------------------------
ConvoyRoutes convoyroutes;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------



Waypoint::Waypoint(short via)
{
	mSector = via;
	mNext = NULL;
}

Waypoint::~Waypoint(void) 
{
	if (mNext != NULL)
	{
		delete mNext;
	}
}

Waypoint*
Waypoint::GetNext()
{
	return mNext;
}

bool
Waypoint::SetNext(short via)
{
	return ((mNext = new Waypoint(via)) != NULL);
}

short
Waypoint::GetPosition()
{
	return mSector;
}

ConvoyRoute::ConvoyRoute(short start)
{
	mStart = start;
	mFinish = 0;
	mVia = NULL;
	mValid = true;
	mFromSuez = false;
	mToSuez = false;
}

ConvoyRoute::~ConvoyRoute(void)
{
	delete mVia;
}

short 
ConvoyRoute::GetStart(void)
{
	return mStart;
}

short 
ConvoyRoute::GetFinish(void)
{
	return mFinish;
}

bool 
ConvoyRoute::AddWaypoint(short via)
{

	Waypoint* w = mVia;
	if (mVia == NULL)
	{
		mValid = mValid && IsValidStep(mStart, via, false);
		return ((mVia = new Waypoint(via)) != NULL);
	}
	else
	{
		while (w->GetNext() != NULL)
		{
			w = w->GetNext();
			if (w->GetPosition() == via)
			{
				return 0;
			}
		}
		mValid = mValid && IsValidStep(w->GetPosition(), via, false);
		return w->SetNext(via);
	}
}

void
ConvoyRoute::SetFinish(short finish)
{
	Waypoint* w = GetLastWaypoint();
	mValid = mValid && IsValidStep(w->GetPosition(), finish, true);
	mFinish = finish;
}

Waypoint*
ConvoyRoute::GetVia(void)
{
	return mVia;
}

Waypoint*
ConvoyRoute::GetLastWaypoint(void)
{
	Waypoint* w = mVia;
	if (mVia == NULL)
	{
		return NULL;
	}
	else
	{
		while (w->GetNext() != NULL)
		{
			w = w->GetNext();
		}
		return w;
	}
}

short
ConvoyRoute::GetLastPosition(void)
{
	Waypoint* w = GetLastWaypoint();
	if (w == NULL)
	{
		return mStart;
	}
	else
	{
		return w->GetPosition();
	}
}

void 
ConvoyRoute::Remove()
{
	mStart = 0;
	mFinish = 0;
	delete mVia;
	mVia = NULL;
}

/* returns true if a convoy can take this route */
bool 
ConvoyRoute::IsValidStep(short sector, short nextsector, bool laststep)
{
	 Neighbor lp, slp;
	 Sector sect = getsector(map, sector);
     short i, currsector;
	 
	 for (i = 1; i<4; i++)
	 {
		switch (i)
		{
		case 1:
			slp = startneighbor(coast1neigh(sect));
			break;
		case 2:
			/* check the second coast */
			if (isbicoastal(getsector(map,sector)))
			{
				slp = startneighbor(coast2neigh(sect));
			}
			else 
			{
				slp = NULL;
			}
			break;
		case 3:
			/* check the third coast */
			if (hasthirdcoast(sect))
			{
				slp = startneighbor(coast3neigh(sect));
			}
			else
			{
				slp = NULL;
			}
			break;
		default:
			slp = NULL;
			break;
		}
		sect = getsector(map, nextsector);
		for (lp = slp; isneighbor(lp); nextneighbor(lp)) 
		{
	  		currsector = nsector(lp);
	  		/* got to destination! */
	  		if (currsector == nextsector)
			{
				if (isfromsuez(lp))
				{
					mFromSuez = true;
				}
				else if (istosuez(lp))
				{
					mToSuez = true;
				}
				if (laststep)
				{
					return (island(sect) != 0);
				}
				else
				{
					return ((issea(sect) || isconvoyable(sect)) && utype(sectunit(sect)) == U_FLEET);
				}
			}
		}
	 }
	 return false;
}

bool
ConvoyRoute::IsValid()
{
	return mValid;
}

bool
ConvoyRoute::IsFromSuez()
{
	return mFromSuez;
}

bool
ConvoyRoute::IsToSuez()
{
	return mToSuez;
}

ConvoyRoutes::ConvoyRoutes(void)
{
	mNumAllocated = 0;
	mNumStored = 0;
	mCurrent = 0;
	mRoutes = NULL;
}

ConvoyRoutes::~ConvoyRoutes(void)
{
	delete [] mRoutes;
}

void 
ConvoyRoutes::Reallocate(short limit)
{
	mNumAllocated = limit;
	unsigned int i;
	ConvoyRoute** newRoutes = NULL;

	if (limit > 0)
	{
		newRoutes = new ConvoyRoute*[mNumAllocated];
	}
	for (i = 0; i < mNumStored; ++i)
	{
		newRoutes[i] = mRoutes[i];
	}
	delete [] mRoutes;
	mRoutes = newRoutes;
}

ConvoyRoute*
ConvoyRoutes::Select(short startsector)
{
	unsigned int i;
	for (i = 0; i < mNumStored; ++i)
	{
		if (mRoutes[i]->GetStart() == startsector)
		{
			mCurrent = i;
			return mRoutes[mCurrent];
		}
	}
	return NULL;
}

int
ConvoyRoutes::Insert(ConvoyRoute* crp)
{
	unsigned int i;
	for (i = 0; i < mNumStored; ++i)
	{
		if (mRoutes[i]->GetStart() == crp->GetStart())
		{
			// delete old convoy route for unit at sector 
			mRoutes[i]->Remove();
			delete mRoutes[i];
			mRoutes[i] = crp;
			mCurrent = i;
			return 1;
		}
	}
	if (i == mNumStored)
	{
		Reallocate(mNumStored+1);
		mNumStored++;
		mRoutes[i] = crp;
		mCurrent = i;
		return 2;
	}

	return 0;
}

bool
ConvoyRoutes::Remove(short startsector)
{
	ConvoyRoute* crp = Select(startsector);
	unsigned int i;

	if (crp == NULL)
	{
		return false;
	}
	else
	{
		mRoutes[mCurrent]->Remove();
		delete mRoutes[mCurrent];
		for (i=mCurrent; i < mNumStored - 1; )
		{
			mRoutes[i] = mRoutes[++i];
		}
		mRoutes[i] = NULL;
		Reallocate(--mNumStored);
		return true;
	}
}

void
ConvoyRoutes::Clear()
{
	unsigned int i;
	for (i = 0; i < mNumStored; ++i)
	{
		mRoutes[i]->Remove();
	}
	delete [] mRoutes;
	mCurrent = 0;
	mNumStored = 0;
	Reallocate(0);
}
