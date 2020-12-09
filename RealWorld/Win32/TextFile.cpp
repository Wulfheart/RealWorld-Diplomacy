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
#include <string.h>
#include "TextFile.h"
#include "Parse.h"

TextFile::TextFile()
{
	mFile = NULL;
	mCurrentLine = 0;
}

TextFile::~TextFile()
{
	Close();
}
	
bool TextFile::Open(const char* filename, bool clear, bool gameFile)
{
	if (clear)
		mFile = fopen((char*)filename, "w");
	else
		mFile = fopen((char*)filename, "r+");
	return (mFile != NULL);
}

void TextFile::Close()
{
	if (mFile != NULL)
	{
		fclose(mFile);
		mFile = NULL;
	}
}
	
bool TextFile::ReadLine(char* string, int len)
{
	int rc;
	char *sp;
	bool comment = true;
	
	if (string == NULL)
		return false;
	if (feof(mFile))
		return false;
	do
	{
		mCurrentLine++;
		sp = string;
		while (len--)
		{
			rc = getc(mFile);
			if (rc == EOF)
				break;
			if (rc == '\n')
				break;
			if (rc == '\r')
			{
				// remove any trailing line feed as well
				rc = getc(mFile);
				if (rc != '\n')
				{
					ungetc(rc, mFile);
					rc = '\r';
				}
				break;
			}
			*sp++ = rc;
		}
		if (len)
			*sp = '\0';
		if (*string != '#')
			comment = false;
		if (rc == EOF)
			break;
	} while (comment);
	
	if (!comment)
	{
		return true;
	}
	else
		return false;
/*
	while (!feof(mFile))
	{
		char* result = fgets(string, len, mFile);
		if (result == NULL && !feof(mFile))
			return false;
		else
		{
			mCurrentLine++;
			if (string[strlen(string)-1] == '\n')
				string[strlen(string)-1] = '\0';
			if (*string != '#')
				return true;
		}
	}
	return false;
*/
}

bool TextFile::ReadChar(char& c)
{
	int rc = getc(mFile);
	if (rc == EOF)
		return false;
	else
	{
		c = (char) rc;
		return true;
	}
}

bool TextFile::WriteLine(char* line, short width, short indent)
{
	int result;
	long count;
	char *sp1, *sp2, *sp3, newstring[128];
	short i, spaces = 0;
	
	if ((int)strlen(line) <= width)
	{
		result = fprintf(mFile, "%s\n", line);
	}
	else
	{
		sp1 = line;
		while ((int)strlen(sp1) > width-spaces)
		{
			sp2 = skipword(sp1);
			while (sp2-sp1 < (int)strlen(sp1))
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
			count = strlen(newstring);
			result = fprintf(mFile, "%s\n", newstring);
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
			count = strlen(newstring);
			result = fprintf(mFile, "%s\n", newstring);
		}
	}
	
	return (result != 0);
}

bool TextFile::AtEOF()
{
	if (mFile == NULL)
		return true;
	return (feof(mFile) != 0);
}


bool TextFile::Rewind()
{
	if (mFile == NULL)
		return false;
	rewind(mFile);
	return true;
}