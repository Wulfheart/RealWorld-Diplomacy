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

#include <ctype.h>
#include <locale>
#include "MapData.h"
#include "Parse.h"
#include "Game.h"
/* #include "order.h"
   #include "fails.h"
 */
using namespace std;

static locale loc;
static locale loc2 ("German_Germany");

int 
emptyline (char *s)
{
     if (s == NULL)
	  return 1;
     while (isspace(*s)) s++;
     if (*s == '\000')
	  return 1;

     return 0;
}
 
void 
uppercase (char *s)
{
     if (s == NULL)
	  return;

     for (; *s; s++)
	  if (islower(*s))
	       *s = toupper(*s);
}

void
capitalize(char *s)
{
	if (s == NULL)
		return;
		
	if (islower(*s))
		*s = toupper(*s);
	s++;
	for (; *s; s++)
		if (isupper(*s))
			*s = tolower(*s);
			
}

bool
strnsame(char *s1, char *s2, int n)
{
	bool same;
	if (*s1 == NULL || *s2 == NULL)
		return false;
	
	same = 1;
	for (; *s1 && *s2 && n; s1++, s2++, n--)
	{
		if (*s1 != *s2 && toupper(*s1) != *s2 && *s1 != toupper(*s2))
		{
			same = 0;
			break;
		}
	
	}
	if (n != 0)
		return 0;
	return same;
	
}

char *
samestring(const char *line, const char *key)
{
	if (line == NULL || key == NULL)
		return NULL;
		
	while (*line != '\0' && 
			(*line == *key || toupper(*line) == *key || *line == toupper(*key)))
	{	line++; key++; }
	
	if ( *key == '\0' )
		return (char*)line;
	else
		return NULL;
}
			

void 
appendtabs (char *s)
{

     if (s == NULL)
	  return;

/*     strncat(s, "\t\t\t\t\t", 5 - (strlen(s))/TABLEN); */
     
}


/* return pointer to beginning of next word in string s */
/* assumes there might be leading blanks */
char *
skipword (char *s)
{
	if (s == NULL)
	  	return s;

     s = skipspace(s);    		/* skip leading blanks */

     // while (isgraph(*s, loc2)) s++;   /* skip word */
	 // while (isgraph(*s)) s++;   /* skip word */

	 while (!(isspace(*s) || *s == ',' || *s == '.' || *s == ':' ||  *s == '\0')) s++;
	 
     s = skipspace(s);    		/* skip trailing blanks */

     if (s == NULL || *s == '\0')                /* fell off the end? */
	  	return NULL;
	  
     return s;
}

char *
skipspace (char *s)
{
	if (s == NULL)
		return s;

     while (isspace(*s) || *s == ',' || *s == '.' || *s == ':') s++;
     
     if (*s == '\0')
	  return NULL;

     return s;
}

/* returns length of next word in string */
/* assumes no leading blanks */
short 
wordlen (char *s)
{
     char *sp = s;
	 
     if (s == NULL)
	  return 0;

     while (isgraph(*sp, loc2)) sp++;
	 // while (isgraph(*sp)) sp++;
	  
     return sp-s;
}

/* returns pointer to remainder of s if it's a comment */
/* skips leading blanks */
char *
findcomment (char *s)
{
     s = skipspace(s);
     if (s == NULL)
	  return NULL;

     if (*s == '#')
	  return s+1;
     else if (*s == '/' && *(s+1) == '/')
	  return s+2;
     else
	  return NULL;
}

/* return text stripped of tabs and deal with comments */
/* affects original text where comments are concerned. */
char *
striptext (char *orig)
{
     char *copy, *comment = NULL;

     copy = skipspace(orig);
     if (wordlen(copy) == 0) 
	    return NULL;
     while (*copy) 
     {
	  	/* strip tab */
	  	if (*copy == '\t') 
	       *copy = ' ';
	  	/* deal w/comment */
	  	else if ((comment = findcomment(copy)) != NULL) 
	  	{
	       *copy = '\0';    /* order is everything up to this comment */
	       break;
	  	}
	  	copy++;
     }
     return comment;
}

