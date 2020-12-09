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
#ifndef __MapDataDefs__
#define __MapDataDefs__

#include "Neighbor.h"
#include "Unit.h"
#include "Dislodge.h"
#include "Order.h"
#include "MacTypes.h"

const int SECT_NONE        = 0;
const int MAX_SECT = 1024;
const int MAX_CANAL = 10;

/* sector type flags */
#define F_SUPP 0x1
#define F_LAND 0x2
#define F_SEA  0x4
#define F_ICE  0x8         /* ice bound in Fall */
#define F_CONV 0x10        /* coastal region that can be used to convoy  */
#define F_CAN1 0x20		   /* canal gate (body of water at beginning or end) */
#define F_CAN2 0x40		   /* canal center (land through which the canal passes) */
#define F_SFP  0x100       /* special fleet placement in unicoastal region */
#define F_BLD  0x200       /* special build rule (Hanse) */
#define F_SBJ  0x400       /* special build rule (Hanse) */
#define F_SIB  0x800       /* special build rule (1900) */

/* country numbers */
#define CN_NONE 0
#define MAX_COUN 64

#define MAX_NAMES 8

class Rgn;

struct mapnode {
        char *names[MAX_NAMES]; /* name of region */
        Unit unit;              /* unit, if any */
        unsigned short type;     /* land, sea or coast; supply sector or not */
        CoastType coast1;   /* direction of coast1 (NSEW) */
        CoastType coast2;   /* direction of coast2 */
		CoastType coast3;   /* direction of coast3 */
        NeighborList land;      /* regions adjacent by land */
        NeighborList sea1;
        NeighborList sea2;
		NeighborList sea3;
        Point land_loc;         /* location of unit */
        Point coast1_loc;
        Point coast2_loc;
		Point coast3_loc;       /* used for third coast or special fleet placement */
        Point name_loc;         /* location of name */
        Rgn* rgn;               /* regions describing sector */
        Rgn* coast1_rgn;
        Rgn* coast2_rgn;
		Rgn* coast3_rgn;
};
/* type has five bits:
 *      0x1 is supply center; 0x2 is army access; 0x4 is fleet access
 *      0x8 is ice;  0x10 is isthmus
 *      other types have been added, check flags 
 */

typedef struct mapnode *Sector;

#define kHomeCenters 16

struct cnode {
        char name[32];
        char adjective[32];
        char abbreviation;
        short builds;
        short home[kHomeCenters];
		short sib[kHomeCenters];   /* Non-Centers where units can be built (like Sib in 1900) */
        short numCenters;
		short numSibCenters;
		//short numLands;          /* owned number of non-supply-sectors */
        short patID;
        short colorID;
};

struct canalstruct {
		short num;					 /*number of canal*/
		Sector gate1;				 
		Sector gate2;				 
		Sector center;	
		short resolution;            /*resolution code for center*/
		Order sc_order;				 /*order permitted by SC-command*/
		struct canalstruct *next;
};

typedef struct canalstruct *CanalList, *Canal;

enum CanalType { C_NON = 0, C_GATE, C_CENTER };

extern struct mapnode		*map;       /* map data */
extern struct cnode			*countries; /* country data */
extern struct canalstruct   *canals;	/* canal data */
extern UnitList     *units;
extern SupplyList   *supplies;
extern SupplyList   *lands;						/* land list */
extern Orderlist    *orders;
extern short dislodges;
extern DislodgeList dslglist;

extern short total_supply;
extern short total_land;        /* number of non-supply land sectors */
extern short win_supply;


extern char *scoast[5], *lcoast[5];         /* coastnames */

extern short NUM_COUN;
extern short NUM_SECT;


bool                        AllocMapData(void);
bool                        ReadMapData(void);
void                        CleanMapData(void);
void                        SectorToPoint(Point* thePoint, short sector, CoastType coast);
short                       FindSector(short h, short v, CoastType *coast);
bool						CrossesIsthmus(UnitType unit, short start, CoastType s_coast, short finish);
bool						FromSuez(UnitType unit, short start, CoastType s_coast, short finish);
bool						ToSuez(UnitType unit, short start, CoastType s_coast, short finish);
int                         neighborsector(UnitType unit, short start, CoastType s_coast, short finish,
                                                                   CoastType f_coast, short supp);
Neighbor                    findneighbor(UnitType unit, short start, CoastType s_coast, short finish,
                                                                   CoastType f_coast, short supp);

void                        unitmapconsist();
CanalList    				newcanal();
int							addcanal(Canal *c, Sector s, CanalType ctype, short num);
void						DisposeCanal(Canal c);
void						ClearCanals(void);
void						ClearCanalOrders(void);
Canal						getcanal(Sector s);
short						getcanalnumber(Canal *c, Sector s, CanalType ctype);
short						getcanalowner(Canal c);
bool						issamecanal(Sector s1, Sector s2);
Sector						getcanalsector(Sector s);
Order						findorderbysector(Orderlist olist, UnitType unit, Sector sector);

#define initcanal(c) c = NULL
#define iscanal(c) ((c) != NULL)
#define cnum(c) ((c)->num)
#define cgate1(c) ((c)->gate1)
#define cgate2(c) ((c)->gate2)
#define ccenter(c) ((c)->center)
#define corder(c) ((c)->sc_order)
#define setcorder(c,o) ((c)->sc_order = o)
#define nextcanal(c) ((c) = (c)->next)
#define sunit2string(unit) (unit == U_FLT?"F":(unit == U_WNG?"W":"A"))
#define lunit2string(unit) (unit == U_FLT?"Fleet":(unit == U_WNG?"Wing":"Army"))
#define sreg2string(reg) (map[reg].names[1])
#define lreg2string(reg) (map[reg].names[0])
#define scoast2string(cst) (scoast[cst])
#define lcoast2string(cst) (lcoast[cst])
#define country2string(coun) (countries[coun].name)
#define cntadj2string(coun) (countries[coun].adjective)

#define getsector(m,index) (m+(index))
#define issupp(r) ((r)->type & F_SUPP)
#define island(r) ((r)->type & F_LAND)
#define issea(r) ((r)->type & F_SEA)
#define iscoastal(r) (((r)->type & F_LAND) && ((r)->type & F_SEA))
#define isspecial(r) (iscoastal(r) && ((r)->coast1 == C_NONE) && ((r)->type & F_SFP))
#define iswingonly(r) (!((r)->type & F_LAND || (r)->type & F_SEA))
#define isbicoastal(r) ((r)->coast1 != C_NONE)
#define hasthirdcoast(r) ((r)->coast3 != C_NONE)
#define isice(r) ((r)->type & F_ICE)
#define startcanal() (canals)
#define belongstocanal(r1) ((r1)->type & F_CAN1)  
#define iscanalcenter(r1) ((r1)->type & F_CAN2)  
#define isconvoyable(r) ((r)->type & F_CONV)
#define haschaosbuildrule(r) ((r)->type & F_BLD)
#define issbj(r) ((r)->type & F_SBJ)
#define issib(r) ((r)->type & F_SIB)
#define landneigh(r) ((r)->land)
#define seaneigh(r) ((r)->sea1)
#define coast1neigh(r) ((r)->sea1)
#define coast2neigh(r) ((r)->sea2)
#define coast3neigh(r) ((r)->sea3)

#define startsector(m) ((m)+1)
#define issector(r) (sectindex(r) <= NUM_SECT)
#define nextsector(r) ((r)++)
#define sectindex(r) ((r)-map)
#define sectname(r) ((r)->names[0])

#define sname(r) ((r)->name)
#define ssname(r) ((r)->shortname)
#define scoast1(r) ((r)->coast1)
#define scoast2(r) ((r)->coast2)
#define scoast3(r) ((r)->coast3)

#define sectunit(s) ((s)->unit)

char *string2sector(short *sector, char *s);
char *string2coast(CoastType *coast, char *s);
char *string2country(short *country, char *s);
CoastType char2coast(char coastdata);
char coast2char(CoastType coast);
short char2country(char c);
short namelen();
int canalcount();

#endif