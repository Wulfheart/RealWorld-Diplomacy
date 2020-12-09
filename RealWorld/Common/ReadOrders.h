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
#ifndef _ReadOrdersDefs_
#define _ReadOrdersDefs_

#include "MacTypes.h"
#include "Order.h"

class TextFile;

bool LoadOrders(const FSSpec* filename);
bool LoadGameOrders(TextFile& fp);
void ResolveOrders();
void UndoResolve();
void CommitOrders();
void OutputOrders();
void OutputStatus();
void NextOrders();
void PasteOrders();
bool WriteGameOrders(TextFile& fp);
bool SaveReport();
bool WriteResults( TextFile& file, short season, short year, char* gamename, 
				   Orderlist neworders[], UnitList newunits[], SupplyList newsupplies[], 
					short numunits[], short numsupplies[], 
					short& newdislodges, const DislodgeList& newdslglist, short& newadjustments, 
					short numbuilds[], bool status = false );
bool SaveReportCSV();
bool WriteResultsCSV( TextFile& file, short season, short year, char* gamename, 
				   Orderlist neworders[], UnitList newunits[], SupplyList newsupplies[], 
					short numunits[], short numsupplies[], 
					short& newdislodges, const DislodgeList& newdslglist, short& newadjustments, 
					short numbuilds[], bool status = false );

bool PreviousSeason();
bool NextSeason();

#endif
