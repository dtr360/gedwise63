////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise Verson 6.0 - 6.2
//
// FILE:          Defines.c
//
// AUTHOR:        Daniel T. Rencricca: October 15, 2004
//
// DESCRIPTION:   This file includes define statements common to multiple
//                GedWise project files.
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2005 Battery Park Software Corporation.
// All rights reserved.
// Starting Size: 100,674 bytes
// Ending Size:   112,677 (version 6.0 - Released Nov 15, 2004)
// Ending Size:   112,645 (version 6.1 - Released Jun 1,  2005)
// Ending Size:   113,198 (version 6.2 - Released Jan 15, 2006)
// Ending Size:   113,164 (version 6.3 - Released )
////////////////////////////////////////////////////////////////////////////////////
// Changes Since Release on Nov 15, 2004

//	12/22/2004 Improved Individual List screen to allow "." so that when user enters
//    		      the "x" character, it will be more easily recognized.
//	12/22/2004 Fixed RelCalcNum2 to stop reinit when starting up GedWise.
// 03/28/2005 Reduce Minimum OS Version to 3.1.
// 05/18/2005 Fixed the NavKeyHit function so Nav buttons will work with Treo Devices.
// 06/06/2005 Fixed cPalmPath constant to remove last "/" so that the directory can
//				        be found properly by  VFSFileOpen function. (size 112,680K)
// 12/12/2005 Fixed DetailViewHandleVirtual to work with TX handhelds.
// 12/20/2005 Added SetNavFocusRing to handled focus ring problems.
// 12/20/2005 Added test for sysFtrNumFiveWayNavVersion feature in startup function.
// 12/20/2005 Fixed keyDownEvent events in DBListHandleEvent, NoteViewHandleEvent
//				        & AliaListHandleEvent functions to handle NavKeyHit. 

////////////////////////////////////////////////////////
// VERSION SETTINGS 												//
////////////////////////////////////////////////////////
//#define GREMLINS  	// set for Gremlins testing
//#define BETA  		// set for Beta version	
//#define REGISTERED	// do not define
////////////////////////////////////////////////////////

#ifdef GREMLINS
 #undef ERROR_CHECK_LEVEL
 #define ERROR_CHECK_LEVEL ERROR_CHECK_FULL
 #define DO_NOT_ALLOW_ACCESS_TO_INTERNALS_OF_STRUCTS
#else
 #undef ERROR_CHECK_LEVEL
 #define ERROR_CHECK_LEVEL ERROR_CHECK_NONE
 #pragma macsbug off
#endif

#ifdef GREMLINS  
 //#define SCROLL_TEST  // turns left/right scrollers on all the time.
 //#define GREMLINS_NO_SEARCH // exclude GedWise db searches to save time
 //#define GREMLINS_MEM_CHK  // do extensive memory checking (really slow on old devices)
 #define EMULATOR
#endif

// Update codes used to determine how the Event List view should be redrawn.
#define updateRedrawAll       0x01
#define updateViewReInit      0x02
#define NO_REC						0xffff
#define NO_REC_LONG				0xffffffff
#define FAMI_DLM  				'@'
#define SOUC_DLM  				'S'
#define ALIA_DLM					'A'
#define DBaseCountMax			100 // max number of databases at one time
#define MaxRCLvls					25
#define DV_FRM_BOUNDS			{0, 61, 160, 99} // incl. borders

// Define Standard Colors
#define	RGB_CT_BLACK			{0x00, 0x00, 0x00, 0x00} // as black as possible
#define	RGB_CT_GREEN			{0x00, 0x00, 0xAA, 0x00} // as green as possible
#define	RGB_CT_BLUE				{0x00, 0x00, 0x00, 0xAA} // as blue as possible

// Define OS versions
#ifdef EMULATOR
#define ROM_MIN_VER	sysMakeROMVersion(3,1,0,sysROMStageDevelopment,0) // minimum version
#define ROM_EC_VER	sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0) // expansion card version
#define ROM_35_VER 	sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0) // Palm OS version 3.5
#define ROM_60_VER	sysMakeROMVersion(6,0,0,sysROMStageDevelopment,0) // Palm OS version 3.5
#else
#define ROM_MIN_VER	sysMakeROMVersion(3,1,0,sysROMStageRelease,0) 
#define ROM_EC_VER	0x04000000
#define ROM_35_VER	sysMakeROMVersion(3,5,0,sysROMStageRelease,0)
#define ROM_60_VER	0x06000000
#endif

// Extract the bit at position index from bitfield.  0 is the high bit.
#define BitAtPosition(pos)             ((UInt16)1 << (pos))
#define GetBitMacro(bitfield, index)	((bitfield) & BitAtPosition(index))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define odd(a)	(((a) % 2 != 0) ? (1) : (0))

////////////////////////////////////////////////////////////////////////////////////
// Registration Codes
////////////////////////////////////////////////////////////////////////////////////
// Registration Code Format (version 1.2):	GV-Y9X315a
// Registration Code Format (version 2.0):	GB.X8Z741c
// Registration Code Format (version 3.0):	RACW6B414G 
// Registration Code Format (version 4.0):	237D93855C
// Registration Code Format (version 4.0b):	BETA000000
// Registration Code Format (version 5.0):	88535483195
// Registration Code Format (version 5.0):	72746393328
// Registration Code Format (version 5.0):	72749363911
// Registration Code Format (version 5.0):	88534492850
// Registration Code Format (version 6.0):	64992291184
// Registration Code Format (version 6.0):	64997262933
// Registration Code Format (version 6.0):	33855972993
// Registration Code Format (version 6.0):	33859996762

// (C4 x C6) - (C4 + C6) = C7C8
// 9*3 = 27 - 9+3 = 15
// 8*7 = 56 - 8+7 = 41
// 6*4 = 24 - 6+4 = 14
// 9*8 = 72 - 9+8 = 55
// (C4 x C6) - (C4 + C6) + (C9-C10) = C7C8     Vers 5.0
// (5*8) = 40 - (5+8) = 27 + (9-5) = 31
// (6*9) = 54 - (6+9) = 39 + (2-8) = 33
// (9*6) = 54 - (9+6) = 39 + (1-1) = 39
// (4*9) = 36 - (4+9) = 23 + (5-0) = 28

// (C4 x C6) - (C4 + C6) + (C9-C10) = C7C8     Vers 6.0
// (2*9) = 18 - (2+9) =  7 + (8-4) = 11
// (7*6) = 42 - (7+6) = 29 + (3-3) = 29
// (5*7) = 35 - (5+7) = 23 + (9-3) = 29
// (9*9) = 81 - (9+9) = 63 + (6-2) = 67

#define CHK_TOT_CHARS	11
#define CHK_CHR_0a		'6'
#define CHK_CHR_1a		'4'
#define CHK_CHR_2a		'9'
#define CHK_CHR_3a		'9'
#define CHK_CHR_5a		'2'

#define CHK_CHR_0b		'3'
#define CHK_CHR_1b		'3'
#define CHK_CHR_2b		'8'
#define CHK_CHR_3b		'5'
#define CHK_CHR_5b		'9'
