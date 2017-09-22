////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise 6.0
//
// FILE:          AppMisc.c
//
// AUTHOR:        Daniel T. Rencricca: Oct 15, 2004
//
// DESCRIPTION:   Header file for AppMisc.c
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2004 Battery Park Software ("BPS").  
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	UInt16	person;
} QueueHistType;

typedef struct {
	UInt16	person;
	UInt16	level;
} DescListType;

typedef struct {
	UInt16	husbRecN; // record in IndiDb
	UInt16	wifeRecN; // record in IndiDB
	UInt16	chilRecN; // record in IndiDB
	UInt16	parent; // 0 = father, 1 = mother
} RCListType;

typedef struct {
	UInt16	iPos;
	UInt16	jPos;
} RCHistType;

#if defined (BETA) || defined (GREMLINS)
 #define LongSplashDelay   2	 		// 2 seconds to display splash scrn
 #define TrialPeriod			0x3C		// 60 days
#else
 #define LongSplashDelay   5	 		// 5 seconds to display splash scrn
 #define TrialPeriod       0x1E 		// 30 days
#endif

#define SecondsPerDay      0x00015180 	//86400 // seconds per day
#define ShortSplashDelay	.50  	// 1/2 seconds to display splash scrn


// Relationship Calculator Defines
#define RC_ARR_SZ				32767 // 15 generations of ancestors (zero-based)
#define RC_ARR_GEN			14    // 15 generations of ancestors (zero-based)
#define RC_HIST_MAX			50		// max relationships per search
#define RC_MAX_GEN_FAST		14 	// 15 max generations for fast search
#define RC_MAX_GEN_SLOW		24 	// 25 max generations for slow search

// RC Array Size translation to generation number
//15	16383	- 32766  : 16384
//14	8191 	- 16382  : 8192
//13	4095	- 8190	: 4096
//12	2047	- 4094	: 2048
//11	1023	- 2046	: 1024
//10	511	- 1022	: 512
//9	255	- 510		: 256
//8	127	- 254		: 128
//7	63		- 126		: 64
//6	31		- 62		: 32
//5	15		- 30		: 16
//4	7		- 14		: 8
//3	3		- 6		: 4
//2	1 	   - 2		: 2
//1	0					: 1 IndiN

// Descendancy Chart and Ancestry Chart Defines
#define CHART_FONT		stdFont
#define CHART_FONT_SZ	11
#define ANCST_FONT		stdFont
#define ANCST_FONT_SZ	12

#define QHIST_MAX	 		100
#define DL_MAX 			250

// Relationship Calc Defines
#define SELECT_IND		"Select Individual"
#define SELECT_IND_LEN	17

// Preference Defines
#define PERSONS_W_CHIL 	"Persons having children"
#define MARRIED_NO_CHIL	"Persons married and childless"

#define RC_REL_STR		"Relationship #"
#define RC_REL_STR_LEN	14


#define SCRDEL_STR		"Screen Delay:"
#define SCRDEL_STR_LEN	13

#define TRLDAYS_STR		"Trial Days Remaining: "

////////////////////////////////////////////////////////////////////////////////////
//	Internal Function Prototypes																	 //
////////////////////////////////////////////////////////////////////////////////////
static void		SetFocus (const UInt16 controlID);
static void*	GetObjectPtr (const UInt16 objectID);
static void		GetObjectBounds (const UInt16 objectID, RectanglePtr rect);
static void 	ShowObject(const UInt16 objectID, const Boolean showObj);
static Int16	GetControlValue (const UInt16 controlP);
void				SetControlValue (const UInt16 controlID, const Boolean value);
void 				EraseRectangleObject (const UInt16 objectId);

void 				DrawScreenLines (const UInt16 whichForm);
static void 	DrawObjectFrame (const UInt16 objectId, const Int16 adjXAmt, const Int16 setYAmt, const Boolean grayFrame);
void				DrawCharsColorI (const Char* chars, RGBColorType colorRGB, Coord x, Coord y);
Boolean			RefFinderStr (UInt16 keyNumber, Char key, Char* field, Char* recordP);
UInt16			RefCounter (Char key, Char* field);						
						
static void 	RegisterCodeEntry (void);
Boolean 			RegisterHandleEvent (EventPtr event);

static Boolean ShowSplash (Boolean doDelay);
static void		SplashGotoForm (void);
Boolean 			SplashHandleEvent (EventPtr event);

static void 	JumpListDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
Boolean 			JumpListHandleEvent (EventPtr event);

static void 	PreferencesSave(void);
static void 	PreferencesInit (void);
Boolean 			PreferencesHandleEvent (EventPtr event);

static void 	AncestryDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void 	AncestrySetRow (TablePtr table, Int16 row1, Int16 row2);
static void 	AncestryLoadTable (void);
Boolean 			AncestryHandleEvent (EventPtr event);

static void 	DescendancyDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void 	DescendancyScroll (WinDirectionType direction);
static void 	DescendancyLoadTable (void);
static Boolean	DescendancyLoadData (void);
static void 	DescendancyChildsChildren (UInt16 indiRecN);
static void 	DescendancyInit (void);
Boolean 			DescendancyHandleEvent (EventPtr event);

UInt16 			DaysInMon (UInt16 month, UInt16 year);
static Boolean DateCalc (void);
static void 	DateCalcCheck (EventPtr event);				
static void 	DateCalcInitForm (Boolean openInit);
Boolean 			DateCalcHandleEvent (EventPtr event);

static Char*	RetNumEnd (UInt16	number);
static void 	RelCalcShow (UInt16 i, UInt16 j, UInt16 sex);
static void		RelCalcGetParents (UInt16 indiRecN, UInt16 *husbRecN, UInt16 *wifeRecN);
static void		RelCalcInitIndi (UInt16* indiRecN, UInt16 buttonID);
static Boolean	RelCalcIncr (RCListType* RCList, Int16* pos);
static Boolean	RelCalcInitArr (UInt16 indiN);
static Boolean	RelCalc (UInt16 indiN1, UInt16 indiN2, Int16* i, Int16* j, UInt16* maxI, UInt16* MaxJ, Int16 *y, Boolean* cont, Boolean swapped);
static void 	RelCalcSearch (EventPtr	event);
static void 	RelCalcInit (void);
Boolean 			RelCalcHandleEvent (EventPtr event);