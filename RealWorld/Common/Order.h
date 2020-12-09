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
#ifndef _OrderDefs_
#define _OrderDefs_

#include "Fails.h"
#include "MacTypes.h"
#include "Unit.h"

class DislodgeList;

#define MAX_ORD 256

struct ordstruct {
     char *text;
     short country;
     UnitType unit;
     short start;
     CoastType s_coast;
     short type;
     short finish;
     CoastType f_coast;
     struct ordstruct *suborder;
     short fails;
     short index;
};

typedef struct ordstruct Orderlist[MAX_ORD];
typedef struct ordstruct *Order;
#define noorder NULL

/* order types */
#define O_UNKN   0
#define O_NONE   0
#define O_MOVE   1
#define O_SUPP   2
#define O_HOLD   3
#define O_CONV   4
#define O_BUILD  5
#define O_FLEET  5
#define O_REMOVE 6
#define O_PENDING 7
#define O_WING   7
#define O_WAIVE   8
#define O_ARMY    8
#define O_DEF_REM 9
#define O_DISBAND 10
#define O_NMR     11
#define O_NMR_DIS 12
#define O_RETR    13
#define O_STAT    14
#define O_TRAFO   15
#define O_MAX     15

/* function prototypes */
Order neworder();
void deleteorder(Order order);
void initolist(Orderlist list);
void cleanorders(Orderlist list);
void clearorder(short index);
void deleteallorder(Orderlist olist[], short sector);
bool addorder(Orderlist olist, Order op, bool select);
Order copyorder(Order op);
void copyolist(Orderlist dest, Orderlist source);
Order findorder(Orderlist olist, UnitType unit, short sector);
Order findorderbyindex(short index);
bool appendorder(Orderlist olist, Order op);
bool appendnmr(short country, UnitType unit, short sector, CoastType coast);
char *string2otype(short *type, char *s);
void sorder2string(char *s, Order o, bool stat, bool use_country);
void order2string(char *s, Order o, bool stat, bool use_country);
void order2stringCSV(char *s, Order o, bool stat, bool use_country);
bool string2order(Order order, char *s);
void checkorder(Order op, bool ign_units);
void exec_orders(Orderlist orders[], bool moves);
int movesto(Orderlist olist, short sector);
bool haveorders();
int onehome(short c, SupplyList& supply);
int allhome(short c, SupplyList& supply);
int oursupply (short reg, short c);
int ourreservesupply (short reg, short c);

int countbuilds(UnitList units[], SupplyList supplies[], const DislodgeList& dislodges, short builds[]);
int countblocked(SupplyList supplies);
void sim_next_phase(UnitList newunits[], SupplyList newsupplies[], SupplyList newlands[],
                                        short numunits[], short numsupplies[], short numlands[],
                                        short& dislodges, DislodgeList& newdslglist, short& newadjustments,
                                        short numbuilds[], bool moves);

bool OrderToRect(Order op, Rect *theRect);

/* macros */
#define setotext(o, s) ((o)->text = s)
#define setocountry(o, c) ((o)->country = c)
#define setounit(o, u) ((o)->unit = u)
#define setostart(o, r) ((o)->start = r)
#define setoscoast(o, c) ((o)->s_coast = c)
#define setotype(o, t) ((o)->type = t)
#define setofcoast(o, c) ((o)->f_coast = c)
#define setofinish(o, r) ((o)->finish = r)
#define setosuborder(o, sub) ((o)->suborder = sub)
#define setofails(o, f) ((o)->fails = f)

#define otext(o) ((o)->text)
#define ocountry(o) ((o)->country)
#define ounit(o) ((o)->unit)
#define ostart(o) ((o)->start)
#define oscoast(o) ((o)->s_coast)
#define otype(o) ((o)->type)
#define ofcoast(o) ((o)->f_coast)
#define ofinish(o) ((o)->finish)
#define osuborder(o) ((o)->suborder)
#define ofails(o) ((o)->fails)
#define oindex(o) ((o)->index)

#define orderfails(o) ((o) != NULL && (o)->fails != R_NONE && (o)->fails != R_SUCCESS)
#define obadsyntax(o) ((o)->fails == R_SYNTAX || (o)->fails == R_BADCOUN || \
                                           (o)->fails == R_BADUNIT || (o)->fails == R_BADTYPE || \
                                           (o)->fails == R_BADREGN)

#define startorder(ol) (ol)
#define isorder(o) ((o) != NULL && ((o)->text != NULL))
#define getotype(o) (isorder(o)?(o)->type:O_UNKN)
#define nextorder(o) ((o)++)

#endif