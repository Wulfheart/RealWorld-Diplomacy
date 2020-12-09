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
#ifndef __InfoDisplayDefs__
#define __InfoDisplayDefs__

#define kInfoTextItem		1

extern HWND infoWindow;

bool NewInfoWindow(void);
void CloseInfoWindow(void);

void CleanInfoText(void);
bool LoadInfoText(char* filename);
void AddInfoTextLine(char *line);

//void DragInfo(HWND theWindow, EventRecord *event);
//void InfoContent (HWND theWindow, EventRecord *event, short item);
bool IsInfoSelection();
void CopyInfoSelection();
void ClearInfoSelection();
void DrawInfoContents(HWND theWindow);
void InfoActivate(HWND theWindow, bool activating);

void GetInfoRect(Rect& rect);

bool IsInfoWindow(HWND window);
void HideInfoWindow(void);
void ShowInfoWindow();

#endif