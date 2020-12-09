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
#include "MapContent.h"
#include <string.h>
#include "Game.h"
#include "Image.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "Offscreen.h"
#include "Order.h"
#include "OrdersDisplay.h"
#include "Strings.h"
#include "Util.h"
#include "WindowUtil.h"
#include "Variants.h"
#include "ConvoyRoutes.h"

Order		lastorder = NULL;
bool		can_undo = false;

extern Image*		cMapPict;
extern OffscreenPtr	mapPort;
extern ConvoyRoutes convoyroutes;
extern bool			gAllowsTrafo; 

void DoDislodgePalette(Point where);
void DoAdjustPalette(Point where);
void DoMovePalette(Point where);

bool SelectClick(short sector);

void DoMoveClick(short sector, CoastType coast, bool shiftclick);
void DoSuppClick(short sector, CoastType coast);
void DoConvClick(short sector, CoastType coast);
void DoTrafoClick(short sector, CoastType coast);
void DoBuildFleet(short sector, CoastType coast);
void DoBuildArmy(short sector);
void DoBuildWing(short sector);
void DoRemove(short sector, CoastType coast);
void DoDisband(short sector, CoastType coast);
void DoRetreat(short sector, CoastType coast);

void ReplaceOrder(Order op);
void RedrawOrder(Order oldorder);


//-------------------------------------------------------------------------------
// Order clicks
//-------------------------------------------------------------------------------
static short            firstclick = 0;
static short            secondclick = 0;
static ConvoyRoute*		cp = NULL;

void ClearClicks()
{
	firstclick = 0;
	secondclick = 0;
}

void RedrawPalette(short oldmode)
{
	if (orderPalette.GetOrderMode() != oldmode)
	{
		UpdateMapPalette();
	}
	
	if (firstclick)
		ResetSelectedUnit();

	ClearClicks();
}

void DoOrderClick(short sector, CoastType coast, bool shiftclick)
{
	char string[256];
	
	if (orderPalette.IsActive() && !gCommit && !winner)
	{
		switch (orderPalette.GetOrderMode())
		{
		case O_MOVE:
			DoMoveClick(sector, coast, shiftclick);
			break;
		
		case O_SUPP:
			DoSuppClick(sector, coast);
			break;
		
		case O_CONV:
			DoConvClick(sector, coast);
			break; 

		case O_TRAFO:
			DoTrafoClick(sector, coast);
			break;

		case O_FLEET:
			DoBuildFleet(sector, coast);
			break;
			
		case O_ARMY:
			DoBuildArmy(sector);
			break;
		
		case O_WING:
			DoBuildWing(sector);
			break;
		
		case O_REMOVE:
			if (dislodges)
				DoDisband(sector, coast);
			else 
				DoRemove(sector, coast);
			break;
		
		case O_RETR:
			DoRetreat(sector, coast);
			break; 
			
		default:
			break;
		}
	}
	
	if (winner)
	{
		GetIndCString(string, kSpecialStringsID, kNoMoreOrdersID);
		WinDialog(string, countries[winner].name, slistsize(supplies[winner]));
		return;
	}

}

void ReplaceOrder(Order op)
{
	Rect rect1, rect2, cliprect;
	Order oldorder;
	short type;
	
	if (op == NULL)
		return;
		
	if ((type = otype(op)) == O_CONV || (NUM_SECT > 125 && !mapPort->IsMonochrome()) 
		|| NUM_SECT > 175)
	{
		ShowWaitCursor();
	}
				
	cMapPict->GetRect(rect1);
	(void) OrderToRect(op, &rect1);
	if ((oldorder = findorder(orders[ocountry(op)], U_NONE, ostart(op))) != NULL)
	{
		cMapPict->GetRect(rect2);
		(void) OrderToRect(oldorder, &rect2);
		cliprect = Union(rect1, rect2);
		oldorder = copyorder(oldorder);
	}
	else
		cliprect = rect1;
		
	InsetRect(&cliprect, -4, -4);
		
	if (!addorder(orders[ocountry(op)], op, false))
		SysBeep(5);
	else
	{
		deleteorder(lastorder);
		lastorder = oldorder;
		can_undo = (lastorder != NULL);
		
//CONVERT		RectRgn(mapPort->clipRgn, &cliprect);
		DrawNewMap();
		UpdateMapDisplay(&cliprect);
//CONVERT		RectRgn(mapPort->clipRgn,&mapPort->portRect);
		gResolve = true;
		ClearClicks();
		SetGameChanged(true);
	}
	
	if (type == O_CONV || (NUM_SECT > 125 && !mapPort->IsMonochrome()) || NUM_SECT > 175)
		ShowArrowCursor();

}

bool
SelectClick(short sector)
{
	Rect rect1;
	
	if (map[sector].unit != NULL)
	{
		BlinkSector(sector, ucoast(map[sector].unit));
		SetSelectedSector(sector, false);
	/*	oldorder = findorder(orders[uowner(map[sector].unit)], utype(map[sector].unit), sector);
		SelectOrderLine(oindex(oldorder)); */
		RedrawUnit(mapPort->IsMonochrome(), utype(map[sector].unit), sector,
				 ucoast(map[sector].unit), uowner(map[sector].unit), false, false);
		GetUnitRect(&rect1, utype(map[sector].unit), sector, ucoast(map[sector].unit));
		UpdateMapDisplay(&rect1);
		firstclick = sector;
		if (cp != NULL)
		{
			// delete incomplete convoy route
			cp->Remove();
			delete cp;
		}
		return true;
	}
	else
	{
		SysBeep(3);
		ClearClicks();
		return false;
	}
}

void RedrawOrder(Order oldorder)
{
	Rect cliprect;
	
	if ((otype(oldorder) == O_REMOVE || otype(oldorder) == O_DEF_REM
			|| otype(oldorder) == O_DISBAND) && !obadsyntax(oldorder))
		RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), ostart(oldorder),
		oscoast(oldorder), ocountry(oldorder), false, true);
	else
		RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), ostart(oldorder),
				oscoast(oldorder), ocountry(oldorder), false, false);
	GetUnitRect(&cliprect, ounit(oldorder), ostart(oldorder), oscoast(oldorder));
	UpdateMapDisplay(&cliprect);
}

void
DoMoveClick(short sector, CoastType coast, bool shiftclick)
{		
	Order op = NULL;
	int i;

	if (!firstclick)
	{
		(void) SelectClick(sector);
	}
	else if (firstclick)
	{
		ClearSelectedSector();
		if (utype(map[firstclick].unit) == U_FLEET)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);
		if (shiftclick && (utype(map[firstclick].unit) == U_ARMY))
		{
			// create convoi route
			if (cp == NULL)
			{
				cp = new ConvoyRoute(firstclick);
			}
			if (cp->IsValidStep(cp->GetLastPosition(), sector, false)) 
			{
				cp->AddWaypoint(sector);
			}
		}
		else
		{
			op = neworder();
			ocountry(op) = uowner(map[firstclick].unit);
			ounit(op) = utype(map[firstclick].unit);
			ostart(op) = firstclick;
			oscoast(op) = ucoast(map[firstclick].unit);
			if (firstclick != sector)
			{
				ofinish(op) = sector;
				if (utype(map[firstclick].unit) == U_FLEET)
					ofcoast(op) = coast;
				else
					ofcoast(op) = C_NONE;
				otype(op) = O_MOVE;
			}
			else
			{
				ofinish(op) = 0;
				ofcoast(op) = C_NONE;
				otype(op) = O_HOLD;
			}				
			otext(op) = (char *) new char[2];
			strcpy(otext(op), "*");
			if (cp != NULL)
			{
				if (cp->IsValidStep(cp->GetLastPosition(), sector, true))
				{
					cp->SetFinish(sector);
					i = convoyroutes.Insert(cp);
					// content of *cp must not be deleted, since it is used in the convoy list
					cp = NULL;
				}
				else
				{
					// delete incomplete convoy route
					cp->Remove();
					delete cp;
					// delete existing convoy route from this sector
					convoyroutes.Remove(sector);
				}
			}
			else
			{
				// delete existing convoy route from this sector
				convoyroutes.Remove(sector);
			}

			ReplaceOrder(op);
		}
	}
}

void
DoTrafoClick(short sector, CoastType coast)
{		
	Order op = NULL;
	Rect rect1;

	if (!firstclick)
	{
		(void) SelectClick(sector);
	}
	else if (firstclick == sector 
			&& utype(map[firstclick].unit) != U_WING 
			&& issupp(&map[firstclick])
			&& iscoastal(&map[firstclick]))
	{
		ClearSelectedSector();
		if (utype(map[firstclick].unit) == U_ARMY)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);
		op = neworder();
		ocountry(op) = uowner(map[firstclick].unit);
		ounit(op) = utype(map[firstclick].unit);
		ostart(op) = firstclick;
		oscoast(op) = ucoast(map[firstclick].unit);
		ofinish(op) = sector;
		if (utype(map[firstclick].unit) == U_ARMY)
			ofcoast(op) = coast;
		else
			ofcoast(op) = C_NONE;
		otype(op) = O_TRAFO;
						
		otext(op) = (char *) new char[2];
		strcpy(otext(op), "*");
		
		ReplaceOrder(op);

		// delete existing convoy route from this sector
		convoyroutes.Remove(sector);
	}
	else
	{
		SysBeep(3);
		ClearSelectedSector();
		RedrawUnit(mapPort->IsMonochrome(), utype(map[firstclick].unit), firstclick,
					ucoast(map[firstclick].unit), uowner(map[firstclick].unit), false, false);
		GetUnitRect(&rect1, utype(map[firstclick].unit), firstclick, ucoast(map[firstclick].unit));
		UpdateMapDisplay(&rect1);
		firstclick = 0;
		secondclick = 0;
	}
}

void
DoSuppClick(short sector, CoastType coast)
{		
	Order op = NULL;
	Rect rect1;
	char				text[256];
	
	if (!firstclick)
		(void) SelectClick(sector);
	else if (!secondclick)
	{
		if (map[sector].unit != NULL && sector != firstclick)
		{
			BlinkSector(sector, ucoast(map[sector].unit));
			secondclick = sector;
		}
		else
		{
			SysBeep(3);
			ClearSelectedSector();
			RedrawUnit(mapPort->IsMonochrome(), utype(map[firstclick].unit), firstclick,
					 ucoast(map[firstclick].unit), uowner(map[firstclick].unit), false, false);
			GetUnitRect(&rect1, utype(map[firstclick].unit), firstclick, ucoast(map[firstclick].unit));
			UpdateMapDisplay(&rect1);
			firstclick = 0;
			secondclick = 0;
		}
	}
	else if (firstclick && secondclick)
	{
		ClearSelectedSector();
		if (utype(map[secondclick].unit) == U_FLEET)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);
		op = neworder();
		ocountry(op) = uowner(map[firstclick].unit);
		ounit(op) = utype(map[firstclick].unit);
		ostart(op) = firstclick;
		oscoast(op) = ucoast(map[firstclick].unit);
		otype(op) = O_SUPP;
		op->suborder = neworder();
		ocountry(op->suborder) = uowner(map[secondclick].unit);
		ounit(op->suborder) = utype(map[secondclick].unit);
		ostart(op->suborder) = secondclick;
		oscoast(op->suborder) = ucoast(map[secondclick].unit);
		if (secondclick != sector)
		{
			ofinish(op->suborder) = sector;
			if (utype(map[secondclick].unit) == U_FLEET)
				ofcoast(op->suborder) = coast;
			else
				ofcoast(op->suborder) = C_NONE;
			otype(op->suborder) = O_MOVE;
		}
		else
		{
			ofinish(op->suborder) = 0;
			ofcoast(op->suborder) = C_NONE;
			otype(op->suborder) = O_HOLD;
		}
		ofinish(op) = ostart(op->suborder);
		ofcoast(op) = oscoast(op->suborder); 
		order2string(text, op, false, false);				
		otext(op) = new char[strlen(text)+1];
		strcpy(otext(op), text);
		otext(op->suborder) = new char[2];
		strcpy(otext(op->suborder), "*");
		
		ReplaceOrder(op);

		// delete existing convoy route from this sector
		convoyroutes.Remove(sector);
	}
}

void
DoConvClick(short sector, CoastType)
{		
	Order				op = NULL;
	char				text[256];
	Rect				rect1;
	
	if (!firstclick)
		(void) SelectClick(sector);
	else if (!secondclick)
	{
		if (map[sector].unit != NULL && utype(map[sector].unit) == U_ARMY && sector != firstclick)
		{
			BlinkSector(sector, ucoast(map[sector].unit));
			secondclick = sector;
		}
		else
		{
			SysBeep(3);
			ClearSelectedSector();
			RedrawUnit(mapPort->IsMonochrome(), utype(map[firstclick].unit), firstclick,
					 ucoast(map[firstclick].unit), uowner(map[firstclick].unit), false, false);
			GetUnitRect(&rect1, utype(map[firstclick].unit), firstclick, ucoast(map[firstclick].unit));
			UpdateMapDisplay(&rect1);
			firstclick = 0;
			secondclick = 0;
		}
	}
	else if (firstclick && secondclick)
	{
		ClearSelectedSector();
		if (sector == firstclick || sector == secondclick || !island(map+sector))
		{
			SysBeep(3);
			RedrawUnit(mapPort->IsMonochrome(), utype(map[firstclick].unit), firstclick,
					 ucoast(map[firstclick].unit), uowner(map[firstclick].unit), false, false);
			GetUnitRect(&rect1, utype(map[firstclick].unit), firstclick, ucoast(map[firstclick].unit));
			UpdateMapDisplay(&rect1);
			firstclick = 0;
			secondclick = 0;
			return;
		}
		
		BlinkSector(sector, C_NONE);
		op = neworder();
		ocountry(op) = uowner(map[firstclick].unit);
		ounit(op) = utype(map[firstclick].unit);
		ostart(op) = firstclick;
		oscoast(op) = ucoast(map[firstclick].unit);
		otype(op) = O_CONV;
		op->suborder = neworder();
		ocountry(op->suborder) = uowner(map[secondclick].unit);
		ounit(op->suborder) = utype(map[secondclick].unit);
		ostart(op->suborder) = secondclick;
		oscoast(op->suborder) = ucoast(map[secondclick].unit);
		ofinish(op->suborder) = sector;
		ofcoast(op->suborder) = C_NONE;
		otype(op->suborder) = O_MOVE;
		ofinish(op) = ostart(op->suborder);
		ofcoast(op) = oscoast(op->suborder); 
		order2string(text, op, false, false);				
		otext(op) = new char[strlen(text)+1];
		strcpy(otext(op), text);
		otext(op->suborder) = new char[2];
		strcpy(otext(op->suborder), "*");
		
		ReplaceOrder(op);

		// delete existing convoy route from this sector
		convoyroutes.Remove(sector);
		
	}
}


void
DoBuildFleet(short sector, CoastType coast)
{		
	Order					op, oldorder = NULL;
	short 					c;
	bool				bad;
	
	if (firstclick && sector != firstclick)
	{
		SysBeep(3);
		if (firstclick == -1)
			GetSelectedSector(&firstclick, &bad);
		ClearSelectedSector();
		for (c = 1; c <= NUM_COUN; c++)
		{
			if ((oldorder = findorder(orders[c], U_NONE, firstclick)) != NULL)
			{
				RedrawOrder(oldorder);
				break;			
			}
		}
		ClearClicks();
		return;
	}
			
	
	bad = false;
	if (!issea(map+sector) 
		|| (map[sector].unit != NULL)
		|| !(issupp(map+sector)
			|| (gIsHanse && issbj(map+sector))
			|| (gHasEmergencyRule && issib(map+sector))))
		bad = true;
		
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (inslist(supplies[c], sector) 
			|| (gIsHanse && c==8 && issbj(map+sector))
			|| (gHasEmergencyRule 
					&& onehome(c, supplies[c]) 
					&& !allhome(c, supplies[c])
					&& ourreservesupply(sector, c))
			)
				break;
	}
	if (c > NUM_COUN || countries[c].builds <= 0)
		bad = true;
	
	if (!firstclick)
	{	
		if ((oldorder = findorder(orders[c], U_NONE, sector)) == NULL)
			oldorder = findorder(orders[uowner(map[sector].unit)], utype(map[sector].unit), sector);
		if (bad && oldorder == NULL)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
				
		BlinkSector(sector, coast);
		SetSelectedSector(sector, false);
		if (oldorder != NULL && !orderfails(oldorder))
			RedrawOrder(oldorder);
			
		firstclick = bad ? -1 : sector;
	}
	else
	{
		if (bad)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		
		BlinkSector(sector, coast);
		ClearSelectedSector();
		ClearClicks();
		op = neworder();
		ocountry(op) = c;
		ounit(op) = U_FLEET;
		ostart(op) = sector;
		oscoast(op) = coast;
		ofinish(op) = 0;
		ofcoast(op) = C_NONE;
		otype(op) = O_BUILD;
		otext(op) = new char[2];
		strcpy(otext(op), "*");
	
		ReplaceOrder(op);
	}
}


void
DoBuildArmy(short sector)
{		
	Order					op, oldorder = NULL;
	short c;
	bool bad;
	
	if (firstclick && sector != firstclick)
	{
		SysBeep(3);
		if (firstclick == -1)
			GetSelectedSector(&firstclick, &bad);
		ClearSelectedSector();
		for (c = 1; c <= NUM_COUN; c++)
		{
			if ((oldorder = findorder(orders[c], U_NONE, firstclick)) != NULL)
			{
				RedrawOrder(oldorder);
				break;
			}
		}
		ClearClicks();
		return;
	}
	
	bad = false;		
	if (!island(map+sector) 
		|| (map[sector].unit != NULL)
		|| !(issupp(map+sector)
			|| (gHasEmergencyRule && issib(map+sector))))
		bad = true;
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (inslist(supplies[c], sector) 
			|| (gHasEmergencyRule 
					&& onehome(c, supplies[c]) 
					&& !allhome(c, supplies[c])
					&& ourreservesupply(sector, c))
			)
			break;
	}
	if (c > NUM_COUN || countries[c].builds <= 0)
		bad = true;
		
	if (!firstclick)
	{
		if ((oldorder = findorder(orders[c], U_NONE, sector)) == NULL)
			oldorder = findorder(orders[uowner(map[sector].unit)], utype(map[sector].unit), sector);
		if (bad && oldorder == NULL)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		BlinkSector(sector, C_NONE);
		SetSelectedSector(sector, false);
		if (oldorder != NULL)
			RedrawOrder(oldorder);
		firstclick = bad ? -1: sector;
	}
	else
	{
		if (bad)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		BlinkSector(sector, C_NONE);
		ClearSelectedSector();
		ClearClicks();
		op = neworder();
		ocountry(op) = c;
		ounit(op) = U_ARMY;
		ostart(op) = sector;
		oscoast(op) = C_NONE;
		ofinish(op) = 0;
		ofcoast(op) = C_NONE;
		otype(op) = O_BUILD;
		otext(op) = new char[2];
		strcpy(otext(op), "*");
	
		ReplaceOrder(op);
	}
}

void
DoBuildWing(short sector)
{		
	Order					op, oldorder = NULL;
	short c;
	bool bad;
	
	if (firstclick && sector != firstclick)
	{
		SysBeep(3);
		if (firstclick == -1)
			GetSelectedSector(&firstclick, &bad);
		ClearSelectedSector();
		for (c = 1; c <= NUM_COUN; c++)
		{
			if ((oldorder = findorder(orders[c], U_NONE, firstclick)) != NULL)
			{
				RedrawOrder(oldorder);
				break;
			}
		}
		ClearClicks();
		return;
	}
	
	bad = false;		
	if ((map[sector].unit != NULL)
		|| !(issupp(map+sector)
			|| (gHasEmergencyRule && issib(map+sector))))
		bad = true;
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (inslist(supplies[c], sector) 
			|| (gHasEmergencyRule 
					&& onehome(c, supplies[c]) 
					&& !allhome(c, supplies[c])
					&& ourreservesupply(sector, c))
			)
			break;
	}
	if (c > NUM_COUN || countries[c].builds <= 0)
		bad = true;
		
	if (!firstclick)
	{
		if ((oldorder = findorder(orders[c], U_NONE, sector)) == NULL)
			oldorder = findorder(orders[uowner(map[sector].unit)], utype(map[sector].unit), sector);
		if (bad && oldorder == NULL)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		BlinkSector(sector, C_NONE);
		SetSelectedSector(sector, false);
		if (oldorder != NULL)
			RedrawOrder(oldorder);
		firstclick = bad ? -1: sector;
	}
	else
	{
		if (bad)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		BlinkSector(sector, C_NONE);
		ClearSelectedSector();
		ClearClicks();
		op = neworder();
		ocountry(op) = c;
		ounit(op) = U_WNG;
		ostart(op) = sector;
		oscoast(op) = C_NONE;
		ofinish(op) = 0;
		ofcoast(op) = C_NONE;
		otype(op) = O_BUILD;
		otext(op) = new char[2];
		strcpy(otext(op), "*");
	
		ReplaceOrder(op);
	}
}


void
DoRemove(short sector, CoastType coast)
{		
	Order					op, oldorder = NULL;
	Rect					cliprect;
	short c;
	bool bad;
	
	if (firstclick && sector != firstclick)
	{
		SysBeep(3);
		if (firstclick == -1)
			GetSelectedSector(&firstclick, &bad);
		ClearSelectedSector();
		for (c = 1; c <= NUM_COUN; c++)
		{
			if ((oldorder = findorder(orders[c], U_NONE, firstclick)) != NULL)
			{
				RedrawOrder(oldorder);
				break;
			}
		}			
		if (oldorder == NULL && map[firstclick].unit != NULL)
		{
			RedrawUnit(mapPort->IsMonochrome(), utype(map[firstclick].unit), firstclick,
				 ucoast(map[firstclick].unit), uowner(map[firstclick].unit), false, false);
			GetUnitRect(&cliprect, utype(map[firstclick].unit), firstclick, ucoast(map[firstclick].unit));
			UpdateMapDisplay(&cliprect);
		}
		ClearClicks();
		return;
	}
	
	bad = (map[sector].unit == NULL || countries[uowner(map[sector].unit)].builds >= 0);
			
	if (!firstclick)
	{
		if (map[sector].unit == NULL || 
			(oldorder = findorder(orders[uowner(map[sector].unit)], U_NONE, sector)) == NULL)
		{
			for (c = 1; c <= NUM_COUN; c++)
			{
				if ((oldorder = findorder(orders[c], U_NONE, sector)) != NULL)
					break;
			}
		}
		if (bad && oldorder == NULL)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		BlinkSector(sector, C_NONE);
		SetSelectedSector(sector, false);
		if (oldorder != NULL)
			RedrawOrder(oldorder);
		firstclick = bad ? -1: sector;
	}
	else
	{
		if (bad)
		{
			ClearClicks();
			SysBeep(3);
			return;
		}
		BlinkSector(sector, coast);
		ClearSelectedSector();
		ClearClicks();
		op = neworder();
		ocountry(op) = uowner(map[sector].unit);
		ounit(op) = utype(map[sector].unit);
		ostart(op) = sector;
		oscoast(op) = ucoast(map[sector].unit);
		ofinish(op) = 0;
		ofcoast(op) = C_NONE;
		otype(op) = O_REMOVE;
		otext(op) = new char[2];
		strcpy(otext(op), "*");
	
		ReplaceOrder(op);
	}
}



void
DoRetreat(short sector, CoastType coast)
{		
	Order					op, oldorder = NULL;
	Rect  rect1;
	UnitType type;
	
	if (!firstclick)
	{
		Dislodge* dislodge = dslglist.Find( sector );
		if (dislodge == NULL)
		{
			SysBeep(5);
			return;
		}
		type = dislodge->Unit();
		if (type == U_FLEET)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);
		SetSelectedSector(sector, true);
		if ((oldorder = findorder(orders[dislodge->Owner()], type, sector)) != NULL)
		{
			if ((otype(oldorder) == O_REMOVE || otype(oldorder) == O_DEF_REM
				|| otype(oldorder) == O_DISBAND) && !obadsyntax(oldorder))
				RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), sector,
				 	oscoast(oldorder), ocountry(oldorder), true, true);
			else
				RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), sector,
				 	oscoast(oldorder), ocountry(oldorder), true, false);
			GetUnitRect(&rect1, ounit(oldorder), sector, oscoast(oldorder));
		}
		else
		{
		/*	SelectOrderLine(oindex(oldorder)); */
			RedrawUnit(mapPort->IsMonochrome(), type, sector,
				dislodge->Coast(), dislodge->Owner(), true, false); 
			GetUnitRect(&rect1, type, sector, dislodge->Coast());
		}
		InsetRect(&rect1, -4, -4);
		UpdateMapDisplay(&rect1);
	
		firstclick = sector;
	}
	else if (firstclick)
	{
		ClearSelectedSector(); 
		Dislodge* dislodge = dslglist.Find( firstclick );
		if (firstclick == sector || 
			!innlist(dislodge->Retreats(), sector, coast, false))
		{
			SysBeep(5);
			type = dislodge->Unit();
			if ((oldorder = findorder(orders[dislodge->Owner()], type, firstclick)) != NULL)
			{
				if ((otype(oldorder) == O_REMOVE || otype(oldorder) == O_DEF_REM
					|| otype(oldorder) == O_DISBAND) && !obadsyntax(oldorder))
					RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), firstclick,
				 		oscoast(oldorder), ocountry(oldorder), true, true);
				else
					RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), firstclick,
				 		oscoast(oldorder), ocountry(oldorder), true, false);
				GetUnitRect(&rect1, ounit(oldorder), firstclick, oscoast(oldorder));
			}
			else
			{
				RedrawUnit(mapPort->IsMonochrome(), dislodge->Unit(), firstclick,
					dislodge->Coast(), dislodge->Owner(), true, false); 
				GetUnitRect(&rect1, dislodge->Unit(), firstclick, dislodge->Coast());
			}
			InsetRect(&rect1, -4, -4);
			UpdateMapDisplay(&rect1);
			firstclick = 0;
			secondclick = 0;
			return;
		}
		if (dislodge->Unit() == U_FLEET)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);
			
		op = neworder();
		ocountry(op) = dislodge->Owner();
		ounit(op) = dislodge->Unit();
		ostart(op) = dislodge->Sector();
		oscoast(op) = dislodge->Coast();
		ofinish(op) = sector;
		if (ounit(op) == U_FLEET)
			ofcoast(op) = coast;
		else
			ofcoast(op) = C_NONE;
		otype(op) = O_MOVE;
		otext(op) = new char[2];
		strcpy(otext(op), "*");
		
		ReplaceOrder(op);  /**** need offset */
	}
}


void
DoDisband(short sector, CoastType coast)
{		
	Order					op, oldorder = NULL;
	short i;
	Rect rect1;
	UnitType type;
	
	if (!firstclick)
	{
		Dislodge* dislodge = dslglist.Find( sector );
		if (dislodge == NULL)
		{
			SysBeep(3);
			return;
		}
		type = dislodge->Unit();
		if (type == U_FLEET)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);
		SetSelectedSector(sector, true);
		if ((oldorder = findorder(orders[dislodge->Owner()], type, sector)) != NULL)
		{
			if ((otype(oldorder) == O_REMOVE || otype(oldorder) == O_DEF_REM
				|| otype(oldorder) == O_DISBAND) && !obadsyntax(oldorder))
				RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), sector,
				 	oscoast(oldorder), ocountry(oldorder), true, true);
			else
				RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), sector,
				 	oscoast(oldorder), ocountry(oldorder), true, false);
			GetUnitRect(&rect1, ounit(oldorder), sector, oscoast(oldorder));
		}
		else
		{
			RedrawUnit(mapPort->IsMonochrome(), type, sector,
					dislodge->Coast(), dislodge->Owner(), true, false); 
			GetUnitRect(&rect1, type, sector, dislodge->Coast());
		}
		InsetRect(&rect1, -4, -4);
		UpdateMapDisplay(&rect1);
	
		firstclick = sector;
	}
	else if (firstclick)
	{
		ClearSelectedSector(); 
		Dislodge* dislodge = dslglist.Find( firstclick );
		if (firstclick != sector)
		{
			SysBeep(3);
			type = dislodge->Unit();
			if ((oldorder = findorder(orders[dislodge->Owner()], type, firstclick)) != NULL)
			{
				if ((otype(oldorder) == O_REMOVE || otype(oldorder) == O_DEF_REM
					|| otype(oldorder) == O_DISBAND) && !obadsyntax(oldorder))
					RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), firstclick,
				 		oscoast(oldorder), ocountry(oldorder), true, true);
				else
					RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), firstclick,
				 		oscoast(oldorder), ocountry(oldorder), true, false);
				GetUnitRect(&rect1, ounit(oldorder), firstclick, oscoast(oldorder));
			}
			else
			{
				RedrawUnit(mapPort->IsMonochrome(), dislodge->Unit(), firstclick,
					dislodge->Coast(), dislodge->Owner(), true, false); 
				GetUnitRect(&rect1, dislodge->Unit(), firstclick, dislodge->Coast());
			}
			InsetRect(&rect1, -4, -4);
			UpdateMapDisplay(&rect1);
			ClearClicks();
			return;
		}
		if (dislodge->Unit() == U_FLEET)
			BlinkSector(sector, coast);
		else
			BlinkSector(sector, C_NONE);

		op = neworder();
		i = secondclick;
		ocountry(op) = dislodge->Owner();
		ounit(op) = dislodge->Unit();
		ostart(op) = sector;
		oscoast(op) = dislodge->Coast();
		ofinish(op) = 0;
		ofcoast(op) = C_NONE;
		otype(op) = O_DISBAND;
		otext(op) = new char[2];
		strcpy(otext(op), "*");
	
		ReplaceOrder(op);
	}
}


void
ClearLastOrder()
{
	deleteorder(lastorder);
	lastorder = NULL;
	can_undo = false;
}

bool
LastOrderSet()
{
	return (can_undo && !firstclick && !secondclick);
}


void
UndoLastOrder()
{
	Order op, oldorder = NULL;
	Rect rect1, rect2, cliprect;
	
	op = lastorder;			
	lastorder = NULL;
	can_undo = false;

	cMapPict->GetRect(rect1);
	(void) OrderToRect(op, &rect1);
	if ((oldorder = findorder(orders[ocountry(op)], U_NONE, ostart(op))) != NULL)
	{
		cMapPict->GetRect(rect2);
		(void) OrderToRect(oldorder, &rect2);
		cliprect = Union(rect1, rect2);
		oldorder = copyorder(oldorder);
	}
	else
		cliprect = rect1;

	InsetRect(&cliprect, -4, -4);
	if (!addorder(orders[ocountry(op)], op, false))
		SysBeep(3);
	else
	{
		if (can_undo)
			deleteorder(lastorder);
		lastorder = oldorder;
		can_undo = (oldorder != NULL);

//CONVERT		RectRgn(mapPort->clipRgn, &cliprect);
		
    	ClearClicks();
    	ClearOrderSelections();
    	ClearSelectedSector();

		DrawNewMap();
		UpdateMapDisplay(&cliprect);
		gResolve = true;
		SetGameChanged(true);
		
//CONVERT		RectRgn(mapPort->clipRgn, &mapPort->portRect);
	}
}

void ClearSelectedUnit()
{
	Rect cliprect;
	Order op = NULL;
	short i;
	short s;
	bool d;

	GetSelectedSector(&s, &d);
	if (s == 0)
		return;
		
	for (i = 1; i <= NUM_COUN; i++)
	{
		if ((op = findorder(orders[i], U_NONE, s)) != NULL)
			break;
	}
	
	cMapPict->GetRect(cliprect);
	if (lastorder != NULL)
		deleteorder(lastorder);
	if (op != NULL)
	{		
		OrderToRect(op, &cliprect);
		lastorder = copyorder(op);
		can_undo = true;
		clearorder(op->index);
	}
	else
	{
		can_undo = false;
		lastorder = NULL;
	}
	
	ClearSelectedSector();
	ClearClicks();
//CONVERT	RectRgn(mapPort->clipRgn, &cliprect);
	DrawNewMap();
	UpdateMapDisplay(&cliprect);
//CONVERT	RectRgn(mapPort->clipRgn,&mapPort->portRect);
}
