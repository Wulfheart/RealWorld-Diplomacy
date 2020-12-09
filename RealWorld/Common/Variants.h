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
struct variant 
{
	short version;
	char directory[128];
	char name[128];
	char map[128];
	char countries[128];
	char game[128];
	char bwmap[128];
	char colormap[128];
	char polymap[128];
	char info[128];
	unsigned int flags;
};

// Buildmode Flags
#define VF_ABERRATION 	0x1
#define VF_CHAOS  		0x2
#define VF_HANSEBUILD   0x10
// Icon Sizing Flag
#define VF_LARGEICONS   0x8
// Wing Inclusion Flag
#define VF_HASWINGS		0x4
// Canal Usability Flags
#define VF_ACC			0x20
#define VF_ACS			0x40
#define VF_ACR			0x80
#define VF_ACWU			0x100
// Canal Permission Control Flags
#define VF_CPC_STAY		0x200
#define VF_CPC_ANYUNIT	0x400
#define VF_CPC_OWNER	0x800
// Hanse Variant Flag
#define VF_HANSE        0x1000
// Paradox Resolution Flag
#define VF_SZYKMAN      0x2000
// Trafo Rule Flag
#define VF_TRAFO        0x4000
// Emergency Rule Flag
#define VF_EMR          0x8000
// Buildmode Constants
#define VBM_STANDARD	0
#define VBM_ABERRATION	1
#define VBM_CHAOS		2
#define VBM_HANSE		3
// Canal Permission Mode Constants
#define VCP_NONE		0
#define VCP_STAY		1
#define VCP_HOLD		2
#define VCP_ANYUNIT		3
#define VCP_OWNER		4

extern struct variant *variantInfo;
extern short gBuildMode;
extern short gCanalPermissionMode;
extern bool gHasWings;
extern bool gHasLargeIcons;
extern bool gIsHanse;
extern bool gAllowsCanalConvoys;
extern bool gAllowsCanalSupports;
extern bool gAllowsCanalRetreats;
extern bool gAllowsCanalWingUsage;
extern bool gAllowsTrafo;
extern bool gHasEmergencyRule;

int InitVariants(void);
bool SetVariant(short index);
bool SetVariantByName( char* name );
bool ChangeVariant(short index);
bool NewVariant(short index);
void EvaluateFlags(unsigned int flags);
unsigned int SetFlags(void);
int string2variant(char *variant);

#define NoCanalPermissionCheck (gCanalPermissionMode == VCP_NONE)

