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
#include "Dislodge.h"
#include "Unit.h"
#include "MapData.h"
#include <string.h>
#include "Strings.h"
#include "Parse.h"

//---------------------------------------------------------------------------
// @ Dislodge::Dislodge()
//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
Dislodge::Dislodge() :
	unit( U_NONE ),
	u_sector( 0 ),
	u_coast( C_NONE ),
	u_own( 0 ),
	type( DB_GEN ),
	retreats( 0 ),
	next( 0 )
{
		
}


//---------------------------------------------------------------------------
// @ Dislodge::~Dislodge()
//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
Dislodge::~Dislodge()
{
	cleanneighbors(&retreats);
}


//---------------------------------------------------------------------------
// @ dislodge2string()
//---------------------------------------------------------------------------
// Build string from dislodge info
//---------------------------------------------------------------------------
void dislodge2string(char *s, Dislodge *dislodge)
{
	if (dislodge == NULL)
	{
		*s = '\0';
		return;
	}
	strcpy(s, cntadj2string(dislodge->u_own));
	s += strlen(s);
	*s = ' ';
	s++;
	strcpy(s, sunit2string(dislodge->unit));
	s += strlen(s);
	*s = ' ';
	s++;
	strcpy(s, sreg2string(dislodge->u_sector));
	s += strlen(s);
	strcpy(s, scoast2string(dislodge->u_coast));
	s += strlen(s);
	*s = ' ';
	s++;
	nlist2string(s, dislodge->retreats);
	s += strlen(s);
	*s = '\0';
	
}

void dislodge2CSV(char *s, Dislodge *dislodge)
{
	if (dislodge == NULL)
	{
		*s = '\0';
		return;
	}
	strcpy(s, country2string(dislodge->u_own));
	s += strlen(s);
	*s = ',';
	s++;
	strcpy(s, sunit2string(dislodge->unit));
	s += strlen(s);
	*s = ',';
	s++;
	strcpy(s, sreg2string(dislodge->u_sector));
	s += strlen(s);
	*s = ',';
	s++;
	strcpy(s, scoast2string(dislodge->u_coast));
	s += strlen(s);
	*s = ',';
	s++;
	nlist2CSV(s, dislodge->retreats);
	s += strlen(s);
	*s = '\0';
	
}


//---------------------------------------------------------------------------
// @ dislodge2lstring()
//---------------------------------------------------------------------------
// Build string using long province names from dislodge info
//---------------------------------------------------------------------------
void 
dislodge2lstring(char *s, Dislodge *dislodge)
{
	char pstring[256];
	
	if (dislodge == NULL)
	{
		*s = '\0';
		return;
	}
	strcpy(s, cntadj2string(dislodge->u_own));
	s += strlen(s);
	*s = ' ';
	s++;
	strcpy(s, sunit2string(dislodge->unit));
	s += strlen(s);
	*s = ' ';
	s++;
	strcpy(s, lreg2string(dislodge->u_sector));
	s += strlen(s);
	strcpy(s, scoast2string(dislodge->u_coast));
	s += strlen(s);
	if (dislodge->type != DB_GEN)
	{
		GetIndCString(pstring, kDislodgeStringsID, dislodge->type);
		strcpy(s, pstring);
		s += strlen(s);
	}
	else if (dislodge->retreats != NULL)
	{
		GetIndCString(pstring, kDislodgeStringsID, DB_GEN);
		strcpy(s, pstring);
		s += strlen(s);
		nlist2lstring(s, dislodge->retreats);
		s += strlen(s);
	}
	else
	{
		GetIndCString(pstring, kDislodgeStringsID, DB_DSB);
		strcpy(s, pstring);
		s += strlen(s);
	}
	*s = '\0';
	
}  
     
//---------------------------------------------------------------------------
// @ string2dislodge()
//---------------------------------------------------------------------------
// Parse dislodge info from string
//---------------------------------------------------------------------------
int string2dislodge(Dislodge *dislodge, char *string)
{
	if ( dislodge == 0 || string == 0 )
		return 0;

	char *word, *match;
	short sector, owner;
	CoastType coast;
	UnitType unit;
	NeighborList nl = NULL;
	
	word = skipspace(string);
	
	if (samestring(word, "The "))
		word = skipword(word);
	
	word = string2country(&owner, word);
	if (owner == 0)
		return 0;
		
	word = string2unit(&unit, word);
	if (unit == 0)
		return 0;
		
	if (samestring(word, "in "))
		word = skipword(word);
		
	if (samestring(word, "the "))
		word = skipword(word);
			
	word = string2sector(&sector, word);
	if (sector == 0)
		return 0;
		
	word = string2coast(&coast, word);
	
	match = samestring(word, "with no valid retreats");
	if (match)
	{
		nl = NULL;
	}
	else
	{
		match = samestring(word, "can retreat to ");
		if (match)
			word = match;	
		string2nlist(word, &nl);
	}
		
	dislodge->Set( unit, sector, coast, owner, DB_GEN, nl );

	return 1;

}


//---------------------------------------------------------------------------
// @ DislodgeList::DislodgeList()
//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
DislodgeList::DislodgeList() :
	mList( 0 ),
	mSize( 0 )
{
}


//---------------------------------------------------------------------------
// @ DislodgeList::~DislodgeList()
//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
DislodgeList::~DislodgeList()
{
	Clean();
}

//---------------------------------------------------------------------------
// @ DislodgeList::DislodgeList()
//---------------------------------------------------------------------------
// Copy constructor
//---------------------------------------------------------------------------
DislodgeList::DislodgeList( const DislodgeList& list )
{
	Copy( list );
}

//---------------------------------------------------------------------------
// @ DislodgeList::operator=()
//---------------------------------------------------------------------------
// Assignment operator
//---------------------------------------------------------------------------
DislodgeList& DislodgeList::operator=( const DislodgeList& list )
{
	if (&list == this)
		return *this;
	
	Clean();
	Copy( list );	
		
	return *this;	
}

//---------------------------------------------------------------------------
// @ DislodgeList::Copy()
//---------------------------------------------------------------------------
// Protected copy method (for above two methods)
//---------------------------------------------------------------------------
void DislodgeList::Copy( const DislodgeList& list )
{
	for ( Dislodge* current = list.Start();
		 current != list.End();
		 current = list.Next( current ) )
	{
		NeighborList retreats;
		copyneighbors( &retreats, current->Retreats() );
		Add( current->Unit(), current->Sector(), current->Coast(), current->Owner(),
			 current->Type(), retreats );
	}
}


//---------------------------------------------------------------------------
// @ DislodgeList::Add()
//---------------------------------------------------------------------------
// Add new dislodge to list (parameters version)
//---------------------------------------------------------------------------
bool 
DislodgeList::Add( UnitType unit, short sector, CoastType coast, short owner, DislodgeType type, 
					NeighborList retreats )
{
    Dislodge *nlist;
     
	if (!(nlist = new Dislodge()))
	  	return false; 

	nlist->Set( unit, sector, coast, owner, type, retreats );

	if ( !Add( nlist ) )
	{
		delete nlist;
		return false;
	}
	
	return true;
}


//---------------------------------------------------------------------------
// @ DislodgeList::Add()
//---------------------------------------------------------------------------
// Add new dislodge to list (Dislodge* version)
//---------------------------------------------------------------------------
bool 
DislodgeList::Add( Dislodge* dislodge )
{
    Dislodge *u1, *u2;
    
	u1 = mList;
	u2 = NULL;
	while (u1) 
	{
		// don't add if one with this province already there
	  	if (u1->Sector() == dislodge->Sector()) 
	  		return false;
		if (u1->Sector() > dislodge->Sector()) 
	    	break;	  
	  	u2 = u1;
	  	u1 = u1->next;
	}
    
    // if at head of list
    if (!u2) 
    {
	  	dislodge->next = mList;
	  	mList = dislodge;
    }
    // insert in middle
    else 
    {
	  	dislodge->next = u1;
	  	u2->next = dislodge;
	}

	++mSize;
	
	return true;
}


//---------------------------------------------------------------------------
// @ DislodgeList::Delete()
//---------------------------------------------------------------------------
// Delete dislodge from list
//---------------------------------------------------------------------------
bool DislodgeList::Delete( short sector )
{
    Dislodge *p=NULL;
	Dislodge *o=mList;

    // find the right dislodge (if any)
  	while (o != NULL) 
  	{
	  	if (o->Sector() == sector) 
	  		break;
	  	p = o;
	  	o = o->next;
    }
    if (o == NULL)
	  	return false;

	// if at head of list
    if (p == NULL) 
	  	mList = mList->next;
	// delete from middle
   	else
	  	p->next = o->next;

	--mSize;
    delete o;
         
 	return true;
}


//---------------------------------------------------------------------------
// @ DislodgeList::Clean()
//---------------------------------------------------------------------------
// Clear out list
//---------------------------------------------------------------------------
void DislodgeList::Clean()
{
	Dislodge *o, *p;

   	o = mList;
   	while (o) 
   	{
	  	p = o->next;
	  	delete o;
	  	o = p;
   	}

   	mList = 0;
   	mSize = 0;
}


//---------------------------------------------------------------------------
// @ DislodgeList::Contains()
//---------------------------------------------------------------------------
// Returns true if list contains dislodge at this province
//---------------------------------------------------------------------------
bool DislodgeList::Contains( short sector ) const
{
	return ( Find( sector ) != NULL );
}


//---------------------------------------------------------------------------
// @ DislodgeList::Contains()
//---------------------------------------------------------------------------
// Returns first dislodge (hopefully there's only one) at this province
//---------------------------------------------------------------------------
Dislodge *DislodgeList::Find( short sector ) const
{
    Dislodge *o;

    o = mList;
  	while (o != NULL) 
  	{
	  	if (o->Sector() == sector) 
	  		break;
	  	o = o->next;
    }
    return o;
}

