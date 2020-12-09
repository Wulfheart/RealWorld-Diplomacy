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
#include <stdio.h>
#include <string.h>
#include "MapDisplay.h"
#include "EditMap.h"
#include "Game.h"
#include "Global.h"
#include "History.h"
#include "Image.h"
#include "InfoDisplay.h"
#include "MapContent.h"
#include "MapData.h"
#include "MacTypes.h"
#include "Offscreen.h"
#include "Order.h"
#include "OrderPalette.h"
#include "Preferences.h"
#include "Rgn.h"
#include "ScrollableWindow.h"
#include "Strings.h"
#include "Unit.h"
#include "Util.h"
#include "Variants.h"
#include "WindowUtil.h"

ScrollableWindow* mapWindow = NULL;

OffscreenPtr	mapPort = NULL;
Rect		mapRect = {0,0,0,0};

ImagePtr	mapPict = NULL;
// min/max dimensions of window

ImagePtr	cMapPict = NULL;
OrderPalette orderPalette;

#undef ABERRATION 
#undef YOUNGSTOWN 

// local prototypes 
void CloseMapWindow(void);
void PostPaint( Rect* rect );

bool
InitMapDisplay(void)
{
	ShowLoadingDialog();
	
	// open map 
	mapPict = new Image();
	if (mapPict == NULL || !mapPict->Initialize(variantInfo->bwmap))
	{
		gErrNo = errNoError;
		if (!mapPict->Initialize(variantInfo->colormap))
		{
			HideLoadingDialog();
			return false;
		}
	}
	mapPict->GetRect(mapRect);
	
#ifdef ABERRATION	
	mapRect.right = 800;
	mapRect.bottom = 640;
	mapRect.left = 0;
	mapRect.top = 0;
#endif
#ifdef YOUNGSTOWN	
	mapRect.right = 1200;
	mapRect.bottom = 900;
	mapRect.left = 0;
	mapRect.top = 0;
#endif
	
	// create our offscreen worlds 

	// read the full map
	cMapPict = new Image();
	if (cMapPict == NULL || !cMapPict->Initialize(variantInfo->colormap))
	{
		delete cMapPict;
		cMapPict = NULL;
		HideLoadingDialog();
		return false;
	}

	// create the offscreen buffers
	mapPort = new Offscreen();
	if (!mapPort->Initialize( mapWindow, cMapPict ))
		return false;
	mapWindow->SetOffscreen( mapPort );
	
	if (!ReadMapData())
	{
		HideLoadingDialog();
		return false;	
	}
		
 	if (!LoadInfoText(variantInfo->info))
	{
		gErrNo = errBadInfo;	
 	    strcpy(gErrorString, variantInfo->info);
		HideLoadingDialog();
		return false;	
	}
 	
	HideLoadingDialog();

	return true;

}  //** InitMapDisplay 


void CleanMapDisplay()
{
	if (mapPort != NULL)
	{
		delete mapPort;
	}
	mapPort = NULL;

	if (mapPict != NULL)
		delete mapPict;
	mapPict = NULL;

	if (cMapPict != NULL)
		delete cMapPict;
	cMapPict = NULL;

	CleanMapData();
	
}

//---------------------------
// SaveMapDisplay()
//---------------------------
// Save copy of map to a file
//---------------------------
void SaveMapDisplay()
{
	char prompt[256];
	char filename[256];

	GetIndCString(prompt, kSpecialStringsID, kSaveMapStringID);
	if (strlen(gamename) == 0)
	{
		GetIndCString(filename, kSpecialStringsID, kDefaultGameNameID);
	}
	else
		strcpy((char *)filename, gamename);

	mapWindow->SaveToFile(prompt, filename);
}


bool
AllocMapWindow()
{
	// create the map window
	mapWindow = new ScrollableWindow();
	mapWindow->SetWindowRect( prefs.mMapWindowRect );

	// if can't allocate for some reason 
	if (mapWindow == NULL || !mapWindow->Initialize(kAppName, 640, 480))
	{
		gErrNo = errMem;
		strcpy(gErrorString, "Map window creation failed");
		return false;
	}

	mapWindow->SetPaintCallback( PostPaint );

	return true;
}


/* bring up a new window with the map in it */
bool
NewMapWindow(void)
{
	(void) mapWindow->Reset();
	orderPalette.SetRect(mapWindow);
	return true;
	
} /* NewMapWindow */



/* handle clicks in map window */
void MapContent(Point local, Point global, bool shiftclick)
{
	short					part;
	CoastType					coast;
	
	if (!mapWindow->OnInContent(local.h, local.v))
	{
		if (orderPalette.InPalette(local))
		{
			if (EditModeOn())
			{
				EditSeason();
			}
			else
			{	
				RedrawPalette(orderPalette.DoContent(local));
			}
		}
		else 
		{
			part = FindSector(local.h + mapWindow->GetHorizVal(),
								local.v + mapWindow->GetVertVal(), 
								&coast);
			if (part) 
			{
				if (EditModeOn())
					DoEditClick(global, part);
				else if (history.AtEnd())
					DoOrderClick(part, coast, shiftclick);
			}
		}
	}
		
}	/* MapContent */


/* blink sector on and off */
void BlinkSector(short sector, CoastType coast)
{
	short 		hdiff, vdiff;
	Rect		windowRect;
	Rgn*		blinkrgn;

	mapWindow->SetAsCurrent();
	
	mapWindow->ClipToScrollbars();

	hdiff = mapWindow->GetHorizVal();
	vdiff = mapWindow->GetVertVal();
	
	if (coast == C_NONE || map[sector].coast1 == C_NONE)
		blinkrgn = map[sector].rgn;
	else if (coast == map[sector].coast1)
		blinkrgn = map[sector].coast1_rgn;
	else if (coast == map[sector].coast2)
		blinkrgn = map[sector].coast2_rgn;
	else if (coast == map[sector].coast3)
		blinkrgn = map[sector].coast3_rgn;
	else
		blinkrgn = map[sector].rgn;
		
	blinkrgn->Offset(-hdiff, -vdiff);
	PenNormal();
	blinkrgn->Draw();
	
	// The GLIBC sleep() argument is in seconds, not microseconds	
 	#ifdef __GLIBC__	
 		sleep((unsigned int) 125/1000);	
 	#else
		sleep((unsigned int) 250);	
	#endif
	blinkrgn->Offset(hdiff, vdiff);
	blinkrgn->GetRect( windowRect );
	mapWindow->PaintBody(&windowRect);

	mapWindow->ClipToFull();
}

void UpdateMapPalette()
{
	Rect clearRect = orderPalette.GetRect();
	mapWindow->Refresh( &clearRect );
}	

void 
UpdateMapDisplay( Rect* rect )
{
	mapWindow->Paint( rect );
}

void 
PostPaint( Rect* )
{
	orderPalette.Draw();
}

void
SizeMapWindow( ScrollableWindow* theWindow, int w, int h )
{
	theWindow->OnSize( w, h );
	orderPalette.SetRect(theWindow);
}

void CopyMapToScrap()
{
	mapWindow->CopyToScrap();
}

void
ShowMapWindow()
{
	mapWindow->Show(true);
}


void
HideMapWindow()
{
	mapWindow->Show(false);
}


void ActivateMap(bool activating)
{
	orderPalette.Activate(mapWindow, activating && !EditModeOn());
}


void
SetMapTitle(char *s)
{
	mapWindow->SetTitle(s);
}
