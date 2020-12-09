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
#ifndef __StatusDisplayDefs__
#define __StatusDisplayDefs__

#define kStatusSupplyItem		1
#define kStatusDislodgeItem		2
#define kStatusExpectingItem	3

extern HWND statusWindow;

bool NewStatusWindow(void);
void CloseStatusWindow(void);

void CleanCentersList(void);
void CleanDislodgeList(void);
void AddCenterLine(char *line);
void AddDislodgeLine(char *line);
void SetExpecting(char *line);

//void DragStatus(HWND theWindow, EventRecord *event);
//void StatusContent (HWND theWindow, EventRecord *event, short item);
bool IsStatusSelection();
void CopyStatusSelection();
void ClearStatusSelections();
void ClearSupplySelection();
void ClearDislodgeSelection();
void DrawStatusContents(HWND theWindow);
void StatusActivate(HWND theWindow, bool activating);

void GetStatusRect(Rect& rect);

bool IsStatusWindow(HWND window);
void HideStatusWindow(void);
void ShowStatusWindow();

#endif