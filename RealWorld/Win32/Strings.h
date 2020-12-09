//===============================================================================
// @ Strings.h
// ------------------------------------------------------------------------------
// String table access.
//
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
//
// Change history:
//
// 99-Dec-22	JMV	First version
//
//===============================================================================
#ifndef _STRINGSH_
#define _STRINGSH_

void GetIndCString(char *string, int typeID, int stringID);
void SubstituteCName(char *string);
void SubstituteOldCName(char *string);

// String resources
#define kErrorStringsID		2000

#define kSeasonStringsID    7000

#define kSpecialStringsID   3000
#define kSaveGameStringID	1
#define kShowMapStringID	2
#define kHideMapStringID	3
#define kShowOrdersStringID 4
#define kHideOrdersStringID	5
#define kPrintStringID		6
#define kNoMoreOrdersID		7
#define kOwnershipID		8
#define kUnownedID			9
#define kAdjustmentsID		10
#define kRemovingID			11
#define kBuildingID			12
#define kExpectRetreatID	13
#define kExpectBuildID		14
#define kExpectMoveID		15
#define kForID				16
#define kEditingID			17
#define kFleetID			18
#define kDefaultMapTitleID	19
#define kDefaultGameNameID	20
#define kUndoID				21
#define kUndoResolveID		22
#define kUndoOrderID		23
#define kErrorWindowTitleID	24
#define kArmyID				25
#define kSaveMapStringID	26
#define kBranchStringID		27
#define kPreviousStringID	28
#define kNoResolvedStringID	29
#define kOverResolvStringID	30
#define kSaveResultsID		31
#define kSaveStatusID		32
#define kNoDislodgeInfoID	33
#define kWingRemovingID		34
#define kWingBuildingID		35

#define kResolutionStringsID	132

#define kMoveTypeStringsID	1000

#define kCoastStringsID		8000

#define kDislodgeStringsID	4000

#define kPatternStringsID	5000

#define kColorStringsID		6000


#endif