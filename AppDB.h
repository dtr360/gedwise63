////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise 6.0
//
// FILE:          AppDB.h
//
// AUTHOR:        Daniel T. Rencricca: October 15, 2004
//
// DESCRIPTION:   Header for the AppDB routines
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2004 Battery Park Software Corporation.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////

// Postions for drawing on Databases List Screen
#define DB_OPEN_X				5
#define DB_NAME_X				13
#define DB_NAMS_X				110
#define DB_SIZE_X				147
#define DB_CARD_X				148
#define DB_APPNAME			"GedWise"
#define DB_HH_STR				"Handheld"
#define DB_CARD_STR			"card"
#define DB_BEAM_STR			"Beam"
#define DB_COPY_STR			"copy"
#define DB_DEL_STR			"delete"
#define DB_DBCNT_STR			"%s Database"
#define DB_CARDNUM_STR		"Card%u"


// Defines for FileRefNum array
#define COPY_HH				0xffff
#define COPY_BEAM				0xffff-1

#define MAX_FNAME_SZ			255 // file and directory name max size
#define MAX_LNAME_SZ			10
#define CPYDB_RN_AR_SZ		6   // size of FileRefNum array (6*11=66)
#define CPYDB_LIST_SZ		66  // room for 6 items at 11 bytes each
#define OUR_FNAME_LEN		FULL_DB_NAME_LEN + PALM_PATH_LEN + 4 // leave room for extension
#define SCH_KEY_LEN			22  // size of search key
#define SCH_HDR_LEN			SCH_KEY_LEN + 9 // header for search screen
#define SCH_REC_FLD_LEN	350 // must be >= ADDR_LEN in GedWise PC program
#define SCH_EXT_WID			136 // width to be used to draw person's name in FldSearch
#define SCH_ETYPE_WID		20  // width of event type.
#define SCH_ARR_SZ			250 // Use 100 for testing
#define SCH_PAGE_ROWS		10  // rows per page 
#define SCH_NO_EVEN			"No Events Found"
#define SCH_NO_EVEN_LEN		15
#define SCH_NOT_AVAIL		"NA"
#define SCH_HEAD_SCH			"Srch: "
#define SCH_HEAD_EVEN		"Events on %s"

// Holds information on each GedWise database in memory
typedef struct {
	Char			dbName[USER_DB_NAME_SZ+1];
	LocalID		dbID;		 // database ID; = zero if db on Exp Card
	UInt16		location; // card number (for pdb in RAM) or volume (for pdb on card)
	UInt16		size;
	} DbArrayType;		

// Search Results row types
typedef enum {
	RowIndi,
	RowHusb,
	RowWife,
	RowNoRecFnd   // no matching records found in FldSearch
	} FldSrchRows;

// Type of data in SrchArray (from last search done).
typedef enum {
	SrchDateEven,
	SrchPlacEven,
	SrchNewSrch,
	SrchNoData
	} SrchATp;

// Event Search options
typedef enum { // THE ORDER OF THESE MUST MATCH ORDER ON SEARCH FORM
	SrchBir,
	SrchBap,
	SrchChr,
	SrchDth,
	SrchBur,
	SrchCrm,
	SrchMar,
	SrchCen,
	SrchRes,
	SrchOptTtl  // THIS MUST EQUAL TOTAL SELECT EVENTS ON SEARCH FORM
	} SrchOTp;

////////////////////////////////////////////////////////////////////////////////////
//	Internal Function Prototypes																	 //
////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

static Int16 	GetControlValue (const UInt16 controlID);
static void* 	GetObjectPtr (UInt16 objectID);
static void 	GetObjectBounds (const UInt16 objectID, RectanglePtr rect);
static void 	ShowObject (UInt16 objectID, Boolean showObj);

static Int16 	StrCmpMatches (Char *s1, Char *s2);
Boolean 			IndiLookupFName (Char *key, UInt16 *recordP, Boolean *completeMatch);
Boolean     	IndiLookupLName (Char *key, UInt16 *recordP,	Boolean *completeMatch);	

UInt16 			IndiDbNumRecords (void);

MemHandle 		DbQueryRecord (UInt16 recIndex);
void 				DbMemHandleUnlock (MemHandle* recH);
static Err 		DbQueryRecordKey (UInt16 index, Char** recordKey, MemHandle* recH);
Err 				DbGetRecord (CmbDBases dbType, UInt32 index, DBRecordType *rec, MemHandle *recordH);
void				DbUnpackRecord (PackedDBRecord2 *src, DBRecordType *dest);

Err 				NoteRecFinder (Char *key,  DBRecordType *rec, MemHandle *rH);

void 				ClearField (UInt16 frmID);
static void		ProgressBarUpdate (UInt16 progBarID, Boolean updateBar);
Boolean 			SearchCanceled (UInt16 ctlId);

static Boolean FindPerson (Char *srchKey, Boolean srchIdNo, UInt16 *recordP);
Boolean 			FindPersonHandleEvent (EventPtr event);

static void 	EvenSearchInit (Boolean getSel);
Boolean			EvenSearchHandleEvent (EventPtr event);

static void 	FldSrchListDrawRec (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void 	FldSrchDrawRecAtRow (const Int16 row);
static void 	FldSrchListInit (void);
static void 	FldSrchUpdScrollers (void);
static void 	FldSrchScroll (DirType direction);
static Boolean FldSrchCompRec (Char* recKeyP, UInt16 keyLen, Char* recKey);
static void 	FldSrchListLoadTable (TablePtr table, Int16 *row, DBRecordType eRec, UInt32 eRecN);
static void 	FldSrchSearchRecs (Boolean contSearch);
static void 	FldSrchRedrawTable (void);
static void		FldSrchDrawForm  (void);
Boolean 			FldSrchHandleEvent (EventPtr event);

static void		ThisDayNewCheckDate (void);
Boolean 			ThisDayNewHandleEvent (EventPtr event);

static Err 		WriteDBData (const void* dataP, UInt32* sizeP, void* userDataP);
static Err 		SendDatabase (const UInt16 dbNum);

static Err 		CopyDbToCard (UInt16	dbNum, UInt16 volRefNum);
static Err 		CopyDbToHH (UInt16	dbNum);
static Err		CopyDbGetList (Boolean fileOnHH,	UInt16* itemTot);
static void 	CopyDbFreeData (void);
Boolean 			CopyDbHandleEvent (EventPtr event);

DBInfoPtr 		VfsAppInfoGetPtr (FileRef fileRef, const UInt16 volRefNum,
 				 		const Char* userName);
static void 	VfsGetDatabaseList (const Boolean createList);
static Int16 	DBCompareRecords (DbArrayType *db1, DbArrayType *db2, UInt16);
Err 				GetDatabaseList (const Boolean createList);
static Err 		DeleteDatabase (UInt16 dbNum);
static void 	DBListDrawRecord (void *table, Int16 row, Int16 column, RectanglePtr bounds);
static void 	DBListLoadTable (void);
static void 	DBListScroll (WinDirectionType direction);
static void 	DBListLoadTable (void);
static void 	DBListInit (void);
static void		DBListShowInfo (Boolean show);
static Err	 	DBListGetInfo (UInt16 dbNum);
static void		DBListScreenInit (void);
Boolean 			DBListHandleEvent (EventPtr event);

#ifdef __cplusplus
}
#endif


