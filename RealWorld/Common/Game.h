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
#ifndef __GameDefs__
#define __GameDefs__

#include "Unit.h"
#include "MacTypes.h"

#define S_NONE 		0
#define S_SPRING	1
#define S_SUMMER	2
#define S_FALL		3
#define S_AUTUMN	4
#define S_WINTER	5
#define S_MAX 		6

extern bool gGameRunning;
extern bool gSaveDislodge;
extern bool gIgnoreUnits;
extern bool gAddUnitToOrder;
extern bool gStrictNMR;
extern bool gResolve;
extern bool gCommit;
extern bool gPreview;
/* used to determine if simulation or the real thing */
extern bool gSzykman; 
/* use the rule of Simon Szykman to avoid convoy paradoxa */
extern bool gIgnoreConvoyRoutes;

extern char gamename[];
extern short season;
extern short year;
extern short adjustments;

extern short winner;

bool NewGame(void);
bool OpenGame(void);
bool OpenFinderGame(const FSSpec& fName);
bool CloseGame(void);
bool SaveGame(void);
bool SaveGameAs(void);
bool SaveGameAsRP(void);
bool RevertGame(void);
bool QuitGame(void);

short string2season(char *gPointer);
void nextseason(void);

void SetGameChanged(bool isChanged);	
bool IsGameChanged(void);



#endif
