//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// Modifications (C) 2005 by Dirk Brüggemann
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
#include <stdio.h>
#include <ctype.h>
#include "MacTypes.h"
#include "Graph.h"
#include "MapData.h"
#include "Neighbor.h"
#include "Order.h"
#include "Parse.h"
#include "TextFile.h"
#include "Unit.h"
#include "Util.h"
#include "Variants.h"
#include "Strings.h"
#include "Rgn.h"

#undef USE_PLY
#ifdef USE_PLY
#define DUMP
#endif
//short season;

#define MAX_CANALS 10

bool pictVersion2 = false;

struct mapnode *map = NULL;             /* map data */

struct cnode *countries = NULL;
struct canalstruct *canalalloc = NULL;
struct canalstruct *canals = NULL;

UnitList   *units = NULL;
SupplyList *supplies = NULL;
SupplyList *lands = NULL;
Orderlist  *orders = NULL;

short dislodges = 0;
DislodgeList dslglist;

short total_supply = 0;
short total_lands = 0;
short win_supply = 0;

short NUM_SECT;
short NUM_COUN;

char *scoast[5] = {"", "(sc)", "(nc)", "(ec)", "(wc)"};
char *lcoast[5] = {"", "Southern", "Northern", "Eastern", "Western"};
char *sscoast[5] = {"", "(S)","(N)","(E)","(W)"};
char *slcoast[5] = {"", "(south coast)","(north coast)","(east coast)","(west coast)"};
char *jcoast[5] = {"", "/sc", "/nc", "/ec", "/wc"};

/* prototypes */
bool ReadMapResource(void);
bool ReadCountryResource(void);
void readnames(struct mapnode *mapDataPtr, char *string);
bool LoadCountryIcons();
void CleanIcons();

int unitthere (Sector sect, short country, UnitType unit, CoastType coast);
short char2country(char c);

int totalcanalalloc = 0;

int canalcount()
{
	return totalcanalalloc;
}

void alloccanal()
{
	register short i;
	
	if ((canalalloc = (struct canalstruct *) new canalstruct[MAX_CANALS]) == NULL)
        {
                gErrNo = errMem;
                sprintf(gErrorString, "Couldn't allocate canal data Ñ low on memory?");
				return;
        }
        memset(canalalloc, 0, (MAX_CANALS)*sizeof(struct canalstruct));

		for (i = 0; i < MAX_CANALS; i++)
		canalalloc[i].num = 0;
}

void dealloccanal()
{
	if (canalalloc != NULL)
		free(canalalloc);
}

bool
AllocMapData(void)
{
        /* allocate map */
        if (map != NULL)
                return true;

        if ((map = (struct mapnode *) new mapnode[MAX_SECT+1]) == NULL)
        {
                sprintf(gErrorString, "Couldn't allocate map data Ñ low on memory?");
                gErrNo = errMem;
                return false;
        }
        memset(map, 0, (MAX_SECT+1)*sizeof(struct mapnode));

        if ((units = (UnitList *) new UnitList[MAX_COUN+1]) == NULL)
        {
                sprintf(gErrorString, "Couldn't allocate unit data Ñ low on memory?");
                gErrNo = errMem;
                return false;
        }
        memset(units, 0, (MAX_COUN+1)*sizeof(UnitList));

        if ((supplies = (SupplyList *) new SupplyList[MAX_COUN+1]) == NULL)
        {
                gErrNo = errMem;
                sprintf(gErrorString, "Couldn't allocate supply data Ñ low on memory?");
                return false;
        }
        memset(supplies, 0, (MAX_COUN+1)*sizeof(SupplyList));

		if ((lands = (SupplyList *) new SupplyList[MAX_COUN+1]) == NULL)
        {
                gErrNo = errMem;
                sprintf(gErrorString, "Couldn't allocate land data Ñ low on memory?");
                return false;
        }
        memset(lands, 0, (MAX_COUN+1)*sizeof(SupplyList));

        if ((orders = (Orderlist *) new Orderlist[MAX_COUN+1]) == NULL)
        {
                gErrNo = errMem;
                sprintf(gErrorString, "Couldn't allocate order data Ñ low on memory?");
                return false;
        }
        memset(orders, 0, (MAX_COUN+1)*sizeof(Orderlist));

        if ((countries = (struct cnode *) new cnode[MAX_COUN+1]) == NULL)
        {
                gErrNo = errMem;
                sprintf(gErrorString, "Couldn't allocate country data Ñ low on memory?");
                return false;
        }
        memset(countries, 0, (MAX_COUN+1)*sizeof(struct cnode));
		
/*        for (i = 1; i <= 4; i++)
        {
                strncpy(scoast[i], kCoastStrings[i], 4);
        }
*/        return true;
}


bool
ReadMapData(void)
{
        register short i;
		int    scanresult;
        char   line[BUFSIZ];
        struct mapnode *mapPtr;
#ifdef USE_PLY
        Point    points[1024];
        int                 numPoints;
#endif

        if (!ReadCountryResource())
                return false;

        if (!ReadMapResource())
                return false;

        if (!LoadCountryIcons())
                return false;

        // open the file
        char filename[256];
        strcpy(filename, variantInfo->polymap);
        filename[strlen(filename)-3] = 'r';
        filename[strlen(filename)-2] = 'g';
        filename[strlen(filename)-1] = 'n';
        FSSpec fsspec;
        TextFile infile;
        MakeFSSpec(fsspec, filename);
        if (!infile.Open(fsspec, false))
        {
                sprintf(gErrorString, "Couldn't open region file %s", variantInfo->polymap);
                gErrNo = errBadPoly;
                return false;
        }
        infile.ReadLine(line, BUFSIZ-1);

        // read in all the polys
        map[0].unit = NULL;
        // now read in the point and poly data
        for (i = 1, mapPtr = map+1; i <= NUM_SECT; i++, mapPtr++)
        {
                // get the location
                infile.ReadLine(line, BUFSIZ-1);
                
				scanresult = sscanf(line, "%hd,%hd,%hd,%hd", &mapPtr->land_loc.h, &mapPtr->land_loc.v, &mapPtr->coast3_loc.h, &mapPtr->coast3_loc.v);
				if (scanresult == 4)
				{
					// Special coordinates are given,
					// add Special Fleet Placement Flag 
					// if a special fleet placement is required
					if (iscoastal(mapPtr) && (!isbicoastal(mapPtr)))
					{
						mapPtr->type |= F_SFP;
					}
				}
				
				infile.ReadLine(line, BUFSIZ-1);
                sscanf(line, "%hd,%hd", &mapPtr->name_loc.h, &mapPtr->name_loc.v);
                mapPtr->rgn = new Rgn();
                mapPtr->rgn->Read(infile);
                if (mapPtr->coast1 != C_NONE)
                {
                        // get the location
                        infile.ReadLine(line, BUFSIZ-1);
                        sscanf(line, "%hd,%hd", &mapPtr->coast1_loc.h, &mapPtr->coast1_loc.v);
                        mapPtr->coast1_rgn = new Rgn();
                        mapPtr->coast1_rgn->Read(infile);
                        // get the location
                        infile.ReadLine(line, BUFSIZ-1);
                        sscanf(line, "%hd,%hd", &mapPtr->coast2_loc.h, &mapPtr->coast2_loc.v);
                        mapPtr->coast2_rgn = new Rgn();
                        mapPtr->coast2_rgn->Read(infile);
						if (mapPtr->coast3 != C_NONE)
						{
							// get the location
							infile.ReadLine(line, BUFSIZ-1);
							sscanf(line, "%hd,%hd", &mapPtr->coast3_loc.h, &mapPtr->coast3_loc.v);
							mapPtr->coast3_rgn = new Rgn();
							mapPtr->coast3_rgn->Read(infile);
						}
						else 
						{
                        mapPtr->coast3_rgn = NULL;
						}
					}
                else
                {
				        mapPtr->coast1_rgn = mapPtr->coast2_rgn = mapPtr->coast3_rgn = NULL;
                }
        }

        infile.Close();

        for (i = 1; i <= NUM_COUN; i++)
            cleanunits(&units[i]);

    for (i = 1; i <= NUM_COUN; i++)
            cleansupplies(&supplies[i]);

    for (i = 1; i <= NUM_COUN; i++)
            initolist(orders[i]);

    newgraph();

        return true;

}

bool
ReadMapResource(void)
{
        struct mapnode *mapp;
        char line[BUFSIZ], *lp, *lp2;
        int                numnames;

		// clear old canal info
		ClearCanals();

        // open the file
        FSSpec filename;
        TextFile infile;
        MakeFSSpec(filename, variantInfo->map);
        if (!infile.Open(filename, false))
        {
                sprintf(gErrorString, "Couldn't open map file %s", variantInfo->map);
                gErrNo = errBadMapData;
                return false;
        }

		// read name and type data
        total_supply = 0;
        NUM_SECT = 0;
        mapp = map+1;
		
    while (!infile.AtEOF() && NUM_SECT <= MAX_SECT)
    {
                infile.ReadLine(line, BUFSIZ);
                if (strncmp(line, "-1", 2) == 0)
                        break;

                // get the main name
                lp = strchr(line, ',');
                if (lp == NULL)
                {
                        sprintf(gErrorString, "Missing comma at line %d", infile.GetCurrentLine());
                        gErrNo = errBadMapData;
                        infile.Close();
                        return false;
                }
                *lp = '\0';
                lp++;
                mapp->names[0] = new char[strlen(line)+1];
                strcpy(mapp->names[0], line);

                while (isspace(*lp))
                        lp++;

                // get type
                mapp->type = 0;
                // it's a home center
                if (isupper(*lp) || isdigit(*lp))
                {
                        short country = char2country(*lp);
                        if (countries[country].numCenters < kHomeCenters)
                        {
                                countries[country].home[countries[country].numCenters++] = mapp-map;
                        }
                        mapp->type |= F_SUPP;
                        mapp->type |= F_LAND;
                        total_supply++;
                        // can do coastal convoys
                        if (lp[1] == 'c' || lp[1] == 'w')
                        {
                                mapp->type |= F_CONV;
                                ++lp;
                        }
						else if (lp[1] == 'b')
						{
							mapp->type |= F_BLD;
							++lp;
						}
						else if (isdigit(lp[1]))
						{
							mapp->type |= F_CAN2;
							addcanal(&canals, mapp, C_CENTER, lp[1]);
							++lp;
						}
				}
                // it's a non-home center
                else if (*lp == 'x')
                {
                        mapp->type |= F_SUPP;
                        mapp->type |= F_LAND;
                        total_supply++;
                        // can do coastal convoys
                        if (lp[1] == 'c' || lp[1] == 'w')
                        {
                                mapp->type |= F_CONV;
                                ++lp;
                        }
						else if (lp[1] == 'b')
						{
							mapp->type |= F_BLD;
							++lp;
						}
						else if (isdigit(lp[1]))
						{
							mapp->type |= F_CAN2;
							addcanal(&canals, mapp, C_CENTER, lp[1]);
							++lp;
						}
                }
                else if (*lp == 'l')
                {
                        mapp->type |= F_LAND;
                        // can do coastal convoys
                        if (lp[1] == 'c' || lp[1] == 'w')
                        {
                                mapp->type |= F_CONV;
                                ++lp;
                        }
						else if (lp[1] == 's')
						{
								mapp->type |= F_SBJ;
                                ++lp;
						}
						else if (isdigit(lp[1]))
						{
							mapp->type |= F_CAN2;
							addcanal(&canals, mapp, C_CENTER, lp[1]);
							++lp;
						}
                }
                else if (*lp == 'w')
				{
                        mapp->type |= F_SEA;
						// Störtebekerjäger Build Region (Hanse Variant)
						if (lp[1] == 's')
						{
								mapp->type |= F_SBJ;
                                ++lp;
						}
						// canal
						else if (isdigit(lp[1]))
						{
							mapp->type |= F_CAN1;
							addcanal(&canals, mapp, C_GATE, lp[1]);
							++lp;
						}
				}	
                // ice
                else if (*lp == 'v')
                        mapp->type |= (F_SEA | F_ICE);
				// convoyable coast
                else if (*lp == 'c')
                        mapp->type |= (F_LAND | F_SEA | F_CONV);
				// air space (wing only)
				else if (*lp == 'a')
				{
					// set no flags: keep out armies and fleets
				}
				// land where units can be built by some country (like Siberia in 1900)
				else if (*lp == 's' && (isupper(lp[1]) || isdigit(lp[1])))
                {
						short country = char2country(lp[1]);
                        if (countries[country].numSibCenters < kHomeCenters)
                        {
                                countries[country].sib[countries[country].numSibCenters++] = mapp-map;
                        }
                        mapp->type |= (F_LAND | F_SIB);
						++lp;
                }
                // unknown
                else
                {
                        sprintf(gErrorString, "Invalid province type at line %d", infile.GetCurrentLine());
                        gErrNo = errBadMapData;
                        infile.Close();
                        return false;
                }
				// if a non-supply-sector is assigned to a country
				if ((isupper(lp[1]) || isdigit(lp[1])) && island(mapp) && !issupply(mapp))
				{
					short country = char2country(lp[1]);
                    
					//countries[country].numLands++;
                    total_lands++;
                }

				lp += 2;

        /*        if (islower(*lp))
                        *lp = toupper(*lp); */

                // clear name list
                for (int i = 1; i < MAX_NAMES; i++)
                {
                        mapp->names[i] = NULL;
                }

                // get the other names
                numnames = 1;
                while (*lp != '\0' && numnames < MAX_NAMES)
                {
                        lp2 = strchr(lp, ' ');
                        if (lp2 != NULL)
                                *lp2 = '\0';
                        mapp->names[numnames] = new char[strlen(lp)+1];
                        strcpy(mapp->names[numnames], lp);
                        numnames++;
                        if (lp2 == NULL)
                                break;
                        lp = lp2+1;
                        while (isspace(*lp))
                                lp++;
                }
                if (numnames == 1)
                {
                        sprintf(gErrorString, "Missing abbreviation at line %d", infile.GetCurrentLine());
                        gErrNo = errBadMapData;
                        infile.Close();
                        return false;
                }

                // clear neighbor lists
                mapp->land = NULL;
                mapp->sea1 = NULL;
                mapp->sea2 = NULL;
				mapp->sea3 = NULL;
                mapp->coast1 = C_NONE;
                mapp->coast2 = C_NONE;
				mapp->coast3 = C_NONE;

                mapp->unit = NULL;

                NUM_SECT++;
                mapp++;
    }

		// clean up incomplete canals
		Canal c;
		for (c = startcanal(); iscanal(c); nextcanal(c))
		{
			if (cnum(c) > 0)
			{
				if (cgate1(c) == NULL || cgate2(c) == NULL || ccenter(c) == NULL)
				{
					DisposeCanal(c);
				}
			}
		}

        if (NUM_SECT > MAX_SECT)
        {
                sprintf(gErrorString, "Too many spaces - limit is %d", MAX_SECT);
                gErrNo = errBadMapData;
                infile.Close();
                return false;
        }

        // now read neighbor data
    while (!infile.AtEOF())
    {
            char* type;
                infile.ReadLine(line, BUFSIZ);
                if (strncmp(line, "-1", 2) == 0)
                        break;

                mapp = map+1;
                short sector;
                (void) string2sector(&sector, line);
/*                while (strncmp(line, mapp->shortname, 3) && mapp < map+NUM_SECT)
                        mapp++;
*/
                mapp = map+sector;
                if (sector == 0)
                {
                        sprintf(gErrorString, "Couldn't find matching province for line %d", infile.GetCurrentLine());
                        gErrNo = errBadMapData;
                        infile.Close();
                        return false;
                }
                type = strchr(line, '-');
                if (type == NULL)
                {
                        sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                        gErrNo = errBadMapData;
                        infile.Close();
                        return false;
                }
                type++;
                lp = strchr(line, ':');
                if (lp == NULL)
                {
                        sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                        gErrNo = errBadMapData;
                        infile.Close();
                        return false;
                }
                lp = skipspace(lp);
                // if neighbor data is empty, ignore it
                if (lp == NULL || strlen(lp) == 0)
                        continue;
                // otherwise, we keep going
                if (type[1] == 'c')
                {
                        if (type[0] == 'x')
                        {
                                  string2nlist(lp, &(mapp->sea1));
                                  if (mapp->sea1 == NULL)
                                  {
                                        sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                                        gErrNo = errBadMapData;
                                        infile.Close();
                                        return false;
                                  }
                                 mapp->type |= F_SEA;
                        }
						else
                        {
                                if (mapp->coast1 == 0)
                                {
                                          string2nlist(lp, &(mapp->sea1));
                                          if (mapp->sea1 == NULL)
                                          {
                                                sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                                                gErrNo = errBadMapData;
                                                infile.Close();
                                                return false;
                                          }
                                        mapp->coast1 = char2coast(type[0]);
                                        mapp->type |= F_SEA;
                                }
                                else if (mapp->coast2 == 0)
                                {
                                          string2nlist(lp, &(mapp->sea2));
                                          if (mapp->sea2 == NULL)
                                          {
                                                sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                                                gErrNo = errBadMapData;
                                                infile.Close();
                                                return false;
                                          }
										  mapp->coast2 = char2coast(type[0]);
										  mapp->type |= F_SEA;
								 }
								 else 
                                 {
                                          string2nlist(lp, &(mapp->sea3));
                                          if (mapp->sea2 == NULL)
                                          {
                                                sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                                                gErrNo = errBadMapData;
                                                infile.Close();
                                                return false;
                                          }
										  mapp->coast3 = char2coast(type[0]);
										  mapp->type |= F_SEA;
								 }
                        }
                }
				// adjacent places for wing-only sectors
				else if (type[1] == 'a')
				{
					string2nlist(lp, &(mapp->land));
                        if (mapp->land == NULL)
                        {
                                sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                                gErrNo = errBadMapData;
                                infile.Close();
                                return false;
                        }
				}
                // otherwise is land sector
                else
                {
                        string2nlist(lp, &(mapp->land));
                        if (mapp->land == NULL)
                        {
                                sprintf(gErrorString, "Couldn't parse neighbor data at line %d", infile.GetCurrentLine());
                                gErrNo = errBadMapData;
                                infile.Close();
                                return false;
                        }
                        mapp->type |= F_LAND;
				}
        }

        /* if winning supply info given */
        if (variantInfo->flags & 0xFFFF0000)
                win_supply = (variantInfo->flags & 0xFFFF0000) >> 16;
        else
                win_supply = (total_supply/2)+1;

        infile.Close();

        return true;
}

void readnames(struct mapnode *mapDataPtr, char *string)
{
        short num_names = 0;
        char *cp = string;

        if (string == NULL)
                return;

        while (num_names < MAX_NAMES && cp != NULL)
        {
                if (*cp == '|' || *cp == '\0')
                {
                        mapDataPtr->names[num_names] = new char[cp-string + 1];
                        (void) strncpy(mapDataPtr->names[num_names], string, cp-string);
                        mapDataPtr->names[num_names][cp-string] = '\0';
                        num_names++;
                        if (*cp == '\0')
                                break;
                        string = cp+1;
                }
                cp++;
        }
        while (num_names < MAX_NAMES)
                mapDataPtr->names[num_names++] = NULL;

}


#if FALSE
WriteMapResource()
{

        char string[256];
        register short i,j;
        int    size;
        short  offset;
        Handle gHandle;
        Ptr    gPtr;

        char coastchar[5] = {'\0', 'S', 'N', 'E', 'W' };

        size = sizeof(short) +
                NUM_SECT*(1 + 4 + sizeof(unsigned char) + sizeof(char) + sizeof(char) + 3*sizeof(Point) + 3);
        gHandle = NewHandle(size);

        HLock(gHandle);
        gPtr = *gHandle;
        offset = 0;

        *((short *)gPtr) = NUM_SECT;
        gPtr += sizeof(short);
        offset += sizeof(short);

        for (i = 1; i <= NUM_SECT; i++)
        {
                size += strlen((*map)[i].name);
                gPtr = AdjustHandle(gHandle, size, offset);
                strcpy(gPtr, (*map)[i].name);
                gPtr += (strlen((*map)[i].name) + 1);
                offset += (strlen((*map)[i].name) + 1);

                BlockMove((*map)[i].shortname, gPtr, 4);
                gPtr += 3;
                *gPtr = ' ';
                gPtr++;
                offset += 4;

                *gPtr = (*map)[i].type;
                gPtr++;
                offset++;

                *gPtr = coastchar[(*map)[i].coast1];
                gPtr++;
                offset++;

                *gPtr = coastchar[(*map)[i].coast2];
                gPtr++;
                offset++;

                for (j = 0; j < 256; j++)
                        string[j] = '\0';
                nlist2string(string, (*map)[i].land);
                size += strlen(string);
                gPtr = AdjustHandle(gHandle, size, offset);
                strcpy(gPtr, string);
                gPtr += (strlen(string) + 1);
                offset += (strlen(string) + 1);

                for (j = 0; j < 256; j++)
                        string[j] = '\0';
                nlist2string(string, (*map)[i].sea1);
                size += strlen(string);
                gPtr = AdjustHandle(gHandle, size, offset);
                strcpy(gPtr, string);
                gPtr += (strlen(string) + 1);
                offset += (strlen(string) + 1);

                for (j = 0; j < 256; j++)
                        string[j] = '\0';
                nlist2string(string, (*map)[i].sea2);
                size += strlen(string);
                gPtr = AdjustHandle(gHandle, size, offset);
                strcpy(gPtr, string);
                gPtr += (strlen(string) + 1);
                offset += (strlen(string) + 1);
        }

        HUnlock(gHandle);

        AddResource(gHandle, 'MapD', 130, "\p");
        if (ResError() != noErr)
        {
                gErrNo = errFile;
                return false;
        }

        WriteResource(gHandle);

}

#endif

bool
ReadCountryResource(void)
{
        register short i, j;
        short  version;
        FSSpec filename;
        TextFile infile;
        char line[256];
        MakeFSSpec(filename, variantInfo->countries);
        if (!infile.Open(filename, false))
        {
                sprintf(gErrorString, "Couldn't open cnt file %s", variantInfo->countries);
                gErrNo = errBadCountryData;
                return false;
        }

        // read version and number of countries
        if (!infile.ReadLine(line, 255) || sscanf(line, "%hd", &version) != 1)
        {
                infile.Close();
                sprintf(gErrorString, "Couldn't read version at line %d", infile.GetCurrentLine());
                gErrNo = errBadCountryData;
                return false;
        }
        if (!infile.ReadLine(line, 255) || sscanf(line, "%hd", &NUM_COUN) != 1)
        {
                infile.Close();
                sprintf(gErrorString, "Couldn't read number of countries at line %d", infile.GetCurrentLine());
                gErrNo = errBadCountryData;
                return false;
        }

        if (NUM_COUN > MAX_COUN)
        {
                infile.Close();
                sprintf(gErrorString, "Too many countries - max is ", MAX_COUN);
                gErrNo = errBadCountryData;
                return false;
        }

        // read country data
        for (i = 1; i <= NUM_COUN; i++)
        {
                char patternName[256], colorName[256];
                // get country data from next line
                if (!infile.ReadLine(line, 255)
                        || sscanf(line, "%s %s %c %s %s",
                                        countries[i].name,
                                        countries[i].adjective,
                                        &countries[i].abbreviation,
                                        patternName,
                                        colorName)
                                         != 5)
                {
                        infile.Close();
                        sprintf(gErrorString, "Couldn't read country data at line %d", infile.GetCurrentLine());
                        gErrNo = errBadCountryData;
                        return false;
                }

                // home sectors
                countries[i].numCenters = 0;

				// other pre-assigned land sectors
				//countries[i].numLands = 0;

                // pattern and color
                char rsrcName[256];
                countries[i].patID = -1;
                j = 0;
                GetIndCString(rsrcName, kPatternStringsID, j);
                while (strlen(rsrcName) != 0)
                {
                        if (strcmp(rsrcName, patternName) == 0)
                        {
                                countries[i].patID = j;
                                break;
                        }
                        ++j;
                        GetIndCString(rsrcName, kPatternStringsID, j);
                }
                if (countries[i].patID < 0)
                {
                        infile.Close();
                        gErrNo = errBadCountryData;
                        sprintf(gErrorString, "Invalid pattern at line %d", infile.GetCurrentLine());
                        return false;
                }

                countries[i].colorID = -1;
				SubstituteCName(colorName);
                j = 0;
                GetIndCString(rsrcName, kColorStringsID, j);
                while (strlen(rsrcName) != 0)
                {
                        if (strcmp(rsrcName, colorName) == 0)
                        {
                                countries[i].colorID = j;
                                break;
                        }
                        ++j;
                        GetIndCString(rsrcName, kColorStringsID, j);
                }
                if (countries[i].colorID < 0)
                {
                        infile.Close();
                        gErrNo = errBadCountryData;
                        sprintf(gErrorString, "Invalid color at line %d", infile.GetCurrentLine());
                        return false;
                }
        }

        infile.Close();

        return true;
}



void
CleanMapData(void)
{
        register short i, j;
#if CONVERT
        long totalsize, memory;
        Size maxsize, grow;
#endif

/*        memory = FreeMem(); */

        for (i = 1; i <= NUM_SECT; i++)
        {
                for (j = 0; j < MAX_NAMES; j++)
                {
                        if (map[i].names[j] != NULL)
                                delete [] map[i].names[j];
                        map[i].names[j] = NULL;
                }
                if (map[i].land != NULL)
                        cleanneighbors(&(map[i].land));
                if (map[i].sea1 != NULL)
                        cleanneighbors(&(map[i].sea1));
                if (map[i].sea2 != NULL)
                        cleanneighbors(&(map[i].sea2));
				if (map[i].sea3 != NULL)
                        cleanneighbors(&(map[i].sea3));


                if (map[i].rgn != NULL)
                        delete map[i].rgn;
                map[i].rgn = NULL;
                if (map[i].coast1 != C_NONE)
                {
                        if (map[i].coast1_rgn != NULL)
                                delete map[i].coast1_rgn;
                        map[i].coast1_rgn = NULL;
                        if (map[i].coast2_rgn != NULL)
                                delete map[i].coast2_rgn;
                        map[i].coast2_rgn = NULL;
						if (map[i].coast3_rgn != NULL)
                                delete map[i].coast3_rgn;
                        map[i].coast3_rgn = NULL;
                }
        }

        NUM_SECT = 0;

        for (i = 0; i <= NUM_COUN; i++)
        {
                if (units != NULL)
                        cleanunits(&units[i]);
                if (supplies != NULL)
                        cleansupplies(&supplies[i]);
                if (orders != NULL)
                        cleanorders(orders[i]);
        }

    dslglist.Clean();
    dislodges = 0;

    deletegraph();

        CleanIcons();

        NUM_COUN = 0;

/*        printf("CleanMapData: %ld\n", FreeMem() - memory); */
}



/*
 * map-based parsing routines
 */

char *
string2coast(CoastType *coast, char *s)
{
        register char *next_word;
        register short i;

        *coast = C_NONE;
        if (s == NULL)
                return NULL;

        for (i = 1; i <= 4; i++)
        {
                if ((next_word = samestring(s, jcoast[i])) != NULL)
                {
                        *coast = (CoastType)i;
                        return skipspace(next_word);
                }
                if ((next_word = samestring(s, scoast[i])) != NULL)
                {
                        *coast = (CoastType)i;
                        return skipspace(next_word);
                }
                if ((next_word = samestring(s, lcoast[i])) != NULL)
                {
                        *coast = (CoastType)i;
                        return skipspace(next_word);
                }
                if ((next_word = samestring(s, sscoast[i])) != NULL)
                {
                        *coast = (CoastType)i;
                        return skipspace(next_word);
                }
                if ((next_word = samestring(s, slcoast[i])) != NULL)
                {
                        *coast = (CoastType)i;
                        return skipspace(next_word);
                }
        }
        return s;
}

char *
string2country(short *country, char *s)
{
        register short i;
        register char *next_word, *match;

        if (s == NULL)
                return NULL;

        next_word = s;
        *country = C_NONE;
        for (i = 1; i <= NUM_COUN; i++)
        {
                if ((match = samestring(s, countries[i].name)) != NULL)
                {
                        if (*country == 0)
                        {
                                *country = i;
                                next_word = match;
                        }
                        else if (*country == i)
                        {
                                if (next_word < match)
                                        next_word = match;
                        }
                        else
                                return s;
                }
                if ((match = samestring(s, countries[i].adjective)) != NULL)
                {
                        if (*country == 0)
                        {
                                *country = i;
                                next_word = match;
                        }
                        else if (*country == i)
                        {
                                if (next_word < match)
                                        next_word = match;
                        }
                        else
                                return s;
                }
        }

        s = skipspace(next_word);
        return s;
}

short
char2country(char c)
{
        register short i;

        for (i = 1; i <= NUM_COUN; i++)
        {
                if (countries[i].abbreviation == c)
                        return i;
        }

        return C_NONE;
}

char *
string2sector(short *sector, char *s)
{
        register short i, j;
        register char *next_word = s, *match;

        *sector = 0;
        for (i = 1; i <= NUM_SECT; i++)
        {
                for (j = 0; j < MAX_NAMES; j++)
                {
                        if (map[i].names[j] != NULL && (match = samestring(s, map[i].names[j])) != NULL)
                        {
                                if (*sector == 0 || next_word < match)
                                {
                                        *sector = i;
                                        next_word = match;
                                }
                                else if (*sector != i && next_word == match)
                                {
                                        *sector = 0;
                                        return s;
                                }
                        }
                }
        }

        return skipspace(next_word);
}


CoastType
char2coast(char coastdata)
{
        switch(coastdata)
        {
        case 'N':
        case 'n':
                return C_NO;
                break;

        case 'S':
        case 's':
                return C_SO;
                break;

        case 'E':
        case 'e':
                return C_EA;
                break;

        case 'W':
        case 'w':
                return C_WE;
                break;

        default:
                return C_NONE;
                break;
        }
}

char
coast2char(CoastType coast)
{
        switch(coast)
        {
        case C_NO:
                return 'n';
                break;

        case C_SO:
                return 's';
                break;

        case C_EA:
                return 'e';
                break;

        case C_WE:
                return 'w';
                break;

        default:
                return '*';
                break;
        }
}

/* returns relevant Neighbor if unit can get from start to finish */
/* a little too picky right now about defining coastlines -- could be
 * more lenient (i.e. Mar-Spa really only means Mar-Spa(sc)) */
Neighbor findneighbor (UnitType unit, short start, CoastType s_coast, short finish, CoastType f_coast,
                    short supp)
{
	 Neighbor np;

     switch (unit) {
     case U_ARM:
          return getneighbor(map[start].land, finish, C_NONE, supp);

     case U_FLT:
          if (s_coast == 0)
               if (map[start].coast1 != 0)
                    return NULL;
               else
                    return getneighbor(map[start].sea1, finish, f_coast, supp);
          else if (s_coast == map[start].coast1)
               return getneighbor(map[start].sea1, finish, f_coast, supp);
          else if (s_coast == map[start].coast2)
               return getneighbor(map[start].sea2, finish, f_coast, supp);
		  else if (s_coast == map[start].coast3)
               return getneighbor(map[start].sea3, finish, f_coast, supp);
          else
               return NULL;

          break;

     case U_WNG:
		  np = getneighbor(map[start].land, finish, C_NONE, supp);	
		  if (np != NULL) 
			  return getneighbor(map[start].land, finish, C_NONE, supp);
		  
		  np = getneighbor(map[start].sea1, finish, C_NONE, supp);	
		  if (np != NULL) 
			  return getneighbor(map[start].sea1, finish, C_NONE, supp);
		  
		  np = getneighbor(map[start].sea2, finish, C_NONE, supp);	
		  if (np != NULL) 
			  return getneighbor(map[start].sea2, finish, C_NONE, supp);
		  
		  return getneighbor(map[start].sea3, finish, C_NONE, supp);	
		  
		  break;

     default:
          return NULL;
     }
}


/* returns true if unit can get from start to finish */
/* a little too picky right now about defining coastlines -- could be
 * more lenient (i.e. Mar-Spa really only means Mar-Spa(sc)) */
int neighborsector (UnitType unit, short start, CoastType s_coast, short finish, CoastType f_coast,
                    short supp)
{
     return (findneighbor(unit, start, s_coast, finish, f_coast, supp) != NULL);
}

//	
// returns true if link between sectors exists and is from the atlantic region to the Suez region
//	
bool	
ToSuez( UnitType unit, short start, CoastType s_coast, short finish )	
{	
 	Neighbor np = 0;	

 	np = findneighbor(unit, start, s_coast, finish, C_NONE, true);	
 	        
	if (np)	
	{
 	            return (istosuez(np));	
	}
 	else	
	{
 	            return false;	
	}
}

//	
// returns true if link between sectors exists and is from the Suez region to the atlantic region
//	
bool	
FromSuez( UnitType unit, short start, CoastType s_coast, short finish )	
{	
 		Neighbor np = 0;	

 	np = findneighbor(unit, start, s_coast, finish, C_NONE, true);	
 	        
	if (np)	
	{
 	            return (isfromsuez(np));	
	}
 	else	
	{
 	            return false;	
	}
}

//	
// returns true if link between sectors exists and is "isthmus"	
//	
bool	
CrossesIsthmus( UnitType unit, short start, CoastType s_coast, short finish )	
{	
 		Neighbor np = 0;	

 	np = findneighbor(unit, start, s_coast, finish, C_NONE, true);	
 	        
	if (np)	
	{
 	            return (isisthmus(np));	
	}
 	else	
	{
 	            return false;	
	}
}

int unitthere (Sector sect, short country, UnitType unit, CoastType coast)
{
     return (inulist(units[country], unit, sectindex(sect), coast));

}

void
SectorToPoint(Point *thePoint, short sector, CoastType coast)
{
        if (coast)
        {
                if (coast == map[sector].coast1)
                        *thePoint = map[sector].coast1_loc;
                else if (coast == map[sector].coast2)
                        *thePoint = map[sector].coast2_loc;
                else if (coast == map[sector].coast3)
                        *thePoint = map[sector].coast3_loc;
     			else
                        *thePoint = map[sector].land_loc;
        }
        else
                *thePoint = map[sector].land_loc;
}

/* return index to sector under Point in theWindow */
short
FindSector(short h, short v, CoastType *coast)
{
        register short i;
/*CONVERT (if necessary)
        RECT windowRect;

        // make sure we're really in the body of the window
        GetWindowRect(theWindow, windowRect);
        windowRect.right -= 15;
        windowRect.bottom -= 15;
        if (!PtInRect(where, &windowRect))
                return 0;
*/
        for (i = 1; i <= NUM_SECT; i++)
                if (map[i].rgn->IsPointInside(h, v))
                {
                        if (map[i].coast1 != C_NONE)
                        {
                                if (map[i].coast1_rgn->IsPointInside(h, v))
                                        *coast = map[i].coast1;
                                else if (map[i].coast2_rgn->IsPointInside(h, v))
                                        *coast = map[i].coast2;
                                else if (hasthirdcoast(&map[i]))
								{
									 if (map[i].coast3_rgn->IsPointInside(h, v))
                                        *coast = map[i].coast3;
								}
                                else
                                        *coast = C_NONE;
                        }
                        else
                                *coast = C_NONE;
                        return i;
                }

        return 0;
}


//
// make units and map consistent
//
void
unitmapconsist()
{
        register short i;
        // clear out all units
        for (i = 1; i <= NUM_SECT; i++)
        {
                map[i].unit = NULL;
        }

        for (i = 1; i <= NUM_COUN; i++)
        {
                addunits2map(units[i]);
        }
}

short 
namelen()
{
	unsigned short i, max;

	max = 0;
    for (i = 1; i <= NUM_COUN; i++)
    {
            if (strlen(country2string(i)) > max) 
                    max = strlen(country2string(i));
    }

    return max;
}

Canal newcanal()
{
	register short i;
	
	if (canalalloc == NULL)
		alloccanal();
	
	for (i = 0; i < MAX_CANALS; i++)
	{
		if (canalalloc[i].num == 0)
		{
			canalalloc[i].gate1 = NULL;
			canalalloc[i].gate2 = NULL;
			canalalloc[i].center = NULL;
			canalalloc[i].resolution = R_NONE;
			canalalloc[i].sc_order = NULL;
			canalalloc[i].next = NULL;
			totalcanalalloc++;
			return canalalloc+i;
		}
	}

	return NULL;
}

void 
ClearCanals()
{
	Canal c1 = NULL;
	Canal c2 = NULL;

	if (!(canalalloc == NULL))
	{
		c1 = startcanal();
	}
	while (iscanal(c1))
	{
		c2 = c1->next;
		if (cnum(c1) != 0) DisposeCanal(c1);
		c1 = c2;
	}
	//dealloccanal();
}


void 
DisposeCanal(Canal c)
{
	totalcanalalloc--;
	c->num = 0;
	c->gate1 = NULL;
	c->gate2 = NULL;
	c->center = NULL;
	c->resolution = R_NONE;
	c->sc_order = NULL;
	c->next = NULL;
}

/* add canal to canallist */
/* returns 1 if worked, 0 otherwise */
int
addcanal(Canal *c, Sector s, CanalType ctype, short num)
{
     Canal c1 = *c;
	 Canal c2 = NULL;

     while (iscanal(c1) && cnum(c1)>0) {
          if (cnum(c1) == num)
               break;
		  c2 = c1;
          c1 = nextcanal(c1);
     }

	if (iscanal(c1))
	{
     /* canal exists already */
		if (cnum(c1) == 0)
		{
			c1->num = num;
			totalcanalalloc++;
		}
		switch (ctype)
		{
		case C_GATE:
			if (cgate1(c1) == NULL)
			{
				c1->gate1 = s;              
				return 1;
			}
			else if (cgate2(c1) == NULL)
			{
				c1->gate2 = s;              
				return 1;
			}
			else
			{
				return 0;
			}
			break;
		
		case C_CENTER:
			if (ccenter(c1) == NULL)
			{
				c1->center = s;              
				return 1;
			}
			else
			{
				return 0;
			}
			break;
		
		default: 
			return 0;
		}
    }
	else if (ctype == C_GATE || ctype == C_CENTER)
	{
        if (!(c1 = newcanal()))
		{
                  return 0;
		}

		c1->num = num;
		if (ctype == C_GATE)
		{
			c1->gate1 = s;
		}
		else if (ctype == C_CENTER)
		{
			c1->center = s;
		}
		c1->next = *c;
		*c = c1;
		return 1;
	}
    return 0;
}

Canal getcanal(Sector s)
{
	Canal c1;

	if (belongstocanal(s) || iscanalcenter(s))
	{
		for (c1 = startcanal(); iscanal(c1); nextcanal(c1))
		{
			if (s == cgate1(c1) 
				|| s == cgate2(c1)
				|| s == ccenter(c1))
			{
				return c1;
			}
		}
	}
	return NULL;
}

short getcanalnumber(Sector s, CanalType ctype)
{
	Canal c1;

	if (belongstocanal(s) || iscanalcenter(s))
	{
		for (c1 = startcanal(); iscanal(c1); nextcanal(c1))
		{
			if (((s == cgate1(c1) || s == cgate2(c1)) && ctype == C_GATE)
					|| (s == ccenter(c1) && ctype == C_CENTER))
			{
				return cnum(c1);
			}
		}
	}
	return 0;
}

Sector getcanalsector(Sector s)
{
	Canal c1;

	if (iscanalcenter(s))
	{
		return s;
	}
	else if (belongstocanal(s))
	{
		for (c1 = startcanal(); iscanal(c1); nextcanal(c1))
		{
			if (s == cgate1(c1) || s == cgate2(c1))
			{
				return ccenter(c1);
			}
		}
	}
	return NULL;
}

short getcanalowner(Canal c)
{
	register short i;
    
    for (i = 1; i <= NUM_COUN; i++)
    {
		if (inslist(supplies[i],sectindex(ccenter(c))))
		{
			return i;
		}
		if (inslist(lands[i],sectindex(ccenter(c))))
		{
			return i;
		}
	}
	return 0;
}

bool issamecanal(Sector s1, Sector s2)
{
	Canal c1;

	for (c1 = startcanal(); iscanal(c1); nextcanal(c1))
	{
		if ((s1 == cgate1(c1) && s2 == cgate2(c1))
				|| (s1 == cgate2(c1) && s2 == cgate1(c1)))
		{
			return true;
		}
	}
	return false;
}

void 
ClearCanalOrders()
{
	Canal c1;

	for (c1 = startcanal(); iscanal(c1); nextcanal(c1))
	{
		corder(c1) = NULL;
	}
}

Order
findorderbysector(Orderlist olist, UnitType unit, Sector sector)
{
     Order cp;

     for (cp = olist; cp-olist < MAX_ORD && isorder(cp); nextorder(cp))
     {
             if ((unit == C_NONE || ounit(cp) == unit) && getsector(map,ostart(cp)) == sector)
                          return cp;
     }
          return NULL;
}

