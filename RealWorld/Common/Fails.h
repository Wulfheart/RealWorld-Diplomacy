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
#ifndef _FailsDefs_
#define _FailsDefs_

#define R_NONE     0
#define R_SUCCESS  1
#define R_FAILS    2
#define R_DISLODGE 3
#define R_BOUNCES  4
#define R_BNCE_DIS 5
#define R_SYNTAX   6
#define R_BADUNIT  7
#define R_BADTYPE  8
#define R_BADREGN  9
#define R_NSU     10
#define R_NSO     11
#define R_NSO_DIS 12
#define R_OTM     13
#define R_DISBAND 14
#define R_DUPLIC  15
#define R_CUT     16
#define R_CUT_DIS 17
#define R_BLOCKED 18
#define R_COMMENT 19
#define R_MAYBE   20
#define R_BADCOAST 21
#define R_BADCOUN 22
#define R_RETREAT 23
#define R_ADJUST  24
#define R_CANTBUILD 25
#define R_NORIGHT 26
#define R_CANAL   27
#define R_PARADOX 28
#define R_MAX     29

extern char *kResolutionStrings[];

#endif
