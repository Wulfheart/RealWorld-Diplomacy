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
#ifndef _UnitDefs_
#define _UnitDefs_

/*
 * unit.h
 *
 * declarations for unit and supply sector lists
 *
 */

/* unit types */
enum UnitType { U_NONE = 0, U_FLEET, U_ARMY, U_WING };
#define U_NON U_NONE
#define U_FLT U_FLEET
#define U_ARM U_ARMY
#define U_WNG U_WING

/* coasts */
enum CoastType { C_NONE = 0, C_SO, C_NO, C_EA, C_WE, C_HOP };
#define BlockType CoastType

struct unitstruct {
     UnitType type;
     short sector;
     CoastType coast;
     short owner;
     struct unitstruct *next;
};
typedef struct unitstruct *Unit, *UnitList;

#define initulist(lp) lp = NULL

Unit newunit();
void disposeunit(Unit up);
int addunit(UnitList *lp, UnitType unittype, short sector, CoastType coast, short owner);
int deleteunit(UnitList *lp, short n);
void cleanunits(UnitList *lp);
void removewings(void);
int deleteallunit(UnitList us[], short s);

int  inulist(UnitList lp, UnitType unittype, short sector, CoastType coast);
Unit getunit(UnitList lp, UnitType unittype, short sector, CoastType coast);
void unit2string(char *s, Unit lp, bool abbrev = false);
void ulist2string(char *s, UnitList lp, bool abbrev = false);
UnitList string2ulist(char *s, short country);
char *string2unit(UnitType *unit, char *s);
int  ulistsize(UnitList lp);

void unit_consist(UnitList *lp);
void addunits2map (UnitList& lp);

#define startunit(l) (l)
#define isunit(i) ((i) != NULL)
#define nextunit(i) ((i) = (i)->next)

#define utype(i)  (isunit(i)?(i)->type:U_NONE)
#define usector(i) (isunit(i)?(i)->sector:SECT_NONE)
#define ucoast(i) (isunit(i)?(i)->coast:C_NONE)
#define uowner(i) (isunit(i)?(i)->owner:CN_NONE)

/*
 * supply sector lists
 *
 * we're cheating a little here and using unit procedures...
 *
 */

typedef UnitList SupplyList;
typedef Unit Supply;

#define S_BLOCKED C_SO		/* use coast for "blocked" status */
#define S_FREE C_NONE		

#define initslist(lp)            lp = NULL

#define addsupply(lp, reg, owner)       addunit(lp, U_NONE, reg, C_NONE, owner)
#define deletesupply(lp, reg)    deleteunit(lp, reg)
#define cleansupplies(lp)        cleanunits(lp)
#define deleteallsupply(us, reg) deleteallunit(us, reg)

#define inslist(lp, r)           inulist(lp, U_NONE, r, C_NONE)
#define getsupply(lp, r)         getunit(lp, U_NONE, r, C_NONE)
void printsupply();
void printslist();
SupplyList string2slist(char *s, short country);
int slistsize(SupplyList lp);
void slist2string(char *s, UnitList lp, bool abbrev = false);
void supply2string(char *s, Unit lp, bool abbrev = false);
int blocksupply (SupplyList sp[], short sector);
void unblockallsupply (SupplyList sp[]);

#define startsupply(l) (l)
#define issupply(i) ((i) != NULL)
#define nextsupply(i) ((i) = (i)->next)
#define isblocked(i) ((i)->coast == S_BLOCKED)
#define ssector(i) ((i)->sector)

#endif
