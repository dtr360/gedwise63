////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise Version 6.0
//
// FILE:          AppMain.h
//
// AUTHOR:        Daniel T. Rencricca: October 15, 2004
//
// DESCRIPTION:   Header File for AppMain routines
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2005 Battery Park Software Coroporation.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Global Defines
////////////////////////////////////////////////////////////////////////////////////
#define APP_PREF_VERS_NO			0x06
#define APP_PREF_ID	            0x06
#define APP_DB_VERS					0x06  ///// MUST MATCH GEDWISE PC PROGRAM //////
#define APP_CREATOR              'Rend'
#define APP_CREATOR_STR				"Rend"
#define APP_DB_TYPE					'gedw'

#define JUMP_MAX						11 // max jump list array
#define USER_DB_NAME_SZ				6  // size (characters) of user db name
#define FULL_DB_NAME_LEN			11 // length (characters) of db name (xxxxxx-Rend)
#define DBLIST_FONT					stdFont
#define DBLIST_FONT_SZ				11
#define TITLE_FONT 					boldFont  // user cannot change

#define EXPORT_DATA_MAX_SZ 		3000 // max possible size of Export Memo

#define SPACE_BEF_LSPAN  			3 
#define LSPAN_COL_WIDTH				50

// Defines for Detail View Windows
#define DV_LINES_MAX					300 // max lines for Detail View Gadget
#define MARGIN_WID	            2   // margin size (pixels)
#define HEAD_WID   		         32  // pixels to indent text aft header
#define SUB_HEAD_WID             8   // pixels to indent line aft header line

// Defines for IndiSummForm
#define INDI_SUMM_EVEN_INDENT		24  // indentation for event info (pixels)


// Defines for Misc Strings & String Lengths
#define UNK_STR						"Unknown"
#define UNK_STR_LEN					7
#define NO_CHIL_STR					"No Child Data"
#define NO_CHIL_STR_LEN				13
#define NO_EVEN_STR					"No Event Data"
#define NO_EVEN_STR_LEN				13
#define NO_FAMI_STR					"No Families"
#define NO_FAMI_STR_LEN				11
#define NO_DATE_STR					"No Date"
#define NO_REC_STR					"Record Not Found!!"
#define NOTE_SEP_STR					"------------  Note  ------------"
#define NOTE_STR 						" Note  "
#define FNAME_STR						"%s/%s-%s.pdb"
#define DBNAME_STR					"%s-%s"
#define SHORTENED_FLD_STR			"..."
#define SHORTENED_FLD_LEN			3
#define FLD_SEP_STR	 				", "
#define FLD_SEP_LEN					2
#define PALM_PATH_STR				"/PALM/Launcher"
#define PALM_PATH_LEN				15 // length much match PALM_PATH_STR
#define SCIT_HEAD_STR				"Citation %i of %i"

// Defines for Event description array
#define TOT_EVEN_L 					59  // MUST MATCH EvenDesc array!
#define EVEN_POS						45  // position of 'Event' description in EvenDescL
#define EVEN_NOTE_POS				46	 // position of 'Note' description in EvenDescL
#define EVEN_BIRT_STR				"1" // position of 'Birth' description in EvenDescS
#define EVEN_DEAT_STR				"4" // position of 'Death' description in EvenDescS

////////////////////////////////////////////////////////////////////////////////////
//	Global Defines
////////////////////////////////////////////////////////////////////////////////////
// Scroll rate values
#define scrollDelay					1
#define scrollAcceleration			2
#define scrollSpeedLimit			5

////////////////////////////////////////////////////////////////////////////////////
// Record Structure Defines
// These MUST match definitions in GedWise PC program
////////////////////////////////////////////////////////////////////////////////////
#define XREF_LEN    					21  	// max size of a cross reference
#define NREF_LEN    					8   	// max size of numeric cross reference
#define DB_REC_MAX					40	  	// number of records packed in mega record

////////////////////////////////////////////////////////////////////////////////////
// Internal Structures
////////////////////////////////////////////////////////////////////////////////////

typedef enum { // Preference Fields. MUST MATCH order of CheckBoxes on Pref 1 Screen
	EventsFirst,		// show child info first
	HideSplash,			// hide the splash screen
	ExcludeFind,		// don't search app db's with global find
	ReturnLast,			// return to last Indiv. after exit
   SepNoteView,		// always use seperate window for notes
   FullNoteScreen,	// always display Notes in large window
   ScrollOneI,			// scroll only 1 line at a time in ind list scrn
   ScrollOneD,			// scroll only 1 line at a time in ind desc scrn
	DisplayBlank,		// display "Unknown" for missing bir/dth
   HideJump,			// hide jump list button
   UseEvenDesc,		// use event description if not date on Event List
   
   LastFileOnCrd,		// true if db is on Expansion Card
   Registered,      	// true if user has registered copy
   TotPrefFlds
	} PrefFlds;

#define LAST_PREF1_CHKBX	UseEvenDesc  // MUST BE LAST USER PREF IN ABOVE FIELDS
#define FIRST_PREF1_RSC		Pref1StartEventsCheckbox

typedef enum { // Preference Fonts.  DO NOT CHANGE THE ORDER OF THESE
	IndiListFont,
	IndiSummFont,
   DetailViewFont,
   IndiSummNameFont,
   TotPrefFnts
	} PrefFontFlds;

#define LAST_PREF2_FNT		DetailViewFont
#define FIRST_PREF2_FNT		Pref2IndiListstdFontPushButton

// Preference Structure
typedef struct {
   LocalID			dbIDOld;  // used in Global Search routine
   UInt32      	InstallDate;
   Char				LastFileName[USER_DB_NAME_SZ+1];
   UInt16 			LastFileLoc;
	UInt16			LastRecN;  // last person viewed before exiting
	UInt16			RelCalcRecN2; // rec number of individual 2
	UInt16			Jump[JUMP_MAX]; // last 10 individuals visited
	FontID      	PrefFonts[TotPrefFnts];
   Boolean			PrefFlds[TotPrefFlds];
   RGBColorType	Color1;
   RGBColorType	Color2;
} AppPreferenceType;

 // GedWise Databases
typedef enum {
	IndiDB,
	EvenDB,
	SouCDB,
	RepCDB,
	FamiDB,
	ChilDB,
	SourDB,
	RepoDB,
	NoteDB,
	DBCount
	} CmbDBases;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Generic DB Record --
////////////////////////////////////////////////////////////////////////////////////

typedef union {
	struct {
		unsigned field16    	:1;
		unsigned field15    	:1;
		unsigned field14		:1;
		unsigned field13  	:1;
		unsigned field12  	:1;
		unsigned field11  	:1;
		unsigned field10  	:1;
		unsigned field09  	:1;
		unsigned field08  	:1;
		unsigned field07  	:1;
		unsigned field06  	:1;
		unsigned field05  	:1;
		unsigned field04  	:1;
		unsigned field03  	:1;
		unsigned field02  	:1;
		unsigned field01  	:1;
	} bits;						// 16 bits total
	UInt16 allBits;
} DBRecordFlags;

// This is the unpacked record form as used by the app. Pointers are 
// either NULL or point to strings elsewhere on a card.  All strings 
// are null character terminated.
typedef struct {
	Char* fields[16];
} DBRecordType;

typedef struct {
	UInt16				recSize;
   DBRecordFlags   	flags;
   Char              firstField;
} PackedDBRecord2;

typedef struct {
	UInt16				recSize;
   DBRecordFlags   	flags;
   Char              firstField;
} PackedDBRecord3;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Individual Record --
////////////////////////////////////////////////////////////////////////////////////

#define indiDBSearchFlds	7 // used in Search function (only first 7 searchable)

typedef enum {
	indiLName,			// set if record contains a name (bit 0)
	indiFName,			// set if record contains a firstName
   indiLspan,			// set if record contains a lifeSpan
	indiSex,				// set if record contains a sex
	indiAlias,			// set if record contains a alias
	indiNo,				// record must contains an individual reference no.
   indiTitle,			// set if record has title name
   indiFamSNo,			// set if record contains a famSNo(s)
   indiFamCNo,			// set if record contains a child-to-family ref no.
   indiEvenBNo,		// set if record contains birth event
   indiEvenNo,			// set if record contains life event(s)
   indiEvenDNo,		// set if record contains death event
   indiNoteNo,			// set if record contains a note		
   indiSouCNo,			// set if record contains source(s)		
   indiRefn,			// set if record contains reference no
   indiChiFlg			// set if individual has children
} IndiFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Event Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum { 
   evenType,		// the event type
   evenDesc,		// set if record contains event description
	evenDate,		// set if record containns date
	evenPlac,		// set if record contains place data
	evenAddr,		// set if record contains address
	evenAge,			// set if record contains age at event
	evenAgnc,		// set if record contains responsible agency data
	evenCaus,		// set if record contains cause of event data
	evenHAge,  		// set if record contains husband age at time of event
	evenWAge,		// set if record contains wife age at time of event
	evenReli,		// set if record contains religion
	evenStatL,  	// set if record contains LDS status data
	evenTempL,		// set if record contains LDS temple data
	evenNoteNo, 	// set if record contains note
	evenSouCNo,		// set if record contains source
	evenOrgNo		// originating record no in IndiDB/FamiDB (preceeded by I or F)
	} EventFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Source Citation Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
   souCSourNo,		// set if record contains a source reference no.
   souCNoteNo,		// set if record contains a note reference no.
	souCPage,		// set if record contains page number
	souCEven,		// set if record contains events recorded
	souCRole,		// set if record contains role
	souCQuay,		// set if record contains a quality rating
	souCDate,		// set if record contains a date
	souCText			// set if record contains text
	} SouCFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Source Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
   sourRepCNo,		// set if record contains a repository citation no.
   sourNoteNo,		// set if record contains a note reference no.
   sourTitl,		// set if record contains a title
   sourEven,		// set if record contains events recorded info
   sourDate,		// set if record contains date info
	sourPlac,		// set if record contains place info
	sourAgnc,		// set if record contains agency info
	sourAuth,		// set if record contains authority info
	sourText,		// set if record contains text from source
	sourPubl, 		// set if record contains publication info
	sourNumb 		// set if record contains microfilm number
	} SourFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Repository Citation Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
   repCRepoNo,		// set if record contains a repository ref no.
   repCNoteNo,		// set if record contains a note reference no.
	repCCaln,		// set if record contains a call number
	repCMedi			// set if record contains a media type
	} RepCFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Repository Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
   repoNoteNo,		// set if record contains a note reference no.
	repoName,		// set if record contains a repository name
	repoAddr 		// set if record contains an address
	} RepoFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Child Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum { 
   chilIndiNo,		// record must contain a individual referenc no.
	chilPedi,		// set if record contains a pedigree
	chilNoteNo,		// set if record contains a note
	chilSouCNo,		// set if record contains source(s)
	chilNum			// childs sort count number
	} ChilFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Family Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum { 
   famiHusbNo,		// set if record contains husb reference number
	famiWifeNo,		// set if record contains wife reference number
	famiNChi,		// set if record contains number of children 
	famiEvenMNo,	// set if record contains primary marriage event number
	famiEvenNo,		// set if record contains event number(s)
	famiNoteNo,		// set if record contains a note
	famiSouCNo,		// set if record contains event source(s)
	famiChiRec,		// first child record in FamCDB 
	famiChiCnt		// set if record contains actual child count
	} FamiFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Note Record --
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
   noteNo,			// record must contain a note reference number
	noteText			// record must contain note text.
	} NoteFields;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- GedWise Application Info Block --
////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	UInt16 	dbVers;
	Char 		gedDate[15];
	Char 		gedFile[26];
	Char	 	loadDate[15];
	UInt16	peopleCnt;
	UInt16	startDB[DBCount];
	} ApplInfoType;

typedef ApplInfoType* DBInfoPtr;

////////////////////////////////////////////////////////////////////////////////////
#pragma mark -- Draw Fields --
////////////////////////////////////////////////////////////////////////////////////

// The following is used for DetailViewDraw in order to draw the fields
// in the gadgets for the Detail Windows. This order must not change and
// has to match the order in the respective records above.
typedef enum {
	indiLNameD 			= 0,	
	indiFNameD,	
   indiLspanD,	
	indiSexD,		
	indiAliasD,	
	indiNoD,
	indiTitleD,		
   indiFamSNoD,
   indiFamCNoD,
   indiBEvenNo,
   indiEvenNoD,
   indiDEvenNo,
   indiNoteNoD,
   indiSouCNoD,
   indiRefnD,
   indiChiFlgD,
   famiHusbNoD			= 16,	
	famiWifeNoD,	
	famiNChiD,
	famiEvenMNoD,
	famiEvenNoD,	
	famiNoteNoD,	
	famiSouCNoD,
	famiChiRecD,
	famiChiCntD,	
   chilIndiNoD 		= 32,
	chilPediD,	
	chilNoteNoD,	
	chilSouCNoD,	
	chilNumD,		
   evenTypeD 			= 48,	
   evenDescD,	
	evenDateD,	
	evenPlacD,	
	evenAddrD,	
	evenAgeD,		
	evenAgncD,	
	evenCausD,	
	evenHAgeD,  	
	evenWAgeD,	
	evenReliD,
	evenStatLD,  
	evenTempLD,	
	evenNoteNoD, 
	evenSouCNoD,
   evenOrgNoD,
   souCSourNoD 		= 64,	
   souCNoteNoD,
	souCPageD,	
	souCEvenD,	
	souCRoleD,	
	souCQuayD,	
	souCDateD,	
	souCTextD,	
   sourRepCNoD			= 80,
   sourNoteNoD,
	sourTitlD,
   sourEvenD,
   sourDateD,
	sourPlacD,
	sourAgncD,
	sourAuthD,
	sourTextD,
	sourPublD,
	sourNumbD, 	
   repCRepoNoD 		= 96,
   repCNoteNoD,
	repCCalnD,
	repCMediD,
   repoNoteNoD			= 112,
	repoNameD,
	repoAddrD,
   noteNoD 				= 128,
	noteTextD,
	detailViewOther 	= 144,
	detailViewsouCDescD,
	detailViewrepCDescD,
	detailViewBlankLine,
	detailViewSpacerLine,
	detailViewError
} DrawFieldsD;

typedef enum {  // the following order MUST MATCH that in DrawFieldsD
	Indi,
	Fami,
	Chil,
	Even,
	SouC,
	Sour,
	RepC,
	Repo,
	Note
	} RecType;

typedef enum {  // keys for 5-way navigator
	NavNone,
	NavUp,
	NavDn,
	NavL,
	NavR,
	NavSel
	} DirType;


////////////////////////////////////////////////////////////////////////////////////
//	Internal Function Prototypes																	 //
////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

static void 	DetailViewInitPriorLines (void);
static void 	DetailViewGetSouCList (Char* SouCStr);
static UInt16 	DetailViewNoteSpace (char *aNote);
static Boolean DetailViewCheckNote (Char *noteXRef, Boolean *useNoteView, UInt16 noteButton);
static UInt16  DetailViewCalcNextLine (UInt16 i, UInt16 oneLine);
static void 	DetailViewDraw (DBRecordType rec, RecType recType, UInt16 viewGadget, UInt16 upButton, UInt16 downButton);
static void    DetailViewAddField (char* aField, const UInt16 fieldNum, UInt16 *width, const UInt16 maxWidth, const UInt16 indentation);
static Boolean DetailViewHandleVirtual (EventPtr event, UInt16 viewGadget);
static UInt16  DetailViewScrollOnePage (UInt16 newTopRecordViewLine, WinDirectionType direction, UInt16 viewGadget);
static void    DetailViewScroll (WinDirectionType direction, UInt16 viewGadget);
static void 	DetailViewNewLine (UInt16 *width);
static void 	DetailViewNewLineIf (UInt16 *width);
static void 	DetailViewAddSpaceForText (const Char* string, UInt16 *width);
static void 	DetailViewPositionTextAt (UInt16 *width);
static void 	DetailViewDrawXButton (void);
static void 	DetailViewInit (void);
static Boolean DetailViewResizeForm (Boolean newForm);

static Char*	EventDesc (DBRecordType* record);
 		 void 	SetNavFocusRing (const UInt16 objID);
static void 	SetFocus (const UInt16 controlID);
static void 	ShowObject (const UInt16 objectID, const Boolean showObj);
static void*	GetObjectPtr (const UInt16 objectID);
static void		GetObjectBounds (const UInt16 objectID, RectanglePtr rect);
void    			ResetScrollRate(void);
void    			AdjustScrollRate(void);
void 				DrawForm (void);
void 				UpdateScrollers (UInt16 leftButton, UInt16 rightButton, Boolean scrollableL, Boolean scrollableR);
void 				UpdateLeftRightScrollers (UInt16 upButton, UInt16 downButton, Boolean scrollableLeft, Boolean scrollableRight);
Boolean 			NavKeyHit (EventType *eventP, DirType *navDir);
Boolean 			MenuDoCommand (UInt16 command);

static void    IndiSummUpdFamiButtons (void);
static void 	IndiSummNameDrawRecord (void);
static void 	IndiSummEventsDrawRecord  (UInt16 buttonID);   					
static void    IndiSummDrawEvenFld (DBRecordType *record, RectanglePtr bounds);
static void 	IndiSummLoadData (void);
static void 	IndiSummCtlHighlight (UInt16 objectID, Boolean highlight);
static Boolean IndiSummCtlSelected (UInt16 objectID);
static void 	IndiSummHighlightMatchCtrl (TablePtr	evenLstTableP);
static void 	IndiSummScrollFami (DirType direction);
static void 	IndiSummFamiOrEvenButton (TablePtr evenLstTableP);
static void 	IndiSummChilOrEvenButton (TablePtr evenLstTableP);
		 void 	IndiSummUpdForms (Boolean updTopForm);
static void 	IndiSummDrawData (Boolean updateTbl);
static Boolean IndiSummResizeForm (void);
static Boolean IndiSummHandleEvent (EventPtr event);

static void 	IndiViewInit (void);
static Boolean IndiViewHandleEvent (EventPtr event);

static void 	AliaListDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void 	AliaListScroll (DirType direction);
static void    AliaListLoadTable (void);
static Boolean AliaListHandleEvent (EventPtr event);

static void 	AliaViewInit (void);
static Boolean AliaViewHandleEvent (EventPtr event);

static void    RepCViewInit (void);
static Boolean RepCViewHandleEvent (EventPtr event);

static void    RepoViewInit (void);
static Boolean RepoViewHandleEvent (EventPtr event);

static void 	SouCListScroll (DirType direction);
static void    SouCNumberDraw (void);
static void    SouCViewInit (void);
static void    SouCUpdateScrollButtons (void);

static void    SourViewInit (void);
static Boolean SourViewHandleEvent (EventPtr event);

static void    EvenSummaryDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void    EvenListLoadTable (void);
static void    EvenListScroll (DirType direction);
static void 	EvenLoadData (void);
static void    EvenListInit (void);

static void    EvenViewInit (void);
static Boolean EvenViewHandleEvent (EventPtr event);

static Boolean	GetRefNumPos (Char* key, Char* field, Char delim, UInt16* counter);
Boolean        ParentFinder (Boolean swapIndi);

static void 	FamiMarrDrawRecord (void);
static void    FamiSpouDrawRecord (void);
static void    FamiNumberDraw (Boolean updateScrollers);
static Boolean FamiListScroll (DirType direction);

static void 	FamiViewInit (void);
static Boolean FamiViewHandleEvent (EventPtr event);

static UInt16  ChilSeekLastRecord (UInt16 probe);
static void    ChilListInit (void);
static void    ChilListScroll (DirType direction);
static void    ChilLoadData (void);
static void    ChilListLoadTable (void);
static void 	ChilViewInit (void);
static Boolean ChilViewHandleEvent (EventPtr event);
	            
static void 	UpdateScrollBar (UInt16 fldId, UInt16 sclbarId);
static void 	ScrollField (Int16 linesToScroll, Boolean updateSclbar, UInt16 fldId, UInt16 sclbarId);
static void 	ScrollPage (WinDirectionType direction, UInt16 fldId, UInt16 sclbarId);

static Boolean	NoteViewResizeForm (Boolean newForm);
static Boolean NoteViewHandleEvent (EventType* event);

static void 	ExportMemoSaveMemo (void);
static void		ExportMemoInit (void);
static Boolean ExportMemoHandleEvent (EventType* event);

static void 	DetermineName (DBRecordType *recordP, 
               Int16 *fieldSeparatorWidth,
               Char **name1, Int16 *name1Length, Int16 *name1Width, 
               Char **name2, Int16 *name2Length, Int16 *name2Width, Int16 nameExtent,
               Boolean surnameFirst);
void 				DrawRecordName (
               Char *name1, Int16 name1Length, Int16 name1Width, 
               Char *name2, Int16 name2Length, Int16 name2Width,
               Int16 nameExtent, Int16 *x, Int16 y,  
               Int16 fieldSeparatorWidth, Boolean priorityIsName1, Boolean surnameFirst);
void 				DrawRecordNameAndLifespan (DBRecordType *record, RectanglePtr bounds, Boolean drawLifeSpan, Boolean surnameFirst);
void 				DrawNameLifespanColor (DBRecordType *rec, RectanglePtr bounds,	Boolean drawLifeSpan, Boolean surnameFirst);

static void 	ListViewSetButtonTxt (void);
static Boolean ListViewLookupStr (EventPtr event);
static void    ListViewDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void 	ListClearLookupStr (void);
static void 	ListViewScroll (DirType direction, UInt16 units, Boolean forcePage);
static void	 	ListViewSelectRec (UInt16 recN, Boolean forceSelect);
void 				ListViewLoadTable (void);
static void 	ListViewInit (void);
static void 	ListViewDrawForm (Boolean newForm);
static Boolean	ListViewResizeForm (void);
static Boolean ListViewHandleEvent (EventPtr event);

static Boolean ApplicationHandleEvent (EventPtr event);
static Err     RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags);

Err 				OpenDatabase (Char* userName, UInt16 mode, UInt16 location,	Boolean onExpCard);
void 				CloseDatabase (void);
static void 	StartApplication (void);
static void    StopApplication (void);
DBInfoPtr 		AppInfoGetPtr (DmOpenRef dbP, LocalID dbID);
void 				DbGetIndexes (DmOpenRef openDbRef, CmbDBases dbType, UInt16 *indexStart, UInt16 *indexEnd);

static void    Search (FindParamsPtr findparams);
static void    GoToItem (GoToParamsPtr goToParams, Boolean launchingApp); 
 						
static void 	EventLoop (void);
static UInt32  AppPilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
void 				AppLoadPrefs (Boolean reloadPrefs);
void 				AppSavePrefs (void);

#ifdef __cplusplus 
}
#endif