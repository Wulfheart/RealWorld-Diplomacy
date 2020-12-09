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
#include <string.h>
#include <stdlib.h>
#include "ReadOrders.h"
#include "EditMap.h"
#include "MapDisplay.h"
#include "MapData.h"
#include "MapContent.h"
#include "OrdersDisplay.h"
#include "StatusDisplay.h"
#include "Parse.h"
#include "Game.h"
#include "History.h"
#include "Util.h"
#include "Order.h"
#include "Unit.h"
#include "MenuUtil.h"
#include "Strings.h"
#include "TextFile.h"
#include "WindowUtil.h"
#include "Variants.h"

#define NO_COUNTRY 1

int  GetFileType(TextFile& fp);
void ReadJudgeFile(TextFile& fp, short filetype);
void ReadOrdersFile(TextFile& fp);
void ScanJudgeInfo(char *line, int type);
void ScanUnits(char *line, short filetype);
void ScanSupplies(char *line);
void ScanOrder(char *line, short filetype);
void ScanDislodge(char *line, short filetype);
void OutputErrors();
void PreLoadSetup();
void PostLoadCleanup();
char *getsline(char *s, int size, TextFile& fp, bool order_line);

void resolve_orders(Orderlist orders[], bool ign_units);

static bool moves = false;
static bool dslg_read = false;
static bool supp_read = false;
static bool unit_read = false;
static short filetype = 0;

#define kNonJudgeFile 0
#define kStatusFile 1
#define kMovementFile 2
#define kRetreatFile 3
#define kAdjustFile 4


static short last_dislodges, last_adjustments;
static bool found_dislodge = false;
static bool unresolved_orders = false;

void PreLoadSetup()
{
	register short i;

	cleanorders(orders[0]);
	SelectOrderLine(-1);
	
	/* if didn't finish from last time */
	if (gCommit)
	{
		dislodges = last_dislodges;
		if (last_dislodges == 0)
			dslglist.Clean();
		adjustments = last_adjustments;
		if (last_adjustments == 0)
			for (i = 0; i <= NUM_COUN; i++)
				countries[i].builds = 0;
	}
	else
	{
		last_dislodges = dislodges;
		last_adjustments = adjustments;
	}
	
	gSaveDislodge = false;
	if (gIgnoreUnits)
	{
   		dslglist.Clean();
 		dislodges = 0;
 		adjustments = 0;
		for (i = 0; i <= NUM_COUN; i++)
		{
			if (units != NULL)
				cleanunits(&units[i]);
			countries[i].builds = 0;
			if (supplies != NULL)
				cleansupplies(&supplies[i]);
			if (lands != NULL)
				cleansupplies(&lands[i]);
		}
		CleanOrdersList();
		for (i = 1; i <= NUM_COUN; i++)
		{
			if (orders != NULL)
			cleanorders(orders[i]);
		} 
 	}
 	if (dislodges > 0)
 	{
 		for (Dislodge* current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next( current ))
 		{
 			if (current->Retreats() != NULL)
  				current->SetType( DB_GEN );
  		}
  		gSaveDislodge = (dislodges != 0
  						&& (season == S_SUMMER || season == S_AUTUMN)
  						);
  	}
		
/*	gamename[0] = '\0';
	season = S_NONE; */
	
	if ((dislodges 
		&& (season == S_SUMMER || season == S_AUTUMN)
		)
		|| adjustments)
		moves = false;
	else
		moves = true;
		
	dslg_read = false;
	supp_read = false;
	unit_read = false;
	
	unresolved_orders = (haveorders() && gResolve);
}

void PostLoadCleanup()
{
	SetGameChanged(true);
	if (filetype != kNonJudgeFile)
	{
		if (filetype == kStatusFile)
		{
			gResolve = haveorders();
			gCommit = false;
		}
		else
		{
			gResolve = false;
			gCommit = true;
		}
	}
	else if (gResolve)
		gCommit = false;
	else
		gCommit = true;
		
	// unresolved orders trump everything
	if (unresolved_orders)
	{
		gResolve = true;
		gCommit = false;
		unresolved_orders = false;
	}
	
 	if (filetype == kStatusFile)
	{
		short numbuilds[MAX_COUN+1];
		if (season == S_WINTER && dislodges == 0)
		{
			adjustments = countbuilds(units, supplies, dslglist, numbuilds);
			for (int c = 1; c <= NUM_COUN; ++c)
			{
				countries[c].builds = numbuilds[c];
			}
		}
	}

	if (gCommit && !gResolve)
		history.CopyCurrentState();
		
   	ClearClicks();
    ClearOrderSelections();
    ClearSelectedSector();
    ClearLastOrder();
   	orderPalette.Reset();
 
	OutputStatus();
	
	if (filetype == kStatusFile && !haveorders())
		NextOrders(); 
	else if (gIgnoreUnits)
		OutputOrders();
	
	DrawNewMap();
	UpdateMapDisplay(NULL);
		
	if (isorder(orders[0]))
		OutputErrors();
}

bool
LoadOrders(const FSSpec* filename)
{
	char string[256];
	TextFile fp;
	register short i;
	char fTitleName[BUFSIZ] = {0};
	FSSpec fName;
	
	if (!gIgnoreUnits && winner && history.AtEnd())
	{
		GetIndCString(string, kSpecialStringsID, kNoMoreOrdersID);
		WinDialog(string, countries[winner].name, slistsize(supplies[winner]));                         
		return false;
	}
		
	if (gCommit)
	{
		char string[256];
		GetIndCString(string, kSpecialStringsID, kOverResolvStringID);
		if (!CheckAction(string))
			return false;		
	}
		
	if (filename == NULL || EmptyFSSpec(*filename))
	{		
		if (!GetFile(fName, fTitleName, textFilter))
			return false;

		if (!fp.Open(fName, false))
		{
			gErrNo = errFile;
			char basename[256];
			GetBasename(basename, fName);
			sprintf(gErrorString, "Couldn't open file %s", basename);
			return false;
		}
	}
	else if (!fp.Open(*filename, false))
	{
		gErrNo = errFile;
		char basename[256];
		GetBasename(basename, *filename);
		sprintf(gErrorString, "Couldn't open file %s", basename);
		return false;
	}
	
	ShowWaitCursor();
	
	PreLoadSetup();

	filetype = GetFileType(fp); 
	if (!gSaveDislodge)
		gSaveDislodge = (filetype == kStatusFile);
		
	if (strlen(gamename) > 0)
	{
		SetMapTitle(gamename);
	}
	ShowMapWindow();

/*	if (OrdersHidden())
		HideOrdersWindow();
	SelectWindow(ordersWindow); */
		
	gResolve = true;
	switch (filetype)
	{
		case kStatusFile:
			if (!gIgnoreUnits)
			{
   				dslglist.Clean();
 				dislodges = 0;
 				adjustments = 0;
				for (i = 0; i <= NUM_COUN; i++)
				{
					if (units != NULL)
						cleanunits(&units[i]);
					countries[i].builds = 0;
					if (supplies != NULL)
						cleansupplies(&supplies[i]);
					if (lands != NULL)
						cleansupplies(&lands[i]);
				}
				CleanOrdersList();
				for (i = 1; i <= NUM_COUN; i++)
				{
					if (orders != NULL)
					cleanorders(orders[i]);
				} 
 			}
 			/* fall through */
		case kMovementFile:
		case kAdjustFile:
		case kRetreatFile:
			ReadJudgeFile(fp, filetype);
			if (filetype == kRetreatFile)
				gSaveDislodge = (dislodges != 0
								 && (season == S_SUMMER || season == S_AUTUMN)
								 ); 
			break;
		
		default:
			fp.Rewind();
			ReadOrdersFile(fp);
			break;
	}
	
	fp.Close();

	PostLoadCleanup();
	
	ShowArrowCursor();

	return true;
}


bool
LoadGameOrders(TextFile& fp)
{
	char string[256];
	
	if (!gIgnoreUnits && winner && history.AtEnd())
	{
		GetIndCString(string, kSpecialStringsID, kNoMoreOrdersID);
		WinDialog(string, countries[winner].name, slistsize(supplies[winner]));                         
		return false;
	}
		
	ShowWaitCursor();
	
	PreLoadSetup();

	filetype = kNonJudgeFile;
		
	if (strlen(gamename) > 0)
	{
		SetMapTitle(gamename);
	}
//CONVERT	ShowMapWindow();

/*	if (OrdersHidden())
		HideOrdersWindow();
	SelectWindow(ordersWindow); */
		
	gResolve = true;
	ReadOrdersFile(fp);
	
	fp.Close();

	PostLoadCleanup();
	
	ShowArrowCursor();

	return true;
}

void
PasteOrders()
{
	FSSpec filename;
	
	if (CopyScrapToFile(filename))
	{
		LoadOrders(&filename);
		DeleteTempFile(filename);
	}
}


bool
WriteGameOrders(TextFile& fp)
{
	short i;
	Order order;
	char  text[512];
	long count;
	
	for (i = 1; i <= NUM_COUN; i++)
	{
		for (order = startorder(orders[i]); isorder(order); nextorder(order))
		{
	       	order2string(text, order, false, true);
//	       	strcat(text, "\n");
	       	count = strlen(text);
		   	if (count > 0 && !fp.WriteLine(text))
		   	{
				gErrNo = errFile;
				return false;
			}
		}	
	}
	
	return true;
}

void
ResolveOrders()
{
	register short i;
	Order order;
	char text[256];
	char string[256];
	
	if (!gResolve)
		return;
		
	if (winner && history.AtEnd())
	{
		GetIndCString(string, kSpecialStringsID, kNoMoreOrdersID);
		WinDialog(string, countries[winner].name, slistsize(supplies[winner]));                         
		return;
	}

	ShowWaitCursor();
		
	gSaveDislodge = (dislodges != 0);
	moves = !(dislodges || adjustments);

	HideErrorWindow();
	ClearOrderSelections();
		
	for (i = 1; i <= NUM_COUN; i++)
	{
		for (order = startorder(orders[i]); isorder(order); nextorder(order))
		{
			checkorder(order, gIgnoreUnits);
			if (orderfails(order))
			{
	       		order2string(text, order, false, false);
		   		SetOrderLine(text, oindex(order));
		   	}
		}	
	}
/*	resolve_orders(orders, gIgnoreUnits && filetype != kNonJudgeFile); */
	resolve_orders(orders, false);
	ClearCanalOrders();

	history.CopyCurrentState();
		
	gResolve = false;
	gCommit = true;
	SetGameChanged(true);
	
    ClearClicks();
    ClearOrderSelections();
    ClearSelectedSector();
    ClearLastOrder();

	DrawNewMap();
	UpdateMapDisplay(NULL);

 	OutputStatus();
	ShowArrowCursor();

}

void
UndoResolve()
{
	short i;
	Order order;
	char text[256];
	
	ShowWaitCursor();
	
	ClearOrderSelections();
	for (i = 1; i <= NUM_COUN; i++)
	{
		for (order = startorder(orders[i]); isorder(order); nextorder(order))
		{
			if (orderfails(order))
			{
				setofails(order, R_NONE);
	       		order2string(text, order, false, false);
		   		SetOrderLine(text, oindex(order));
		   	}
		   	else if (otype(order) == O_DEF_REM)
		   		clearorder(order->index);
		   	else
		   		setofails(order, R_NONE);
		}
	}
	
	gResolve = true;
	gCommit = false;
	SetGameChanged(true);
	
	/* this can only happen after a move phase */
	if ((dislodges != 0) != gSaveDislodge)
	{
		dislodges = 0;
		gSaveDislodge = true;
  		dslglist.Clean();
	}
	/* reset dislodge info if there are any */
	else if (dislodges)
	{
 		for (Dislodge* current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next( current ))
 		{
			current->SetType( DB_GEN );
  		}
 	}
	
	orderPalette.Reset();
	DrawNewMap();
	UpdateMapDisplay(NULL);

 	OutputStatus();
	
	ShowArrowCursor();
}

void
CommitOrders()
{
	if (!gCommit)
		return;
		
	ShowWaitCursor();
	
	HideErrorWindow();
	
	exec_orders(orders, moves);
		
	gCommit = false;
	gResolve = true;
	gSaveDislodge = true;
		
	NextOrders();
	
	history.AppendCurrentState();
	
	DrawNewMap();
	orderPalette.Reset();
	UpdateMapDisplay(NULL);
	
	OutputStatus();
	
	if (winner && history.AtEnd())
		WinDialog("", countries[winner].name, slistsize(supplies[winner]));                         
		

	ShowArrowCursor();

	gIgnoreUnits = false;
	(void) CheckIgnoreUnitsItem(gIgnoreUnits);
		
}

bool PreviousSeason()
{
	if (history.AtBeginning())
		return true;
		
	if (EditModeOn())
		return true;
		
	if (history.AtEnd() && haveorders() && gCommit)
	{
		char string[256];
		GetIndCString(string, kSpecialStringsID, kPreviousStringID);
		if (!CheckAction(string))
			return true;		
	}
		
	ShowWaitCursor();
	
	HideErrorWindow();
    ClearClicks();
    ClearOrderSelections();
    ClearSelectedSector();
    ClearLastOrder();
	
	if (!history.Previous())
		return false;
	
	gCommit = false;
	gResolve = true;
	gSaveDislodge = true;
		
	// check for orders
	bool noOrders = true;
	for (int c = 1; c <= NUM_COUN; ++c)
	{
		if (isorder(orders[c]))
		{
			noOrders = false;
			break;
		}
	}
	if (noOrders)
	{
		NextOrders();
	}
	else
	{
		CleanOrdersList();
		OutputOrders();
	}
	
	DrawNewMap();
	orderPalette.Reset();
	UpdateMapDisplay(NULL);
	
	OutputStatus();
	
	if (winner && history.AtEnd())
		WinDialog("", countries[winner].name, slistsize(supplies[winner]));                         

	ShowArrowCursor();

	gIgnoreUnits = false;
	(void) CheckIgnoreUnitsItem(gIgnoreUnits);
		

	return true;
}

bool NextSeason()
{
	if (history.AtEnd())
		return true;
		
	if (EditModeOn())
		return true;
		
	ShowWaitCursor();
	
	HideErrorWindow();
    ClearClicks();
    ClearOrderSelections();
    ClearSelectedSector();
    ClearLastOrder();
	
	if (!history.Next())
		return false;
	
	gCommit = false;
	gResolve = true;
	gSaveDislodge = true;
		
	// check for orders
	bool noOrders = true;
	for (int c = 1; c <= NUM_COUN; ++c)
	{
		if (isorder(orders[c]))
		{
			noOrders = false;
			break;
		}
	}
	// if there are none, we need to create some
	if (noOrders)
	{
		NextOrders();
	}
	// otherwise, use what's there
	else
	{
		CleanOrdersList();
		OutputOrders();
	}
	
	DrawNewMap();
	orderPalette.Reset();
	UpdateMapDisplay(NULL);
	
	OutputStatus();
	
	if (winner && history.AtEnd())
		WinDialog("", countries[winner].name, slistsize(supplies[winner]));                         

	ShowArrowCursor();

	gIgnoreUnits = false;
	(void) CheckIgnoreUnitsItem(gIgnoreUnits);
		

	return true;
}

int
GetFileType(TextFile& fp)
{
	char line[BUFSIZ];

	while (!fp.AtEOF())
	{
		if (fp.ReadLine(line, BUFSIZ))
		{
			if (!strncmp(line, "Status ", 7))
			{
				ScanJudgeInfo(line, kStatusFile);
				return kStatusFile;
			}
			else if (!strncmp(line, "Movement", 8))
			{
				ScanJudgeInfo(line, kMovementFile);
				return kMovementFile;
			}	
			else if (!strncmp(line, "Retreat", 7))
			{
				ScanJudgeInfo(line, kRetreatFile);
				return kRetreatFile;
			}
			else if (!strncmp(line, "Adjustment", 10))
			{
				ScanJudgeInfo(line, kAdjustFile);
				return kAdjustFile;
			}
		}
		
	}
	return kNonJudgeFile;
}

void
ScanJudgeInfo(char *line, int type)
{
	char *word;
	char *gnp;
	
	word = skipword(line);
	while (word != NULL && string2season(word) == S_NONE)
		word = skipword(word);
	
	if (word == NULL)
	{
		season = S_NONE;
		return;
	}
	season = string2season(word);
	/* set proper seasons for Status file */
	if (type == kRetreatFile || (type == kStatusFile && !strncmp(line+14, "Retreat", 7)))
	{
		if (season == S_SPRING)
			season = S_SUMMER;
		else if (season == S_FALL)
			season = S_AUTUMN;
	}
	else if (type == kAdjustFile || (type == kStatusFile && !strncmp(line+14, "Adjustment", 10)))
	{
		if (season == S_FALL)
			season = S_WINTER;
	}
	else if (type == kMovementFile || (type == kStatusFile && !strncmp(line+14, "Movement", 8)))
	{
		if (season == S_AUTUMN)
			season = S_FALL;
	}
	word = skipword(word);
	if (word == NULL)
	{
		season = S_NONE;
		return;
	}
	// skip the 'of'
	if (word[0] == 'o' && word[1] == 'f')
		word = skipword(word);
	if (word != NULL)
		year = atoi(word);
	else
		season = S_NONE;
		
	gamename[0] = '\0';
	word = strchr(word, '(');
	if (word != NULL)
	{
		word++;
		gnp = gamename;
		while (*word != '.' && *word != ')' && gnp-gamename < 30)
			*gnp++ = *word++;
		*gnp = '\0';
	}
}


void
ReadJudgeFile(TextFile &fp, short filetype)
{
	char line[BUFSIZ];
	bool d = false, s = false, u = false;
	short i;
	Order order;
	
	found_dislodge = false;
	
	/* read orders first */
	while (!fp.AtEOF())
	{
		if (getsline(line, BUFSIZ, fp, true) != NULL)
		{
			if (!strncmp(line, "The", 3))
			{
				d = true;
				break;
			}
			else if (!strncmp(line, "Unit locations", 14))
			{
				u = true;
				break;
			}
			else if (!strncmp(line, "Ownership", 9))
			{
				s = true;
				break;
			}
			if (strlen(line) > 0)
				ScanOrder(line, filetype);
		}
	}

	/* read a blank line */
/*	(void) fgets(line, BUFSIZ, fp); */

	if (d)
	{
		bool lineSeen = false;		// if a non-blank line is seen
		while (getsline(line, BUFSIZ, fp, false) != NULL)
		{
			// skip over empty lines
			if (strlen(line) == 0)
			{
				// unless we've already seen a non-blank line
				if (lineSeen) 
					break;
			}
			// non-blank line
			else
			{
				lineSeen = true;
				ScanDislodge(line, filetype); 
			}
		}
		if (getsline(line, BUFSIZ, fp, false) != NULL)
		{
			if (!strncmp(line, "Ownership", 9))
			{
				s = true;	
			}
			else if (!strncmp(line, "Unit locations", 14))
			{
				u = true;	
			}
		}
	}
	
	// skip unit locations
	if (u)
	{
		while (getsline(line, BUFSIZ, fp, false) != NULL)
		{
			if (!strncmp(line, "Ownership", 9))
			{
				s = true;
				break;	
			}
			if (gIgnoreUnits || filetype == kStatusFile)
				ScanUnits(line, filetype);
		}
	}
	
	if (s)
	{
		if (!gIgnoreUnits)
		{
			for (i = 0; i <= NUM_COUN; i++)
			{
				if (supplies != NULL)
					cleansupplies(&supplies[i]);
				if (lands != NULL)
					cleansupplies(&lands[i]);
			}
		}
		/* skip blank lines */
		while (getsline(line, BUFSIZ, fp, false) != NULL && strlen(line) == 0)
		{
		}
			
		do
		{
			if (strlen(line) == 0 || strnsame(line,"Unowned:",8))
				break;
			ScanSupplies(line);
		}
		while (getsline(line, BUFSIZ, fp, false) != NULL);
		if (strnsame(line,"Unowned:", 8))
			ScanSupplies(line);
	}
		
	// if the orders say there's a dislodge, but none loaded
	if (found_dislodge && dislodges == 0)
	{
		// if already resolved, got a problem
		if (!gResolve && gCommit)
		{
			char string[256];
			GetIndCString(string, kSpecialStringsID, kNoDislodgeInfoID);
			ErrorString(string);
		}
		/* not much choice, clear everything out */
		for (i = 1; i <= NUM_COUN; i++)
		{
			for (order = startorder(orders[i]); isorder(order); nextorder(order))
			{
				if (orderfails(order))
				{
					setofails(order, R_NONE);
	       			order2string(line, order, false, false);
		   			SetOrderLine(line, oindex(order));
		   		}
		   	}
		}
		gResolve = true;
		gCommit = false;
	}
	
}

void
ReadOrdersFile(TextFile& fp)
{
	char line[BUFSIZ], otherline[BUFSIZ];
	short i;
	Order order;
	short currentCountry = CN_NONE, c;

	found_dislodge = false;
	
	/* read orders first */
	while (!fp.AtEOF())
	{
		if (fp.ReadLine(line, BUFSIZ))
		{
			if (strlen(line) > 0)
			{
				// get country, if any
				string2country(&c, line);
				if (c != CN_NONE)
				{
					currentCountry = c;
					// if country is not the only word on the line
					if (skipword(line) != NULL)
						ScanOrder(line, 0);
				}
				// if no country, append current country
				else
				{
					strcpy(otherline, country2string(currentCountry));
					strcat(otherline, ": ");
					strcat(otherline, line);
					ScanOrder(otherline, 0);
				}
				
			}
		}
	}	
	
	if (found_dislodge && dislodges == 0)
	{
		// if already resolved, got a problem
		if (!gResolve && gCommit)
		{
			char string[256];
			GetIndCString(string, kSpecialStringsID, kNoDislodgeInfoID);
			ErrorString(string);
		}
		/* not much choice, clear resolutions out */
		for (i = 1; i <= NUM_COUN; i++)
		{
			for (order = startorder(orders[i]); isorder(order); nextorder(order))
			{
				if (orderfails(order))
				{
					setofails(order, R_NONE);
	       			order2string(line, order, false, false);
		   			SetOrderLine(line, oindex(order));
		   		}
		   	}
		}
		gResolve = true;
		gCommit = false;
	}	
	
}

void
ScanUnits(char *line, short filetype)
{
	char *word;
	static short oldcountry=0;
	short country=0, sector=0;
	CoastType coast=C_NONE;
	UnitType unit;
	
	word = skipspace(line);
	word = string2country(&country, word);
	if (country == 0)
	{
		country = oldcountry;
	}
	else
	{
		oldcountry = country;
	}	
	while (wordlen(word) != 0)
	{
		word = string2unit(&unit, word);
		if (unit == 0)
		{
			word = skipword(word);
			continue;
		}
		word = string2sector(&sector, word);
		if (sector == 0)
		{
			word = skipword(word);
			continue;
		}
		word = string2coast(&coast, word);
		
		// check to see if any order has "created" the unit
		bool dontAdd = false;
		if ( filetype != kStatusFile )
		{
			Order o;
			for (o = startorder(orders[country]); isorder(o); nextorder(o))
			{
				if (otype(o) == O_MOVE && ofinish(o) == sector && !orderfails(o))
				{
					dontAdd = true;
					break;
				}
				else if (otype(o) == O_BUILD && ostart(o) == sector)
				{
					dontAdd = true;
					break;
				}
			}	
		}
		
		// only add unit if not in dislodge list and not in units list
		Dislodge* dislodge = dslglist.Find( sector );
		if ( !dontAdd && ( dislodge == NULL || dislodge->Owner() != country )
			&& !inulist(units[country], unit, sector, C_NONE ) )
		{
			if (!addunit(&units[country], unit, sector, coast, country))
			{
				cleanunits(&units[country]);
				break;
			}
		}
	}
	
	unit_read = true;
	
}

void
ScanSupplies(char *line)
{
	char *word;
	static short oldcountry=0;
	short country=0, sector=0;
	
	word = skipspace(line);
	
	if (strnsame(word,"Unowned:", 8))
	{
		country = 0;
		word = skipword(word);
	}
	else
	{
		word = string2country(&country, word);
		if (country == 0)
			country = oldcountry;
		else
		{
			oldcountry = country;
			initslist(supplies[country]);
		}
	}	
		
	while (wordlen(word) != 0)
	{
		word = string2sector(&sector, word);
		if (sector == 0)
		{
			word = skipword(word);
			continue;
		}
		if (!addsupply(&supplies[country], sector, country))
		{
			cleansupplies(&supplies[country]);
			break;
		}
	}
	
	supp_read = true;
	
}


void
ScanOrder(char *line, short filetype)
{
	Order order;
	bool status;
	
	order = neworder();
	if (!(status = string2order(order, line)))
	{
		if (ofails(order) == R_RETREAT)
			ScanDislodge(line, filetype);
		
	}
	
	// if this order fails for reasons other than bad syntax (i.e. it's pre-resolved)
	if (ofails(order) && !obadsyntax(order))
	{
		gResolve = false;
		gCommit = true;
	}
	
	if (ofails(order) == R_DISLODGE || ofails(order) == R_BNCE_DIS 
		|| ofails(order) == R_NSO_DIS || ofails(order) == R_DISBAND
		|| ofails(order) == R_CUT_DIS)
		found_dislodge = true;
		
	if (status || ofails(order) == R_RETREAT)
	{
		if (gIgnoreUnits || filetype == kStatusFile)
		{
			if (!obadsyntax(order) && ounit(order) != 0 && otype(order) != O_REMOVE 
				&& otype(order) != O_DEF_REM && otype(order) != O_DISBAND 
				&& otype(order) != O_NMR_DIS 
				&& ((gIgnoreUnits && filetype != kStatusFile) || (otype(order) == O_NMR && ofails(order) != R_RETREAT)))
					addunit(&units[ocountry(order)], ounit(order), ostart(order), oscoast(order), ocountry(order));
			(void) appendorder(orders[ocountry(order)], order);
		}
		else
			(void) addorder(orders[ocountry(order)], order, false);
	}
	else
		(void) appendorder(orders[0], order);
}


void
ScanDislodge(char *line, short filetype)
{
	Dislodge* dislodge = new Dislodge();
	if (string2dislodge(dislodge, line) && dslglist.Add( dislodge ))
	{
		if (filetype == kStatusFile )
		{
			// delete from unit list, if there
			deleteunit(&units[dislodge->Owner()], dislodge->Sector());
		}
		dislodges = dslglist.Size();
	}
	else
		delete dislodge;
	dslg_read = true;
}

void
OutputOrders()
{
	char string[BUFSIZ];
	short c;
	Order o;
	
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!isorder(orders[c]))
			continue;
		strcpy(string, country2string(c));
		strcat(string, ": ");
		(void) AddOrderLine(string);
		for (o = startorder(orders[c]); isorder(o); nextorder(o))
		{
			order2string(string, o, false, false); 
			oindex(o) = AddOrderLine(string);
		}
		(void) AddOrderLine("");
	}				
}

void
OutputErrors()
{
	char string[BUFSIZ];
	Order o;
	
	ClearErrorList();
	if (!isorder(orders[0]))
		return;
	for (o = startorder(orders[0]); isorder(o); nextorder(o))
	{
		order2string(string, o, false, true); 
		oindex(o) = AddErrorLine(string);
	}
	ShowErrorWindow();
}


void OutputStatus()
{
	char string[4096], seasonstr[256];
	char *sp;
	short c, i, numunits;
	short stringlen, maxlen;
	char pstring[256];
	bool emr;
	char emr_text[18] = "(EM rule applies)";

	maxlen = namelen()+2;
	CleanCentersList();
	GetIndCString(pstring, kSpecialStringsID, kOwnershipID);
	AddCenterLine(pstring);
	AddCenterLine("");
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!issupply(supplies[c]))
			continue;
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
				strcat(string, " ");
		}
		sp = string + strlen(string);
		slist2string(sp, supplies[c]);
		AddCenterLine(string);
	}
	// unowned supply centers 
	if (issupply(supplies[0]))
	{
		GetIndCString(pstring, kSpecialStringsID, kUnownedID);
		strcpy(string, pstring);
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
			strcat(string, " ");
		sp = string + strlen(string);
		slist2string(sp, supplies[0]);
		AddCenterLine(string);
	}
	
	AddCenterLine("");
	GetIndCString(pstring, kSpecialStringsID, kAdjustmentsID);
	AddCenterLine(pstring);
	AddCenterLine("");
	for (c = 1; c <= NUM_COUN; c++)
	{
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
			strcat(string, " ");
		}
		sp = string+strlen(string);
		numunits = ulistsize(units[c]);
		for (Dislodge* current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next( current ))
		{
          	if (current->Owner() == c)
          	  	numunits++;
        }
		emr = (gHasEmergencyRule && ((onehome(c, supplies[c]) && !allhome(c, supplies[c])) != 0));
		if (countries[c].builds < 0)
		{
			if (gHasWings)
			{
				GetIndCString(pstring, kSpecialStringsID, kWingRemovingID);
				sprintf(sp, pstring,
				    slistsize(supplies[c]), 
					countblocked(supplies[c]), 
					numunits, -countries[c].builds, 
					(emr ? emr_text : ""));
			}
			else
			{
				GetIndCString(pstring, kSpecialStringsID, kRemovingID);
				sprintf(sp, pstring,  
				    slistsize(supplies[c]),
					numunits, 
					-countries[c].builds,
					(emr ? emr_text : ""));
			}
		}
		else
		{
			if (gHasWings)
			{
				GetIndCString(pstring, kSpecialStringsID, kWingBuildingID);
				sprintf(sp, pstring,
				    slistsize(supplies[c]), 
					countblocked(supplies[c]), 
					numunits, 
					countries[c].builds,
					(emr ? emr_text : ""));
			}
			else
			{
				GetIndCString(pstring, kSpecialStringsID, kBuildingID);
				sprintf(sp, pstring,
				    slistsize(supplies[c]),
					numunits, 
					countries[c].builds,
					(emr ? emr_text : ""));
			}
		}
				
		AddCenterLine(string);
	}
	
	CleanDislodgeList();
	for (Dislodge* current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next( current ))
	{
		dislodge2lstring(string, current);
		strcat(string, ".");
		AddDislodgeLine(string);
	}
	
	if (dislodges)
		GetIndCString(pstring, kSpecialStringsID, kExpectRetreatID);
	else if (adjustments)
		GetIndCString(pstring, kSpecialStringsID, kExpectBuildID);
	else
		GetIndCString(pstring, kSpecialStringsID, kExpectMoveID);	
	strcpy(string, pstring);

	if (season != S_NONE)
	{
		GetIndCString(pstring, kSpecialStringsID, kForID);
		strcat(string, pstring);
		GetIndCString(seasonstr, kSeasonStringsID, season); 
		strcat(string, seasonstr);
		strcat(string, " ");
		NumToString(year, (unsigned char*)seasonstr);
		PtoCstr((unsigned char *)seasonstr);
		strcat(string, seasonstr);
	}
	strcat(string, ".");
	SetExpecting(string);

}



#if NO_COUNTRY
/* doesn't assume that orderlist is empty */
/* doesn't display Country: Order */
void NextOrders()
{
	register short i, c;
	char string[256], pstring[256], *sp;
	Unit up;
	Order op;
	
	for (i = 0; i <= NUM_COUN; i++)
	{
		if (orders != NULL)
			cleanorders(orders[i]);
	} 
	CleanOrdersList();
	if (dislodges
		&& (season == S_SUMMER || season == S_AUTUMN)
		)
	{
		for (Dislodge* current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next( current ))
		{
			strcpy(string, country2string(current->Owner()));
			strcat(string, ": ");
			sp = string+strlen(string);
			unit2string(sp, (Unit)(current));
			op = neworder();
			otext(op) = new char[strlen(string)+1];
			strcpy(otext(op), string);
			ocountry(op) = current->Owner();
			ounit(op) = current->Unit();
			ostart(op) = current->Sector();
			oscoast(op) = current->Coast();
			otype(op) = O_NMR;
			ofails(op) = R_NONE;
			appendorder(orders[ocountry(op)], op); 
		}
		for (c = 1; c <= NUM_COUN; c++)
		{
			if (!isorder(startorder(orders[c])))
				continue;
			strcpy(string, country2string(c));
			strcat(string, ": ");
			(void) AddOrderLine(string);
			for (op = startorder(orders[c]); isorder(op); nextorder(op))
			{
				order2string(string, op, false, false);
				oindex(op) = AddOrderLine(string); 
			}
			(void) AddOrderLine("");
		}
	}
	else if (adjustments)
	{
		for (c = 1; c <= NUM_COUN; c++)
		{
			if (countries[c].builds == 0)
				continue;
			strcpy(string, country2string(c));
			strcat(string, ": ");
			(void) AddOrderLine(string);
			i = countries[c].builds;
			i = (i > 0)?i:-i;
			while (i--)
			{
				op = neworder();
				strcpy(string, country2string(c));
				strcat(string, ": ");
				if (countries[c].builds > 0)
				{
					GetIndCString(pstring, kMoveTypeStringsID, O_BUILD);
					strcat(string, pstring);
					otype(op) = O_BUILD;
				}
				else
				{
					GetIndCString(pstring, kMoveTypeStringsID, O_REMOVE);
					strcat(string, pstring);
					otype(op) = O_REMOVE;
				}
				otext(op) = new char[strlen(string)+1];
				strcpy(otext(op), string);
				ocountry(op) = c;
				ounit(op) = U_NONE;
				ostart(op) = 0;
				oscoast(op) = C_NONE;
				ofails(op) = R_NONE;
				order2string(string, op, false, false);
				oindex(op) = AddOrderLine(string); 
				appendorder(orders[ocountry(op)], op); 
			}
			(void) AddOrderLine("");
		}
	}
	else
	{	
		for (c = 1; c <= NUM_COUN; c++)
		{
			if (!isunit(units[c]))
				continue;
			strcpy(string, country2string(c));
			strcat(string, ": ");
			(void) AddOrderLine(string);
			for (up = startunit(units[c]); isunit(up); nextunit(up))
				appendnmr(c, utype(up), usector(up), ucoast(up));

			(void) AddOrderLine("");
		}
	}

}

#else
/* doesn't assume that orderlist is empty */
void NextOrders()
{
	register short i, c;
	char string[256], *sp;
	char pstring[256];
	Unit up;
	Order op;
	
	for (i = 0; i <= NUM_COUN; i++)
	{
		if (orders != NULL)
			cleanorders(orders[i]);
	} 
	CleanOrdersList();
	if (dislodges)
	{
		for (i = 0; i < MAX_DSLG; i++)
		{
			if (isdislodge(dslglist+i))
			{
				strcpy(string, country2string(down(dslglist+i)));
				strcat(string, ": ");
				sp = string+strlen(string);
				unit2string(sp, (Unit)(dslglist+i));
				op = neworder();
				otext(op) = (char *) NewPtr(strlen(string)+1);
				strcpy(otext(op), string);
				ocountry(op) = down(dslglist+i);
				ounit(op) = dunit(dslglist+i);
				ostart(op) = dsector(dslglist+i);
				oscoast(op) = dcoast(dslglist+i);
				otype(op) = O_NMR;
				ofails(op) = R_NONE;
				order2string(string, op, false, true);
				oindex(op) = AddOrderLine(string); 
				appendorder(orders[ocountry(op)], op); 
			}
		}
		(void) AddOrderLine("");
	}
	else if (adjustments)
	{
		for (c = 1; c <= NUM_COUN; c++)
		{
			if (countries[c].builds == 0)
				continue;
			i = countries[c].builds;
			i = (i > 0)?i:-i;
			while (i--)
			{
				op = neworder();
				strcpy(string, country2string(c));
				strcat(string, ": ");
				if (countries[c].builds > 0)
				{
					GetIndCString(pstring, kMoveTypeStringsID, O_BUILD);
					strcat(string, pstring);
					otype(op) = O_BUILD;
				}
				else
				{
					GetIndCString(pstring, kMoveTypeStringsID, O_REMOVE);
					strcat(string, pstring);
					otype(op) = O_REMOVE;
				}
				otext(op) = (char *) NewPtr(strlen(string)+1);
				strcpy(otext(op), string);
				ocountry(op) = c;
				ounit(op) = U_NONE;
				ostart(op) = 0;
				oscoast(op) = C_NONE;
				ofails(op) = R_NONE;
				order2string(string, op, false, true);
				oindex(op) = AddOrderLine(string); 
				appendorder(orders[ocountry(op)], op); 
			}
			(void) AddOrderLine("");
		}
	}
	else
	{	
		for (c = 1; c <= NUM_COUN; c++)
		{
			if (!isunit(units[c]))
				continue;
			for (up = startunit(units[c]); isunit(up); nextunit(up))
			{
				strcpy(string, country2string(c));
				strcat(string, ": ");
				sp = string + strlen(string);
				unit2string(sp, up);
				op = neworder();
				otext(op) = (char *) NewPtr(strlen(string)+1);
				strcpy(otext(op), string);
				ocountry(op) = c;
				ounit(op) = utype(up);
				ostart(op) = usector(up);
				oscoast(op) = ucoast(up);
				otype(op) = O_NMR;
				ofails(op) = R_NONE;
				lorder2string(string, op, false, true);
				oindex(op) = AddOrderLine(string); 
				appendorder(orders[ocountry(op)], op); 
			}
			(void) AddOrderLine("");
		}
	}

}
#endif

bool 
SaveReport()
{
	char prompt[256], defaultName[256];
	FSSpec filename;
	TextFile file;
	bool	writeStatus = false;
	
	// can't save results if orders aren't resolved
	if (history.AtEnd() && !gCommit)
	{
		writeStatus = true;
	}

	// set up the dialog
	GetIndCString(prompt, kSpecialStringsID, kSaveGameStringID);
	strcpy(defaultName, gamename);
	strcat(defaultName, ".txt");
	
	// get the save file name
	if (!PutFile(filename, prompt, defaultName, textFilter))
		return false;
		
	// open the file as non-game file
	if (!file.Open(filename, true, false))
	{
		file.Close();
		return false;
	}
	
	bool result;
	short numunits[MAX_COUN+1], numsupplies[MAX_COUN+1], 
				numlands[MAX_COUN+1], numbuilds[MAX_COUN+1];
	if (writeStatus)
	{
		for (int c = 1; c <= NUM_COUN; ++c)
		{
			numunits[c] = ulistsize(units[c]);
			numsupplies[c] = ulistsize(supplies[c]);
			numlands[c] = ulistsize(lands[c]);
			numbuilds[c] = countries[c].builds;
		}	
		result = WriteResults( file, season, year, gamename, orders, 
								units, supplies, numunits, numsupplies, 
								dislodges, dslglist, adjustments, 
								numbuilds, true );
	}
	else if (!history.AtEnd())
	{
		result = history.WriteResults( file, gamename );
	}
	else
	{
		// simulate next phase
		UnitList newunits[MAX_COUN+1];
		SupplyList newsupplies[MAX_COUN+1];
		SupplyList newlands[MAX_COUN+1];
		DislodgeList newdslglist;
		short newdislodges = 0;
		short newadjustments = 0;
		sim_next_phase(newunits, newsupplies, newlands, numunits, numsupplies, numlands,
				   newdislodges, newdslglist, newadjustments, numbuilds, 
				   (season == S_SPRING || season == S_FALL));
		
		result = WriteResults( file, season, year, gamename, orders, 
								newunits, newsupplies, 
								numunits, numsupplies,
								newdislodges, newdslglist, newadjustments, 
								numbuilds );
		// cleanup
		newdslglist.Clean();
		for (int c = 0; c <= NUM_COUN; ++c)
		{
			// delete units and supplies
			cleanunits(&newunits[c]);
			cleansupplies(&newsupplies[c]);
			cleansupplies(&newlands[c]);
		}	
	}
				   
	// done
	file.Close();

	return result;
}

bool WriteResults( TextFile& file, short season, short year, char* name, 
				   Orderlist neworders[], UnitList newunits[], 
				   SupplyList newsupplies[], short numunits[], short numsupplies[], 
					short& newdislodges, const DislodgeList& newdslglist, short& newadjustments, 
					short numbuilds[], bool status )
{
	// write out header
	char string[BUFSIZ];
	char seasonstr[BUFSIZ];
	char pstring[256];
	char *sp;
	short c, i, unitcount, bc;
	short stringlen, maxlen;

	maxlen = namelen() + 2;
	memset(string, 0, sizeof(string));
	if ( status )
	{
		strcpy(string, "Status of the ");
	}
	if (season == S_SUMMER || season == S_AUTUMN)
		strcat(string, "Retreat orders for ");
	else if (season == S_WINTER)
		strcat(string, "Adjustment orders for ");
	else
		strcat(string, "Movement results for ");
	if (season != S_NONE)
	{
		short reportSeason = season;
		if (reportSeason == S_AUTUMN)
			reportSeason = S_FALL;
		else if (reportSeason == S_SUMMER)
			reportSeason = S_SPRING;
		GetIndCString(seasonstr, kSeasonStringsID, reportSeason); 
		strcat(string, seasonstr);
		strcat(string, " of ");
		NumToString(year, (unsigned char*)seasonstr);
		PtoCstr((unsigned char *)seasonstr);
		strcat(string, seasonstr);
	}
	strcat(string, ".  (");	
	strcat(string, name);
	strcat(string, ")");	
	(void) file.WriteLine(string);
	(void) file.WriteLine("");

	// write out orders
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!isorder(neworders[c]))
			continue;
		Order o;
		// write out all orders
		for (o = startorder(neworders[c]); isorder(o); nextorder(o))
		{
			strcpy(string, country2string(c));
			strcat(string, ": ");
			order2string(string+strlen(string), o, false, false);
			strcat(string, "."); 
			file.WriteLine(string, 80, maxlen);
		}
		(void) file.WriteLine("");
	}				
	
	// write out dislodges
	if (newdislodges > 0)
	{
		(void) file.WriteLine("The following units were dislodged:");
		(void) file.WriteLine("");
		int count = 0;
		for (Dislodge* current = newdslglist.Start(); current != newdslglist.End(); current = newdslglist.Next( current ))
		{
			count++;
			dislodge2lstring(string, current);
			strcat(string, ".");
			(void) file.WriteLine(string);
		}
		(void) file.WriteLine("");	
	}

	// units
	GetIndCString(pstring, kSpecialStringsID, kOwnershipID);
	(void) file.WriteLine("Unit locations:");
	(void) file.WriteLine("");
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!issupply(newunits[c]))
			continue;
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
			strcat(string, " ");
		}
		sp = string + strlen(string);
		ulist2string(sp, newunits[c]);
        sp = string + strlen(string);
        *sp++ = '.';
        *sp = '\0';
		
		// include dislodged units if status or spring or fall
		if ( status || season == S_SPRING || season == S_FALL )
		{
			if ( newdislodges )
			{
				for (Dislodge* current = newdslglist.Start(); current != newdslglist.End(); current = newdslglist.Next( current ))
    	 	    {
          			if (current->Owner() == c
						&& !inulist(units[c], current->Unit(), current->Sector(),
								current->Coast()) )
          			{
          				sp = string + strlen(string) - 1;
		          		*sp++ = ',';
		          		*sp++ = ' ';
 		         		strcpy(sp, sunit2string(current->Unit()));
		          		sp = string + strlen(string);
		          		*sp++ = ' ';
		          		strcpy(sp, lreg2string(current->Sector()));
 						strcat(sp, scoast2string(current->Coast()));
		         		sp = string + strlen(string);
 		         		*sp++ = '.';
 	 	        		*sp = '\0';
 	 	        	}
  	        	}
          	}
        }
		
		(void) file.WriteLine(string, 80, maxlen);
	}
	(void) file.WriteLine("");
	
//	if (newadjustments > 0 && newdislodges == 0)
	{
	// write out supply centers
	GetIndCString(pstring, kSpecialStringsID, kOwnershipID);
	(void) file.WriteLine("Ownership of supply centers:");
	(void) file.WriteLine("");
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!issupply(newsupplies[c]))
			continue;
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
			strcat(string, " ");
		}
		sp = string + strlen(string);
		slist2string(sp, newsupplies[c]);
		(void) file.WriteLine(string, 80, maxlen);
	}
	// unowned supply centers 
	if (issupply(newsupplies[0]))
	{
		GetIndCString(pstring, kSpecialStringsID, kUnownedID);
		strcpy(string, pstring);
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
			strcat(string, " ");
		sp = string + strlen(string);
		slist2string(sp, newsupplies[0]);
		(void) file.WriteLine(string, 80, maxlen);
	}
	(void) file.WriteLine("");
	
	// adjustments
	for (c = 1; c <= NUM_COUN; c++)
	{
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
			strcat(string, " ");
		}
		sp = string+strlen(string);
		unitcount = numunits[c];
		if (newdislodges)
		{
			for (Dislodge* current = newdslglist.Start(); current != newdslglist.End(); current = newdslglist.Next( current ))
          		if (current->Owner() == c
					&& !inulist(units[c], current->Unit(), current->Sector(),
								current->Coast()) )
          	  		unitcount++;
		}
        sprintf(sp, "%2d Supply center", numsupplies[c]);
        sp = string+strlen(string);
        if (numsupplies[c] == 1)
       	{
        	sprintf(sp, " ");
        }
        else
        {
         	sprintf(sp, "s");
        }
		
		sp = string+strlen(string);

		if (gHasWings) 
       	{
			bc = countblocked(newsupplies[c]);
        	sprintf(sp, " (%2d blocked), ", bc);
        }
        else
        {
         	sprintf(sp, ", ");
        }

		sp = string+strlen(string);
		sprintf(sp, " %2d Unit", unitcount);
        
		if (unitcount == 1)
	  	{
	  		strcat(sp, ":   ");
	  	}
	  	else
	  	{
	  		strcat(sp, "s:  ");
	  	}
        
		sp = string+strlen(string);

		if (numbuilds[c] < 0)
		{
			sprintf(sp, "Removes %2d unit", -numbuilds[c]);
		}
		else
		{
			sprintf(sp, "Builds  %2d unit", numbuilds[c]);
		}
		if (numbuilds[c] == 1 || numbuilds[c] == -1)
		{
			strcat(sp, ".");
		}
		else
		{
			strcat(sp, "s.");
		}		
		(void) file.WriteLine(string);
	}
	(void) file.WriteLine("");
	}

	// write out moves expected
	if (!status)
	{
		strcpy(string, "The next phase of ");
		strcat(string, gamename);
		if (newdislodges)
			strcat(string, " will be Retreats for ");
		else if (newadjustments)
			strcat(string, " will be Adjustments for ");
		else
			strcat(string, " will be Movement for ");

		if (season != S_NONE)
		{
			short nextSeason = season, nextYear = year;
			switch (season)
			{
			case S_SPRING:
				if (newdislodges)
					nextSeason = S_SPRING;
				else
					nextSeason = S_FALL;
				break;
		
			case S_SUMMER:
				nextSeason = S_FALL;
				break;
		
			case S_FALL:
				if (newdislodges)
					nextSeason = S_FALL;
				else if (newadjustments)
					nextSeason = S_WINTER;
				else
				{
					nextSeason = S_SPRING;
					nextYear++;
				}
				break;
		
			case S_AUTUMN:
				if (newadjustments)
					nextSeason = S_WINTER;
				else
				{
					nextSeason = S_SPRING;
					nextYear++;
				}
				break;
		
			case S_WINTER:
				nextSeason = S_SPRING;
				nextYear++;
				break;
			}
			GetIndCString(seasonstr, kSeasonStringsID, nextSeason); 
			strcat(string, seasonstr);
			strcat(string, " of ");
			NumToString(nextYear, (unsigned char*)seasonstr);
			PtoCstr((unsigned char *)seasonstr);
			strcat(string, seasonstr);
		}
		strcat(string, ".");
		(void) file.WriteLine(string);
	}
	
	return true;
}

bool 
SaveReportCSV()
{
	char prompt[256], defaultName[256];
	FSSpec filename;
	TextFile file;
	bool	writeStatus = false;
	
	// can't save results if orders aren't resolved
	if (history.AtEnd() && !gCommit)
	{
		writeStatus = true;
	}

	// set up the dialog
	GetIndCString(prompt, kSpecialStringsID, kSaveGameStringID);
	strcpy(defaultName, gamename);
	strcat(defaultName, "_");
	NumToString(year, (unsigned char*)defaultName);
	PtoCstr((unsigned char *)defaultName);
	strcat(defaultName, "_");
	NumToString(season, (unsigned char*)defaultName);
	PtoCstr((unsigned char *)defaultName);
	strcat(defaultName, ".csv");
	
	// get the save file name
	if (!PutFile(filename, prompt, defaultName, csvFilter))
		return false;
		
	// open the file as non-game file
	if (!file.Open(filename, true, false))
	{
		file.Close();
		return false;
	}
	
	bool result;
	short numunits[MAX_COUN+1], numsupplies[MAX_COUN+1], 
				numlands[MAX_COUN+1], numbuilds[MAX_COUN+1];
	if (writeStatus)
	{
		for (int c = 1; c <= NUM_COUN; ++c)
		{
			numunits[c] = ulistsize(units[c]);
			numsupplies[c] = ulistsize(supplies[c]);
			numlands[c] = ulistsize(lands[c]);
			numbuilds[c] = countries[c].builds;
		}	
		result = WriteResultsCSV( file, season, year, gamename, orders, 
								units, supplies, numunits, numsupplies, 
								dislodges, dslglist, adjustments, 
								numbuilds, true );
	}
	else if (!history.AtEnd())
	{
		result = history.WriteResults( file, gamename );
	}
	else
	{
		// simulate next phase
		UnitList newunits[MAX_COUN+1];
		SupplyList newsupplies[MAX_COUN+1];
		SupplyList newlands[MAX_COUN+1];
		DislodgeList newdslglist;
		short newdislodges = 0;
		short newadjustments = 0;
		sim_next_phase(newunits, newsupplies, newlands, numunits, numsupplies, numlands,
				   newdislodges, newdslglist, newadjustments, numbuilds, 
				   (season == S_SPRING || season == S_FALL));
		
		result = WriteResultsCSV( file, season, year, gamename, orders, 
								newunits, newsupplies, 
								numunits, numsupplies,
								newdislodges, newdslglist, newadjustments, 
								numbuilds );
		// cleanup
		newdslglist.Clean();
		for (int c = 0; c <= NUM_COUN; ++c)
		{
			// delete units and supplies
			cleanunits(&newunits[c]);
			cleansupplies(&newsupplies[c]);
			cleansupplies(&newlands[c]);
		}	
	}
				   
	// done
	file.Close();

	return result;
}

bool WriteResultsCSV( TextFile& file, short season, short year, char* name, 
				   Orderlist neworders[], UnitList newunits[], 
				   SupplyList newsupplies[], short numunits[], short numsupplies[], 
					short& newdislodges, const DislodgeList& newdslglist, short& newadjustments, 
					short numbuilds[], bool status )
{
	// write out header
	char string[BUFSIZ];
	char seasonstr[BUFSIZ];
	char pstring[256];
	char *sp;
	short c, i, unitcount, bc;
	short stringlen, maxlen;

	maxlen = namelen() + 2;
	memset(string, 0, sizeof(string));
	if ( status )
	{
		strcpy(string, "status");
	}
	else
	{
		strcpy(string, "results");
	}
	(void) file.WriteLine(string);
	
	strcpy(string, name);
	if (season != S_NONE)
	{
		strcat(string, ",");
		NumToString(year, (unsigned char*)seasonstr);
		PtoCstr((unsigned char *)seasonstr);
		strcat(string, seasonstr);
		strcat(string, ",");
		short reportSeason = season;
		if (reportSeason == S_AUTUMN)
			reportSeason = S_FALL;
		else if (reportSeason == S_SUMMER)
			reportSeason = S_SPRING;
		GetIndCString(seasonstr, kSeasonStringsID, reportSeason); 
		strcat(string, seasonstr);
	}
	(void) file.WriteLine(string);

	if (season == S_SUMMER || season == S_AUTUMN)
		strcat(string, "retreats");
	else if (season == S_WINTER)
		strcat(string, "adjustments");
	else
		strcat(string, "movements");
	(void) file.WriteLine(string);
	(void) file.WriteLine("");

	// write out orders
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!isorder(neworders[c]))
			continue;
		Order o;
		// write out all orders
		for (o = startorder(neworders[c]); isorder(o); nextorder(o))
		{
			strcpy(string, country2string(c));
			strcat(string, ",");
			order2stringCSV(string+strlen(string), o, false, false);
			file.WriteLine(string);
		}
		(void) file.WriteLine("");
	}				
	
	// write out dislodges
	if (newdislodges > 0)
	{
		(void) file.WriteLine("dislodges");
		int count = 0;
		for (Dislodge* current = newdslglist.Start(); current != newdslglist.End(); current = newdslglist.Next( current ))
		{
			count++;
			dislodge2CSV(string, current);
			(void) file.WriteLine(string);
		}
	}

	// units
	GetIndCString(pstring, kSpecialStringsID, kOwnershipID);
	(void) file.WriteLine("units");
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!issupply(newunits[c]))
			continue;
		strcpy(string, country2string(c));
		strcat(string, ",");
		stringlen = strlen(string);
		sp = string + strlen(string);
		UnitList lp = newunits[c];
		unit2string(sp, lp, true);
		(void) file.WriteLine(string);
		lp = lp->next;
		while (lp != NULL)
		{
			strcpy(string, country2string(c));
			strcat(string, ",");
			stringlen = strlen(string);
			sp = string + strlen(string);	
			unit2string(sp, lp, true);
			(void) file.WriteLine(string);
			lp = lp->next;
		}
        
		// include dislodged units if status or spring or fall
		if ( status || season == S_SPRING || season == S_FALL )
		{
			if ( newdislodges )
			{
				for (Dislodge* current = newdslglist.Start(); current != newdslglist.End(); current = newdslglist.Next( current ))
    	 	    {
          			if (current->Owner() == c
						&& !inulist(units[c], current->Unit(), current->Sector(),
								current->Coast()) )
          			{
          				sp = string + strlen(string) - 1;
		          		*sp++ = ',';
		          		*sp++ = ' ';
 		         		strcpy(sp, sunit2string(current->Unit()));
		          		sp = string + strlen(string);
		          		*sp++ = ' ';
		          		strcpy(sp, lreg2string(current->Sector()));
 						strcat(sp, scoast2string(current->Coast()));
		         		sp = string + strlen(string);
 		         		*sp++ = '.';
 	 	        		*sp = '\0';
 	 	        	}
  	        	}
          	}
        }
		
		(void) file.WriteLine(string, 80, maxlen);
	}
	(void) file.WriteLine("");
	
//	if (newadjustments > 0 && newdislodges == 0)
	{
	// write out supply centers
	GetIndCString(pstring, kSpecialStringsID, kOwnershipID);
	(void) file.WriteLine("Ownership of supply centers:");
	(void) file.WriteLine("");
	for (c = 1; c <= NUM_COUN; c++)
	{
		if (!issupply(newsupplies[c]))
			continue;
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
			strcat(string, " ");
		}
		sp = string + strlen(string);
		slist2string(sp, newsupplies[c]);
		(void) file.WriteLine(string, 80, maxlen);
	}
	// unowned supply centers 
	if (issupply(newsupplies[0]))
	{
		GetIndCString(pstring, kSpecialStringsID, kUnownedID);
		strcpy(string, pstring);
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
			strcat(string, " ");
		sp = string + strlen(string);
		slist2string(sp, newsupplies[0]);
		(void) file.WriteLine(string, 80, maxlen);
	}
	(void) file.WriteLine("");
	
	// adjustments
	for (c = 1; c <= NUM_COUN; c++)
	{
		strcpy(string, country2string(c));
		strcat(string, ": ");
		stringlen = strlen(string);
		for (i = 0; i < (maxlen-stringlen); i++)
		{
			strcat(string, " ");
		}
		sp = string+strlen(string);
		unitcount = numunits[c];
		if (newdislodges)
		{
			for (Dislodge* current = newdslglist.Start(); current != newdslglist.End(); current = newdslglist.Next( current ))
          		if (current->Owner() == c
					&& !inulist(units[c], current->Unit(), current->Sector(),
								current->Coast()) )
          	  		unitcount++;
		}
        sprintf(sp, "%2d Supply center", numsupplies[c]);
        sp = string+strlen(string);
        if (numsupplies[c] == 1)
       	{
        	sprintf(sp, " ");
        }
        else
        {
         	sprintf(sp, "s");
        }
		
		sp = string+strlen(string);

		if (gHasWings) 
       	{
			bc = countblocked(newsupplies[c]);
        	sprintf(sp, " (%2d blocked), ", bc);
        }
        else
        {
         	sprintf(sp, ", ");
        }

		sp = string+strlen(string);
		sprintf(sp, " %2d Unit", unitcount);
        
		if (unitcount == 1)
	  	{
	  		strcat(sp, ":   ");
	  	}
	  	else
	  	{
	  		strcat(sp, "s:  ");
	  	}
        
		sp = string+strlen(string);

		if (numbuilds[c] < 0)
		{
			sprintf(sp, "Removes %2d unit", -numbuilds[c]);
		}
		else
		{
			sprintf(sp, "Builds  %2d unit", numbuilds[c]);
		}
		if (numbuilds[c] == 1 || numbuilds[c] == -1)
		{
			strcat(sp, ".");
		}
		else
		{
			strcat(sp, "s.");
		}		
		(void) file.WriteLine(string);
	}
	(void) file.WriteLine("");
	}

	// write out moves expected
	if (!status)
	{
		strcpy(string, "The next phase of ");
		strcat(string, gamename);
		if (newdislodges)
			strcat(string, " will be Retreats for ");
		else if (newadjustments)
			strcat(string, " will be Adjustments for ");
		else
			strcat(string, " will be Movement for ");

		if (season != S_NONE)
		{
			short nextSeason = season, nextYear = year;
			switch (season)
			{
			case S_SPRING:
				if (newdislodges)
					nextSeason = S_SPRING;
				else
					nextSeason = S_FALL;
				break;
		
			case S_SUMMER:
				nextSeason = S_FALL;
				break;
		
			case S_FALL:
				if (newdislodges)
					nextSeason = S_FALL;
				else if (newadjustments)
					nextSeason = S_WINTER;
				else
				{
					nextSeason = S_SPRING;
					nextYear++;
				}
				break;
		
			case S_AUTUMN:
				if (newadjustments)
					nextSeason = S_WINTER;
				else
				{
					nextSeason = S_SPRING;
					nextYear++;
				}
				break;
		
			case S_WINTER:
				nextSeason = S_SPRING;
				nextYear++;
				break;
			}
			GetIndCString(seasonstr, kSeasonStringsID, nextSeason); 
			strcat(string, seasonstr);
			strcat(string, " of ");
			NumToString(nextYear, (unsigned char*)seasonstr);
			PtoCstr((unsigned char *)seasonstr);
			strcat(string, seasonstr);
		}
		strcat(string, ".");
		(void) file.WriteLine(string);
	}
	
	return true;
}

char *
getsline(char *s, int size, TextFile& fp, bool order_line)
{
	char *sp;
	bool eol=true, space=false;
	
	sp = (char*) s;
	
	while (size--)
	{
		if (!fp.ReadChar(*sp))
			break;

		switch (*sp)
		{
		case '.':
			space = false;
			/* catch the case of "St.\r" -- not end of line */
			eol = !(*(sp-2) == 'S' && *(sp-1) == 't');
			sp++;
			break;
						
		case ')':
			space = false;
			/* end of line only if "*)" */
			eol = (*(sp-1) == '*');
			sp++;
			break;

		case '\r':
#ifdef WIN32
			break;
#endif
		case '\n':
			if (eol)
				goto out;
			else
			{
				eol = true;
				space = true;
				if (!order_line)
					*sp++ = ' '; 
			}
			break;
			
		case ' ':
			if (!space)
			{
				sp++;
				space = true;
			}
			break;
			
		default:
			eol = false;
			space = false;
			sp++;
			break;
		}
	}
out:
	*sp = '\0';
	return s;
	
}
