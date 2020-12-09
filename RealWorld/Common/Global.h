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
//===============================================================================
// @ Global.h
//-------------------------------------------------------------------------------
// Global variables for project.
//
// Change history:
//
// 98-Jun-22	JMV	First version
// 00-Jan-02	JMV Cleaned out
//
//===============================================================================


#ifndef __GlobalDefs__		// Prevent multiple includes
#define __GlobalDefs__

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

class ScrollableWindow;
class Offscreen;
class Image;

//-------------------------------------------------------------------------------
//-- External Variables ---------------------------------------------------------
//-------------------------------------------------------------------------------

extern bool		gDone;
extern bool		gInBackground;
extern bool     gDemo;

extern const char *kAppName;
extern const char *kOldAppName;
extern const char *kDataDirectory;
extern const char *kBitmapExtension;
extern const char *kSeparator;
extern const char *kIconPath;

extern ScrollableWindow*	mapDisplay;
extern Offscreen*			mapPort;
extern Image*				mapPict;
extern Image*				cMapPict;

#endif  // __GlobalDefs__

