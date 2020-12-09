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

class TextFile
{
public:
	TextFile();
	~TextFile();
	
	bool Open(const char* filename, bool clear, bool gameFile = true);
	void Close();
	bool Rewind();
	
	bool ReadLine(char* string, int len);
	bool ReadChar(char& c);
	bool WriteLine(char* string, short width = 16384, short indent = 0);

	bool AtEOF();
	
	inline long GetCurrentLine()  { return mCurrentLine; }
	
private:
	FILE* mFile;
	long  mCurrentLine;
};