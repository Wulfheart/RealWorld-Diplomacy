//===============================================================================
// @ Strings.cpp
// ------------------------------------------------------------------------------
// String table access.
//
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
//
// Change history:
//
// 99-Dec-22	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <assert.h>
#include <string.h>
#include "Strings.h"

//-------------------------------------------------------------------------------
//-- Constants ------------------------------------------------------------------
//-------------------------------------------------------------------------------

const char *kSeasonStrings[] = { "None", "Spring", "Summer", "Fall", "Autumn", "Winter" };
const char *kCoastStrings[] = {"(sc)", "(nc)", "(ec)", "(wc)"};

const char *kMoveTypeStrings[] = 
{	
	"None", 
	" -", 
	" Supports", 
	" Hold", 
	" Convoys", 
	"Build", 
	"Remove", 
	"builds pending",
	"Build waived", 
	"Defaults, removing", 
	"disband", 
	", no move received", 
	", no move received, disbanding",
	"retreat", 
	"stat", 
	" Trafo"
};

const char *kErrorStrings[] =
{
	"No error",
	"Bad ROMs",
	"Old system",
	"Bad system",
	"No menu bar",
	"Bad dialog",
	"Bad game data",
	"Bad map data",
	"Bad country data",
	"Bad image",
	"Out of memory",
	"Error reading file",
	"Can't find string",
	"Bad menu",
	"Printing error",
	"No TEXT element in scrap",
	"Can't find temp file volume",
	"Can't create temp file",
	"Can't open temp file",
	"Can't write to temp file",
	"Bad polygon data",
	"Bad variant data",
	"Bad history data",
	"Unexpected error"
};

const char* kSpecialStrings[] =
{
	"Foobar",
	"Save Game",
	"Show Map",
	"Hide Map",
	"Show Orders",
	"Hide Orders",
	"PrintStringID",
	"No further orders accepted.",
	"Ownership:",
	"Unowned:",
	"Adjustments:",
	"Supp %2d Unit %2d Remove %2d %s",
	"Supp %2d Unit %2d Build %2d %s",
	"Expecting retreats ",
	"Expecting builds ",
	"Expecting moves ",
	"for ",
	"Editing...",
	"Fleet",
	"Map",
	"Realpolitik",
	"&Undo\tCtrl+Z",
	"&Undo Resolve\tCtrl+Z",
	"&Undo Order\tCtrl+Z",
	"Invalid Orders",
	"Army",
	"Save Map",
	"This will clear all following history.  Are you sure you want to do this?",
	"This will clear any existing orders.  Are you sure you want to do this?",
	"These orders haven't been resolved yet.  Please resolve or save results for a previous phase.",
	"This will overwrite resolved orders.  Are you sure you want to do this?",
	"Save Results...",
	"Save Status...",
	"No dislodge info found -- removing resolution.",
	"Supp %2d (Blocked %2d) Unit %2d Remove %2d %s",
	"Supp %2d (Blocked %2d) Unit %2d Build %2d %s"
};


const char* kDislodgeStrings[] =
{
	" can retreat to ",
	" no moves received, disbanded",
	" no retreat received, disbanded",
	" given illegal retreat, disbanded",
	" has no retreats, disbanded"
};

const char *kPatternNames[] = 
{
	"Random", 
	"ThinDiag",
	"Hash", 
	"Gray", 
	"Quilt", 
	"Sparse", 
	"Diag", 
	"ThinHoriz", 
	"Horiz", 
	"Vert", 
	"Stripe",
	"Flood",
	"None",
	""
};

const char *kColorNames[] = 
{
	"Black",
	"Blue",
	"Brown",
	"Cyan",
	"Forest",
	"Green",
	"Magenta",
	"Purple",
	"Red",
	"White",
	"Yellow",
	"Crimson",
	"Navy",
	"Teal",
	"Olive",
	"Charcoal",
	"Orange",
	"Tan",
	"Amethyst",
	"Gold",
	"Beige", 
	"Pink",
	"Khaki",
	"Periwinkle",
	"Aquamarine",
	"Cornflower",
	"Slate",
	"Gray",
	"KellyGreen",
	"Chartreuse",
	"Cobalt",
	"Brass",
	"Azure",
	"DarkPurple",
	"Bronze",
	"DarkTeal",
	"Indigo",
	"Blood",
	"Sienna",
	"Tomato",
	"Turquoise",
	"Rust",
	"Blocked",
	""
};

const char *kTranslateColorNames[24][2] = 
{
	{"Australian", "Magenta"},
	{"Brazilian", "KellyGreen"},
	{"Canadian", "Chartreuse"},
	{"Chinese", "Cornflower"},
	{"Colombian", "Cobalt"},
	{"Egyptian", "Pink"},
	{"English", "Brass"},
	{"French", "Azure"},
	{"German", "Slate"},
	{"Indian", "Forest"},
	{"Indochinese", "DarkPurple"},
	{"Indonesian", "Bronze"},
	{"Italian", "DarkTeal"},
	{"Ivorian", "Khaki"},
	{"Japanese", "Yellow"},
	{"Mexican", "Crimson"},
	{"Mongolian", "Indigo"},
	{"Persian", "Gray"},
	{"Russian", "Blood"},
	{"African", "Sienna"},
	{"Turkish", "Tomato"},
	{"Islandish", "Turquoise"},
	{"American", "Rust"},
	{"", ""}
};

//-------------------------------------------------------------------------------
//-- Functions ------------------------------------------------------------------
//-------------------------------------------------------------------------------


void GetIndCString(char *string, int typeID, int stringID)
{
	switch (typeID)
	{
	case kMoveTypeStringsID:
		strcpy(string, kMoveTypeStrings[stringID]);
		break;

	case kErrorStringsID:
		strcpy(string, kErrorStrings[stringID]);
		break;

	case kSpecialStringsID:
		strcpy(string, kSpecialStrings[stringID]);
		break;

	case kSeasonStringsID:
		strcpy(string, kSeasonStrings[stringID]);
		break;

	case kDislodgeStringsID:
		strcpy(string, kDislodgeStrings[stringID]);
		break;
		
	case kPatternStringsID:
		strcpy(string, kPatternNames[stringID]);
		break;

	case kColorStringsID:
		strcpy(string, kColorNames[stringID]);
		break;

	default:
		assert(false);
		break;
	}
}

void SubstituteCName(char *string)
{
	short j;
	j = 0;

	while (strlen(kTranslateColorNames[j][0]) != 0)
	{ 
            if (strcmp(string, kTranslateColorNames[j][0]) == 0)
            {
                    strcpy_s(string, 256, kTranslateColorNames[j][1]);
                    break;
            }
            ++j;
    }
}

void SubstituteOldCName(char *string)
{
	short j;
	j = 0;

	while (strlen(kTranslateColorNames[j][1]) != 0)
	{ 
            if (strcmp(string, kTranslateColorNames[j][1]) == 0)
            {
                    strcpy_s(string, 256, kTranslateColorNames[j][0]);
                    break;
            }
            ++j;
    }
}
