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
#ifndef __MapDisplayDefs__
#define __MapDisplayDefs__

#include "ScrollableWindow.h"
#include "OrderPalette.h"
#include "MapData.h"

extern ScrollableWindow* mapWindow;
extern OrderPalette		orderPalette;

bool InitMapDisplay(void);
void CleanMapDisplay();
void SaveMapDisplay();
void DrawNewMap(void);
void CopyMapToScrap();

bool AllocMapWindow();
bool NewMapWindow();
void MapContent(Point local, Point global, bool shiftclick);
void ActivateMap(bool activating);
void UpdateMapPalette();
void UpdateMapDisplay(Rect* rect);
void SizeMapWindow(ScrollableWindow* theWindow, int w, int h);

void ShowMapWindow(void);
void HideMapWindow(void);
void SetMapTitle(char *s);
void BlinkSector(short sector, CoastType coast);

void 			GetUnitRect(Rect *theRect, UnitType type, int sector, CoastType coast);

bool LoadCountryIcons();
void CleanIcons();

void 			DrawUnits(bool BW);
void 			DrawDislodges(bool BW, bool gs);
void 			RedrawUnit(bool BW, UnitType type, short sector, CoastType coast, short country, bool dislodge, bool remove);
void			DrawUnit(bool BW, UnitType type, short sector, CoastType coast, short country);
void			DrawSupplies(bool bw);
void 			DrawOrders(bool bw, bool gs);
void			DrawAdjusts(bool bw, bool gs);
void 			DrawNames(bool bw);

void SetSelectedSector(short s, bool d);
void GetSelectedSector(short *s, bool *d);
void ClearSelectedSector();
void ResetSelectedUnit();

#endif
