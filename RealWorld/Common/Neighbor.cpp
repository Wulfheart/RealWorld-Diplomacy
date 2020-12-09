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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Neighbor.h"
#include "MapData.h"
#include "Parse.h"

#define MAX_NEIGHBORS 4000
struct neighnode *neighalloc = NULL;

int totalalloc = 0;

void allocneighbor()
{
	register short i;
	
	neighalloc = (struct neighnode *) malloc(sizeof(struct neighnode)*MAX_NEIGHBORS);

	for (i = 0; i < MAX_NEIGHBORS; i++)
		neighalloc[i].sector = 0;
}

void deallocneighbor()
{
	if (neighalloc != NULL)
		free(neighalloc);
}


Neighbor newneighbor()
{
	register short i;
	
	if (neighalloc == NULL)
		allocneighbor();
	
	for (i = 0; i < MAX_NEIGHBORS; i++)
	{
		if (neighalloc[i].sector == 0)
		{
			totalalloc++;
			return neighalloc+i;
		}
	}

	return NULL;
}

void disposeneighbor(Neighbor np)
{
	totalalloc--;
	np->sector = 0;
	np->coast = C_NONE;
	np->special = NSP_NONE;
	np->next = NULL;
}

/* returns 1 if only one match to type, sector and coast */
/* returns 0 if duplicate match or no match found */
int innlist (NeighborList lp, short sector, CoastType coast, short igncoast)
{
     Neighbor match = NULL; 

     while (lp != NULL) {
	  if (lp->sector == sector
	      && (igncoast || lp->coast == C_NONE || coast == C_NONE || lp->coast == coast)) {
	       if (match == NULL || igncoast)
		    match = lp;
	       else
		    return 0;
	  }
	  lp = lp->next;
     }

     return (match != NULL);
}

/* returns best match to type, sector and coast */
/* returns NULL if duplicate match or no match found */
Neighbor getneighbor (NeighborList lp, short sector, CoastType coast, short igncoast)
{
     Neighbor match = NULL; 

     while (lp != NULL) {
	  if (lp->sector == sector
	      && (igncoast || lp->coast == C_NONE || coast == C_NONE || lp->coast == coast)) {
	       if (match == NULL || igncoast)
		    match = lp;
	       else
		    return NULL;
	  }
	  lp = lp->next;
     }

     return match;
}

void string2nlist(char *string, NeighborList *lp)
{
	char *word;
	short sector;
	CoastType coast;	
	SpecialType special = NSP_NONE;

	word = skipspace(string);
	
	while (wordlen(word) != 0)
	{
		word = string2sector(&sector, word);
		if (sector == 0)
		{
			cleanneighbors(lp);
			break;
		}
		word = string2coast(&coast, word);
		if (samestring(word, "/mx"))	
 		{	
 			special = NSP_ISTHMUS;	
 			word = skipword( word );	
 		}
		else if (samestring(word, "/ts"))	
 		{	
 			special = NSP_TOSUEZ;	
 			word = skipword( word );	
 		}
		else if (samestring(word, "/fs"))	
 		{	
 			special = NSP_FROMSUEZ;	
 			word = skipword( word );	
 		}
 		else if (samestring(word, "/hx"))	
 		{	
 			special = NSP_JUMP;	
 			word = skipword( word );	
 		}	
		else if (samestring(word, "/cx"))	
 		{	
 			special = NSP_CANAL;	
 			word = skipword( word );	
 		}
		else
		{
			special = NSP_NONE;
		}
		if (!addneighbor(lp, sector, coast, special))
		{
			cleanneighbors(lp);
		}
		if (samestring(word, "or "))
			word = skipword(word);
	}
}

void neighbor2string(char *s, Neighbor lp)
{
	if (lp == NULL)
	{
		*s = '\0';
		return;
	}
	strcpy(s, sreg2string(lp->sector));
	s += strlen(s);
	strcpy(s, scoast2string(lp->coast));
	s += strlen(s);
	*s = '\0';

}

void neighbor2CSV(char *s, Neighbor lp)
{
	if (lp == NULL)
	{
		*s = '\0';
		return;
	}
	strcpy(s, sreg2string(lp->sector));
	s += strlen(s);
	*s = ',';
	s++;
	strcpy(s, scoast2string(lp->coast));
	s += strlen(s);
	*s = '\0';

}

void neighbor2lstring(char *s, Neighbor lp)
{
	if (lp == NULL)
	{
		*s = '\0';
		return;
	}
	strcpy(s, lreg2string(lp->sector));
	s += strlen(s);
	strcpy(s, scoast2string(lp->coast));
	s += strlen(s);
	*s = '\0';

}
     
void nlist2string(char *s, NeighborList lp)
{

    if (lp == NULL) 
    {
    	*s = '\0';
		return;
    }

    neighbor2string(s, lp);
    s += strlen(s);
    lp = lp->next;
    while (lp != NULL) {
    	*s = ' ';
    	s++;
    	neighbor2string(s, lp);
    	s += strlen(s);
		lp = lp->next;
    }
	*s = '\0';
}


void nlist2CSV(char *s, NeighborList lp)
{

    if (lp == NULL) 
    {
    	*s = '\0';
		return;
    }
	*s = ',';
	s++;
    neighbor2CSV(s, lp);
    s += strlen(s);
    lp = lp->next;
    while (lp != NULL) {
    	*s = ',';
    	s++;
    	neighbor2CSV(s, lp);
    	s += strlen(s);
		lp = lp->next;
    }
	*s = '\0';
}

void
nlist2lstring(char *s, NeighborList lp)
{

    if (lp == NULL) 
    {
    	*s = '\0';
		return;
    }

    neighbor2lstring(s, lp);
    s += strlen(s);
    lp = lp->next;
    while (lp != NULL) {
    	strcat(s, " or ");
    	s += strlen(s);
    	neighbor2lstring(s, lp);
    	s += strlen(s);
		lp = lp->next;
    }
	*s = '\0';
}

#if 0     
void 
printneighbor (lp)
Neighbor lp;
{
     prntsreg(lp->sector, stdout);
     prntscst(lp->coast, stdout);
}

void 
printnlist (lp)
NeighborList lp;
{
     if (isneighbor(lp)) {
	  printneighbor(lp);
	  nextneighbor(lp);
     }
     while (isneighbor(lp)) {
	  fputs(", ", stdout);
	  printneighbor(lp);
	  lp = lp->next;
     }
}
#endif

int addneighbor (NeighborList *lp, short s, CoastType c, SpecialType spc)
{
     Neighbor nlist;
     
     if (!(nlist = newneighbor()))
	  return 0;
     nlist->sector = s;
     nlist->coast = c;
	 nlist->special = spc;
     nlist->next = *lp;
     *lp = nlist;

     return 1;
}

int deleteneighbor (NeighborList *lp, short s)
{
     Neighbor p = NULL;
	 Neighbor o = *lp;

     while (o != NULL) {
	  if (o->sector == s) break;
	  p = o;
	  o = o->next;
     }
     if (o == NULL)
	  return 0;

     if (p == NULL) 
	  *lp = (*lp)->next;
     else
	  p->next = o->next;

     disposeneighbor(o);

     return 1;
     
}

void cleanneighbors (NeighborList *lp)
{
     Neighbor o, p;

     o = *lp;
     while (isneighbor(o)) {
	  p = o->next;
	  disposeneighbor(o);
	  o = p;
     }

     *lp = NULL;
}


void copyneighbors( NeighborList* copy, NeighborList original )
{
	initnlist(*copy);
	for (Neighbor np = startneighbor(original); isneighbor(np); nextneighbor(np))
	{
		addneighbor(copy, nsector(np), ncoast(np), nspecial(np));
	}
}
