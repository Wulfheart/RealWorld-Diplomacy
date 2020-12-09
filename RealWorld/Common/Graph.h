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
#include "Order.h"
#include "Unit.h"
#include "Mapdata.h"

typedef struct graphnode {
     short owner;               /* whose move */
     short type;                /* what kind of move? */
     struct graphnode *dest;    /* main pointer of this node */
     struct graphnode *spec;    /* special path for supports and convoys */
     char  mark;                /* used for graph traversal to avoid repeats */
     struct ordstruct *order;   /* pointer to original ordstruct */
     struct mapnode *sector;    /* region of map where unit is */
     short strength;            /* offensive (if moving)/defensive (if not) */
     short resolution;          /* did it work? */
     short bounce;              /* true if moves bounce in this space */
                                /* used for retreats */
} GraphStruct, *GraphNode, Graph[];

#define startnode() (graph+1)
#define nextnode(x) ((x)++)
int isnode (GraphNode x);
#define getnode(x) (graph+(x))
#define gindex(gn) ((gn)-graph)

#define basestrength 2
#define gowner(gp) ((gp)->owner)
#define setgowner(gp, o) ((gp)->owner = o)
#define gtype(gp)  ((gp)->type)
#define setgtype(gp, t) ((gp)->type = t)
#define gdest(gp)  ((gp)->dest)
#define setgdest(gp, d)  ((gp)->dest = d)
#define gspec(gp)  ((gp)->spec)
#define setgspec(gp, s)  ((gp)->spec = s)
#define gmarked(gp) ((gp)->mark)
#define gmark(gp) ((gp)->mark)
#define setgmark(gp, m) ((gp)->mark = m)
#define unsetgmark(gp) ((gp)->mark = 0)
#define gorder(gp)  ((gp)->order)
#define setgorder(gp, o) ((gp)->order = o)
#define gsector(gp)  ((gp)->sector)
#define setgsector(gp, r) ((gp)->sector = r)
#define gstrength(gp)  ((gp)->strength)
#define resetgstrength(gp)  (((gp)->strength) = basestrength)
#define incgstrength(gp) (((gp)->strength)+=basestrength)
#define decgstrength(gp) (((gp)->strength)-=basestrength)
#define decgstrengthhalf(gp) (((gp)->strength)--)
#define gresolution(gp) ((gp)->resolution)
#define gfails(gp) ((gp)->resolution != R_SUCCESS && (gp)->resolution != R_NONE)
#define gresolved(gp) ((gp)->resolution != R_NONE)
#define setgresolution(gp, r) ((gp)->resolution = r)
#define gbounce(gp)  ((gp)->bounce)
#define incgbounce(gp) (((gp)->bounce)+=basestrength)
#define decgbounce(gp) (((gp)->bounce)-=basestrength)
#define decgbouncehalf(gp) (((gp)->bounce)--)

#define convoyed(gp) (gtype(gp) == O_MOVE && isnode(gspec(gp)))
#define canalmove(gp) (gtype(gp) == O_MOVE && issamecanal((gsector(gp)),(gsector(gdest(gp)))))

extern Graph graph;

void newgraph();
void deletegraph();
void initializegraph (short ign_units);
void makegraph (Orderlist *orders, short ign_units);
int addnode (Order op, struct graphnode *node);
void clearmarks();
int canconvoy (GraphNode start,	GraphNode curr);
int canconvoysuezinfo (GraphNode start,	GraphNode curr, bool *FromSuez, bool *ToSuez);
int canconvoynosuez (GraphNode start,	GraphNode curr);
bool isvalidstep(short sector, short nextsector, bool laststep);
bool isconvtosuez (GraphNode start);
bool isconvfromsuez (GraphNode start);
short armydistance (Unit up, short countryindex);
short fleetdistance (Unit up, short countryindex);

