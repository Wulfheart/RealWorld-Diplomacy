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
#ifndef __OrdersDisplayDefs__
#define __OrdersDisplayDefs__

#include "LWindows.h"

extern LWindowPtr ordersWindow, errorWindow;

//CONVERT

bool NewOrdersWindow();
void CloseOrdersWindow(void);
void InitOrdersDisplay(void);
/* CONVERT
void CopyOrdersDisplay(WindowPtr theWindow);
void OrdersContent (WindowPtr theWindow, EventRecord *event);
void DragOrders(WindowPtr theWindow, EventRecord *event);
void ActivateOrders(WindowPtr theWindow, Boolean activating);
void GrowOrders(WindowPtr theWindow, EventRecord *event);
void DrawOrdersContents(WindowPtr theWindow);
*/
void ShowOrdersWindow(void);
void HideOrdersWindow(void);

// CONVERT void KeyOrders(EventRecord *event);
void CleanOrdersList(void);
int AddOrderLine(char *line);
void SetOrderLine(char *line, int index);
void SelectOrderLine(int index);
void ClearOrderSelections();
void ClearSelectedOrders();

void CloseErrorWindow(void);
void ShowErrorWindow();
void HideErrorWindow();
void ClearErrorList(void);
int AddErrorLine(char *line);


#endif
