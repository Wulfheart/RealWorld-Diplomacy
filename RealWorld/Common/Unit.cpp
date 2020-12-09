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
/*
 * unit.c
 *
 * Routines to handle unit and supply center lists
 *
 */

#include "Unit.h"
#include <string.h>
#include "Parse.h"
#include "MapData.h"
#include "Strings.h"

#define MAX_UNITS 512
struct unitstruct unitalloc[MAX_UNITS*2] = {U_NONE,0};
static short numused = 0;

Unit newunit()
{
        register short i;

        for (i = 0; i < MAX_UNITS*2; i++)
                if (unitalloc[i].sector == 0)
                {
                        numused++;
                        return unitalloc+i;
                }

        return NULL;
}

void disposeunit(Unit up)
{
        up->type = U_NONE;
        up->sector = 0;
        up->coast = C_NONE;
        up->next = NULL;
        numused--;
}

/* returns 1 if match to type, sector and coast */
/* returns 0 if no match found */
int  inulist(UnitList lp, UnitType unittype, short sector, CoastType coast)
{

     while (lp != NULL) {
          if (lp->sector == sector && (unittype == U_NON || lp->type == unittype)
              && (lp->coast == C_NONE || coast == C_NONE || lp->coast == coast))
               return 1;

          lp = lp->next;
     }

     return 0;
}

/* returns first match to type, sector and coast */
/* returns NULL if no match found */
Unit
getunit(UnitList lp, UnitType unittype, short sector, CoastType coast)
{

     while (lp != NULL) {
          if (lp->sector == sector && (unittype == U_NON || lp->type == unittype)
              && (lp->coast == C_NONE || coast == C_NONE || lp->coast == coast))
               return lp;

          lp = lp->next;
     }

     return NULL;
}

void
unit2string(char *s, Unit lp, bool abbrev)
{
        if (lp == NULL)
        {
                *s = '\0';
                return;
        }
        strcpy(s, sunit2string(lp->type));
        s += strlen(s);
        *s = ' ';
        s++;
        if (abbrev)
                strcpy(s, sreg2string(lp->sector));
        else
                strcpy(s, lreg2string(lp->sector));
        s += strlen(s);
        strcpy(s, scoast2string(lp->coast));
        s += strlen(s);
        *s = '\0';
}

void
ulist2string(char *line, UnitList lp, bool abbrev)
{
        char* s = line;
    if (lp == NULL)
    {
            *s = '\0';
                return;
    }

    unit2string(s, lp, abbrev);
    s += strlen(s);
        lp = lp->next;
    while (lp != NULL)
    {
            *s++ = ',';
            *s++ = ' ',
            unit2string(s, lp, abbrev);
            s += strlen(s);
                lp = lp->next;
    }
        s = '\0';
}

char *
string2unit(UnitType *unit, char *s)
{
        char *match, *next_word;

        next_word = s;
        *unit = U_NON;
        if ((match = samestring(s, "A ")) || (match = samestring(s, "Army ")))
        {
                *unit = U_ARM;
                next_word = match;
        }
        else if ((match = samestring(s, "F ")) || (match = samestring(s, "Fleet ")))
        {
                *unit = U_FLT;
                next_word = match;
        }
        else if ((match = samestring(s, "W ")) || (match = samestring(s, "Wing ")))
        {
                *unit = U_WNG;
                next_word = match;
        }
        return skipspace(next_word);
}


UnitList
string2ulist(char *string, short country)
{
        char *word;
        UnitList lp;
        UnitType unit;
        short sector;
        CoastType coast;
		
        word = skipspace(string);
        lp = NULL;

        while (wordlen(word) != 0)
        {
                word = string2unit(&unit, word);
                if (unit == U_NON)
                {
                        cleanunits(&lp);
                        return NULL;
                }
                word = string2sector(&sector, word);
                if (sector == 0)
                {
                        cleanunits(&lp);
                        return NULL;
                }
                word = string2coast(&coast, word);
                
 	            if (!addunit(&lp, unit, sector, coast, country))
                {
                        cleanunits(&lp);
                        return NULL;
                }
        }

        return lp;
}



int
ulistsize (UnitList lp)
{
     int size;

     if (lp == NULL) {
          return 0;
     }

     for (size = 1,lp = lp->next; lp != NULL; lp = lp->next, size++)
     {
     }

     return size;

}

void
supply2string(char *s, Unit lp, bool abbrev)
{
        if (lp == NULL)
        {
                *s = '\0';
                return;
        }
        if (abbrev)
		{
                strcpy(s, sreg2string(lp->sector));
				if (lp->coast == S_BLOCKED)
					strcat(s, "(*bl*)");
		}
        else
		{
                strcpy(s, lreg2string(lp->sector));
				if (lp->coast == S_BLOCKED)
					strcat(s, " (*blocked*)");
		}
        s += strlen(s);
        *s = '\0';

}


void
slist2string(char *s, UnitList lp, bool abbrev)
{
    if (lp == NULL)
    {
            *s = '\0';
                return;
    }

    supply2string(s, lp, abbrev);
    s += strlen(s);
        lp = lp->next;
    while (lp != NULL) {
            *s++ = ',';
            *s++ = ' ',
            supply2string(s, lp, abbrev);
            s += strlen(s);
                 lp = lp->next;
   }
   *s++ = '.';
   *s = '\0';

}


SupplyList
string2slist(char *string, short country)
{
        char *word;
        SupplyList lp;
		short sector;
		BlockType block = S_FREE;
		
        word = skipspace(string);
        lp = NULL;

        while (wordlen(word) != 0)
        {
				word = string2sector(&sector, word);
                if (sector == 0)
                {
                        cleanunits(&lp);
                        return NULL;
                }
				// test for (*bl*) here and set "blocked"
				if (word) 
				{
					if (word == strstr(word, "(*bl*)"))
					{
						word = skipword(word);
						block = S_BLOCKED;
					}
				}
		        if (!addunit(&lp, U_NON, sector, block, country))
                {
                        cleanunits(&lp);
                        return NULL;
                }
				block = S_FREE;
		}

        return lp;
}

int
slistsize (SupplyList lp)
{
     int size;

     if (lp == NULL) {
          return 0;
     }

     for (size = 1,lp = lp->next; lp != NULL; lp = lp->next, size++)
     {
     }

     return size;

}

/* add unit to unitlist */
/* returns 1 if worked, 0 otherwise */
int
addunit (UnitList *lp, UnitType u, short s, CoastType c, short o)
{
     Unit nlist, u1, u2;

     u1 = *lp;
     u2 = NULL;
     while (isunit(u1)) {
          if (usector(u1) > s)
               break;
          u2 = u1;
          u1 = u1->next;
     }

     /* already something there */
/*     if (isunit(u1) && usector(u1) == s)
     {
             if (utype(u1) == u)
                     return 1;
             else
                     return 0;
     } */

         if (!(nlist = newunit()))
                  return 0;

     nlist->type = u;
     nlist->sector = s;
     nlist->coast = c;
     nlist->owner = o;

    if (!isunit(u2))
    {
                  nlist->next = *lp;
                  *lp = nlist;
    }
    else
    {
          nlist->next = u1;
          u2->next = nlist;
     }

        if (u != U_NON)
                map[s].unit = nlist;

     return 1;
}


/* delete unit indicated by sector n from list */
/* returns 1 if successful, 0 otherwise */
int
deleteunit (UnitList *lp, short n)
{
     Unit p = NULL;
	 Unit o = *lp;

          while (o != NULL)
          {
                  if (o->sector == n) break;
                  p = o;
                  o = o->next;
    }
    if (o == NULL)
                  return 0;

    if (p == NULL)
                  *lp = (*lp)->next;
           else
                  p->next = o->next;

    if (map[n].unit == o)
             map[n].unit = NULL;
    disposeunit(o);

    return 1;
}

/* delete wings from unitlist */
void
removewings ()
{
	int i;
	Unit o;
	Unit p;

	for (i = 1; i <= NUM_COUN; i++)
	{
		o = units[i];
		p = NULL;

		while (o != NULL)
		{
			if (o->type == U_WNG)
			{
				if (p == NULL)
				{
					units[i] = (units[i])->next;
				}
				else
				{
					p->next = o->next;
				}
				if (map[o->sector].unit == o)
				{
					map[o->sector].unit = NULL;
				}
				deleteallorder(orders, o->sector);
				disposeunit(o);
			}
			else
			{
				p = o;
				o = o->next;
			}
		}
	}
}

/* make the unit list consistant (i.e. remove duplicates, if any) */
void unit_consist(UnitList *lp)
{
        Unit c, o;

        c = *lp;
        while (c != NULL)
        {
                if (c->next != NULL && c->next->sector == c->sector && c->next->type == c->type)
                {
                        o = c->next;
                        c->next = o->next;
                        disposeunit(o);
                }
                else
                        c = c->next;
        }
}


void
cleanunits (UnitList *lp)
{
     Unit o, p;

           o = *lp;
           while (isunit(o))
           {
                  p = o->next;
                  if (map[o->sector].unit == o)
                          map[o->sector].unit = NULL;
                  disposeunit(o);
                  o = p;
           }

           *lp = NULL;
}

void
addunits2map (UnitList& lp)
{
    Unit o;

           o = lp;
           while (isunit(o))
           {
                  map[o->sector].unit = o;
                  o = o->next;
           }
}


/* delete any units in the unit lists that match the sector */
int
deleteallunit (UnitList us[], short r)
{
     register short i;

     for (i = 0; i <= NUM_COUN; i++)
          if (inulist(us[i], U_NON, r, C_NONE))
               deleteunit(&(us[i]), r);

         return 1;
}

/* block the supply center that matches the sector */
int
blocksupply (SupplyList sp[], short sector)
{
     register short i;
	 register SupplyList sl;

	 for (i = 0; i <= NUM_COUN; i++)
	 {
	    sl = sp[i];
		while (sl != NULL) 
		{
			if (sl->sector == sector)
			{
				sl->coast = S_BLOCKED;
				return 1;
			}

			sl = sl->next;
		}
	 }

     return 0;
}

void
unblockallsupply (SupplyList sp[])
{
     register short i;
	 register SupplyList sl;

	 for (i = 0; i <= NUM_COUN; i++)
	 {
	    sl = sp[i];
		while (sl != NULL) 
		{
			sl->coast = S_FREE;
			sl = sl->next;
		}
	 }
}