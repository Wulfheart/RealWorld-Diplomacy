#pragma once

#include "Order.h"
#include "MapData.h"
#include "Neighbor.h"

class Waypoint
{
public:
	Waypoint(short via);
	~Waypoint(void);
	short GetPosition(void);
	Waypoint* GetNext(void);
	bool SetNext(short via);
protected:
	short			mSector;
	Waypoint*		mNext;
};

class ConvoyRoute
{
public:
	ConvoyRoute(short startsector);
	~ConvoyRoute(void);
	short GetStart(void);
	short GetFinish(void);
	short GetLastPosition(void);
	void SetFinish(short finish);
	bool AddWaypoint(short via);
	Waypoint* GetVia(void);
	Waypoint* GetLastWaypoint(void);
	bool IsValidStep(short sector, short nextsector, bool laststep);
	bool IsValid(void);
	bool IsFromSuez(void);
	bool IsToSuez(void);
	void Remove(void);
protected:
	short			mStart;
	short			mFinish;
	Waypoint*		mVia;
	bool			mValid;
	bool			mFromSuez;
	bool			mToSuez;
};


// list of routes for convoys
class ConvoyRoutes
{
public:
	ConvoyRoutes(void);
	~ConvoyRoutes(void);
	ConvoyRoute* Select(short startsector);
	int Insert(ConvoyRoute* crp);
	bool Remove(short startsector);
	void Clear(void);
protected:
	void Reallocate(short limit);
	unsigned int	mNumStored;
	unsigned int	mNumAllocated;
	unsigned int	mCurrent;
	ConvoyRoute** 	mRoutes;
};

