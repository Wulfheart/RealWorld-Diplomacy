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
#include "MacTypes.h"
#include "resource.h"
#include <commdlg.h>
#include <string.h>
#include "Print.h"
#include "Game.h"
#include "MapDisplay.h"
#include "MapData.h"
#include "DibAPI.h"
#include "Util.h"
#include "Image.h"
#include "Offscreen.h"
#include "Strings.h"

//static THPrint pHandle = nil;
static HDC		printerDC = NULL;

extern RECT		mapRect;
extern HINSTANCE	gInstance;
static BOOL		bUserAbort;
static HWND		pDialog;

extern OffscreenPtr	mapPort;
extern ImagePtr	cMapPict;

bool GetPrinterDC(PRINTDLG* pd);

void PrintMap(Rect pageRect, HDC prPort, int page);
void PrintGameInfo(Rect *portRect);
int DetermineNumberOfPagesinDoc(Rect pageRect);
BOOL CALLBACK AbortProc(HDC printerDC, int iCode);
BOOL CALLBACK PrintDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void
InitPrinter()
{
/*	PRINTER_INFO_5	pInfo[3];
	DWORD			dwNeeded, dwReturned;

	if (EnumPrinters (PRINTER_ENUM_DEFAULT, NULL, 5, (LPBYTE) pinfo5,
					  sizeof(pinfo5), &dwNeeded, &dwReturned))
	{
		printerDC = CreateDC(NULL, pinfo5[0].pPrinterName, NULL, NULL);
	}*/
}

void
PageSetup()
{
/*CONVERT	PrOpen();
	if (pHandle != NIL)
	{
		(void) PrValidate(pHandle);
		(void) PrStlDialog(pHandle);
	}
	PrClose();*/
}


void
ClosePrinter()
{
}

int DetermineNumberOfPagesinDoc(Rect pageRect)
{
	int hpages, vpages;

	hpages = (int)((float)(mapRect.right - mapRect.left)/
			  (float)(pageRect.right - pageRect.left) + 0.5f);
	if (hpages == 0)
		hpages = 1;
	vpages = (int)((float)(mapRect.bottom - mapRect.top)/
			  (float)(pageRect.bottom - pageRect.top) + 0.5f);
	if (vpages == 0)
		vpages = 1;
	return hpages*vpages;
}

void PrintMap(Rect pageRect, HDC prPort, int page)
{
	Rect printRect;
	float mapWidth, mapHeight, pageWidth, pageHeight;
	int hpages, vpages, hindex, vindex;
	page--;

	// set printRect 
	mapWidth = (float)(mapRect.right - mapRect.left);
	mapHeight = (float)(mapRect.bottom - mapRect.top);
	pageWidth = (float)(pageRect.right - pageRect.left);
	pageHeight = (float)(pageRect.bottom - pageRect.top);
	hpages = (int)(mapWidth/pageWidth + 0.5f);
	if (hpages == 0)
		hpages = 1;
	vpages = (int)(mapHeight/pageHeight + 0.5f);
	if (vpages == 0)
		vpages = 1;
	vindex = page/hpages;
	hindex = page%hpages;
	
	// if the map is smaller
	if (hpages == 1 && mapWidth <= pageWidth)
	{
		printRect.left = mapRect.left;
		printRect.right = mapRect.right;
	}
	// if more than one page && big map
	else if (hpages > 1 && mapWidth > pageWidth)
	{
		printRect.left = pageRect.left;
		printRect.right = pageRect.left + (int)(pageWidth*hpages);
	}
	// if map is bigger, shrink it
	else
	{
		printRect.left = pageRect.left;
		printRect.right = pageRect.right;
	}
	if (vpages == 1 && mapHeight <= pageHeight)
	{
		printRect.top = mapRect.top;
		printRect.bottom = mapRect.bottom;
	}
	else if (vpages > 1 && mapHeight > pageHeight)
	{
		printRect.top = pageRect.top;
		printRect.bottom = pageRect.top + (int)(pageHeight*vpages);
	}
	else
	{
		printRect.top = pageRect.top;
		printRect.bottom = pageRect.bottom;
	}

	// one last pass
	mapWidth = (float)(printRect.right - printRect.left);
	mapHeight = (float)(printRect.bottom - printRect.top);
	// if map is considerably smaller on both sides than page
	if (pageHeight > 2*mapHeight && pageWidth > 2*mapWidth)
	{
		// scale to page
		float widthScale = (float)pageWidth/(float)mapWidth;
		float heightScale = (float)pageHeight/(float)mapHeight;
		if (heightScale > widthScale)
		{
			printRect.bottom = (int)(printRect.bottom*widthScale);
			printRect.right = (int)(printRect.right*widthScale);
		}
		else
		{
			printRect.bottom = (int)(printRect.bottom*heightScale);
			printRect.right = (int)(printRect.right*heightScale);
		}
	}
	
	// offset printRect to right place 
	OffsetRect(&printRect, (int)(-pageWidth*hindex), (int)(-pageHeight*vindex));

	SetPort(prPort);

	ImagePtr printImage = mapPort->CreateImage();
	printImage->Draw( printRect );

//CONVERT	if ((gShowSupplies || gShowUnits || gShowOrders) 
//		&& vindex == vpages-1 && hindex == 0)
//		PrintGameInfo(&printRect); 

	delete printImage;
}


/* 
 * Win32 version of printloop
 */
bool PrintLoop()
{
	HDC	oldPort;
	PRINTDLG pd;
	Rect pageRect;
	DOCINFO di = { sizeof(DOCINFO), "", NULL };
	bool	bSuccess = true;
	bool	bUseAbort = false;
	int     copies,
            firstPage,
            lastPage,
            numberOfCopies,
            realNumberOfPagesinDoc,
            pageNumber;

	GetPort(&oldPort);

	if ( !GetPrinterDC(&pd) )
		return false;

	pageRect.top = 0;
	pageRect.left = 0;
	pageRect.bottom = GetDeviceCaps(pd.hDC, VERTRES);
	pageRect.right = GetDeviceCaps(pd.hDC, HORZRES);

	pDialog = CreateDialog( gInstance, MAKEINTRESOURCE(IDD_PRINT), mapWindow->GetNative(), 
							PrintDlgProc );
//	SetDlgItemText( pDialog, IDD_FNAME, "Foo" );

	SetAbortProc(pd.hDC, AbortProc);

    if ( StartDoc( pd.hDC, &di ) > 0 )
    {
		// get the number of pages in the document
        realNumberOfPagesinDoc
            = DetermineNumberOfPagesinDoc(pageRect); 

        /**
          Get the number of copies of the document that the user 
          wants printed from the job record
        **/

        numberOfCopies = pd.nCopies;

        /**
         	Get the first and last pages of the document that
        	were requested to be printed by the user from the job record
        **/

        firstPage = pd.nMinPage;
        lastPage = pd.nMaxPage;

        /**
            Print "all" pages in the print loop
        **/

		pd.nMinPage = 1;
        pd.nMaxPage = 9999;
        
        /**
            Determine the "real" number of pages contained in the 			           
            document. Without this test, you would print 9999 pages.
        **/

		if (firstPage == 0)
			firstPage = 1;
		if (lastPage == 0)
			lastPage = 1;
        if (realNumberOfPagesinDoc < lastPage) 
            lastPage = realNumberOfPagesinDoc;

		// Set AbortProc()

        /**
            Print the number of copies of the document
            requested by the user from the Print Job Dialog.
        **/
        for (copies = 1; copies <= numberOfCopies; copies++)
        {
            /**
                Print the range of pages of the document 
                requested by the user from the Print Job Dialog.
             **/
			pageNumber = firstPage;
			while ( pageNumber <= lastPage && bSuccess )
            {
                    
				// start producing the page
				if (StartPage (pd.hDC) < 0)
				{
					bSuccess = false;
					break;
                }
				PrintMap(pageRect, pd.hDC, pageNumber);

				if (EndPage(pd.hDC) < 0)
				{
					bSuccess = false;
					break;
				}

				if (bUserAbort)
					break;

			    pageNumber++;

            }  /**  End pageNumber loop  **/
            
			if (!bSuccess || bUserAbort)
				break;

		} /**  End copies loop  **/

	}
	else
		bSuccess = false;
	
	if (bSuccess)
		EndDoc( pd.hDC );

  	if (!bUserAbort)
    	DestroyWindow( pDialog );

  	SetPort(oldPort);

	DeleteDC( pd.hDC );
  	
	if (!bSuccess)
  	{
  		gErrNo = errPrint;
  		return false;
  	}
  	else 
  		return true;

}  /** PrintLoop **/


bool GetPrinterDC(PRINTDLG* pd)
{
	LPDEVMODE dm;

	memset(pd, 0, sizeof(PRINTDLG));
	pd->lStructSize	= sizeof(PRINTDLG);
	pd->hwndOwner	= mapWindow->GetNative();
	pd->Flags		= PD_RETURNDEFAULT;
	pd->nCopies		= 1;

	if (!PrintDlg(pd))
		return false;
	if (pd->hDevMode == NULL)
		return false;

	pd->Flags &= ~PD_RETURNDEFAULT;
	dm = (LPDEVMODE) GlobalLock (pd->hDevMode);
	if (dm == NULL)
		return false;
	dm->dmOrientation = DMORIENT_LANDSCAPE;
	dm->dmFields |= DM_ORIENTATION;
	GlobalUnlock(pd->hDevMode);
	pd->Flags = PD_ALLPAGES | PD_RETURNDC | PD_NOSELECTION ;
	if (!PrintDlg(pd))
		return false;
	return true;
}


void
PrintGameInfo(Rect *portRect)
{
	short h, v;
	char string[35];
	Rect clearRect;
	short fontNum;
		
	if (season == S_NONE)
		return;
		
	GetFNum(CtoPstr("Times"), &fontNum);
	TextFont(fontNum);
	TextSize(18); 	
//	TextFace(bold); 
	
	h = 2;
	v = portRect->bottom - portRect->top-54;
	MoveTo(h, v);
	SetRect(&clearRect, 2, v-15, 150, v+5);
	EraseRect(&clearRect); 
	
	if (strlen(gamename))
	{
		strcpy((char*) string, gamename);
		DrawString(CtoPstr((char*)string));
		DrawChar(',');
		DrawChar(' ');
	}
	
	GetIndCString(string, kSeasonStringsID, season); 
	DrawString(CtoPstr(string));
	DrawChar(' ');
	NumToString(year, CtoPstr(string));
	DrawString(CtoPstr(string));
	
///	TextSize(9);
//	v += 15;
//	MoveTo(h, v); 
	
//	DrawString("\pBase Map © 1976, Avalon Hill Game Company.");
//	v += 8;
//	MoveTo(h, v); 
//	DrawString("\pElectronic version © 1995, Jim Van Verth."); 
}


BOOL CALLBACK AbortProc(HDC printerDC, int iCode)
{
	MSG msg;
	while (!bUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!pDialog || !IsDialogMessage(pDialog, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return !bUserAbort;
}

BOOL CALLBACK PrintDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		bUserAbort = TRUE;
		DestroyWindow(pDialog);
		pDialog = NULL;
		return TRUE;

	default:
		return FALSE;
	}
}
