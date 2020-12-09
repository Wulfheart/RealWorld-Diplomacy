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
#ifndef _DislodgeDefs_
#define _DislodgeDefs_

#include "Neighbor.h"
#include "Unit.h"

class DislodgeList;

/* disband types */
enum DislodgeType 
{ 
	DB_GEN = 0,
	DB_NMR,    /* no move received */
	DB_NRR,    /* no retreat received */
	DB_ILL,    /* retreats bounced */
	DB_DSB	   /* no retreats, disbanded */
};

class Dislodge 
{
	friend class DislodgeList;
public:
	Dislodge();
	~Dislodge();
	
	inline void Set( UnitType _unit, short sector, CoastType coast, short owner, DislodgeType _type, NeighborList _retreats )
	{
		unit = _unit;
		u_sector = sector;
		u_coast = coast;
		u_own = owner;
		type = _type;
		retreats = _retreats;
	}

	inline UnitType Unit() const	{ return unit; }
	inline short Sector() const	{ return u_sector; }
	inline CoastType Coast() const 	{ return u_coast; }
	inline short Owner() const	{ return u_own; }
	inline DislodgeType Type() const	{ return type; }
	inline NeighborList Retreats() const { return retreats; }
	
	inline void SetType( DislodgeType _type )   { type = _type; }

	friend int string2dislodge( Dislodge* dislodge, char* s );
	friend void dislodge2string( char *s, Dislodge *dislodge );
	friend void dislodge2lstring( char *s, Dislodge *dislodge );
	friend void dislodge2CSV( char *s, Dislodge *dislodge );

protected:
/* this order is important, must be same as unitnode */
	UnitType unit;
	short u_sector;
	CoastType u_coast;
	short u_own;
	DislodgeType type;
	NeighborList retreats; 
	Dislodge *next;

private:	
	Dislodge( const Dislodge& list );
	Dislodge& operator=( const Dislodge& list );
};


// list of dislodges
class DislodgeList
{
public:
	DislodgeList();
	~DislodgeList();
	DislodgeList( const DislodgeList& list );
	DislodgeList& operator=( const DislodgeList& list );
	
	bool Add( UnitType unit, short sector, CoastType coast, short owner, DislodgeType type, NeighborList retreats );
	bool Add( Dislodge* dislodge );
	bool Delete( short sector );
	void Clean();
	bool Contains( short sector ) const;
	Dislodge *Find( short sector ) const;
	inline short Size() const { return mSize; }
	
	inline Dislodge* Start() const						{ return mList; }
	inline Dislodge* End() const						{ return 0; }
	inline Dislodge* Next( Dislodge* dislodge ) const	{ return dislodge->next; }
	
protected:
	void Copy( const DislodgeList& list );
	
	Dislodge* mList;
	short	  mSize;
};


#endif
