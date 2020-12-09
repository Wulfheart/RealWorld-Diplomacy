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

/* some internal definitions */
#define X_ANY  0
#define X_MOVE 1
#define X_HORM 2   /* expecting support or move */

#define TABLEN 8

/* general parsing routines */
short wordlen(char *);
char *skipspace(char *);
char *skipword(char *);
void capitalize(char *s);
void uppercase(char* s);
bool strnsame(char *s1, char *s2, int n);
char *samestring(const char *line, const char *key);
void appendtabs(char* s);
int emptyline(char *s);
char *findcomment(char *s);
char *striptext(char *);

/*
short parsecountry();
short parseunit();
short parseregion();
short parsetype();
*/
