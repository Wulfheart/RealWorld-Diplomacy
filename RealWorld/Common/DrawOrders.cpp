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
#include "MapDisplay.h"
#include "MapData.h"
#include "Unit.h"
#include "Graph.h"
#include "Order.h"
#include "Util.h"
#include "Variants.h"
#include "MacTypes.h"
#include <math.h>
#include <string.h>

void DrawMove(Order o, bool bw, bool gs);
void DrawArrowhead(Point pt1, Point pt2);
void DrawSupp(Order o, bool bw, bool gs);
void DrawConvs(bool bw, bool gs);
void DrawConvArrow(Point pt1, Point pt2, GraphNode h, bool bw, bool gs);
void DrawBuild(Order o, bool bw, bool gs);
void DrawRemove(Order o, bool bw, bool gs);
void DrawX(bool BW, UnitType type, short sector, CoastType coast, short country, short offset);

void DrawDislodge(bool BW, UnitType type, short sector, CoastType coast, short country);

int DrawConvoyRoute(GraphNode start, GraphNode curr,
                    int depth, bool bw, bool gs);

extern bool gSaveDislodge;

void
DrawOrders(bool bw, bool gs)
{
	register short i;
	register Order o;

	for (i = 1; i <= NUM_COUN; i++)
	{
		for (o = startorder(orders[i]); isorder(o); nextorder(o))
		{
			if (otype(o) == O_MOVE && !obadsyntax(o))
				DrawMove(o, bw, gs);
		}
	}

	for (i = 1; i <= NUM_COUN; i++)
	{
		for (o = startorder(orders[i]); isorder(o); nextorder(o))
		{
			if (otype(o) == O_SUPP && !obadsyntax(o))
				DrawSupp(o, bw, gs);
		}
	}
		
	DrawConvs(bw, gs);
}

void
DrawAdjusts(bool bw, bool gs)
{
	register short i;
	register Order o;
	
	PenSize(2,2);
	for (i = 1; i <= NUM_COUN; i++)
	{
		for (o = startorder(orders[i]); isorder(o); nextorder(o))
		{

			if (otype(o) == O_BUILD && !obadsyntax(o))
				DrawBuild(o, bw, gs);
			else if ((otype(o) == O_REMOVE || otype(o) == O_DEF_REM) && !obadsyntax(o))
				DrawRemove(o, bw, gs);
		}
	}
}

void
DrawMove(Order o, bool bw, bool gs)
{
	Point pt1, pt2, pt3;
	float length, t;
	double hdiff, vdiff;
	RGBColor blackColor = {0,0,0}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF};

	// don't draw if start is same as finish
	if (ostart(o) == ofinish(o))
		return;

			/* get start point */
			if (ounit(o) == U_ARM)
				pt1 = map[ostart(o)].land_loc;
			else
			{
				if (ounit(o) == U_FLT && map[ostart(o)].coast1 != C_NONE)
				{
					if (oscoast(o) == map[ostart(o)].coast1)
						pt1 = map[ostart(o)].coast1_loc;
					else if (oscoast(o) == map[ostart(o)].coast2)
						pt1 = map[ostart(o)].coast2_loc;
					else if (hasthirdcoast(&map[ostart(o)]) &&oscoast(o) == map[ostart(o)].coast3)
						pt1 = map[ostart(o)].coast3_loc;
					else
						pt1 = map[ostart(o)].land_loc;
				}
				else if (ounit(o) == U_FLT && isspecial(&map[ostart(o)]))
				{
					pt1 = map[ostart(o)].coast3_loc;
				}
				else
					pt1 = map[ostart(o)].land_loc;
			}
				/* this is a retreat (or should be) */
				if (dislodges > 0 && gSaveDislodge)
				{
					pt1.h += 4;
					pt1.v += 4;
				}

			/* get initial end point */
				if (map[ofinish(o)].coast1 != C_NONE)
				{
					if (ofcoast(o) == map[ofinish(o)].coast1)
						pt2 = map[ofinish(o)].coast1_loc;
					else if (ofcoast(o) == map[ofinish(o)].coast2)
						pt2 = map[ofinish(o)].coast2_loc;
					else if (hasthirdcoast(&map[ofinish(o)]) && ofcoast(o) == map[ofinish(o)].coast3)
						pt2 = map[ofinish(o)].coast3_loc;
					else
						pt2 = map[ofinish(o)].land_loc;
				}
				else if (ounit(o) == U_FLT && isspecial(&map[ofinish(o)]))
				{
					pt2 = map[ofinish(o)].coast3_loc;
				}
				else
					pt2 = map[ofinish(o)].land_loc;
					
			/* move end point back some pixels */
				hdiff = pt1.h-pt2.h;
				vdiff = pt1.v-pt2.v;
				length = (float) sqrt((double)(hdiff*hdiff + vdiff*vdiff));
				t = (length-7)/length;
				pt3.h = (short) (t*pt2.h + (1-t)*pt1.h);
				pt3.v = (short) (t*pt2.v + (1-t)*pt1.v);
							
			/* if b&w clear space */
				if (bw)
				{
					PenSize(3, 3);
					PenPat(&qd.white);
					MoveTo(pt1.h-1, pt1.v-1);
					LineTo(pt3.h-1, pt3.v-1);
				}
			/* otherwise set line color */
				else if (!gs)
				{
					SetCountryRGBColor( countries[ocountry(o)].colorID );
				}
				
				if (orderfails(o))
					PenPat(&qd.gray);
				else
					PenPat(&qd.black);
				PenSize(1,1);
				MoveTo(pt1.h, pt1.v);
				LineTo(pt3.h, pt3.v);
				
				DrawArrowhead(pt1, pt3);
				
				if (!bw)
				{
					RGBForeColor(&blackColor);
					RGBBackColor(&whiteColor); 
				}
				PenNormal();
}


void DrawArrowhead(Point pt1, Point pt2)
{
	PolyHandle poly;
	float a1_h = -5, a1_v = -4, 
		  a2_h = 2,  a2_v = 0, 
		  a3_h = -5, a3_v = 4;
	float cos_theta, sin_theta;
	float length;
	double hdiff, vdiff;

	/* draw the bloody arrowhead */	
	/* get vector */
	hdiff = pt2.h-pt1.h;
	vdiff = pt2.v-pt1.v;
	length = (float) sqrt(hdiff*hdiff + vdiff*vdiff);
				
	cos_theta = (float) hdiff/length;
	sin_theta = (float) vdiff/length;
				
	poly = OpenPoly();
	MoveTo((short)(a1_h*cos_theta - a1_v*sin_theta + pt2.h + 0.5), 
		   short(a1_h*sin_theta + a1_v*cos_theta + pt2.v + 0.5));
	LineTo(short(a2_h*cos_theta - a2_v*sin_theta + pt2.h + 0.5), 
		   short(a2_h*sin_theta + a2_v*cos_theta + pt2.v + 0.5));
	LineTo(short(a3_h*cos_theta - a3_v*sin_theta + pt2.h + 0.5), 
		   short(a3_h*sin_theta + a3_v*cos_theta + pt2.v + 0.5));
	LineTo(short(a1_h*cos_theta - a1_v*sin_theta + pt2.h + 0.5), 
		   short(a1_h*sin_theta + a1_v*cos_theta + pt2.v + 0.5));
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
}

void
DrawSupp(Order o, bool bw, bool gs)
{
	Point pt1, pt2, pt3, pt4;
	Rect circleRect = {-3, -3, 3, 3};
	RGBColor blackColor = {0,0,0}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF}, 
	         grayColor = {0x3F, 0x3F, 0x3F};
	float length, t;
	double hdiff, vdiff;

	/* get the start point */
	if (ounit(o) == U_ARM)
		pt1 = map[ostart(o)].land_loc;
	else
	{
		if (ounit(o) == U_FLT && map[ostart(o)].coast1 != C_NONE)
		{
			if (oscoast(o) == map[ostart(o)].coast1)
				pt1 = map[ostart(o)].coast1_loc;
			else if (oscoast(o) == map[ostart(o)].coast2)
				pt1 = map[ostart(o)].coast2_loc;
			else if (hasthirdcoast(&map[ostart(o)]) && oscoast(o) == map[ostart(o)].coast3)
				pt1 = map[ostart(o)].coast3_loc;
			else
				pt1 = map[ostart(o)].land_loc;
		}
		else if (ounit(o) == U_FLT && isspecial(&map[ostart(o)]))
		{
			pt1 = map[ostart(o)].coast3_loc;
		}
		else
			pt1 = map[ostart(o)].land_loc;
	}

	/* get the end point */
			if (osuborder(o) == NULL)
					return;

			// don't draw if start is same as finish
			if (ostart(o) == ostart(osuborder(o)))
				return;

				if (map[ostart(osuborder(o))].coast1 != C_NONE)
				{
					if (oscoast(osuborder(o)) == map[ostart(osuborder(o))].coast1)
						pt2 = map[ostart(osuborder(o))].coast1_loc;
					else if (oscoast(osuborder(o)) == map[ostart(osuborder(o))].coast2)
						pt2 = map[ostart(osuborder(o))].coast2_loc;
					else if (hasthirdcoast(&map[ostart(osuborder(o))]) && oscoast(osuborder(o)) == map[ostart(osuborder(o))].coast3)
						pt2 = map[ostart(osuborder(o))].coast3_loc;
					else
						pt2 = map[ostart(osuborder(o))].land_loc;
				}
				else if (ounit(osuborder(o)) == U_FLT && isspecial(&map[ostart(osuborder(o))]))
				{
					pt2 = map[ostart(osuborder(o))].coast3_loc;
				}
				else
					pt2 = map[ostart(osuborder(o))].land_loc;
					
	/* if supporting to move, use middle of move as endpoint */
				if (otype(osuborder(o)) == O_MOVE)
				{	
					if (map[ofinish(osuborder(o))].coast1 != C_NONE)
					{
						if (ofcoast(osuborder(o)) == map[ofinish(osuborder(o))].coast1)
							pt3 = map[ofinish(osuborder(o))].coast1_loc;
						else if (ofcoast(osuborder(o)) == map[ofinish(osuborder(o))].coast2)
							pt3 = map[ofinish(osuborder(o))].coast2_loc;
						else if (hasthirdcoast(&map[ofinish(osuborder(o))]) && ofcoast(osuborder(o)) == map[ofinish(osuborder(o))].coast3)
							pt3 = map[ofinish(osuborder(o))].coast3_loc;
						else
							pt3 = map[ofinish(osuborder(o))].land_loc;
					}
					else if (ounit(osuborder(o)) == U_FLT && isspecial(&map[ofinish(osuborder(o))]))
					{
						pt3 = map[ofinish(osuborder(o))].coast3_loc;
					}
					else
						pt3 = map[ofinish(osuborder(o))].land_loc;
					pt4.h = (pt2.h+pt3.h)/2;
					pt4.v = (pt2.v+pt3.v)/2; 
					pt2 = pt4; 
				}
				
	/* if supporting to hold, sock endpoint back a bit */
				if (otype(osuborder(o)) != O_MOVE)
				{
					hdiff = pt1.h-pt2.h;
					vdiff = pt1.v-pt2.v;
					length = (float) sqrt((double)(hdiff*hdiff + vdiff*vdiff));
					t = (length-10)/length;
					pt3.h = short(t*pt2.h + (1-t)*pt1.h);
					pt3.v = short(t*pt2.v + (1-t)*pt1.v);
				}
				else
					pt3 = pt2;
					
	/* set up pen */
				if (bw)
				{
					PenSize(3,3);
					PenPat(&qd.white);
					MoveTo(pt1.h-1, pt1.v-1);
					LineTo(pt3.h-1, pt3.v-1);
				}
				else if (!gs)
				{
					SetCountryRGBColor( countries[ocountry(o)].colorID );
				}
				
				if (orderfails(o))
					PenPat(&qd.gray);
				else
					PenPat(&qd.black);
					
				PenSize(1,1);
				MoveTo(pt1.h, pt1.v);
				LineTo(pt3.h, pt3.v);
				SetRect(&circleRect, -3, -3, 3, 3);
				OffsetRect(&circleRect, pt3.h, pt3.v);
				PaintOval(&circleRect);
				
				if (!bw)
				{
					RGBForeColor(&blackColor);
					RGBBackColor(&whiteColor); 
				}
	PenNormal();
}


int DrawConvoyRoute(GraphNode start, GraphNode curr,
                    int depth, bool bw, bool gs)
// Draw the convoy route for move in 'start'. Currently 
// on 'curr'. Do Depth First Search. If search is
// successfull, draw edge on the fly when function
// returns from recursion.
// Detours are prevented. Edges that move to a node
// which could be directly reached by an earlier node
// on the route, fail. This is achieved by marking
// the nodes, around the earlier nodes. The least
// significant of the mark is set when there was
// at least one successful convoy route. The higher
// bits contain the depth, which will be cleared
// when comming back from recursion.
{
    int     result = 0;
    Neighbor lp;
    GraphNode next;
	Point pt1, pt2;

    if (!isnode(curr) || (gmark(curr) >> 1) != depth)
    {
        // We have already been here or we take a detour.
        return 0;
    }
    if (curr == gdest(start) && curr != start)
    {
        // We found the end.
        // Only a successful convoy when the route is at
        // least 3 nodes (at least one fleet).
        return depth > 2;
    }
    if (start != curr)
    {
        if (gtype(curr) != O_CONV || gdest(curr) != start ||
            gspec(curr) != gdest(start) || gfails(curr))
        {
            // We do not have a convoying fleet that can continue
            // our journey.
            return 0;
        }
    }

    // Mark all neighbours to prevent detour

    for (lp = startneighbor(coast1neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
    {
	    next = getnode(nsector(lp));
        // Do not overwrite a mark set by previous node
        // on current route. Do not mark destination when
        // we start.
        if ((gmark(next) >> 1) == 0 &&
            (start != curr || gdest(start) != next))
        {
            setgmark(next, (gmark(next) & 1) | ((depth + 1) << 1));
        }
    }

    // Check the second coast.
    if (isbicoastal(gsector(curr)))
        for (lp = startneighbor(coast2neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
        {
	        next = getnode(nsector(lp));
            // Do not overwrite a mark set by previous node
            // on current route. Do not mark destination when
            // we start.
            if ((gmark(next) >> 1) == 0 &&
               (start != curr || gdest(start) != next))
            {
                setgmark(next, (gmark(next) & 1) | ((depth + 1) << 1));
            }
    }

	// Check the third coast.
    if (hasthirdcoast(gsector(curr)))
        for (lp = startneighbor(coast3neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
        {
	        next = getnode(nsector(lp));
            // Do not overwrite a mark set by previous node
            // on current route. Do not mark destination when
            // we start.
            if ((gmark(next) >> 1) == 0 &&
               (start != curr || gdest(start) != next))
            {
                setgmark(next, (gmark(next) & 1) | ((depth + 1) << 1));
            }
    }


    // Go into recursion for every neighbor.
    for (lp = startneighbor(coast1neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
    {
	  	next = getnode(nsector(lp));
	  	// Go into recursion for this neighbour.
        if (DrawConvoyRoute(start, next, depth + 1, bw, gs))
        {
            // Successful route.
            // Now, we have to draw an arrow to that 
            // successful node.
			pt1 = gsector(curr)->land_loc;
			pt2 = gsector(next)->land_loc;
            if (next == gdest(start))
			    DrawConvArrow(pt1, pt2, start, bw, gs);
            else
			    DrawConvArrow(pt1, pt2, next, bw, gs);
            result = 1;
        }
     }

     // Check the second coast.
     if (isbicoastal(gsector(curr)))
	    for (lp = startneighbor(coast2neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
        {
	        next = getnode(nsector(lp));
	  	    // Go into recursion for this neighbour.
            if (DrawConvoyRoute(start, next, depth + 1, bw, gs))
            {
                // Successful route.
                // Now, we have to draw an arrow to that 
                // successful node.
			    pt1 = gsector(curr)->land_loc;
			    pt2 = gsector(next)->land_loc;
                if (next == gdest(start))
			        DrawConvArrow(pt1, pt2, start, bw, gs);
                else
			        DrawConvArrow(pt1, pt2, next, bw, gs);
                result = 1;
            }
        }

		// Check the third coast.
		if (hasthirdcoast(gsector(curr)))
	    for (lp = startneighbor(coast3neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
        {
	        next = getnode(nsector(lp));
	  	    // Go into recursion for this neighbour.
            if (DrawConvoyRoute(start, next, depth + 1, bw, gs))
            {
                // Successful route.
                // Now, we have to draw an arrow to that 
                // successful node.
			    pt1 = gsector(curr)->land_loc;
			    pt2 = gsector(next)->land_loc;
                if (next == gdest(start))
			        DrawConvArrow(pt1, pt2, start, bw, gs);
                else
			        DrawConvArrow(pt1, pt2, next, bw, gs);
                result = 1;
            }
        }


    // Clear the neighbours we marked.

    for (lp = startneighbor(coast1neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
    {
	    next = getnode(nsector(lp));
        // Clear only the neighbours *we* marked.
        if ((gmark(next) >> 1) == depth + 1)
        {
            setgmark(next, gmark(next) & 1);
        }
    }

    // Check the second coast.
    if (isbicoastal(gsector(curr)))
        for (lp = startneighbor(coast2neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
        {
	        next = getnode(nsector(lp));
            // Clear only the neighbours *we* marked.
            if ((gmark(next) >> 1) == depth + 1)
            {
                setgmark(next, gmark(next) & 1);
            }
    }
    // Check the third coast.
    if (hasthirdcoast(gsector(curr)))
        for (lp = startneighbor(coast3neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
        {
	        next = getnode(nsector(lp));
            // Clear only the neighbours *we* marked.
            if ((gmark(next) >> 1) == depth + 1)
            {
                setgmark(next, gmark(next) & 1);
            }
    }

    if (result)
        // Mark that there was a successful route.
        setgmark(curr, gmark(curr) | 1);
    return result;
}


void
DrawConvs(bool bw, bool gs)
{
	GraphNode g, h;
	Order o;
	Point pt1, pt2;
	register short i;
	short fontNum;
	RGBColor blackColor = {0,0,0}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF};
	char fontName[256];
	
	strncpy(fontName, "Monaco", 255);
	CtoPstr(fontName);	
 	GetFNum((unsigned char *) fontName, &fontNum);
	TextFont(fontNum);
	TextSize(6); 

	initializegraph(true);
	for (i = 1; i <= NUM_COUN; i++)
	{	
		for (o = startorder(orders[i]); isorder(o); nextorder(o))
		{
			if (!obadsyntax(o) && 
					(otype(o) == O_CONV ||
                     (otype(o) == O_MOVE && ounit(o) == U_ARMY)))
				addnode(o, getnode(ostart(o)));
		}
	}
    clearmarks();
	
	/* set up convoy edges */
	for (g = startnode(); isnode(g); nextnode(g))
	{
		if (gtype(g) == O_MOVE)
        {
            setgmark(g, 1 << 1);
            DrawConvoyRoute(g, g, 1, bw, gs);
            setgmark(g, gmark(g) & 1);
        }
	}
	
	/* draw convoy orders that did not appear in convoy */
	for (g = startnode(); isnode(g); nextnode(g))
	{
		if (gtype(g) == O_CONV && gmark(g) == 0)
		{
			if (isnode(gspec(g)))
			{
				pt1 = gsector(g)->land_loc;
				h = gspec(g);
				pt2 = gsector(h)->land_loc;
				if (gtype(gdest(g)) == O_MOVE &&
                    gdest(gdest(g)) == h)
					DrawConvArrow(pt1, pt2, gdest(g), bw, gs);
				else
					DrawConvArrow(pt1, pt2, g, bw, gs);			
			}
			
			/* look at convoyed army */
			if (isnode(gdest(g)))
			{
				h = gdest(g);
				/* if army can't convoy */ 
				if (gmark(h) == 0)
				{
					pt1 = gsector(h)->land_loc;
					pt2 = gsector(g)->land_loc;
					DrawConvArrow(pt1, pt2, g, bw, gs);
				}
			}
			else if (gdest(g) != NULL)
			{
				h = gdest(g);
				pt1 = gsector(h)->land_loc;
				pt2 = gsector(g)->land_loc;
				DrawConvArrow(pt1, pt2, g, bw, gs);
			}
				
		}
	}
	
	if (!bw)
	{
		RGBForeColor(&blackColor);
		RGBBackColor(&whiteColor); 
	}
}				


#ifdef NOT_DEFINED
void
DrawConvs(bool bw, bool gs)
{
	GraphNode g, h;
	Order o;
	Point pt1, pt2;
	register short i;
	short fontNum;
	RGBColor blackColor = {0,0,0}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF};
	char fontName[256];
	
	strncpy(fontName, "Monaco", 255);
	GetFNum(CtoPstr(fontName), &fontNum);
	TextFont(fontNum);
	TextSize(6); 

	initializegraph(true);
	for (i = 1; i <= NUM_COUN; i++)
	{	
		for (o = startorder(orders[i]); isorder(o); nextorder(o))
		{
			if (!obadsyntax(o) && 
					(otype(o) == O_CONV || otype(o) == O_MOVE))
				addnode(o, getnode(ostart(o)));
		}
	}
	
	/* set up convoy edges */
	for (g = startnode(); isnode(g); nextnode(g))
	{
		if (gtype(g) == O_MOVE)
		 	(void) canconvoy(g, g);
	}
	
	/* draw other convoys */
	for (g = startnode(); isnode(g); nextnode(g))
	{
		if (gtype(g) == O_CONV)
		{
			if (isnode(gspec(g)))
			{
				pt1 = gsector(g)->land_loc;
				h = gspec(g);
				pt2 = gsector(h)->land_loc;
				if (isnode(gspec(h)))
					DrawConvArrow(pt1, pt2, h, bw, gs);
				else
					DrawConvArrow(pt1, pt2, gdest(g), bw, gs);			
			}
			
			/* look at convoyed army */
			if (isnode(gdest(g)))
			{
				h = gdest(g);
				/* if army cannot convoy or it's convoyed to us */ 
				if (!isnode(gspec(h)) || gspec(h) == g)
				{
					pt1 = gsector(h)->land_loc;
					pt2 = gsector(g)->land_loc;
					if (isnode(gspec(g)))
						DrawConvArrow(pt1, pt2, g, bw, gs);
					else
						DrawConvArrow(pt1, pt2, h, bw, gs);
				}
			}
			else if (gdest(g) != NULL)
			{
				h = gdest(g);
				pt1 = gsector(h)->land_loc;
				pt2 = gsector(g)->land_loc;
				if (isnode(gspec(g)))
					DrawConvArrow(pt1, pt2, g, bw, gs);
				else
					DrawConvArrow(pt1, pt2, h, bw, gs);
			}
				
		}
	}
	
	if (!bw)
	{
		RGBForeColor(&blackColor);
		RGBBackColor(&whiteColor); 
	}
}				
#endif

/*
 * draw arrow from point 1 to point 2, base color and failure type on h
 */
void DrawConvArrow(Point pt1, Point pt2, GraphNode h, bool bw, bool gs)
{
	Rect circleRect;
	RGBColor whiteColor = {0xFFFF, 0xFFFF, 0xFFFF};
	Point pt3;

	MoveTo(pt1.h-1, pt1.v-1);
	PenNormal();
	PenPat(&qd.black);
 	if (bw)
	{
		PenSize(3,3);
		PenPat(&qd.white);
		LineTo(pt2.h-1, pt2.v-1);
		PenNormal();
	}
	if (isnode(h))
	{
		if (orderfails(gorder(h)))
			PenPat(&qd.gray);
		if (!bw && !gs)
		{
			short owner = uowner(sectunit(gsector(h)));
			SetCountryRGBColor( countries[owner].colorID );
		}
	}
					
	MoveTo(pt1.h, pt1.v);
	LineTo(pt2.h, pt2.v);
	pt3.h = short((pt1.h + pt2.h)/2.0 + 0.5);
	pt3.v = short((pt1.v + pt2.v)/2.0 + 0.5);
	SetRect(&circleRect, -4+pt3.h, -4+pt3.v, 4+pt3.h, 4+pt3.v);
	EraseOval(&circleRect);
	FrameOval(&circleRect);
	InsetRect(&circleRect, 2, 2);
	FrameArc(&circleRect, 45, -310);
					
	/* draw the bloody arrowhead */	
	DrawArrowhead(pt1, pt2);

	if (!bw)			
		RGBBackColor(&whiteColor);
	PenNormal();			
}	


#if FALSE	
	/* draw the leftovers */
	for (g = startnode(); isnode(g); nextnode(g))
	{
		if (gtype(g) == O_MOVE && canconvoy(g, g))
		{
				h = g;
				pt1 = gsector(g)->land_loc;

				while (isnode(gspec(h)))
				{
					MoveTo(pt1.h-1, pt1.v-1);
					h = gspec(h);
					pt2 = gsector(h)->land_loc;
 					if (bw)
					{
						PenSize(3,3);
						PenPat(white);
						LineTo(pt2.h-1, pt2.v-1);
						PenNormal();
					}
					if (isnode(gspec(h)))
					{
						if (orderfails(gorder(h)))
							PenPat(gray);
						if (!bw && !gs)
						{
							SetCountryRGBColor( countries[ocountry(gorder(h))].colorID );
						}
					}
					else
					{
						if (orderfails(gorder(g)))
							PenPat(gray);
						if (!bw && !gs)
						{
							SetCountryRGBColor( countries[ocountry(gorder(g))].colorID );
						}
					}
					MoveTo(pt1.h, pt1.v);
					LineTo(pt2.h, pt2.v);
					pt3.h = (pt1.h + pt2.h)/2.0 + 0.5;
					pt3.v = (pt1.v + pt2.v)/2.0 + 0.5;
					SetRect(&circleRect, -4+pt3.h, -4+pt3.v, 4+pt3.h, 4+pt3.v);
					EraseOval(&circleRect);
					FrameOval(&circleRect);
					InsetRect(&circleRect, 2, 2);
					FrameArc(&circleRect, 45, -310);
					
					
					/* draw the bloody arrowhead */	
					/* get vector */
					pt3.h = pt2.h-pt1.h;
					pt3.v = pt2.v-pt1.v;
					length = sqrt((double)(pt3.h*pt3.h + pt3.v*pt3.v));
				
					cos_theta = (float)pt3.h/length;
					sin_theta = (float)pt3.v/length;
				
					poly = OpenPoly();
					MoveTo(a1_h*cos_theta - a1_v*sin_theta + pt2.h + 0.5, 
					   a1_h*sin_theta + a1_v*cos_theta + pt2.v + 0.5);
					LineTo(a2_h*cos_theta - a2_v*sin_theta + pt2.h + 0.5, 
					   a2_h*sin_theta + a2_v*cos_theta + pt2.v + 0.5);
					LineTo(a3_h*cos_theta - a3_v*sin_theta + pt2.h + 0.5, 
					   a3_h*sin_theta + a3_v*cos_theta + pt2.v + 0.5);
					LineTo(a1_h*cos_theta - a1_v*sin_theta + pt2.h + 0.5, 
					   a1_h*sin_theta + a1_v*cos_theta + pt2.v + 0.5);
					ClosePoly();
					PaintPoly(poly);
					KillPoly(poly);
							
					pt1 = pt2;
					RGBBackColor(&whiteColor);
					PenNormal();			
				}	
			}
	}
#endif


void DrawBuild(Order o, bool bw, bool gs)
{
	Rect iconRect;
	RGBColor blackColor = {0,0,0};
		
	if (ostart(o) == 0)
		return;
	
	/* draw the outline */
	GetUnitRect(&iconRect, ounit(o), ostart(o), oscoast(o));
	InsetRect(&iconRect, 5, 5);
	
	if (!(bw | gs))
	{
		SetCountryRGBColor( countries[ocountry(o)].colorID );
	}
	if (ofails(o))
		PenPat(&qd.gray);
	PenSize(2,2);
	
	FrameRect(&iconRect);
	
	if (!(bw | gs))
		RGBForeColor(&blackColor);
	PenNormal();
		
	DrawUnit(bw, ounit(o), ostart(o), oscoast(o), ocountry(o));

}

void DrawRemove(Order o, bool bw, bool gs)
{
	if (ostart(o) == 0)
		return;
	
	DrawUnit(bw, ounit(o), ostart(o), oscoast(o), ocountry(o)); 
	if (orderfails(o))
		PenPat(&qd.gray);
	DrawX(bw | gs, ounit(o), ostart(o), oscoast(o), ocountry(o), 0);
	PenNormal();
}


void
DrawX(bool BW, UnitType type, short sector, CoastType coast, short country, short offset)
{
	Rect iconRect;
	RGBColor blackColor = {0,0,0};
//	short offh, offv;

	PenSize(3,3);

	GetUnitRect(&iconRect, type, sector, coast);
	OffsetRect(&iconRect, offset, offset);
	InsetRect(&iconRect, 11, 11);
	
	if (!BW)
	{
		SetCountryRGBColor( countries[country].colorID );
	}
	
	MoveTo((short)iconRect.left, (short)iconRect.top);
	LineTo((short)iconRect.right, (short)iconRect.bottom);
	MoveTo((short)iconRect.left, (short)iconRect.bottom);
	LineTo((short)iconRect.right, (short)iconRect.top);
	
	if (!BW)
		RGBForeColor(&blackColor);

}
