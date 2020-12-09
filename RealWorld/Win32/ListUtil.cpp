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

#include "ListUtil.h"
#include "Parse.h"
#include <string.h>

int
AddListLine(HWND theList, char *line, short width, short indent)
{
	char *sp1, *sp2, *sp3, newstring[128];
	short i, spaces = 0;
	
	if ((short)strlen(line) <= width)
	{
		SendMessage(theList, LB_ADDSTRING, 0, (LPARAM) line);
	}
	else
	{
		sp1 = line;
		while ((short)strlen(sp1) > width-spaces)
		{
			sp2 = skipword(sp1);
			if (sp2 == NULL)
				sp2 = sp1+strlen(sp1);
			while (sp2-sp1 < (short)strlen(sp1))
			{
				sp3 = skipword(sp2);
				if (wordlen(sp3) == 0 || sp3-sp1 > width-spaces)
					break;
				sp2 = sp3;
			}
			for (i = 0; i < spaces; i++)
				newstring[i] = ' ';
			newstring[spaces] = '\0';
			strncat(newstring, sp1, sp2-sp1);
			newstring[sp2-sp1+spaces] = '\0';
			SendMessage(theList, LB_ADDSTRING, 0, (LPARAM) newstring);
			sp1 = sp2;
			if (!spaces)
				spaces = indent;
		}
		if (strlen(sp1) > 0)
		{
			for (i = 0; i < spaces; i++)
				newstring[i] = ' ';
			newstring[spaces] = '\0';
			strcat(newstring, sp1);
			SendMessage(theList, LB_ADDSTRING, 0, (LPARAM) newstring);
		}
	}
	
	return SendMessage(theList, LB_GETCOUNT, 0, 0);
}


void 
ClearListSelections(HWND theList)
{
	SendMessage(theList, LB_SETSEL, 0, -1);
}


int
CopyListSelection(HWND list)
{
	const int	kMaxStringLen = 18000;
	HANDLE hGlobalMemory = GlobalAlloc( GHND, kMaxStringLen+1 );
	char *string = (char*) GlobalLock( hGlobalMemory );
	char *strPtr = string;
	int charsRemaining = kMaxStringLen;
	char listItem[kMaxStringLen+1];
	int itemLength;

	int iCount = SendMessage(list, LB_GETCOUNT, 0, 0);
	for (int i = 0; i < iCount; i++)
	{
		// if this list item is selected
		if (SendMessage(list, LB_GETSEL, i, 0))
		{
			// copy to string
			itemLength = SendMessage(list, LB_GETTEXT, i, (LPARAM) listItem);
			if (itemLength > charsRemaining-1)
				itemLength = charsRemaining-1;
			strncpy(strPtr, listItem, itemLength);
			strPtr += itemLength;
			*strPtr = '\r';
			strPtr++;
			*strPtr = '\n';
			strPtr++;
			charsRemaining -= itemLength+1;
		}

		if (charsRemaining <= 0)
			break;
	}

	GlobalUnlock( hGlobalMemory );
	OpenClipboard( list );
	EmptyClipboard();

	SetClipboardData( CF_TEXT, hGlobalMemory );
	CloseClipboard();

	return kMaxStringLen-charsRemaining;
}


bool IsListSelection(HWND list)
{
	int iCount = SendMessage(list, LB_GETCOUNT, 0, 0);
	for (int i = 0; i < iCount; i++)
	{
		// if this list item is selected
		if (SendMessage(list, LB_GETSEL, i, 0))
		{
			return true;
		}
	}

	return false;
}
