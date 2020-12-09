//  myfile.cpp
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  SaveDIB()           - Saves the specified dib in a file
//  ReadDIBFile()       - Loads a DIB from a file
//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// Based on code by Microsoft Corporation
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
#include <math.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include "dibapi.h"

int gDibError = DIB_ERR_NONE;

/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

#ifdef _MAC
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))
void ByteSwapHeader(BITMAPFILEHEADER* bmiHeader);
void ByteSwapInfo(LPSTR lpHeader, BOOL fWin30Header);
#endif

/*************************************************************************
 *
 * SaveDIB()
 *
 * Saves the specified DIB into the specified CFile.  The CFile
 * is opened and closed by the caller.
 *
 * Parameters:
 *
 * HDIB hDib - Handle to the dib to save
 *
 * CFile& file - open CFile used to save DIB
 *
 * Return value: TRUE if successful, else FALSE or CFileException
 *
 *************************************************************************/

BOOL WINAPI SaveDIB(HDIB hDib, char* filename)
{
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	LPBITMAPINFOHEADER lpBI;   // Pointer to DIB info structure
	DWORD dwDIBSize;

	if (hDib == NULL)
		return FALSE;

	/*
	 * Get a pointer to the DIB memory, the first of which contains
	 * a BITMAPINFO structure
	 */
	lpBI = (LPBITMAPINFOHEADER) ::GlobalLock((HGLOBAL) hDib);
	if (lpBI == NULL)
		return FALSE;

	if (!IS_WIN30_DIB(lpBI))
	{
		::GlobalUnlock((HGLOBAL) hDib);
		return FALSE;       // It's an other-style DIB (save not supported)
	}

	/*
	 * Fill in the fields of the file header
	 */

	/* Fill in file type (first 2 bytes must be "BM" for a bitmap) */
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

	// Calculating the size of the DIB is a bit tricky (if we want to
	// do it right).  The easiest way to do this is to call GlobalSize()
	// on our global handle, but since the size of our global memory may have
	// been padded a few bytes, we may end up writing out a few too
	// many bytes to the file (which may cause problems with some apps).
	//
	// So, instead let's calculate the size manually (if we can)
	//
	// First, find size of header plus size of color table.  Since the
	// first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
	// the size of the structure, let's use this.

	dwDIBSize = *(LPDWORD)lpBI + ::PaletteSize((LPSTR)lpBI);  // Partial Calculation

	// Now calculate the size of the image

	if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))
	{
		// It's an RLE bitmap, we can't calculate size, so trust the
		// biSizeImage field

		dwDIBSize += lpBI->biSizeImage;
	}
	else
	{
		DWORD dwBmBitsSize;  // Size of Bitmap Bits only

		// It's not RLE, so size is Width (DWORD aligned) * Height

		dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) * lpBI->biHeight;

		dwDIBSize += dwBmBitsSize;

		// Now, since we have calculated the correct size, why don't we
		// fill in the biSizeImage field (this will fix any .BMP files which
		// have this field incorrect).

		lpBI->biSizeImage = dwBmBitsSize;
	}


	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)

	bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;

	/*
	 * Now, calculate the offset the actual bitmap bits will be in
	 * the file -- It's the Bitmap file header plus the DIB header,
	 * plus the size of the color table.
	 */
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize
											  + PaletteSize((LPSTR)lpBI);
#ifdef _MAC
	ByteSwapHeader(&bmfHdr);

	// First swap the size field
	*((LPDWORD)lpBI) = SWAPLONG(*((LPDWORD)lpBI));

	// Now swap the rest of the structure (we don't save < Win30 files)
	ByteSwapInfo((LPSTR)lpBI, TRUE);
#endif

	/*
	 * open the file
	 */
	BOOL result = TRUE;

	FILE *file = fopen( filename, "wb" );
	if (file == NULL)
		result = FALSE;

	// Write the file header
	if (result && (fwrite((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), 1, file) != 1
		//
		// Write the DIB header and the bits
		//
		|| 	fwrite(lpBI, dwDIBSize, 1, file) != 1))
	{
		result = FALSE;
	}

	fclose(file);

#ifdef _MAC
	// Swap everything back
	*((LPDWORD)lpBI) = SWAPLONG(*((LPDWORD)lpBI));
	ByteSwapInfo((LPSTR)lpBI, TRUE);
#endif

	::GlobalUnlock((HGLOBAL) hDib);

	return result;
}

/*************************************************************************

  Function:  ReadDIBFile (CFile&)

   Purpose:  Reads in the specified DIB file into a global chunk of
			 memory.

   Returns:  A handle to a dib (hDIB) if successful.
			 NULL if an error occurs.

  Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
			 from the end of the BITMAPFILEHEADER structure on is
			 returned in the global memory handle.

*************************************************************************/


HDIB WINAPI ReadDIBFile(char* filename)
{
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER BI;   // DIB info structure
	LPBITMAPINFOHEADER lpBI;   // Pointer to DIB info structure
	DWORD dwBitsSize;
	HDIB hDIB;
	LPSTR pDIB;
	bool compressed = false;

	/*
	 * open the file
	 */

	FILE *file = fopen( filename, "rb" );
	if (file == NULL)
	{
		gDibError = DIB_ERR_NOTFOUND;
		return NULL;
	}

	/*
	 * get length of DIB in bytes for use when reading
	 */

	struct _stat buf;
	_fstat(_fileno(file), &buf);
	dwBitsSize = buf.st_size;

	/*
	 * Go read the DIB file header and check if it's valid.
	 */
	if (fread((LPSTR)&bmfHeader, sizeof(bmfHeader), 1, file) != 1)
	{
		fclose(file);
		gDibError = DIB_ERR_INVALIDFILE;
		return NULL;
	}
	dwBitsSize -= sizeof(BITMAPFILEHEADER);

#ifdef _MAC
	ByteSwapHeader(&bmfHeader);
#endif
	if (bmfHeader.bfType != DIB_HEADER_MARKER)
	{
		fclose(file);
		gDibError = DIB_ERR_NOTBMP;
		return NULL;
	}

	//
	// read the info header
	//
	lpBI = &BI;
	// needs to be 8-bit image
	if (fread(lpBI, sizeof(BITMAPINFOHEADER), 1, file) != 1 || lpBI->biBitCount != 8)
	{
		fclose(file);
		gDibError = DIB_ERR_NOT8BIT;
		return NULL;
	}
	dwBitsSize -= sizeof(BITMAPINFOHEADER);
	compressed = (BI.biCompression == BI_RLE8);
	BI.biCompression = BI_RGB;

#ifdef _MAC
	// First swap the size field
	*((LPDWORD)pDIB) = SWAPLONG(*((LPDWORD)pDIB));

	// Now swap the rest of the info structure
	ByteSwapInfo(pDIB, IS_WIN30_DIB(pDIB));
#endif

	/*
	 * Allocate memory for DIB
	 */
	long paddedWidth=(BI.biWidth+3)&(~3);
	long dibSize = sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + paddedWidth*BI.biHeight;
	hDIB = (HDIB) ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dibSize);
	if (hDIB == 0)
	{
		fclose(file);
		gDibError = DIB_ERR_INVALIDFILE;
		return NULL;
	}
	pDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
	// copy header
	memcpy(pDIB, lpBI, sizeof(BITMAPINFOHEADER));
	pDIB += sizeof(BITMAPINFOHEADER);
	dibSize -= sizeof(BITMAPINFOHEADER);

	//
	// read the palette
	//
	unsigned int numEntries = BI.biClrUsed;
	if (numEntries == 0)
		numEntries = 256;
	if (fread(pDIB, sizeof(RGBQUAD), numEntries, file) != numEntries)
	{
		::GlobalUnlock((HGLOBAL) hDIB);
		::GlobalFree((HGLOBAL) hDIB);
		fclose(file);
		gDibError = DIB_ERR_INVALIDFILE;
		return NULL;
	}
	dwBitsSize -= numEntries*sizeof(RGBQUAD);
	dibSize -= numEntries*sizeof(RGBQUAD);
	pDIB += numEntries*sizeof(RGBQUAD);

	/*
	 * Go read the bits.
	 */
	if (!compressed)
	{
		if (fread(pDIB, dibSize, 1, file) != 1)
		{
			::GlobalUnlock((HGLOBAL) hDIB);
			::GlobalFree((HGLOBAL) hDIB);
			fclose(file);
			gDibError = DIB_ERR_INVALIDFILE;
			return NULL;
		}
	}
	else
	{
		unsigned char* data = new unsigned char[dwBitsSize];
		if (fread(data, dwBitsSize, 1, file) != 1)
		{
			::GlobalUnlock((HGLOBAL) hDIB);
			::GlobalFree((HGLOBAL) hDIB);
			fclose(file);
			gDibError = DIB_ERR_INVALIDFILE;
			return NULL;
		}

		unsigned char* datap = data;
		int row = 0;
		int index = 0;
		int numBytes;
		int code;
		memset(pDIB, 0, dibSize);
		bool done = false;
		while (!done && datap < data+dwBitsSize && index < dibSize)
		{
			numBytes = *datap++;
			// if simple RLE
			if (numBytes > 0)
			{
				code = *datap++;
				if (index + numBytes > dibSize)
					numBytes = dibSize - index;
				memset(pDIB+index, code, numBytes);
				index += numBytes;
			}
			// special code
			else
			{
				numBytes = *datap++;
				// literal
				if (numBytes >= 3)
				{
					if (index + numBytes > dibSize)
						numBytes = dibSize - index;
					memcpy(pDIB+index, datap, numBytes);
					datap += numBytes;
					index += numBytes;
					if (numBytes & 0x1)
						datap++;
				}
				// end of row
				else if (numBytes == 0)
				{
					row++;
					index = paddedWidth*row;
				}
				// end of file
				else if (numBytes == 1)
				{
					done = true;
				}
				// displacement by x, y
				else if (numBytes == 2)
				{
					code = *datap++;
					index += code;
					code = *datap++;
					row += code;
					index += paddedWidth*code;
				}
			}
		}
		
		delete data;
	}

	::GlobalUnlock((HGLOBAL) hDIB);

	fclose(file);

	return hDIB;
}


#ifdef _MAC
void ByteSwapHeader(BITMAPFILEHEADER* bmfHeader)
{
	bmfHeader->bfType = SWAPWORD(bmfHeader->bfType);
	bmfHeader->bfSize = SWAPLONG(bmfHeader->bfSize);
	bmfHeader->bfOffBits = SWAPLONG(bmfHeader->bfOffBits);
}


void ByteSwapInfo(LPSTR lpHeader, BOOL fWin30Header)
{
	// Note this doesn't swap the bcSize/biSize field.  It assumes that the
	// size field was swapped during read or while setting the fWin30Header
	// flag.

	if (fWin30Header)
	{
		LPBITMAPINFOHEADER lpBMIH = &(LPBITMAPINFO(lpHeader)->bmiHeader);

		//lpBMIH->biSize = SWAPLONG(lpBMIH->biSize);
		lpBMIH->biWidth = SWAPLONG(lpBMIH->biWidth);
		lpBMIH->biHeight = SWAPLONG(lpBMIH->biHeight);
		lpBMIH->biPlanes = SWAPWORD(lpBMIH->biPlanes);
		lpBMIH->biBitCount = SWAPWORD(lpBMIH->biBitCount);
		lpBMIH->biCompression = SWAPLONG(lpBMIH->biCompression);
		lpBMIH->biSizeImage = SWAPLONG(lpBMIH->biSizeImage);
		lpBMIH->biXPelsPerMeter = SWAPLONG(lpBMIH->biXPelsPerMeter);
		lpBMIH->biYPelsPerMeter = SWAPLONG(lpBMIH->biYPelsPerMeter);
		lpBMIH->biClrUsed = SWAPLONG(lpBMIH->biClrUsed);
		lpBMIH->biClrImportant = SWAPLONG(lpBMIH->biClrImportant);
	}
	else
	{
		LPBITMAPCOREHEADER lpBMCH = &(LPBITMAPCOREINFO(lpHeader)->bmciHeader);

		lpBMCH->bcWidth = SWAPWORD(lpBMCH->bcWidth);
		lpBMCH->bcHeight = SWAPWORD(lpBMCH->bcHeight);
		lpBMCH->bcPlanes = SWAPWORD(lpBMCH->bcPlanes);
		lpBMCH->bcBitCount = SWAPWORD(lpBMCH->bcBitCount);
	}
}

#endif
