////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise Version 6.2 & 6.3
//
// FILE:          AppMain.c
//
// AUTHOR:        Daniel T. Rencricca: October 15, 2004
//
// DESCRIPTION:   This is the GedWise application's main module.  This
//                module starts the application, dispatches events, and
//                stops the application.
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2005 Battery Park Software Corporation.  
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////

#include "Defines.h" // switches and global defines. Must be included here
#include <PalmOS.h>
#include <VFSMgr.h>
#include <PalmChars.h>

#include "AppMain_res.h"
#include "AppMain.h"
#include "AppDB.h"
#include "AppMisc.h"
#include "Soundex.h"

////////////////////////////////////////////////////////////////////////////////////
//   Entry Points
////////////////////////////////////////////////////////////////////////////////////
UInt32 PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

////////////////////////////////////////////////////////////////////////////////////
//   Internal Structures
////////////////////////////////////////////////////////////////////////////////////

// Info on how to draw the Detail View Forms
typedef struct {
	UInt16	fieldNum;
	UInt16	length;
	UInt16	offset;
	UInt16 	x;
} DVLineType;

////////////////////////////////////////////////////////////////////////////////////
//   Global Variables
////////////////////////////////////////////////////////////////////////////////////
static DVLineType*	DVLines	        = 0; // holds gadget array
static UInt16		DVLinesMax      = DV_LINES_MAX;
static MemHandle  	DVLinesH        = NULL; // handle to gadget array

static UInt16  TopVisIndiRecN       = 0; // record number in IndiDB
static UInt32  TopVisChilRecN       = 0; // record number in ChilDB
static UInt32  TopVisEvenRecN       = 0; // record number in EvenDB
static UInt16  TopVisAliaNum 	    = 0; // position based on total Aliases
static UInt16  TopVisFamiNum   	    = 0; // position based on total Marriages
static UInt16  TopVisSouCNum   	    = 0; // position based on total Sours

UInt16         CurrentIndiRecN      = NO_REC; // record num in IndiDB
UInt16         CurrentSpouRecN      = NO_REC; // record num in IndiDB
UInt32         CurrentFamiRecN      = NO_REC_LONG; // record num in FamiDB
UInt32         CurrentEvenRecN      = NO_REC_LONG; // record num in EvenDB
static UInt16  CurrentAliaRecN      = NO_REC; 	   // record num in IndiDB
static UInt32  CurrentMarrRecN      = NO_REC_LONG; // record num in EvenDB
static UInt32  CurrentSourRecN      = NO_REC_LONG; // record num in SourDB
static UInt32  CurrentRepoRecN      = NO_REC_LONG; // record num in RepoDB
static UInt32  CurrentRepCRecN      = NO_REC_LONG; // record num in RepoDB

MemHandle       IndiRecH            = NULL;
MemHandle       AliaRecH            = NULL;
MemHandle       EvenRecH            = NULL;
MemHandle       FamiRecH            = NULL;
MemHandle       ChilRecH            = NULL;
MemHandle       SourRecH            = NULL;
MemHandle       SouCRecH            = NULL;
MemHandle       RepoRecH            = NULL;
MemHandle       RepCRecH            = NULL;
MemHandle       NoteRecH            = NULL;

DBRecordType 	IndiRec;
DBRecordType 	AliaRec;
DBRecordType	EvenRec;
DBRecordType	FamiRec;
DBRecordType	ChilRec;
DBRecordType	SourRec;
DBRecordType	SouCRec;
DBRecordType	RepoRec;
DBRecordType	RepCRec;
DBRecordType	NoteRec;

UInt16	        HldIndiRecN		    = NO_REC; 	    // record num in IndiDB
UInt16    	    ListViewSelectRecN	= NO_REC;	    // set when leave ListView
static UInt32   PrimaryBirtRecN		= NO_REC_LONG;  // record number in EvenDB
static UInt32   PrimaryDeatRecN   	= NO_REC_LONG;  // record number in EvenDB 
extern UInt16	RelCalcRecN1;				        // record number in IndiDB
static UInt32	FirstChilRecN		= NO_REC_LONG;  // record number in ChilDB
static UInt32	LastChilRecN		= NO_REC_LONG;  // record number in ChilDB
static UInt32	FirstEvenRecN		= NO_REC_LONG;  // record number in EvenDB
static UInt32	LastEvenRecN		= NO_REC_LONG;  // record number in EvenDB
static UInt16	ChilNumInFam;						// position based on total children

static UInt16   LastAliaNum 		= 0;	        // based on total Aliases
static UInt16   LastSouCNum       	= 0;  	        // based on total Sources
static UInt16 	LastFamiNum       	= 0;  	        // based on total Families

static Char		 NoteXRef[XREF_LEN+1];
static Char*	 SouCList;				            // used in SouCViewHandleEvent
static MemHandle SouCListH = NULL;

static Boolean  AlreadyHaveFamily	= false;
Boolean 	    GetFirstName 		= false;        // used in ListViewHandleEvent
Boolean 		SupportsColor;			            // true if device supports color
Boolean			UseFullNoteScrn		= false;        // true to use entire screen for note
Boolean			RelCalcEntry		= false;    
Boolean			RelCalcGetRec1;

static UInt16	DetailViewLastLine; 	            // Line after last one containing data
static UInt16	DetailViewFirstPlainLine;
static UInt16	TopDetailViewLine;
static UInt16	PriorTopDetailViewLine 	   = 0;     // Used for misc Event Handlers
static UInt16	PriorSouCTopDetailViewLine = 0;     // Used for SouCViewHandleEvent
static UInt16   PriorSourTopDetailViewLine = 0;     // Used for SourViewHandleEvent
static UInt16   PriorRepoTopDetailViewLine = 0;     // Used for RepoViewHandleEvent
static UInt16   PriorRepCTopDetailViewLine = 0;     // Used for RepoViewHandleEvent
static UInt16   SouCViewPriorSouCNumber    = 1;     // this must be set to a 1 here!
static UInt16	PriorSouCFormID;  				    // Used in SouCViewHandleEvent
static UInt16	PriorNoteFormID;  				    // Return to prior form aft Note View
UInt16			PriorFormID;
UInt16			IndiDBNumRecs 		        = 0;    // no. of individuals in open db
UInt16			ExportMemoIndex;

Boolean			AboutMode = false;
static UInt16	IndiListTblRows;
static Char 	DataStr[32];			// used in various locations
Char			DescStr[5];				// used for event description header in DV
UInt16			EvenTypeN;				// used for Note description header in DV
UInt16			TotDatabases;  		    // total databases in memory.
DmOpenRef 		MemoDbRef	= NULL;	    // ref to system's memo database
DmOpenRef		OpenDbRef 	= NULL; 	// ref to the open database in RAM
FileRef			OpenFileRef = NULL; 	// ref to the open database on EC
DbArrayType* 	DatabaseList;     	    // holds database list
Boolean			Graf2Device = false;    // true if device uses Graffiti 2
Boolean			DynInDevice = false;	// true if device has dynamic input area
Boolean			FivWayDevice= false;    // true if device has five way navigator button

// These are used for accelerated scrolling
UInt16 			LastSeconds 		= 0;
UInt16 			ScrollUnits 		= 0;

// These are used to control Individual View Screen
static Boolean	EventsVisible 		= false; // 1=show Events, 0=show Children
static Boolean FamiEventsVisible	= false; // 1=show Family Events, 0=show Children
static Boolean ShowFamilyButtons = false; 

Boolean			UpdateFrm 			= true;
Boolean			Pre35Rom			= false; // indicates pre-OS 3.5 ROM
Boolean			Pre60Rom			= false; // indicates pre-OS 6.0 ROM
Boolean			ExpCardCapable 	    = false; // true if HH is Expansion Card Capable
Boolean			RedrawBaseFrm		= false;

// The following global variable are saved to a preferences state file.
UInt32       	InstallDate 			  = 0;      // days from 1/1/1904 since installation.
Char			DbName[USER_DB_NAME_SZ+1] = "\0";   // name of open db
UInt16			FileLoc					  = 0;	    // volume or card db is on
UInt16			RelCalcRecN2			  = NO_REC; // rec number of indiv 2 in Rel. Calc. Wnd
UInt16			Jump[JUMP_MAX]; 				    // rec numbers of last 10 indiv viewed
RGBColorType	Color1;							    // init in StartApplication
RGBColorType	Color2;							    // init in StartApplication
FontID      	PrefFnts[TotPrefFnts]	    = {stdFont, stdFont, stdFont, largeBoldFont};

#ifdef REGISTERED
Boolean			Prefs[TotPrefFlds] = {false, false, false, true, false, false, false,
												 false, false, false, false, false, true};
#else
Boolean			Prefs[TotPrefFlds] = {false, false, false, true, false, false, false,
												 false, false, false, false, false, false};
#endif

extern Boolean	SrchShowEven;
extern Boolean SrchShowFamiEven;
extern SrchATp	SrchArrayData;

////////////////////////////////////////////////////////////////////////////////////
//	Global Constants
////////////////////////////////////////////////////////////////////////////////////
const Char 		cFieldSepStr[]		= FLD_SEP_STR;
const Char 		cDbNameStr[]		= DBNAME_STR; // format of full db name
const Char 		cFileNameStr[]		= FNAME_STR;  // format of full file name
const Char		cPalmPath[] 		= PALM_PATH_STR;
const Char 		cNoteSepStr[] 		= NOTE_SEP_STR;
const Char 		cErrorNoRec[18]  	= NO_REC_STR;	// size = 18
	  Char 		cUnknownStr[]    	= UNK_STR;

////////////////////////////////////////////////////////////////////////////////////
//	Event Description Array
////////////////////////////////////////////////////////////////////////////////////
Char*	EvenDesc[TOT_EVEN_L][2] = // DO NOT CHANGE FIELD ORDER (MUST MATCH GEDWISE PC)
 	  {
 	    {cUnknownStr, "Unk"},
		{"Birth", "Bir"},
		{"Baptism","Bap"},
		{"Christening", "Chr"},
		{"Death", "Dth"},
		{"Burial", "Bur"},
		{"Cremation", "Crm"},
		{"Probate", "Prb"},
		{"Marriage", "Mar"},
		{"Banns", "Ban"},
				
		{"Marr. Contract", "MaC"},
        {"Marr. License", "MaL"},
        {"Marr. Settlement", "MaS"},
        {"Annulment", "Anl"},
        {"Census", "Cen"},
        {"Divorce", "Div"},
        {"Div. Filing", "DiF"},
        {"Engagement", "Eng"},
        {"Caste", "Cst"},
        {"Description", "Dsc"},

        {"Education", "Edu"},
        {"ID Number", "IdN"},
        {"Nationality", "Nat"},
        {"Child Count", "CCt"},
        {"Marr. Count", "MCt"},
		{"Occupation", "Occ"},
		{"Possessions", "Pos"},
		{"Religion", "Rlg"},
		{"Residence", "Res"},
		{"Social Sec. Number", "SSN"},

		{"Title", "Ttl"},
		{"Adoption", "Adp"},
		{"Bar Mitzvah", "BrM"},
		{"Bas Mitzvah", "BsM"},
		{"Blessing", "Bls"},
		{"Adult Christening", "Chr"},
		{"Confirmation", "Cnf"},
		{"First Communion", "Com"},
		{"Ordination", "Ord"},
		{"Naturalization", "Nat"},

		{"Emigration", "Emg"},
		{"Immigration", "Img"},
		{"Will", "Wil"},
		{"Graduation", "Grd"},
		{"Retirement", "Ret"},
		{"Event", "Evt"},
		{"Note", "Nte"},
		{"LDS Baptism", "LBp"},
		{"LDS Confirmation", "LCf"},
		{"LDS Endowment", "LEn"},

		{"LDS Child Sealing", "LCh"},
		{"LDS Spouse Sealing", "LSp"},
		{"Relationship", "Rel"},
		{"History",  "Hst"}, // nonstandard tag
		{"Military", "Mlt"}, // nonstandard tag
		{"Medical",  "Med"},	// nonstandard tag
		{"Election", "Ele"},	// nonstandard tag
		{"Sp. Death","DSp"}, // nonstandard tag
		{"Degree",   "Deg"} 	// nonstandard tag
	   };

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      PilotMain
//
// DESCRIPTION:   This is the main entry point for the GedWise application.
//
// PARAMETERS:    cmd - word value specifying the launch code. 
//                cmdPB - pointer to structure associated with the launch code. 
//                launchFlags -  provides extra information about the launch.
// 
// RETURNED:      Result of launch.
//
// NOTE:          We need to create a branch island to PilotMain in order to 
//                successfully link this application for the device.
//
// REVISIONS:     None.
//
////////////////////////////////////////////////////////////////////////////////////
UInt32 PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
   return AppPilotMain (cmd, cmdPBP, launchFlags);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewInitPriorLines
//
// DESCRIPTION:   The routine initializes the Prior Detail View Lines.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:	  None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewInitPriorLines (void)
{
	SouCViewPriorSouCNumber 	= 1; // Used for SouCViewHandleEvent
	PriorTopDetailViewLine 		= 0; // Used for EventViewHandleEvent
	PriorSouCTopDetailViewLine = 0; // Used for SouCViewHandleEvent
	PriorSourTopDetailViewLine = 0; // Used for SourViewHandleEvent
	PriorRepoTopDetailViewLine = 0; // Used for RepoViewHandleEvent
	PriorRepCTopDetailViewLine = 0; // Used for RepCViewHandleEvent
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    DetailViewNoteSpace
//
// DESCRIPTION: Checks a note to see if it will fit in the gadget.
//
// PARAMETERS:  -> aNote	- pointer to text string to add 
//
// RETURNED:    the number of lines Note will require to be displayed.
//
// REVISIONS:	 None.
////////////////////////////////////////////////////////////////////////////////////
static UInt16 DetailViewNoteSpace (Char* aNote)
{
   UInt16         length;
   RectangleType  rect;
   UInt16         offset = 0;
   UInt16         newOffset = 0;
   UInt16         counter = 0;
   UInt16         maxWidth;
   FontID         curFont;

  if (aNote == NULL || *aNote == '\0')
      return 0;

   // Get width and height of current drawn window.
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID * 2; // subtract 4 to allow room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);

   while (true) {
	   length = FldWordWrap (&aNote[offset], maxWidth);
		
	   if (aNote[offset + length] != '\0'
		   && !TxtCharIsSpace ((UInt8) aNote[offset + length - 1])
		   && !TxtCharIsDelim ((UInt8) aNote[offset + length - 1]))
		   length = 0; // don't word wrap - try next line

	   newOffset = offset + length;
	   if (newOffset > 0 && aNote[newOffset - 1] == linefeedChr)
		   length--;
		offset = newOffset;

		counter++;
		
		if (counter > (DVLinesMax - DetailViewLastLine)) {
			break;  // no use going any further.
			}
		
		if (aNote[offset] == '\0')
		   break;
	   
   	}

   FntSetFont (curFont);
   return counter;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewCheckNote
//
// DESCRIPTION:   This routine initializes checks the note fields of a record
//                (EventRecord, RepoRec,SouCRec, IndiRec) to see if 
//                it will fit in the gadget viewer.  
//                GedWise application.  It lays out the record on the form.
//                It is assumed that NoteRec is initialized before calling
//                this function.
//
// PARAMETERS:    -> noteXRef  	- 	Note cross reference number.
//                -> useNoteView - 	set to true if not enough room for the note
//                              		in the gadget, thus Note Viewer must be used.
//                -> noteButton 	-	the "Note" button for the form.
//                 
// RETURNED:      The record number of the note in foundRecN, or false if
//                note number was not found in the note field.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean DetailViewCheckNote (Char* noteXRef, Boolean* useNoteView,
   UInt16 noteButton)
{
   UInt16   linesRequired;

	if (noteXRef == NULL || *noteXRef == '\0')
	   goto NoteError;
	   
   StrCopy (NoteXRef, noteXRef);
   DbMemHandleUnlock (&NoteRecH);
   
	if (NoteRecFinder (noteXRef, &NoteRec, &NoteRecH))
      goto NoteError;
         
   if (NoteRec.fields[noteText]) {
     	// make sure there is room for note in gadget
      if (!*useNoteView) {
        	// get size of note text.
        	linesRequired = DetailViewNoteSpace (NoteRec.fields[noteText]);
        	*useNoteView = (linesRequired >= (DVLinesMax - DetailViewLastLine));
	     	}

   	// Hide note button if no note or if using Note View
   	ShowObject (noteButton, true);
      }
   else goto NoteError;

	return true;

	/////////
	NoteError:
	/////////
	
	ShowObject (noteButton, false);
   return false;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewAddNote
//
// DESCRIPTION:   This routine initializes checks the note fields of a record
//                (EventRecord, RepoRec,SouCRec, IndiRec) to see if 
//                it will fit in the gadget viewer.  
//                GedWise application.  It lays out the record on the form.
//                It is assumed that SouCRec is filled before calling
//                this function.
//
// PARAMETERS:    -> noteXRef   - 	Note cross reference number.
//				  -> noteButton - 	the "Note" button id for the form.
//				  -> fieldNum 	- 	note field to add.
//                -> width		- 	width already occupied on the line.
//             	  -> maxWidth	- 	can't add words past this width.
//				  -> titleStr	-	title to add to window.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewAddNote (Char* noteXRef, UInt16 noteButton, UInt16 fieldNum,
			   	UInt16* width, UInt16 maxWidth, Char* titleStr)
{
	Boolean	useNoteView = Prefs[SepNoteView];

	if (DetailViewCheckNote (noteXRef, &useNoteView, noteButton)) {
      
      *DataStr = '\0'; // used in GotoNoteForm  

      if (!useNoteView) {
         DetailViewNewLineIf (width);
         
        	if (DetailViewLastLine < DVLinesMax && DetailViewLastLine > 0) {
   			DVLines[DetailViewLastLine].fieldNum = detailViewSpacerLine;
   			DVLines[DetailViewLastLine].x = MARGIN_WID;
   			DVLines[DetailViewLastLine].offset = 0;
   			DetailViewLastLine++;
   			*width = MARGIN_WID;
   			}  
         DetailViewAddField (NoteRec.fields[noteText], fieldNum, width,
            maxWidth, MARGIN_WID);
         }
      }
       
	if (titleStr != NULL && *titleStr != '\0')
   	StrCopy (DataStr, titleStr);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    DetailViewGetSouCList
//
// DESCRIPTION: The routine initializes the SouCList string for the SouCDetailForm.
//				This string simly holds the list of	sources for a given record.
//   			The SouCListH is freed in SouCViewHandleEvent.
//
// PARAMETERS:  -> SouCStr 	-	pointer to the source citation list string from
//								a locked record.
//
// RETURNED:    Nothing.
//
// REVISIONS:	None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewGetSouCList (Char* SouCStr)
{
   ErrFatalDisplayIf (SouCListH, "DetailViewGetSouCList: SouCListH should be NULL");
   
   SouCListH = MemHandleNew (StrLen (SouCStr)+1);
	
   ErrFatalDisplayIf (!SouCListH, "Out of memory");
   
   SouCList = MemHandleLock (SouCListH);
   StrCopy (SouCList, SouCStr);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	RecordViewCalcNextLine
//
// DESCRIPTION: 	Calculate how far to advance to the next line in a gadget.
// 					A blank line or text which begins to the left of text on the
// 					current line advance the line down.  Multiple blank lines in
// 					succession advance the line down half a line at a time.
//
// PARAMETERS:  	-> i 		  -	the line to base how far to advance
//                -> oneLine - 	the amount which advance one line down.
//
// RETURNED:    	the amount to advance.  Typically oneLine or oneLine / 2.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static UInt16 DetailViewCalcNextLine (UInt16 i, UInt16 oneLine)
{
   // Advance down if the text starts before the text of the current line.
   if (DVLines[i].x == MARGIN_WID || 
      (i > 0 &&
         (DVLines[i].x <= DVLines[i - 1].x ||
          DVLines[i - 1].fieldNum == detailViewBlankLine)) ||
         (i >= DetailViewFirstPlainLine && DVLines[i].x == HEAD_WID)) {
         
      // A non blank line moves down a full line.
      if (DVLines[i].fieldNum != detailViewBlankLine) {
         return oneLine;
         }   
      else {
         // detailViewBlankLine is half-height.
         return oneLine / 2;
         }
      }
   return 0; // stay on the same line.
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DetailViewGetHeader
//
// DESCRIPTION: 	Gets the header string for the data field being drawn.
//						Assumes the header string is 4 characters or less.
//
// PARAMETERS:  	-> recType -	type of record (eg IndiDB, FamiDB, etc)
//						-> headNo  - 	header number to draw.
//
// RETURNED:    	a pointer to a global string containing the header string.
//              	This global string is called DescStr.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Char* DetailViewGetHeader (RecType recType, UInt16 headNo)
{
	UInt16	fieldNo;

   fieldNo = (recType*16) + headNo;  // convert to correct field number

 	switch (fieldNo)
 		{
      case evenDateD:
      case souCDateD:
      case sourDateD:
			StrCopy (DescStr, "Date");
      	break;
      case evenPlacD:
		case sourPlacD:
			StrCopy (DescStr, "Plac");
			break;
      case evenAddrD:
        	StrCopy (DescStr, "Addr");
        	break;
      case evenAgeD:
        	StrCopy (DescStr, "Age");
        	break;
      case sourAgncD:
      case evenAgncD:
        	StrCopy (DescStr, "Agnc");
        	break;
      case evenCausD:
        	StrCopy (DescStr, "Caus");
        	break;
		case evenHAgeD:
			StrCopy (DescStr, "AgH");
			break;
		case evenWAgeD:
			StrCopy (DescStr, "AgW");
			break;
		case evenReliD:
			StrCopy (DescStr, "Reli");
			break;
		case evenStatLD:
			StrCopy (DescStr, "Stat");
			break;
		case evenTempLD:
			StrCopy (DescStr, "Tmpl");
 			break;
 		case souCEvenD:
 		case sourEvenD:
			StrCopy (DescStr, "Even");
      	break;
      case souCRoleD:
			StrCopy (DescStr, "Role");
			break;
      case souCQuayD:
        	StrCopy (DescStr, "Qual");
        	break;
      //case souCDateD: see above
      case sourTextD:
      case souCTextD:
        	StrCopy (DescStr, "Text");
        	break; 
      //case sourTitlD:  ignore
      //case sourEvenD:  see above
      //case sourDateD:  see above
      //case sourPlacD:  see above
      //case sourAgncD:  see above
		//case sourTextD:  see above
      case sourAuthD:
        	StrCopy (DescStr, "Auth");
        	break;
		case sourPublD:
			StrCopy (DescStr, "Publ");
			break;
		case sourNumbD:
			StrCopy (DescStr, "Num");
			break;
		case repCCalnD:
			StrCopy (DescStr, "CalN");
			break;
		case repCMediD:
			StrCopy (DescStr, "Medi");
			break;
		case chilPediD:
			StrCopy (DescStr, "Pedi");
			break;
		case famiNChiD:
			StrCopy (DescStr, "Chil");
			break;
		case indiNoD:
			StrCopy (DescStr, "IdNo");
			break;
		case indiRefnD:
			StrCopy (DescStr, "RefN");
			break;
 		default:
		 	StrCopy (DescStr, EvenDesc[0][1]); // "Unk"
 		 	break;
 		 }
 	return DescStr;
}   

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DetailViewDraw
//
// DESCRIPTION: 	This routine draws the Detail View Gadget
//
// PARAMETERS:		-> rec -		record that will have information drawn
//					-> recType -	type of record (eg IndiDB, FamiDB, etc)
//					-> viewGadget-	name of gadget we are drawing too
//					-> upButton -	name of form's up button
//					-> downButton-	name of form's down button
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewDraw (DBRecordType rec, RecType recType,	UInt16 viewGadget,
					UInt16 upButton, UInt16 downButton)
{
   UInt16 			x;
   UInt16 			i;
   UInt16 			y;
   FontID 			curFont;
   FormPtr  		frm;
   Boolean 			scrollableU;
   Boolean 			scrollableD;
   RectangleType 	rect;
   Int16 			bottomOfRecordViewDisplay; 
	Char			headerStr[6]; // 4-char header + ":" + terminator 
	UInt16			fieldNo;
	UInt16 			evenTypeN;

 	#ifdef GREMLINS_MEM_CHK
  	MemHeapScramble (MemHeapID (0,1));
 	#endif

   frm = FrmGetActiveForm ();
   GetObjectBounds (viewGadget, &rect);
   bottomOfRecordViewDisplay = rect.topLeft.y +  rect.extent.y;
  
	curFont = FntSetFont (PrefFnts[DetailViewFont]);

   // Set event description as form title if drawing Event View Window
   if (recType == Even && rec.fields[evenType]) {
		
		evenTypeN = (UInt16) StrAToI (rec.fields[evenType]);
		
		if (evenTypeN >= TOT_EVEN_L) // make sure Event type is in range
			evenTypeN = 0; // event "Unknown"
		
		FrmCopyTitle (frm, EvenDesc[evenTypeN][0]); // long title
		}

   y = rect.topLeft.y - DetailViewCalcNextLine (TopDetailViewLine, FntLineHeight());
   
   for (i = TopDetailViewLine; i < DetailViewLastLine; i++) {  
      
      y += DetailViewCalcNextLine (i, FntLineHeight ());
      
      // If we are past the bottom stop drawing
      if (y > bottomOfRecordViewDisplay - FntLineHeight ())
         break;
      
      ErrNonFatalDisplayIf (y < rect.topLeft.y, "Drawing out of gadget");
      
      // Convert field number to applicable enum type
      if (DVLines[i].fieldNum < detailViewOther)
      	fieldNo = (recType*16) + DVLines[i].fieldNum;
      else
      	fieldNo = DVLines[i].fieldNum;
      
      if (DVLines[i].offset == 0) // then use special handling
         {
         switch (fieldNo)
            {
            case detailViewBlankLine:
               break;
               
            case detailViewSpacerLine:
               FntSetFont (TITLE_FONT);
               WinDrawChars (cNoteSepStr, sizeof (cNoteSepStr) - 1, MARGIN_WID, y);
               FntSetFont (PrefFnts[DetailViewFont]);
               break;   

            case detailViewError:
               FntSetFont (TITLE_FONT);
               WinDrawChars (cErrorNoRec, sizeof (cErrorNoRec) - 1, MARGIN_WID, y);
               FntSetFont (PrefFnts[DetailViewFont]);
               break;   

            case detailViewrepCDescD:
               WinDrawChars (&RepoRec.fields[repoName][DVLines[i].offset],
                  DVLines[i].length, DVLines[i].x, y);
               break;
                              
            case detailViewsouCDescD:
               WinDrawChars (&SourRec.fields[sourTitl][DVLines[i].offset],
                  DVLines[i].length, DVLines[i].x, y);
               break;
            
            case repoAddrD:
            case souCPageD:
               if (DVLines[i].x > MARGIN_WID)
                  WinDrawChars (cFieldSepStr, 2, DVLines[i].x -
                  	FntCharsWidth (cFieldSepStr, 2), y);
               WinDrawChars (&rec.fields[DVLines[i].fieldNum]
                  [DVLines[i].offset], DVLines[i].length,
                  DVLines[i].x, y);
               break;

            case repoNameD:
            case evenDescD:
               WinDrawChars (&rec.fields[DVLines[i].fieldNum]
               [DVLines[i].offset], DVLines[i].length,
               DVLines[i].x, y);
               break;   

            case indiSexD: 
               FntSetFont (TITLE_FONT);
               WinDrawChars ("Sex:", 4, MARGIN_WID, y);
               x = MARGIN_WID + 21;
               FntSetFont (PrefFnts[DetailViewFont]);
               WinDrawChars (&rec.fields[DVLines[i].fieldNum]
                  [DVLines[i].offset], DVLines[i].length,
                  DVLines[i].x, y);
               x+= 30;
 
               FntSetFont (TITLE_FONT);
               WinDrawChars("Sndx:", 5, x, y);
               x+= 29;
               FntSetFont (PrefFnts[DetailViewFont]);
               WinDrawChars (Soundex (rec.fields[indiLName]), 4, x, y);
               break;    
                 
				case indiRefnD:
				case indiNoD:  
            case evenDateD:
            case evenPlacD:
            case evenAddrD:
            case evenAgeD:
            case evenAgncD:
            case evenCausD:
				case evenHAgeD:
				case evenWAgeD:
				case evenReliD:
				case evenStatLD:
				case evenTempLD:
            case sourTitlD:
            case sourEvenD:
            case sourDateD:
            case sourPlacD:
            case sourAgncD:
            case sourAuthD:
            case sourTextD:
            case sourPublD:
            case sourNumbD:
            case souCEvenD:
            case souCRoleD:
            case souCQuayD:
            case souCDateD:
            case souCTextD:
            case repCCalnD:
            case repCMediD:
            case chilPediD:
            case famiNChiD:
            	FntSetFont (TITLE_FONT);
					StrCopy (headerStr, DetailViewGetHeader (recType,
						DVLines[i].fieldNum));
					StrNCat (headerStr, ":", sizeof (headerStr));
               WinDrawChars (headerStr, StrLen (headerStr), MARGIN_WID, y);
               FntSetFont (PrefFnts[DetailViewFont]);
               WinDrawChars (&rec.fields[DVLines[i].fieldNum]
               	[DVLines[i].offset], DVLines[i].length,
               	DVLines[i].x, y);
               break;  
              
           case indiNoteNoD:
           case famiNoteNoD:
           case chilNoteNoD:
           case repoNoteNoD:
           case repCNoteNoD:
           case souCNoteNoD:
           case sourNoteNoD:
           case evenNoteNoD:
               WinDrawChars (&NoteRec.fields[noteText]
                  [DVLines[i].offset], DVLines[i].length,
                  DVLines[i].x, y);
               break;
                      
            default:
               WinDrawChars (&rec.fields[DVLines[i].fieldNum]
                  [DVLines[i].offset], DVLines[i].length,
                  DVLines[i].x, y);
               break;
            }
         }
         else {
         	switch (fieldNo) {
         		case detailViewsouCDescD:
         		 	WinDrawChars (&SourRec.fields[sourTitl][DVLines[i].offset],
                  DVLines[i].length, DVLines[i].x, y);
         		 	break;
         		case detailViewrepCDescD:
         		 	WinDrawChars (&RepoRec.fields[repoName][DVLines[i].offset],
                  DVLines[i].length, DVLines[i].x, y);
         		 	break;
         		case detailViewBlankLine:
         		case detailViewSpacerLine:
         			break; 	
         		case indiNoteNoD:
         	   case famiNoteNoD:
         		case chilNoteNoD:
         		case repoNoteNoD:
           		case repCNoteNoD:
           		case souCNoteNoD:
           		case sourNoteNoD:
           		case evenNoteNoD:
                 	WinDrawChars (&NoteRec.fields[noteText]
                    	[DVLines[i].offset], DVLines[i].length,
                    	DVLines[i].x, y);
                  break;
               default:
                 	WinDrawChars (&rec.fields[DVLines[i].fieldNum]
                    	[DVLines[i].offset], DVLines[i].length,
                    	DVLines[i].x, y);
						break;
					}
            }
      }  // end for loop
  
   FntSetFont (curFont);
    
  	// Redraw the quick close (X) button on title bar.
   DetailViewDrawXButton ();
   
   // Update the scroll buttons.
   scrollableU = (Boolean) (TopDetailViewLine != 0);
   scrollableD = (Boolean) (i < DetailViewLastLine);
   UpdateScrollers (upButton, downButton, scrollableU, scrollableD);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DetailViewAddField
//
// DESCRIPTION:	Adds a field to the Detail Viewer
//
// PARAMETERS:  	-> aField 		- 	pointer to text string to add 
//              	-> fieldNum 	-	field to add
//              	<> width 		-	width already occupied on the line
//              	-> maxWidth 	- 	can't add words past this width
//              	-> indentation	- 	the amount of indentation wrapped lines of
//                              	  	text should begin with (except the last)
//
// RETURNED:    	width is set to the width of the last line added
//              	returns true entire field fit, or false if it ran out of room.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewAddField (Char* aField, const UInt16 fieldNum,
   UInt16 *width, const UInt16 maxWidth, const UInt16 indentation)
{
   UInt16  	length;
   UInt16  	offset = 0;
   UInt16  	newOffset = 0;

   if (aField == NULL || *aField == '\0')
      return;
   
   // If we're past the maxWidth already then start at the beginning
   // of the next line.
   if (*width >= maxWidth)
      *width = indentation;
      
   do {
		if (DetailViewLastLine >= DVLinesMax) {
		   // sets detailViewLastLine back to end of last fully drawn field.  This
		   // should only happen if in middle of a long note record.
			break;
			}
      
		// Check if we word wrapped in the middle of a word which could 
		// fit on the next line.  Word wrap doesn't work well for use
		// when we call it twice on the same line.
		// The first part checks to see if we stopped in the middle of a line
		// The second part check to see if we didn't stop after a word break
		// The third part checks if this line wasn't as wide as it could be
		// because some other text used up space.
	   length = FldWordWrap (&aField[offset], 
		   maxWidth - *width); // Get number of characters than can be displayed
		   
	   // Check if last character is end-of-string & next to last
	   // character is a white space.
	   if (aField[offset + length] != '\0'
		   && !TxtCharIsSpace ((UInt8) aField[offset + length - 1])
		   && !TxtCharIsDelim ((UInt8) aField[offset + length - 1]) // DTR 4/24/2002
		   && (*width > indentation))
		   // if at start of line (after header), don't word wrap...there must be
		   // some data printed on this line else next line will display incorrectly.
		   if (*width != HEAD_WID)
		   	length = 0; // don't word wrap - try next line
		
	   // Lines returned from FldWordWrap may include a '\n' at the
	   // end.  If present remove it to keep it from being drawn.
	   // length 0 also happens when word wrap fails.
	   newOffset = offset + length;
	   if (newOffset > 0 && aField[newOffset - 1] == linefeedChr)
		   length--;
 
		DVLines[DetailViewLastLine].fieldNum = fieldNum;
		DVLines[DetailViewLastLine].offset = offset;
		DVLines[DetailViewLastLine].x = *width;
		DVLines[DetailViewLastLine].length = length;
		DetailViewLastLine++;  // start next line
		offset = newOffset;

		// Wrap to the start of the next line if there's still more text
		// to draw (so we must have run out of room) or wrap if we
		// encountered a line feed character.
		if (aField[offset] != '\0')
		   *width = indentation;
		else
		   break;
	   
   } while (true);

   // If the last character was a new line then there is no width.
   // Otherwise the width is the width of the characters on the last line.
   if (aField[offset - 1] == linefeedChr)
      *width = MARGIN_WID;
   else
      *width += FntCharsWidth(&aField[
         DVLines[DetailViewLastLine - 1].offset], 
         DVLines[DetailViewLastLine - 1].length);
         
   return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DetailViewScrollOnePage
//
// DESCRIPTION: 	Scrolls a detail window by one page less one line.
//
// PARAMETERS: 	-> newTopRecordViewLine - top line of the display
//              	-> direction   -	up or down.
//						-> viewGadget  - 	gadget to scroll.
//
// RETURNED:    	new newTopRecordViewLine one page away
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
 static UInt16 DetailViewScrollOnePage (UInt16 newTopRecordViewLine, 
   WinDirectionType direction, UInt16 viewGadget)
{
   Int16          offset;
   Int16          stdFontLineHeight;
   RectangleType  rect;
   Int16          viewDisplayHeight;
   float          units;
   UInt16         limit = scrollSpeedLimit;
   FontID   		curFont;			

   units = (float) ScrollUnits / (float) limit;
   
   // scroll up to 2 pages at a time
	if (units == 1)
		units = 2;

   curFont = FntSetFont (PrefFnts[DetailViewFont]);
   stdFontLineHeight = FntLineHeight();
   FntSetFont (curFont);
   
   GetObjectBounds (viewGadget, &rect);
   viewDisplayHeight = (Int16) (rect.extent.y * units);
   
   // When scrolling down we check that we have not left any blank lines at 
   // the bottom of the window by calling this function with DetailViewLastLine
   // as the newTopRecordViewLine.  So the following lines make sure we reset
   // viewDisplayHeight to its maximum value first so that we calculate total
   // screen space properly.
   if (newTopRecordViewLine == DetailViewLastLine)
      viewDisplayHeight = rect.extent.y;
   
   if (direction == winUp)
      offset = -1;
   else
      offset = 1;
   
   while (viewDisplayHeight >= 0 &&
    (newTopRecordViewLine > 0 || direction == winDown) && 
    (newTopRecordViewLine < (DetailViewLastLine - 1) || direction == winUp)) {
      newTopRecordViewLine += offset;
      viewDisplayHeight -= DetailViewCalcNextLine (newTopRecordViewLine,
         stdFontLineHeight);
       };
      
   // Did we go too far?
   if (viewDisplayHeight < 0) {
      // The last line was too much so remove it
      newTopRecordViewLine -= offset;
      
      // Also remove any lines which don't have a height
      while (DetailViewCalcNextLine (newTopRecordViewLine, 2) == 0)
         {
         newTopRecordViewLine -= offset;   // skip it
         }
      }
    
   return newTopRecordViewLine;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewScroll
//
// DESCRIPTION:   Scrolls text in a gadget object. Used by all Detailt View
//                functions.
//
// PARAMETERS:    -> direction   - winUp or winDown
//                -> viewGadget  - gadget to scroll.
//
// RETURNED:      True if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewScroll (WinDirectionType direction, UInt16 viewGadget)
{
   Int16		lastRecordViewLine;
   UInt16	newTopRecordViewLine;
   
	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus (0);
   
   AdjustScrollRate (); // adjust scroll rate if necessary
   	
   newTopRecordViewLine = TopDetailViewLine;
   
	if (direction == winUp) {
		newTopRecordViewLine = DetailViewScrollOnePage (newTopRecordViewLine, direction,
		   viewGadget);
		}
   else { // direction == winDown
		// Two part algorithm.
		// 1) Scroll down one page; 2) Scroll up one page from the bottom.
		// Use the higher of the two positions.  Find the line one page down.

		newTopRecordViewLine = DetailViewScrollOnePage (newTopRecordViewLine,
			direction, viewGadget);

		// Find the line at the top of the last page 
		lastRecordViewLine = DetailViewScrollOnePage (DetailViewLastLine, winUp,
		   viewGadget);

		// we shouldn't be past the top line of the last page
		if (newTopRecordViewLine > lastRecordViewLine)
		   newTopRecordViewLine = lastRecordViewLine;
		}

   if (newTopRecordViewLine != TopDetailViewLine) {
      TopDetailViewLine = newTopRecordViewLine;
      EraseRectangleObject (viewGadget);     
      
      switch (viewGadget)
         {
         case IndiDetailDetailGadget:
            DetailViewDraw (IndiRec, Indi, IndiDetailDetailGadget,
            	IndiDetailScrollUpRepeating, IndiDetailScrollDownRepeating);
            break;
         case ChilDetailDetailGadget:
            DetailViewDraw (ChilRec, Chil, ChilDetailDetailGadget,
            	ChilDetailScrollUpRepeating, ChilDetailScrollDownRepeating);
            break;
         case EvenDetailDetailGadget:
  	         DetailViewDraw (EvenRec, Even, EvenDetailDetailGadget,
  	         	EvenDetailScrollUpRepeating, EvenDetailScrollDownRepeating);
            break;
         case SouCDetailDetailGadget:
         	DetailViewDraw (SouCRec, SouC, SouCDetailDetailGadget,
         		SouCDetailScrollUpRepeating, SouCDetailScrollDownRepeating);
            break;
         case SourDetailDetailGadget:
         	DetailViewDraw (SourRec, Sour, SourDetailDetailGadget,
         		SourDetailScrollUpRepeating, SourDetailScrollDownRepeating);
            break;
         case RepoDetailDetailGadget:
				DetailViewDraw (RepoRec, Repo, RepoDetailDetailGadget,
					RepoDetailScrollUpRepeating, RepoDetailScrollDownRepeating);
            break;
        case RepCDetailDetailGadget:
        		DetailViewDraw (RepCRec, RepC, RepCDetailDetailGadget,
        			RepCDetailScrollUpRepeating, RepCDetailScrollDownRepeating);
            break;
         case FamiDetailDetailGadget:
         	DetailViewDraw (FamiRec, Fami, FamiDetailDetailGadget,
         		FamiDetailScrollUpRepeating, FamiDetailScrollDownRepeating);
            break;
         case AliaDetailAliaDetailGadget:
         	DetailViewDraw (AliaRec, Indi, AliaDetailAliaDetailGadget,
         		AliaDetailScrollUpRepeating, AliaDetailScrollDownRepeating);
            break;
         default:
            break;
         }   
      }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewNewLine
//
// DESCRIPTION:   Adds the next field at the start of a new DetailView line
//
// PARAMETERS:    <> width - 	width already occupied on the line
//
// RETURNED:      width is set
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewNewLine (UInt16* width)
{
   if (DetailViewLastLine >= DVLinesMax)
      return;
      
   if (*width == MARGIN_WID) {
      DVLines[DetailViewLastLine].fieldNum = detailViewBlankLine;
      DVLines[DetailViewLastLine].x = MARGIN_WID;
      DetailViewLastLine++;
      }
   else
      *width = MARGIN_WID;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewNewLineIf
//
// DESCRIPTION:   Adds a blank line to DVLines if last line was not
//                a blank line.
//
// PARAMETERS:    <- width -	width already occupied on the line 
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewNewLineIf (UInt16* width)
{
   // If the line above isn't blank and isn't first line then add a blank line
   if (DetailViewLastLine > 0 &&
      DVLines[DetailViewLastLine - 1].fieldNum != detailViewBlankLine) {
      DetailViewNewLine (width);
      }
}
   
////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:      DetailViewAddSpaceForText
//
// DESCRIPTION:   Adds space for text to the RecordViewLines info. 
//
// PARAMETERS:    -> string - pointer to text to leave space for
//                <- width - 	width already occupied on the line
//
// RETURNED:      width is increased by the width of the text
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewAddSpaceForText (const Char* string, UInt16* width)
{
   *width += FntCharsWidth (string, StrLen (string));
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewPositionTextAt
//
// DESCRIPTION:   Position the following text at the given position. 
//
// PARAMETERS:    <> width -	width already occupied on the line
//
// RETURNED:      width is increased if the position is greater
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewPositionTextAt (UInt16* width)
{
   if (*width < HEAD_WID)
      *width = HEAD_WID;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewQuickClose
//
// DESCRIPTION:   Called when user hits X Close Button to exit from a
//						detail view form.  It reinitializes the Prior Detail
//						View Lines, closes all forms and sends a FrmGotoForm
//						to return to Indivual Summary Screen. 
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewQuickClose (void)
{
	if (DVLines) {
		MemHandleFree (DVLinesH);
      DVLines = NULL;
		}
		
 	if (SouCListH) {
		MemHandleFree (SouCListH);
   	SouCListH = NULL;
   	}
   	
  	DbMemHandleUnlock (&NoteRecH);  		
	DetailViewInitPriorLines ();
	FrmReturnToForm (0);
	
	if (Pre35Rom) {  // do following for backward compatablity
		UpdateFrm = true;
		FrmUpdateForm (IndiSummForm, updateViewReInit);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewDrawXButton
//
// DESCRIPTION:   Draw the X Button in the Detail View Form tital area.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewDrawXButton (void)
{
	MemHandle		xButtonH;
	BitmapPtr 		xButtonP;
	RectangleType 	rect;

	FrmGetFormBounds (FrmGetActiveForm (), &rect);

	xButtonH = DmGetResource (bitmapRsc, CloseBoxBitmap); // draw x sign bitmap
   xButtonP = MemHandleLock (xButtonH);
   WinDrawBitmap (xButtonP, rect.extent.x-15, 1);
   MemHandleUnlock (xButtonH);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewInit
//
// DESCRIPTION:   Initializes the Detail View array and related variables. 
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DetailViewInit (void)
{
   ErrFatalDisplayIf (DVLines, "DetailViewInit: DVLines should be 0");

   DVLinesH = NULL;
   
   // Allocate the record view lines (dynamic heap) and return a handle to it.
   while (DVLinesH == NULL) {
		DVLinesH = MemHandleNew (sizeof (DVLineType) * DVLinesMax);
		if (!DVLinesH)
			DVLinesMax = DVLinesMax / 2;
		if (DVLinesMax < 30) break;
		}
		
   ErrFatalDisplayIf (!DVLinesH, "DetailViewInit: Out of memory");
   DVLines = MemHandleLock (DVLinesH);  

   DetailViewLastLine = 0; // updated in DetailViewAddField
   TopDetailViewLine  = 0; // updated in DetailViewScroll
   DetailViewFirstPlainLine = 0;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewResizeForm
//
// DESCRIPTION:   This routine resizes and draws the Individual Summary Form.
//
// PARAMETERS:   	-> newForm	-	true if drawing form for first time (eg on
// 										frmOpenEvent).
//
// RETURNED:      True if form was resized, else false (no changed in size).
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean DetailViewResizeForm (Boolean	newForm)
{
	Int16 			extentDelta;
	RectangleType 	curBounds; 		// bounds of active form
	RectangleType	displayBounds; // bounds of screen
	FormPtr			frmP;

	if (!DynInDevice)
		return false;
	
	frmP = FrmGetActiveForm();

	// Get dimensions of current active form.
	WinGetBounds (FrmGetWindowHandle (frmP), &curBounds);
	
	// Get the new display window bounds
	WinGetBounds (WinGetDisplayWindow (), &displayBounds);

	extentDelta = 	(displayBounds.extent.y + displayBounds.topLeft.y) -
						(curBounds.extent.y + curBounds.topLeft.y + 2); // add 2 for bounds

	if (extentDelta == 0) // form has not changed in size, so do nothing
		return false;
	
	curBounds.topLeft.y += extentDelta;
	
	if (!newForm){ // then we are enlarging the screen
		FrmSetActiveForm (FrmGetFormPtr (IndiSummForm));
		IndiSummResizeForm ();
		IndiSummDrawData (true); // update table too
		FrmSetActiveForm (frmP);
		}
	
	WinSetBounds (FrmGetWindowHandle (frmP), &curBounds); // set new form bounds
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetailViewHandleVirtual
//
// DESCRIPTION:   Handles keyDownEvent when up or down button is pressed.
// 
// PARAMETERS:    -> event  	 	- 	a pointer to an EventType structure
//                -> viewGadget  - 	gadget to scroll.
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		Updated 12-13-2005 to remove call to EvtKeydownIsVirtual
////////////////////////////////////////////////////////////////////////////////////
/*static Boolean DetailViewHandleVirtual (EventPtr event, UInt16 viewGadget)
{
	WinDirectionType direction;
	
   if (EvtKeydownIsVirtual (event)) {
	
     	switch (event->data.keyDown.chr)
     		{
        	case vchrPageUp:
        	case vchrPageDown:
				if (event->data.keyDown.chr == vchrPageUp)
					direction = winUp;
				else
					direction = winDown;
					
        	   // Reset scroll rate if not auto repeating
		   	if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
	   			ResetScrollRate();
    	   
        	   DetailViewScroll (direction, viewGadget);
           	return true;
           	break;

          default:
             break;
          }
      }
      
   return false;  // not handled
}
*/

static Boolean DetailViewHandleVirtual (EventPtr event, UInt16 viewGadget)
{
	DirType	navDir;

   if (NavKeyHit (event, &navDir)) {
				
		switch (navDir) {
				
			case NavUp:
			case NavDn:

        		// Reset scroll rate if not auto repeating
   			if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
  					ResetScrollRate ();
	   		
	   		switch (viewGadget) {
	   			
	   			case NoteDetailDetailField:
			   		ScrollPage (navDir == NavUp ? winUp : winDown, NoteDetailDetailField,
							NoteDetailScrollScrollBar);		 
	   				break;
	   				
	   			case 	AliaListAliaListTable:
		   			AliaListScroll (navDir);
						break;
						
					default:
				   	DetailViewScroll (navDir == NavUp ? winUp : winDown, viewGadget);		
						break;	   					
	   			}
      		
        		return true;
					
			case NavL:
			case NavR:
				
				if (viewGadget == SouCDetailDetailGadget) //scroll to next/last citation
					SouCListScroll (navDir);
			
				return true;
					
			default:
				break;
			}
      }

   return false;  // not handled
}


#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummUpdFamiButtons
//
// DESCRIPTION:	Updates the Family Event and Family Child Buttons on the 
//              	Individual Info Form.
//
//						IMPORTANT: EventsVisible and LastEvenRecN must be initialized
//						prior to calling this function.
//
//						The following variables are loaded in this function:
//
//						ShowFamiButtons - Set to true if we are viewing family data
//												and there are family events other than the
//												primary marriage event.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummUpdFamiButtons (void)
{
 	ShowFamilyButtons = false;
   
  	// If showing family data and there are family events...
  	if (!EventsVisible && FirstEvenRecN != NO_REC_LONG) {
   	ShowFamilyButtons = true;
   	SetControlValue (IndiSummChilListButton, !FamiEventsVisible);
      SetControlValue (IndiSummFamiListButton, FamiEventsVisible);
      }
   	
	// Show or hide the family list buttons.
	ShowObject (IndiSummFamiListButton, ShowFamilyButtons);
   ShowObject (IndiSummChilListButton, ShowFamilyButtons);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiSummDrawEvenFld
//
// DESCRIPTION:   Draws the birth date/place and death date/place within the screen
//						bounds passed.
//
// PARAMETERS:    -> record - record to draw.
//                -> bounds - bounds of the draw region.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummDrawEvenFld (DBRecordType* record, RectanglePtr bounds)
{
   Int16    x, y;
   Int16    fieldSeparatorWidth;
   Char     *eDate;
   Char     *ePlace;
   Char     type[5];  // max size of abbr even description is 3 chars
   Int16    dateLength  = 0;
   Int16    placeLength = 0;
   Int16    dateWidth 	= 0;
   Int16    placeWidth 	= 0;
   UInt16   offset 		= 0;
   Boolean  fitsInWidth;
   UInt16   indentation = INDI_SUMM_EVEN_INDENT;
   Int16  	maxWidth;
   FontID   curFont;
   UInt16   shortenedFieldWidth;
   Boolean	onSecondLine = false;
   
   shortenedFieldWidth = FntCharWidth ('.') * 3; // draw if have to cut off field
   fieldSeparatorWidth = FntCharsWidth ("   ", 3); 

   // Draw the abbreviated title of the event field
   x = bounds->topLeft.x;
   y = bounds->topLeft.y;
  
   ErrFatalDisplayIf (StrLen (EvenDesc[StrAToI (record->fields[evenType])][1]) > 3,
   	"IndiSummDrawEvenFld: String length error");
   	
   // Draw 3-character title for the Event
   StrPrintF (type, "%s:", EvenDesc[StrAToI (record->fields[evenType])][1]);
   eDate = record->fields[evenDate];
   ePlace = record->fields[evenPlac];
   
   curFont = FntSetFont (TITLE_FONT);
   WinDrawChars (type, StrLen (type), x, y);
   FntSetFont (PrefFnts[IndiSummFont]);
   
   // Draw the Event field data
   x = indentation;
   maxWidth = bounds->extent.x - x; 

   // If no date or place then set date to "Unknown" string
   if (!eDate && !ePlace && !Prefs[DisplayBlank])
      eDate = cUnknownStr; // string assumed not to need clipping
  
   if (eDate) { // display the Date information
 
		// -- Draw Date on Line 1 -- //
      dateWidth = FntCharsWidth (eDate, StrLen (eDate));
   
   	if (dateWidth > maxWidth) {
   		dateLength = FldWordWrap (eDate, maxWidth);
   		offset = dateLength;
			onSecondLine = true;
			}
   	else {
   		dateLength = StrLen (eDate);
   		}

   	WinDrawChars (eDate, dateLength, x, y);
   	x += dateWidth;
		indentation = SUB_HEAD_WID;
 
   	if (onSecondLine) {
   	
   		// -- Draw Date on Line 2 -- //
  	 		x = indentation;
  			y += FntLineHeight (); // move to next line
   		maxWidth = bounds->extent.x - x; 
   		
   		dateLength = dateWidth = maxWidth; // longer & wider than possible
      	FntCharsInWidth (&eDate[offset], &dateWidth, &dateLength, &fitsInWidth);
			WinDrawChars (&eDate[offset], dateLength, x, y);
			x += dateWidth;
			} // if (onSecondLine)

  		x += fieldSeparatorWidth; // space betw. date and place
  		
  		if (x >= bounds->extent.x && !onSecondLine) { // move to second line
			x = indentation;
			y += FntLineHeight ();
			onSecondLine = true;
  			}

   	maxWidth = bounds->extent.x - x;
   	
   	if (maxWidth < 0)
   		maxWidth = 0;
   	}
   
   if (ePlace) { // Display the Place information
   
    	offset = 0;
    	
    	if  (!onSecondLine) {
    	
			// -- Draw Place on Line 1 -- //    	
    		placeLength = placeWidth = maxWidth; // longer & wider than possible
         
      	FntCharsInWidth (ePlace, &placeWidth, &placeLength, &fitsInWidth);
    		
    		if (placeLength > 0) {

	    		placeLength = FldWordWrap (ePlace, maxWidth);
    		
        		// Check if last character is end-of-string & next-to-last
        		// character is a white space.
	     		if (ePlace[placeLength] != '\0'
		  			&& !TxtCharIsSpace ((UInt8) ePlace[placeLength - 1])
		  			&& !TxtCharIsDelim ((UInt8) ePlace[placeLength - 1])
		  			&& (x > indentation)) {
		  	 		placeLength = 0; // don't word wrap - try next line
		     		}
	  			}
	  			
      	WinDrawChars (ePlace, placeLength, x, y);
      	offset = placeLength;
        	indentation = SUB_HEAD_WID; // indent a little on second line
         y += FntLineHeight (); // move to next line
         x = indentation;
         maxWidth = bounds->extent.x - x;
         
      	} // if (!onSecondLine)
         
      // -- Draw Place on Line 2 -- //         
      placeWidth = FntCharsWidth (&ePlace[offset], StrLen (&ePlace[offset]));
       
      if (placeWidth > maxWidth) {
      	maxWidth -= shortenedFieldWidth;
      	
     		if (maxWidth < 0)
     			maxWidth = 0;
       		
      	placeLength = placeWidth = maxWidth; // longer & wider than possible
             
      	FntCharsInWidth (&ePlace[offset], &placeWidth, &placeLength, &fitsInWidth);

      	if (placeLength > 0) {
      		placeLength = FldWordWrap (&ePlace[offset], maxWidth);
         	WinDrawChars (&ePlace[offset], placeLength, x, y);
         	x += FntCharsWidth (&ePlace[offset], placeLength);
     	   	WinDrawChars (SHORTENED_FLD_STR, 3, x, y); // draw ellipsis
     	   	}
      	}
      else { // if (placeWidth > maxWidth)  so place is not truncated
      	WinDrawChars (&ePlace[offset], StrLen (&ePlace[offset]), x, y);
         }
         
      } // if (ePlace)

   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiSummNameDrawRecord
//
// DESCRIPTION:   Draws the name on the Individual Summary form.  It is called as
//						a callback routine by the table object.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
 
static void IndiSummNameDrawRecord (void)
{
   RectangleType	rect;
   FontID   		curFont= FntSetFont (PrefFnts[IndiSummNameFont]);

	GetObjectBounds (IndiSummNameButton, &rect);
   WinEraseRectangle (&rect, 0);
   DrawRecordNameAndLifespan (&IndiRec, &rect, false, false);
   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummEventsDrawRecord
//
// DESCRIPTION: 	This routine draws the birth and death events on the Indivudual
//					 	Summary Screen.  It is called as a callback routine by
//					 	the table object.
//
// PARAMETERS:  	-> buttonID  - button to draw (IndiSummBirthButton
//											or IndiSummDeathButton).
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
//
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummEventsDrawRecord (UInt16 buttonID)
{
   UInt32 	    	recN;  // record in EvenDB
   DBRecordType   rec;
   MemHandle      recH = NULL;
   RectangleType	rect;

 	ErrFatalDisplayIf (buttonID != IndiSummBirthButton && buttonID != IndiSummDeathButton,
 	 "IndiSummEventsDrawRecord : Invalid Button");

 	GetObjectBounds (buttonID, &rect);
 	WinEraseRectangle (&rect, 0);

 	recN = (buttonID == IndiSummBirthButton ? PrimaryBirtRecN : PrimaryDeatRecN);
 
	if (recN != NO_REC_LONG) {
      DbGetRecord (EvenDB, recN, &rec, &recH);
     	IndiSummDrawEvenFld (&rec, &rect);
     	DbMemHandleUnlock (&recH);
     	}
   else {  // no birth or death data
	   rec.fields[evenDate] = NULL;
  	   rec.fields[evenPlac] = NULL;
      if (buttonID == IndiSummBirthButton)
        	 rec.fields[evenType] = EVEN_BIRT_STR; // default if no birth record
      else
         rec.fields[evenType] = EVEN_DEAT_STR; // default if no death record

      IndiSummDrawEvenFld (&rec, &rect);
      CtlSetEnabled (GetObjectPtr (buttonID), false); // disable button
      }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummCtlHighlight
//
// DESCRIPTION: 	This routine highlights (selects) or unhighlights a control on
//						the Individual Summary Screen.  This only works for one of the
//						the following buttons: IndiSummBirthButton, IndiSummDeathButton,
//						IndiSummNameButton, IndiSummSpouListButton,
//						IndiSummMarrListButton, and IndiSummFamiNoButton.
//
// PARAMETERS:  	-> objectID		- object id of control to highlight.
//						-> highlight	- true to highlight else false.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void IndiSummCtlHighlight (UInt16 objectID, Boolean highlight)
{
	if (!Pre35Rom) {
	
		WinPushDrawState ();
		
		if (highlight) {
			WinSetBackColor (UIColorGetTableEntryIndex (UIObjectSelectedFill));
			WinSetTextColor (UIColorGetTableEntryIndex (UIObjectSelectedForeground));
			}
	   
	   switch (objectID)
	   	{
	   	case IndiSummBirthButton:
		   case IndiSummDeathButton:
	   		IndiSummEventsDrawRecord (objectID);
	   		break;
	   	
	   	case IndiSummNameButton:
		   	IndiSummNameDrawRecord ();
		   	break;
		   
		   case IndiSummSpouListButton:	
		   	FamiSpouDrawRecord ();
		   	break;
		   
		   case IndiSummMarrListButton:
			   FamiMarrDrawRecord ();
			   break;
		
			case IndiSummFamiNoButton:
				FamiNumberDraw (false);
				break;
				
			default:
				break;	
			}
			
		WinPopDrawState ();
		}
		
   else { // Pre35Rom (non-color screen)
	   SetControlValue (objectID, highlight);
   	}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummCtlSelected
//
// DESCRIPTION: 	This routine checks the the given control has been selected by
//						tracking the pen down event.
//
// PARAMETERS:  	-> objectID	-	object id of control entered.
//
// RETURNED:    	True if control has been selected, else false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean IndiSummCtlSelected (UInt16 objectID)
{
	Coord 			newPointX, newPointY;
	Boolean 			isPenDown = true;
	RectangleType 	bounds;
	
	// Highlight the control entered.
	IndiSummCtlHighlight (objectID, true);
        	      
	GetObjectBounds (objectID, &bounds);
        	      
	// Track the pen down event
	while (isPenDown) {
		EvtGetPen (&newPointX, &newPointY, &isPenDown);
		}

	// Unhighlight the control entered.
	IndiSummCtlHighlight (objectID, false);
   		   	
	// The pen up was also in the gadget.
	return (RctPtInRectangle (newPointX, newPointY, &bounds));
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummHighlightSearchCtrl
//
// DESCRIPTION: 	This routine highlights a control button after returning from a
//						Search. 
//
// PARAMETERS:  	-> evenLstTableP - pointer to table in which to highlight data.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummHighlightSearchCtrl (TablePtr	evenLstTableP)
{
	Int16 	row;
   UInt16	objectID = 0; // init

	if (PrimaryBirtRecN == CurrentEvenRecN)
		objectID = IndiSummBirthButton;
		
	else if (PrimaryDeatRecN == CurrentEvenRecN)
		objectID = IndiSummDeathButton;
				
 	else if (CurrentMarrRecN == CurrentEvenRecN)
		objectID = IndiSummMarrListButton;
	
	else if (TblFindRowData (evenLstTableP, CurrentEvenRecN, &row)) {
		TblSelectItem (evenLstTableP, row, 0);
		SysTaskDelay (SysTicksPerSecond() * .85); // highlight for under a second
		TblUnhighlightSelection (evenLstTableP);
		}
			  	
	if (objectID != 0) {
		IndiSummCtlHighlight (objectID, true);
		SysTaskDelay (SysTicksPerSecond() * .85);
		IndiSummCtlHighlight (objectID, false);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummLoadData
//
// DESCRIPTION: 	This routine loads data into the Individual Summary Screen.
//
//						Note: Prior to callign this function the following global
// 					variables must be set:
//
//						CurrentIndiRecN	-	current individual being viewed.
// 					CurrentSpouRecN	-  must be loaded only if AlreadyHaveFamily.
//						FamiRec				-	must be loaded only if AlreadyHaveFamily.
//
//
//						This function loads the the following variables:
//						
//						PrimaryBirtRecN	-	record number in EvenDB.
//   					PrimaryDeatRecN	-	record number in EvenDB.
//   					LastFamiNum			-	total count of individual's families.
//						TopVisFamiNum		-	set only if !AlreadyHaveFamily and
//													!SrchShowFamiEven. Set to 0 if no families.
//						CurrentSpouRecN	-	spouse record number (in IndiDB). Taken from 
//													FamiRec.
//   					CurrentMarrRecN	-	record number (in EvenDB) of current marriage
//													being viewed. Taken from FamiRec.
//						AlreadyHaveFamily	-	re-init to false
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummLoadData (void)
{
   UInt16	xPos, yPos;
   Char		famiXRef[NREF_LEN+1];
  	UInt16 	holdRecN1, holdRecN2; // record in IndiDB

   // Initialize variables.
   CurrentMarrRecN = PrimaryBirtRecN = PrimaryDeatRecN = NO_REC_LONG;
   LastFamiNum     = 0;
   
   // Get the individual record selected from the main pick list.
   DbGetRecord (IndiDB, CurrentIndiRecN, &IndiRec, &IndiRecH);
   
   // If name picked from Individual List is an alias then get primary name.
   if (IndiRec.fields[indiAlias] && !StrCompare (IndiRec.fields[indiAlias], "A")) { 
      CurrentIndiRecN = (UInt16) StrAToI (IndiRec.fields[indiNo]);
      DbMemHandleUnlock (&IndiRecH);
      DbGetRecord (IndiDB, CurrentIndiRecN, &IndiRec, &IndiRecH);
      }

	ErrFatalDisplayIf (!IndiRecH, "IndiSummLoadData: IndiRecH unlocked");

	// Add current individual record number to Jump List. If current record
	// is already in list, add it to top and delete it in other location.
	xPos = yPos = 1;
	holdRecN1 = Jump[yPos-1];
	
	while (yPos < JUMP_MAX) {
		holdRecN2 = Jump[yPos];
		if (holdRecN1 != CurrentIndiRecN) {
			Jump[xPos] = holdRecN1;
			holdRecN1 = holdRecN2;
			xPos++;
			}
		yPos++;
		}
		
	Jump[0] = CurrentIndiRecN; // add person

   // Set record number of primary birth event record, if any.
   if (IndiRec.fields[indiBEvenNo]) {
		PrimaryBirtRecN = (UInt32) StrAToI (IndiRec.fields[indiBEvenNo]);
		}
   
   // Set record number of primary death event record, if any.
   if (IndiRec.fields[indiDEvenNo]) {
		PrimaryDeatRecN = (UInt32) StrAToI (IndiRec.fields[indiDEvenNo]);
		}

   // Find individual's family data, if any, and load the FamiRec.				//
   // AlreadyHaveFamily is set to TRUE in ParentFinder function after the		//
   // Parents button is pressed on the Individual Summary Screen so that		//
   // the Family Record is not reloaded here.											//
   if (!AlreadyHaveFamily) {  
   	
   	// Scroll to correct position in Family List.
   	if (SrchShowFamiEven) { // entering from Search list
			
			StrIToA (famiXRef, (Int32) CurrentFamiRecN);
			
			if (!GetRefNumPos (famiXRef, IndiRec.fields[indiFamSNo], FAMI_DLM,
				&TopVisFamiNum))
				goto NoFamilyData;
			}
      
      else { // not entering from Search list
      
	      TopVisFamiNum = 1; // must init here to 1;
      
	      // Find TopvisibleFamiNum family in the FamiDB, if any 
   	   if (!RefFinderStr (TopVisFamiNum, FAMI_DLM, IndiRec.fields[indiFamSNo],
      	 	famiXRef))
         	goto NoFamilyData;
         
			CurrentFamiRecN = (UInt32) StrAToI (famiXRef); 
			}

		// Get the FamiRec.
		if (DbGetRecord (FamiDB, CurrentFamiRecN, &FamiRec, &FamiRecH) != 0) {
			ErrFatalDisplay ("IndiSummLoadData: Error finding FamiRec");
			goto NoFamilyData;
			}
         
      // Get the Spouse associated with the FamiRec.
      CurrentSpouRecN = NO_REC;
      
      if (FamiRec.fields[famiHusbNo]) {
      	
      	UInt16 husbRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);
      	
      	if (CurrentIndiRecN == husbRecN) {
      	//if (CurrentIndiRecN == (UInt16) StrAToI (FamiRec.fields[famiHusbNo])) {
      		if (FamiRec.fields[famiWifeNo])
      			CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiWifeNo]);
         	}
      	else
      		CurrentSpouRecN = husbRecN;
      		//CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);
			}
 
      }
      
   else { // AlreadyHaveFamily
   
   	ErrFatalDisplayIf (!FamiRecH, "IndiSummLoadData: FamiRecH unlocked");
      AlreadyHaveFamily = false;  // re-init
      // the TopVisFamiNum already set in ParentFinder
      }
  	
  	// Get the LastFamiNum
  	LastFamiNum = RefCounter (FAMI_DLM, IndiRec.fields[indiFamSNo]);
  	ErrFatalDisplayIf (LastFamiNum == 0, "IndiSummLoadData: bad LastFamiNum");
  	
   // Get individual marriage record number (in EvenDB)
   if (FamiRec.fields[famiEvenMNo])
   	CurrentMarrRecN = (UInt32) StrAToI (FamiRec.fields[famiEvenMNo]);
   
   return;
   
   ////////////
   NoFamilyData:
   ////////////
   
   TopVisFamiNum 		= 0;
   CurrentSpouRecN 	= NO_REC;
   CurrentFamiRecN 	= NO_REC_LONG;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummScrollFami
//
// DESCRIPTION: 	This routine scrolls the family list. In doing so, it loads
//						child data variables, redraws the child list, and updates
//						the child count and family number.
//
// PARAMETERS:  	-> direction - direction to scroll families.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummScrollFami (DirType direction)
{
 	if (!FamiListScroll (direction))
 		return; // family list was not scrolled
 		
   ChilLoadData (); // find positions of first and last child in ChilDB
   
   if (!EventsVisible) { // then viewing Family data
   	EvenLoadData ();  // get Family Events, if any
      FamiEventsVisible = false; // default to child list
      ChilListInit ();
      IndiSummUpdFamiButtons ();
      TblRedrawTable ((TablePtr) GetObjectPtr (IndiSummEvenListTable));
      }
   // else if EventsVisible don't need to reload
    
   FamiNumberDraw (true); // needed for update to child count
}


////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummFamiOrEvenButton
//
// DESCRIPTION: 	Handles when user presses either the Family Button or Event 
//						Button on the Individual Summary Screen.  Does initializations
//						and calls routines to show either the Child List or Event List.
//
//						This function sets the the following variables:
//
//						EventsVisible 		-	set to true if showing non-family events, else
//													set to false.
//
//						FamiEventsVisible - 	set to false.
//
// PARAMETERS:  	-> evenLstTableP - pointer to event / child list table.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummFamiOrEvenButton (TablePtr evenLstTableP)
{
	ErrFatalDisplayIf (EventsVisible && FamiEventsVisible, "FamiEventsVisible should not be true here.");
	
	EventsVisible = !EventsVisible; // always call before IndiSummEvenInit
	
	FamiEventsVisible = false;	  // always reset here
	
	SetControlValue (IndiSummEvenButton, EventsVisible);
	SetControlValue (IndiSummFamiButton, !EventsVisible);
	
	TblEraseTable (evenLstTableP);
	
	// Call the following again in case family number changed.
   EvenLoadData (); // get general or family events & init variables.

	if (EventsVisible) {
		EvenListInit ();
	   }
	else {
		ChilLoadData (); // do this in case family number changed
      ChilListInit ();
      }
 
   IndiSummUpdFamiButtons ();
	TblDrawTable (evenLstTableP);
	ShowObject (EventsVisible ? IndiSummFamiButton : IndiSummEvenButton, true); // for Palm III devices - redraws button
	DrawScreenLines (IndiSummForm);
	
	ErrFatalDisplayIf (ShowFamilyButtons && EventsVisible, "IndiSummFamiOrEvenButton: ShowFamilyButtons should false.");
 }

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummChilOrEvenButton
//
// DESCRIPTION: 	Handles when user presses either the Family Child Button or Family 
//						Event Button on the Individual Summary Screen.  Does
//						initializations and calls routines to show either the Child List
//						Family Event List.
//
//						This function sets the the following variables:
//
//						FamiEventsVisible - 	set to true is showing Family Events, else
//													false if showing Children.
//
// PARAMETERS:  	-> evenLstTableP - pointer to Event / child list table.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummChilOrEvenButton (TablePtr evenLstTableP)
{
	FamiEventsVisible = !FamiEventsVisible;
	
	TblEraseTable (evenLstTableP);
  	IndiSummUpdFamiButtons ();

   if (FamiEventsVisible)
     	EvenListInit ();
   else
     	ChilListInit ();

   TblDrawTable (evenLstTableP);
   FamiNumberDraw (true);
   DrawScreenLines (IndiSummForm);
 }

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummUpdForms
//
// DESCRIPTION: 	This routine updates the Individual Summary Form, if needed.
//           		This is done for backward compatability to redraw base form as
//						well as any form on top of it.
//
// PARAMETERS:  	-> updTopForm - true to update top form.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void IndiSummUpdForms (Boolean updTopForm)
{
	FormPtr  frmOrig;
   UInt16	frmID;

	frmID = FrmGetActiveFormID ();

	if (updTopForm || Pre35Rom)
		FrmUpdateForm (frmID, updateViewReInit);

  	//if (Pre35Rom) {
  	
		//if (!updTopForm) // force the update on Palm III's
		//	FrmUpdateForm (frmID, updateViewReInit);
           		 	
		if (frmID != IndiListForm && frmID != IndiSummForm) {
  			frmOrig = FrmGetActiveForm ();
     		FrmSetActiveForm (FrmGetFormPtr (IndiSummForm));
   		IndiSummDrawData (false);
     		FrmSetActiveForm (frmOrig);
     		}
    // 	}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	IndiSummDrawData
//
// DESCRIPTION: 	This routine draws data on the Individual Summary Screen.
//
// PARAMETERS:  	-> updateTbl - true to update the Event/Child table.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiSummDrawData (Boolean updateTbl)
{
	ErrFatalDisplayIf (FamiEventsVisible && EventsVisible,
		"IndiSummDrawData:FamiEventsVisible && EventsVisible cannot occur"); 			

	// Redraw form here (do not call DrawForm to avoid infinite loop)
	FrmDrawForm (FrmGetActiveForm ());

   if (updateTbl) {

		if (EventsVisible || FamiEventsVisible)
			EvenListInit ();
		else
			ChilListInit ();
		
		// Redraw table to update rows and the scroll buttons.  Do not
		// call FrmUdateForm after the Init functions above.
		TblRedrawTable	((TablePtr) GetObjectPtr (IndiSummEvenListTable));
     	}

	IndiSummNameDrawRecord ();
   IndiSummEventsDrawRecord (IndiSummBirthButton);
   IndiSummEventsDrawRecord (IndiSummDeathButton);
   FamiNumberDraw (true);
   FamiSpouDrawRecord ();
   FamiMarrDrawRecord ();
	IndiSummUpdFamiButtons ();
   DrawScreenLines (IndiSummForm);
   
   ShowObject (IndiSummJumpButton, !Prefs[HideJump]);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiSummResizeForm
//
// DESCRIPTION:   This routine resizes and draws the Individual Summary Form.
//
// PARAMETERS:   	None. 
//
// RETURNED:      True if form was resized, else false (no changed in size).
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean IndiSummResizeForm (void)
{
	Int16 			extentDelta;
	UInt16 			numObjects, i;
	Coord 			x, y;
	RectangleType 	objBounds;  // bounds of object being set
	RectangleType 	curBounds;  // bounds of active form
	RectangleType	displayBounds; // bounds of screen
	FormPtr			frmP;
	
	if (!DynInDevice)
		return false;
		
	frmP = FrmGetActiveForm();

	// Get dimensions of current active form.
	WinGetBounds (FrmGetWindowHandle (frmP), &curBounds);
	
	// Get the new display window bounds
	WinGetBounds (WinGetDisplayWindow (), &displayBounds);

	extentDelta = 	(displayBounds.extent.y + displayBounds.topLeft.y) -
						(curBounds.extent.y + curBounds.topLeft.y);
	
	if (extentDelta == 0) // form has not changed in size, so do nothing
		return false;
		
	WinSetBounds (FrmGetWindowHandle (frmP), &displayBounds); // set new form bounds
	
	// Iterate through objects and re-position them.
	numObjects = FrmGetNumberOfObjects (frmP);

	for (i = 0; i < numObjects; i++) {
	
		switch (FrmGetObjectId (frmP, i))
			{
			case IndiSummListButton:
			case IndiSummFamiButton:
			case IndiSummEvenButton:
			case IndiSummScrollUpRepeating:
			case IndiSummScrollDownRepeating:
			case IndiSummSearchBitMap:
			case IndiSummSearchButton:
			case IndiSummJumpButton:
				FrmGetObjectPosition (frmP, i, &x, &y);
				FrmSetObjectPosition (frmP, i, x, y + extentDelta);
				break;

			case IndiSummEvenListTable:
				FrmGetObjectBounds (frmP, i, &objBounds);
				objBounds.extent.y += (extentDelta);
				FrmSetObjectBounds (frmP, i, &objBounds);
				break;
			
			default:
				break;
			} // switch
		
		}  // for

	return true;
}
 
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiSummHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Individual
//                Information Summary Screen.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean IndiSummHandleEvent (EventPtr event)
{
   Boolean     	 	handled = false;
	WinDirectionType	direction;
	TablePtr				evenLstTableP;
	FormPtr				frm;
	DirType				navDir;
	UInt32				hldNo; // record in IndiDB or EvenDB
   
   evenLstTableP = (TablePtr) GetObjectPtr (IndiSummEvenListTable);
   frm 			  = FrmGetActiveForm ();
   
   switch (event->eType)
		{
		 case tblSelectEvent:
		 	{
		 	
        	switch (event->data.tblSelect.column)
         	{
         	case 0: // Select Event or Child from table

				 	hldNo = TblGetRowData (event->data.tblSelect.pTable,
						 event->data.tblSelect.row);

            	if (EventsVisible || FamiEventsVisible) { // open Event Detail form
		         	CurrentEvenRecN = hldNo;
        			  	if (CurrentEvenRecN != NO_REC_LONG)
	             		FrmPopupForm (EvenDetailForm);
           			}
           		else { // open Indi Summary Form for selected Child
	              	CurrentIndiRecN = (UInt16) hldNo;
  	            	FrmGotoForm (IndiSummForm);
      	      	}
         	   break;
         	   
         	case 1: // Select Child-to-Family Link Window from table
               if (TblGetItemInt (event->data.tblSelect.pTable,
               	event->data.tblSelect.row, 1) == 1) {
                	ChilNumInFam = TblGetItemInt (event->data.tblSelect.pTable,
                  	event->data.tblSelect.row, 0);
                  FrmPopupForm (ChilDetailForm);
                  }
             	break;
         	   
         	default:
         	   break;
         	}  // end switch 
         	  
			TblUnhighlightSelection (event->data.tblSelect.pTable); // backward comp.
         handled = true;
         break;
			}
							
      case ctlSelectEvent:
         
         switch (event->data.ctlSelect.controlID)
	        	{
            case IndiSummListButton:
               ListViewSelectRecN = CurrentIndiRecN; // highlight upon return to IndiList
               FrmGotoForm (IndiListForm);
               handled = true;
               break;
               
            case IndiSummFamiButton:
	            if (EventsVisible)
		         	IndiSummFamiOrEvenButton (evenLstTableP);
               handled = true;
               break;               

            case IndiSummEvenButton:
               if (!EventsVisible)
	               IndiSummFamiOrEvenButton (evenLstTableP);
					handled = true;
               break;               
               
            case IndiSummChilListButton:
               if (FamiEventsVisible)
	               IndiSummChilOrEvenButton (evenLstTableP);
               handled = true;
               break;               

            case IndiSummFamiListButton:
               if (!FamiEventsVisible)
	               IndiSummChilOrEvenButton (evenLstTableP);
               handled = true;
               break;                   

            case IndiSummParentsButton:
               AlreadyHaveFamily = ParentFinder (true);
               FrmGotoForm (IndiSummForm);
               handled = true;
               break;

            case IndiSummJumpButton:
     				PriorFormID = IndiSummForm;
     				FrmCloseAllForms ();
     				FrmGotoForm (JumpForm);
               handled = true;
               break;
                              
     			case IndiSummScrollLeftButton:
     				IndiSummScrollFami (NavUp);
     				handled = true;
               break;     				
     			
     			case IndiSummScrollRightButton:
     				IndiSummScrollFami (NavDn);
     				handled = true;
               break;

     			case IndiSummSearchButton:
					PriorFormID = IndiSummForm; // save information on form to return to
   				ListViewSelectRecN = CurrentIndiRecN;
     				
     				if (SrchArrayData == SrchPlacEven) {
	   				FrmCloseAllForms ();
			         FrmGotoForm (SearchForm);
			         }
			      else if (SrchArrayData == SrchDateEven) {
	   				FrmCloseAllForms ();
   					FrmGotoForm (FldSearchForm);
   					}
               
               handled = true;
               break;                   
   
            default:
               break;
         	}

         break;     
     
		case ctlEnterEvent:
		
			switch (event->data.ctlSelect.controlID)
	      	{
        	 	case IndiSummNameButton:
	       		if (IndiSummCtlSelected (IndiSummNameButton)) {
	         		FrmPopupForm (IndiDetailForm);
	         		handled = true;
	         		}
	         	break;
	
        	 	case IndiSummBirthButton:
	        	 	if (IndiSummCtlSelected (IndiSummBirthButton)) {
           	 		CurrentEvenRecN = PrimaryBirtRecN;
           			if (CurrentEvenRecN != NO_REC_LONG)
              			FrmPopupForm (EvenDetailForm);
              		handled = true;
              		}
              break;
               
            case IndiSummDeathButton:
	            if (IndiSummCtlSelected (IndiSummDeathButton)) {
        	 			CurrentEvenRecN = PrimaryDeatRecN;
           			if (CurrentEvenRecN != NO_REC_LONG)
                		FrmPopupForm (EvenDetailForm);
               	handled = true;
               	}
               break;

				case IndiSummSpouListButton:
              	if (IndiSummCtlSelected (IndiSummSpouListButton)) {
              		if (CurrentSpouRecN != NO_REC) {
               	  	CurrentIndiRecN = CurrentSpouRecN;
              			FrmGotoForm (IndiSummForm);
           				}
                	handled = true;
                	}
                break;

     			case IndiSummMarrListButton:
               if (IndiSummCtlSelected (IndiSummMarrListButton)) {
	               CurrentEvenRecN = CurrentMarrRecN;
   	            if (CurrentEvenRecN != NO_REC_LONG)
      	         	FrmPopupForm (EvenDetailForm);
         	      handled = true;
         	      }
               break;

				case IndiSummFamiNoButton:
					if (IndiSummCtlSelected (IndiSummFamiNoButton)) {
						FrmPopupForm (FamiDetailForm); 
               	handled = true;
               	}
               break;      

        		default:
	        		// don't set handled to true
        			break;
				} 
  			break;
     
      case ctlRepeatEvent:
 	      
         switch (event->data.ctlRepeat.controlID)
         	{
         	case IndiSummScrollUpRepeating:
            case IndiSummScrollDownRepeating:
         		if (event->data.ctlRepeat.controlID == IndiSummScrollUpRepeating)
         			direction = NavUp;
         		else 
         			direction = NavDn;
         		
               ErrFatalDisplayIf (FamiEventsVisible && EventsVisible,
						"IndiSummHandleEvent:FamiEventsVisible && EventsVisible cannot occur");
               if (EventsVisible || FamiEventsVisible)
                  EvenListScroll (direction);
               else
                  ChilListScroll (direction);
               // leave unhandled so the buttons can repeat
               break;

            default:
               break;
         	}
         break;

      case keyDownEvent:
      
 			if (NavKeyHit (event, &navDir)) {
				
				switch (navDir) {
					
					case NavUp:
					case NavDn:
            		ErrFatalDisplayIf (FamiEventsVisible && EventsVisible,
							"IndiSummHandleEvent:FamiEventsVisible && EventsVisible cannot occur");
            		if (EventsVisible || FamiEventsVisible)
               		EvenListScroll (navDir);
            		else
                  	ChilListScroll (navDir);
            		handled = true;
						break;
					
					case NavL:
					case NavR:
						IndiSummScrollFami (navDir);
						handled = true;
						break;

					case NavSel:
						ListViewSelectRecN = CurrentIndiRecN;
               	FrmGotoForm (IndiListForm);
               	handled = true;
               	break;

					default:
						break;
					}
         	}
			else if (event->data.keyDown.chr == chrSmall_E) { // event button
				CtlHitControl (GetObjectPtr (IndiSummEvenButton));
           	handled = true;
			  	}
			else if (event->data.keyDown.chr == chrSmall_F) { // family button
				CtlHitControl (GetObjectPtr (IndiSummFamiButton));
           	handled = true;
			  	}			
			else if (event->data.keyDown.chr == chrSmall_L) { // individual list
				CtlHitControl (GetObjectPtr (IndiSummListButton));
           	handled = true;
			  	}			
			else if (event->data.keyDown.chr == chrSmall_P) { // parents icon
				CtlHitControl (GetObjectPtr (IndiSummParentsButton));
           	handled = true;
			  	}			
			else if (event->data.keyDown.chr == chrSmall_M) { // menu
				EvtEnqueueKey (vchrMenu, 0, commandKeyMask);
           	handled = true;
			  	}

         break;

   case menuEvent:
      return MenuDoCommand (event->data.menu.itemID);
      
   case frmGotoEvent:
	case frmOpenEvent:	
		
		// Init CurrentIndiRecN if entering from 'Palm Search' function.
		if (event->eType == frmGotoEvent)
			CurrentIndiRecN = event->data.frmGoto.recordNum;
     
      // Resize the form if necessary.
     	IndiSummResizeForm ();
      
      // Display events or children in list.
      EventsVisible = Prefs[EventsFirst];
      FamiEventsVisible = false;

		WinSetDrawWindow (FrmGetWindowHandle (frm)); // do this instead of DrawForm

	   IndiSummLoadData (); // (#1) get spouse and marriage data (load FamiRec)
   	
   	ChilLoadData (); // (#2) find children, if any, and set child variables

		// If returning from 'Event Search' list then override EventsVisible.
		// Do this before calling IndiSummEvenInit.
		if (SrchShowEven)	//	(#3) check if returning from search
			EventsVisible = true;
		else if (SrchShowFamiEven)
			EventsVisible = false;

		EvenLoadData (); // (#4) load general or family event list variables

		// If returning from 'Event Search' scroll to correct position in event list.
		if (SrchShowFamiEven || SrchShowEven) { // set in FldSrchHandleEvent
			
			// Set the TopVisEvenRecN (CurrentEvenRecN is set in FldSrchHandleEvent).
			// TopVisEvenRecN will be corrected, if needed, in EventListLoadTable.
			if (CurrentEvenRecN >= FirstEvenRecN && CurrentEvenRecN <= LastEvenRecN)
				TopVisEvenRecN = CurrentEvenRecN; // (#5) set TopVisEvenRecN
			
			if (SrchShowFamiEven && FirstEvenRecN != NO_REC_LONG)
				FamiEventsVisible = true;
			}

		// Draw the entire form.
      IndiSummDrawData (true); // (#6) draw form

     	// Show or hide the 'Parents' button.
      if (IndiRec.fields[indiFamCNo] != NULL) {
			ShowObject (IndiSummParentsButton, true);
      	ShowObject (IndiSummParentsBitMap, true);
      	}

		// Show or hide the 'Search List' button.
		if (SrchArrayData != SrchNoData) {
			ShowObject (IndiSummSearchButton, true);
     		ShowObject (IndiSummSearchBitMap, true);
     		}

		DetailViewInitPriorLines ();

    	SetControlValue (IndiSummEvenButton, EventsVisible);
   	SetControlValue (IndiSummFamiButton, !EventsVisible);
 
 		// Highlight the found record on the form.
		if (SrchShowFamiEven || SrchShowEven) { // (set in FldSrchHandleEvent)
			IndiSummHighlightSearchCtrl (evenLstTableP);
   		SrchShowEven = SrchShowFamiEven = false; // re-init
   		}
     
      handled = true;
      
      break;

	case frmUpdateEvent:
		
     	if (UpdateFrm) {
	     	IndiSummResizeForm (); // resize if necessary (for Cobalt)
			IndiSummDrawData (true);
			}
      UpdateFrm = true;
   	handled = true;
      break;
	
	case winDisplayChangedEvent:
		
		if (IndiSummResizeForm ())
			IndiSummDrawData (true);
		handled = true;
		break;
		
  	case frmCloseEvent:
      
      DbMemHandleUnlock (&IndiRecH);
      
      if (SouCListH) {
			MemHandleFree (SouCListH);
   		SouCListH = NULL;
   		}
      
      if (!AlreadyHaveFamily)
         DbMemHandleUnlock (&FamiRecH);

     	UpdateFrm = true; // must re-init here
      break;


	default:
		break;
  	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   	EventDesc
//
// DESCRIPTION:	Returns the long event description given the event tag.
//
// PARAMETERS: 	-> recordP - 	subject event record
//
// RETURNED:   	Pointer to returned description.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Char *EventDesc (DBRecordType *recordP)
{
	UInt16 evenTypeN = (UInt16) StrAToI (recordP->fields[evenType]);

	if (evenTypeN >= TOT_EVEN_L) // make sure Event type is in range
		evenTypeN = 0; // event "Unknown"
		
   // If rec has desc field and rec type is EVEN, then return the desc field
   if (recordP->fields[evenDesc] && evenTypeN == EVEN_POS)
      return recordP->fields[evenDesc];
   else  // convert type field to a long event description
      return EvenDesc[evenTypeN][0];
}


////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	SetNavFocusRing
//
// DESCRIPTION: 	This routine sets the focus ring in the current form to the
//						specified object ID. This routine is backward compatible and
//						calls FrmSetFocus for devices that don't have the Five Way
//						Navigator.
//
//						Note: the FrmNav functions only work on devices where the
//						sysFtrNumFiveWayNavVersion feature is present. The FivWayDevice
//						variable will only be true if this feature is present.
//						FivWayDevice will be false on some Handspring devices (eg the 600)
//					 	which return false when checking for the sysFtrNumFiveWayNavVersion 
//						feature using FtrGet.
//
//						Call with noFocus as the parameter to remove the focus ring.
//					
//
// PARAMETERS:  	-> objID - id of the form object to set focus to.
//
// RETURNED:    	Nothing
//
// REVISIONS:		Created on 12-20-2005.
////////////////////////////////////////////////////////////////////////////////////
void SetNavFocusRing (const UInt16 objID)
{
	FormPtr	frmP = FrmGetActiveForm ();
	
	if (!frmP) return;
		
	if (FivWayDevice) {
	
		if (objID == noFocus)
			FrmNavRemoveFocusRing (frmP);
		else	
			FrmNavObjectTakeFocus (frmP, objID);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	SetFocus
//
// DESCRIPTION: 	This routine returns a pointer to an object in the current form.
//
// PARAMETERS:  	-> controlID - id of the form object to set focus to.
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void SetFocus (const UInt16 controlID)
{
	FormPtr	frm = FrmGetActiveForm ();
	
	FrmSetFocus (frm, (controlID == noFocus) ? noFocus : 
		FrmGetObjectIndex (frm, controlID));
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ShowObject
//
// DESCRIPTION: 	This routine shows or hides an object in the current
//              	form.
//
// PARAMETERS:  	-> objectID - 	id of the form object to hide or show.
//						-> showObj 	-	true to show object, else false to hide
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ShowObject (const UInt16 objectID, const Boolean showObj)
{
   FormPtr 	frm;
   UInt16	objIndx; 
   
   frm 		= FrmGetActiveForm ();
   objIndx 	= FrmGetObjectIndex (frm, objectID);
   
	if (showObj)
      FrmShowObject (frm, objIndx);
   else
   	FrmHideObject (frm, objIndx);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	GetObjectPtr
//
// DESCRIPTION: 	This routine returns a pointer to an object in the current form.
//
// PARAMETERS:  	-> objectID - id of the form object to get pointer to
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void* GetObjectPtr (const UInt16 objectID)
{
   FormPtr frm = FrmGetActiveForm ();
   return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	GetObjectBounds
//
// DESCRIPTION: 	This routine returns a pointer to an object in the current form.
//
// PARAMETERS:  	-> objectID - id of the form object to get pointer to
//						-> rect		- rectangle
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void GetObjectBounds (const UInt16 objectID, RectanglePtr rect)
{
	FormPtr frm = FrmGetActiveForm ();
	FrmGetObjectBounds (frm, FrmGetObjectIndex (frm, objectID), rect);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ResetScrollRate
//
// DESCRIPTION:   This routine resets the scroll rate.
//
// PARAMETERS:    nothing
//
// RETURNED:      nothing
//
// REVISIONS:
////////////////////////////////////////////////////////////////////////////////////
void ResetScrollRate (void)
{
	// Reset last seconds
	LastSeconds = TimGetSeconds();
	// Reset scroll units
	ScrollUnits = 1;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AdjustScrollRate
//
// DESCRIPTION:   This routine adjusts the scroll rate based on current
//                scroll rate, given a certain delay.
//
// PARAMETERS:    nothing
//
// RETURNED:      nothing
//
// REVISIONS:
////////////////////////////////////////////////////////////////////////////////////
void AdjustScrollRate (void)
{
	// Accelerate scroll rate every 3 secs if not already at max sroll speed
	UInt16 newSeconds = (UInt16) TimGetSeconds();
	
	if ((ScrollUnits < scrollSpeedLimit) &&
	    ((newSeconds - LastSeconds) > scrollDelay))
		{
		// Save new seconds
		LastSeconds = newSeconds;
		
		// increase scroll units
		ScrollUnits += scrollAcceleration;
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DrawForm
//
// DESCRIPTION:   This routine draws a form factoring in the OS version.  For OS 3.3
//						and earlier.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DrawForm (void)
{
	FormPtr	origFrm = FrmGetActiveForm ();

  	if (Pre35Rom) { // backward compatibility
  		
  		RectangleType	rect;

		if (RedrawBaseFrm) { // used in NoteView routines
			IndiSummUpdForms (false);
			RedrawBaseFrm = false;
			} 

		// Erase form before draw
		FrmGetFormBounds (origFrm, &rect);
		rect.topLeft.y = 0;
		WinEraseRectangle (&rect, 0);
  	 	}
  	
	SetNavFocusRing (noFocus); // prevents problem with screen artifacts DTR 12-20-2005
		 	
	FrmDrawForm (origFrm);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	UpdateScrollers
//
// DESCRIPTION: 	This routine returns Updates the up/down scroll buttons on the
//						active form.
//
// PARAMETERS:  	-> leftButton 	- id left arrow button.
//						-> leftButton 	- id  right arrow button.
//						-> scrollableU	- true if leftButton is on and selectable.
//						-> scrollableD	- true if rightButton is on and selectable.
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void UpdateScrollers (UInt16 leftButton, UInt16 rightButton, Boolean scrollableL,
	Boolean scrollableR)
{
	FormPtr	frm = FrmGetActiveForm ();
	UInt16	upIndex, downIndex;

	#ifdef SCROLL_TEST
   scrollableL = scrollableR = true;
   #endif

	upIndex = FrmGetObjectIndex (frm, leftButton);
	downIndex = FrmGetObjectIndex (frm, rightButton);
   FrmUpdateScrollers (frm, upIndex, downIndex, scrollableL, scrollableR);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      UpdateLeftRightScrollers
//
// DESCRIPTION:   This routine updates the left and right scroll arrows.
//
// PARAMETERS:    leftButton 		 -	id of left scroller
//						rightButton		 -	id of right scroller
//						scrollableLeft  - 	true if scrollable  up, else false
//						scrollableRight -	true if scrollable down, else false
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void UpdateLeftRightScrollers (UInt16 leftButton, UInt16 rightButton,
	Boolean scrollableL, Boolean scrollableR)
{
	static Char		scrollLChar[2] = "0";
	static Char		scrollRChar[2] = "0";
	ControlType*	leftPtr;
	ControlType*	rightPtr;
	UInt16			leftIndex, rightIndex;
   FormPtr 			frm;

	frm = FrmGetActiveForm ();

	leftIndex 	= FrmGetObjectIndex (frm, leftButton);
	rightIndex 	= FrmGetObjectIndex (frm, rightButton);
   leftPtr 		= FrmGetObjectPtr (frm, leftIndex);
   rightPtr 	= FrmGetObjectPtr (frm, rightIndex);
    
   #ifdef SCROLL_TEST // only used in DEBUG
   scrollableL = scrollableR = true;
   #endif
   
   if (!scrollableL && !scrollableR) {
   	FrmHideObject (frm, leftIndex);
      FrmHideObject (frm, rightIndex);
      return;
      }
 	 
  	scrollLChar[0] = scrollableL ? symbol11LeftArrow : symbol11LeftArrowDisabled;
	CtlSetLabel (leftPtr, scrollLChar);
	CtlSetEnabled (leftPtr, scrollableL);
  	FrmShowObject (frm, leftIndex);

   scrollRChar[0] = scrollableR ? symbol11RightArrow : symbol11RightArrowDisabled;
   CtlSetLabel (rightPtr, scrollRChar);
   CtlSetEnabled (rightPtr, scrollableR);
   FrmShowObject (frm, rightIndex);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      NavKeyHit
//
// DESCRIPTION:   Checks if a 5-way navigation button was pressed by the user.  If
//						so, it returns the direction in navDir.  If the device is not
//						5-way capable, this routine only checks for up and down button
//						presses.
//
// PARAMETERS:   	-> eventP	- 	pointer to an event.
//						<- navDir	- 	direction of button.  If not valid direction this
//											will have the value NavNone.
//
// RETURNED:     	true if a 5-way navigation button was hit or if a 2-way navigation
//						button was hit.
//
// REVISIONS:		05/18/2004 - Added support for Nav buttons on Treo and T5 devices.
////////////////////////////////////////////////////////////////////////////////////
Boolean NavKeyHit (EventType *eventP, DirType *navDir)
{
	*navDir = NavNone; // init

	// The 5-Way Navigator buttons (except up and down) generate a vchrNavChange
	// when pressed, and the keyCode field tells you both the button states
	// (which ones are pressed now) and the recent transition (which were just
	// pressed or released).

	if (EvtKeydownIsVirtual (eventP) ) { 
	
		switch (eventP->data.keyDown.chr) {
		
			case vchrPageUp:
			case vchrRockerUp:
			case vchrThumbWheelUp:
				*navDir = NavUp;
				return true;
		
			case vchrPageDown:	
			case vchrRockerDown:
			case vchrThumbWheelDown:
				*navDir = NavDn;
				return true;
		
			case vchrRockerLeft:
				*navDir = NavL;
				return true;
		
			case vchrRockerRight:
				*navDir = NavR;
				return true;
		
			case vchrRockerCenter:
			case vchrThumbWheelPush:
				*navDir = NavSel;
				return true;		
			
			case vchrNavChange:

				if (eventP->data.keyDown.keyCode & navBitRight)
					*navDir = NavR;
				else if(eventP->data.keyDown.keyCode & navBitLeft)
					*navDir = NavL;
				else if(eventP->data.keyDown.keyCode & navBitSelect)
					*navDir = NavSel;
					
				return true;
				
			default:
				return true;
			}
		
		}
		
	return false;
} 
/*


//#define IsFiveWayNavEvent(eventP)																		\
//	(																												\
//		 ((eventP)->data.keyDown.chr == vchrPageUp ||												\
//		  (eventP)->data.keyDown.chr == vchrPageDown ||												\
//		  (eventP)->data.keyDown.chr == vchrNavChange)												\
//	&&																												\
//		 (((eventP)->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) != 0)			\
//	)

Boolean NavKeyHit (EventType *eventP, DirType *navDir)
{
	*navDir = NavNone; // init
	
	if (IsFiveWayNavEvent(eventP)) { // device is 5-way capable device
		
		if (eventP->data.keyDown.modifiers & autoRepeatKeyMask) {
			
			switch (eventP->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) {
				case navBitUp:
					*navDir = NavUp;
					return true;
				case navBitDown:
					*navDir = NavDn;
					return true;
				case navBitLeft:
					*navDir = NavL;
					return true;
				case navBitRight:
					*navDir = NavR;
					return true;
				default:
					return true;
				}				
			}
			
		else {
		
			switch (eventP->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) {
				case (navBitUp | navChangeUp):
					*navDir = NavUp;
					return true;
				case (navBitDown | navChangeDown):
					*navDir = NavDn;
					return true;
				case (navBitLeft | navChangeLeft):
					*navDir = NavL;
					return true;
				case (navBitRight | navChangeRight):
					*navDir = NavR;
					return true;
				case navChangeSelect:
					*navDir = NavSel;
					return true;
				//case navChangeUp:
				//case navChangeDown:
				//case navChangeLeft:
				//case navChangeRight:
				//	return true;
				default:
					return true;
				}				
			}
		} // not 5-way button hit
	else if (EvtKeydownIsVirtual (eventP)) { // up/down for non 5-way capable device
		if (eventP->data.keyDown.chr == vchrPageUp) {
			*navDir = NavUp;
			return true;
			}
		else if (eventP->data.keyDown.chr == vchrPageDown) {
			*navDir = NavDn;
			return true;
			}
		}
		
	return false;
}
*/

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ResizeForm
//
// DESCRIPTION:   This routine repositions a form on the screen in response to a 
//						screen resize event.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
/*Boolean ResizeForm (void)
{
	Int16 			extentDelta;
	RectangleType 	curBounds; 		// bounds of active form
	RectangleType	displayBounds; // bounds of screen
	FormPtr			frmP;

	if (!DynInDevice)
		return false;
	
	frmP = FrmGetActiveForm();

	// Get dimensions of current active form.
	WinGetBounds (FrmGetWindowHandle (frmP), &curBounds);
	
	// Get the new display window bounds
	WinGetBounds (WinGetDisplayWindow (), &displayBounds);

	extentDelta = 	(displayBounds.extent.y + displayBounds.topLeft.y) -
						(curBounds.extent.y + curBounds.topLeft.y + 2); // add 2 for bounds

	if (extentDelta == 0) // form has not changed in size, so do nothing
		return false;
	
	curBounds.topLeft.y += extentDelta;
	
	WinSetBounds (FrmGetWindowHandle (frmP), &curBounds); // set new form bounds

	return true;
}
*/
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      MenuDoCommand
//
// DESCRIPTION:   This routine performs the menu command specified.
//
// PARAMETERS:    -> command  - menu item id
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean MenuDoCommand (UInt16 command)
{   
//#ifdef GREMLINS
//if ((command != ListViewToolsSearch) && (command != ToolsSearch) &&
//	(command != ListViewToolsOnThisDay) && (command != ToolsOnThisDay) && 
//	(command != ListViewOptionsDatabase))
//	return true;
//#endif

   switch (command)
   	{
		case IndiListToolsDateCalc:
		case ToolsDateCalc:
		case IndiListToolsRelationshipCalc:
   	case ToolsRelationshipCalc:
		case IndiListToolsEventSearch:
		case ToolsEventSearch:			
		case IndiListToolsOnThisDay:
		case ToolsOnThisDay:
		case IndiListOptionsDatabase:
      case IndiSummOptionsDatabase:
      case ToolsMemoExport:
      case ToolsJumpList:
		case IndiListToolsJumpList:
			// Save information about form to return to
			PriorFormID = FrmGetActiveFormID ();
   		if (PriorFormID != IndiListForm)
   			PriorFormID = IndiSummForm;
   		ListViewSelectRecN = CurrentIndiRecN;
   		FrmCloseAllForms ();
			break;
			
		default:
			break;
		}
		
   switch (command)
   	{
      case IndiListOptionsKeyboard:
         SysKeyboardDialog (kbdDefault);
         return true;

	   case IndiListOptionsPreferences:
	   case IndiSummOptionsPreferences:
         FrmPopupForm (Pref1Form);
		 	return true;

		case IndiListOptionsFontsColors:
	   case IndiSummOptionsFontsColors:
         FrmPopupForm (Pref2Form);
		 	return true;

		case IndiListToolsSoundexCode:
		case ToolsSoundexCode:
         FrmPopupForm (SoundexForm);
         return true;

		case IndiListToolsDateCalc:
		case ToolsDateCalc:
        FrmGotoForm (DateCalcForm);
        return true;

		case IndiListToolsRelationshipCalc:
   	case ToolsRelationshipCalc:
         FrmGotoForm (RelCalcForm);
         return true;
         
		case IndiListToolsEventSearch:
		case ToolsEventSearch:
         FrmGotoForm (SearchForm); 
         return true;

		case IndiListToolsOnThisDay:
		case ToolsOnThisDay:
   		FrmGotoForm (FldSearchForm);
         return true;

		case IndiListToolsFindPerson:
		case ToolsFindPerson:
   		FrmPopupForm (FindPersonForm);
         return true;

      case ToolsAncestryChart:
   		FrmCloseAllForms ();
         FrmGotoForm (AncestryForm);
         return true;

      case ToolsDescendancyChart:
   		FrmCloseAllForms ();
         FrmGotoForm (DescendancyForm);
         return true;

		case ToolsJumpList:
		case IndiListToolsJumpList:
			FrmGotoForm (JumpForm);
			return true;

		case ToolsMemoExport:
			FrmGotoForm (ExportMemoForm);
			return true;

      case HelpAboutGedWise:
      	AboutMode = true;
     	 	FrmPopupForm (SplashForm);
         return true;
      	
      	/*
      	{
         FormPtr	about;
         about = FrmInitForm (SplashForm);
         FrmSetActiveForm (FrmGetFormPtr (SplashForm));  
         ShowObject (SplashVersionLabel, false);
         ShowObject (SplashVerFullLabel, true);
         ShowObject (SplashEmailLabel, true);
         ShowObject (SplashDeveloperLabel, true);
         ShowObject (SplashOKButton, true);
         if (Prefs[Registered])
				ShowObject (SplashRegisteredLabel, true);
			else
         	ShowObject (SplashUnregisteredLabel, true);
         
         FrmDrawForm (about);
         FrmDoDialog (about);
         FrmReturnToForm (0);
         IndiSummUpdForms (false);
  	      return true;
      	}
      */
  		case IndiListOptionsDatabase:
      case IndiSummOptionsDatabase:
   		FrmGotoForm (DatabasesForm);
         return true;
      
      case HelpGedWiseHelp:
       	{
       	UInt16 	strId;
       	
       	switch (FrmGetActiveFormID ())
         	{
         	case IndiListForm:
         		strId = ListHelpString;
         		break;
            case DatabasesForm: 
         		strId = DatabasesHelpString;
         		break;
         	default:
         		strId = SummHelpString;
         		break;
         	}
       	
       	FrmHelp (strId);  		
         return true;
       	}
       	
      case HelpRegistrationCode:
      	if (!Prefs[Registered])
      		FrmPopupForm (RegisterForm);
      	else
      		FrmAlert (RegisteredAlert);
      	return true;

     	default:
        break;  
   	}
 
   return false;
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    IndiViewInit
//
// DESCRIPTION: This routine initializes the "Record View" of the 
//              Address application.  Most importantly it lays out the
//              record and decides how the record is drawn.
//
// PARAMETERS:  frm - pointer to the view form.
//
// RETURNED:    true if the event has handle and should not be passed
//              to a higher level handler.
//
// REVISIONS:	 None.
////////////////////////////////////////////////////////////////////////////////////
static void IndiViewInit (void)
{
   UInt16 			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   FontID 			curFont;
	
	DetailViewInit (); // initialize gadget array
		
   // Check Indivdual Record to see if there is a source.
	if (IndiRec.fields[indiSouCNo] != NULL)
		ShowObject (IndiDetailCiteButton, true);
	
   // Check IndiDB to see if there if individual has an Alias.
   LastAliaNum = TopVisAliaNum = 0;

   // Get the number of aliases the individual has.
	if (IndiRec.fields[indiAlias])
	   LastAliaNum = RefCounter (ALIA_DLM, IndiRec.fields[indiAlias]);
	
	if (LastAliaNum > 0)
		TopVisAliaNum = 1;

   ShowObject (IndiDetailAliasButton, (Boolean) (LastAliaNum != 0));
      
   // Get width and height of current drawn window.
   FrmGetFormBounds (FrmGetActiveForm(), &rect);
   maxWidth = rect.extent.x - MARGIN_WID * 2; // subtract to allow room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);
   
   if (IndiRec.fields[indiFName])
      DetailViewAddField(IndiRec.fields[indiFName], indiFName, &width,
         maxWidth, MARGIN_WID);
         
   // Separate the last name from the first name as long as they are together
   // on the same line.
   if (width > MARGIN_WID)
    	DetailViewAddSpaceForText (" ", &width);
   if (!IndiRec.fields[indiLName])
      IndiRec.fields[indiLName] = cUnknownStr;
   DetailViewAddField(IndiRec.fields[indiLName], indiLName, &width,
      maxWidth, MARGIN_WID);

   // Separate the title from rest of name as long as they are together
   // on the same line. DTR: Added 2 Jun 2003
   if (IndiRec.fields[indiTitle]) {
      if (width > MARGIN_WID)
    		DetailViewAddSpaceForText (" ", &width);
   	DetailViewAddField (IndiRec.fields[indiTitle], indiTitle, &width,
      	maxWidth, MARGIN_WID);
		}

   // If the line above isn't blank then add a blank line
   DetailViewNewLineIf (&width);
       
   DetailViewFirstPlainLine = DetailViewLastLine;   
   
   if (!IndiRec.fields[indiSex])
      IndiRec.fields[indiSex] = "?";
   DetailViewPositionTextAt (&width);
   DetailViewAddField (IndiRec.fields[indiSex], indiSex, &width,
      maxWidth, HEAD_WID);
 
   DetailViewNewLineIf (&width); 
   DetailViewPositionTextAt (&width);;
   DetailViewAddField (IndiRec.fields[indiNo], indiNo, &width,
      maxWidth, HEAD_WID);

	if (IndiRec.fields[indiRefn]) {
   	DetailViewNewLineIf (&width);
   	DetailViewPositionTextAt (&width);
   	DetailViewAddField (IndiRec.fields[indiRefn], indiRefn, &width,
      	maxWidth, HEAD_WID);
		}
  
   // Check record to see if there is a note and set note title
   DetailViewAddNote (IndiRec.fields[indiNoteNo], IndiDetailNoteButton,
   	indiNoteNo,	&width, maxWidth, "Individual");

   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    IndiViewHandleEvent
//
// DESCRIPTION: This routine is the event handler for the Individual Detail
//              Window of the Individual Summary Screen.
//
// PARAMETERS:  -> event - a pointer to an EventType structure
//
// RETURNED:    true if the event has handle and should not be passed to a higher
//              level handler.
//
// REVISIONS:	 None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean IndiViewHandleEvent (EventPtr event)
{
   Boolean handled = false;

   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
      		case IndiDetailCloseButton:
            case IndiDetailDoneButton:
            	DetailViewQuickClose ();
               handled = true;
               break;
               
            case IndiDetailCiteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
					DetailViewGetSouCList (IndiRec.fields[indiSouCNo]);
					// already made sure SouCList cannot be NULL
               FrmGotoForm (SouCDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
         
            case IndiDetailNoteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               FrmGotoForm (NoteDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
               
            case IndiDetailAliasButton:
               FrmGotoForm (AliaListForm);
       			UpdateFrm = false;
               handled = true;
               break;
             
            case IndiDetailInfoButton:
            	FrmHelp (IndiHelpString);
               handled = true;
               break;
                    
            default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, IndiDetailDetailGadget);
         break;
         
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case IndiDetailScrollUpRepeating:
				case IndiDetailScrollDownRepeating:
					ResetScrollRate(); // reset scroll rate
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
  
       case ctlRepeatEvent:

			 switch (event->data.ctlRepeat.controlID)
         	{
            case IndiDetailScrollUpRepeating:
					DetailViewScroll (winUp, IndiDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case IndiDetailScrollDownRepeating:
					DetailViewScroll (winDown, IndiDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);

	   case frmOpenEvent:
	    	DetailViewResizeForm (true);    
         IndiViewInit ();
         DrawForm ();

         // Load TopDetailViewLine, but only after calling IndiViewInit. We
         // use TopDetailViewLine when we return from SouCViewForm.
         TopDetailViewLine = PriorTopDetailViewLine;
         DetailViewDraw (IndiRec, Indi, IndiDetailDetailGadget,
         	IndiDetailScrollUpRepeating, IndiDetailScrollDownRepeating);
         
         PriorSouCFormID = PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ());
         PriorTopDetailViewLine = 0; // reset to 0
         
         SetNavFocusRing (IndiDetailDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;

	   case frmUpdateEvent:
	   	DrawForm ();
		   if (event->data.frmUpdate.updateCode == updateViewReInit) {
		      MemHandleFree (DVLinesH);
            DVLines = NULL;
            IndiViewInit ();
            }
            
         DetailViewDraw (IndiRec, Indi, IndiDetailDetailGadget,
         	IndiDetailScrollUpRepeating, IndiDetailScrollDownRepeating);
   	   handled = true;
         break;
	
		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
      	
				DrawForm ();
			  	DetailViewDraw (IndiRec, Indi, IndiDetailDetailGadget,
         		IndiDetailScrollUpRepeating, IndiDetailScrollDownRepeating);
         		
         	SetNavFocusRing (IndiDetailDoneButton); // DTR 12-20-2005	
      		}
      	handled = true;
			break;
			
     case frmCloseEvent:  
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&NoteRecH);
         break;

	   default:
		   break;
   	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AliaListDrawRecord
//
// DESCRIPTION:   This routine draws an Alias List entry into the 
//                the AliaListTable.  It is called as a callback
//                routine by the table object.
//
// PARAMETERS:    table  - pointer to the address list table
//                row    - row number, in the table, of the item to draw
//                column - column number, in the table, of the item to draw
//                bounds - bounds of the draw region
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
 static void AliaListDrawRecord (void* table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   DBRecordType	rec;
   MemHandle     	recH = NULL;
   FontID 	     	curFont;

   if (DbGetRecord (IndiDB, TblGetRowData (table, row), &rec, &recH) != 0) {
	   ErrNonFatalDisplay ("AliaListDrawRecord: Record not found");
   	return;
   	}
 
   curFont = FntSetFont (PrefFnts[IndiSummFont]);
   DrawRecordNameAndLifespan (&rec, bounds, false, false);
   DbMemHandleUnlock (&recH);
	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AliaListScroll
//
// DESCRIPTION:   This routine scrolls the Alias list in the direction
//                specified.
//
// PARAMETERS:    direction	- up (NavUp) or down (NavDn)
//
// RETURNED:      Nothing
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void AliaListScroll (DirType direction)
{
	Int16	   	row;
	TablePtr	   table;
	UInt16 		newTopVisibleNum;

	// Before processing the scroll, close the command bar if it is open.
	MenuEraseStatus (0);

	table = (TablePtr) GetObjectPtr (AliaListAliaListTable);
	row 	= TblGetNumberOfRows (table) - 1;
   
	// There must be at least one row in the table.
	newTopVisibleNum = TopVisAliaNum;

	// scroll the table down.
	if (direction == NavDn) {
	   newTopVisibleNum += row; // scroll one page at a time.
	   if (newTopVisibleNum + row + 1 > LastAliaNum) {
	   	if (LastAliaNum - 1 > row)
	     		newTopVisibleNum = LastAliaNum - row;
	     	else
	     		newTopVisibleNum = 1;
	     	}
      }
   else { // scroll the table up
		if (newTopVisibleNum > 1 + row)
      	newTopVisibleNum -= row;
    	else
  			newTopVisibleNum = 1;
      }

	// Avoid redraw if no change
	if (TopVisAliaNum != newTopVisibleNum) {
		TopVisAliaNum = newTopVisibleNum;
		AliaListLoadTable ();
		TblRedrawTable (table);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AliaListLoadTable
//
// DESCRIPTION:   This routine loads alias database records into the Alias List
//                table.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
 ////////////////////////////////////////////////////////////////////////////////////
static void AliaListLoadTable (void)
{
   Int16			row = 0; // init
   Int16			numRows;
   UInt16		recNum; // record in IndiDB
   TablePtr		table;
   Char			aliaXRef[NREF_LEN+1];
   Boolean		scrollableU;
   Boolean		scrollableD;
  
   table 	= (TablePtr) GetObjectPtr (AliaListAliaListTable);
   numRows 	= TblGetNumberOfRows (table);
  
   TblSetColumnUsable (table, 0, true);
   TblSetCustomDrawProcedure (table, 0, AliaListDrawRecord);

   recNum = TopVisAliaNum;

   if (LastAliaNum > 0) { // LastAliasNum set in IndiViewInit

	   while (row < numRows && recNum <= LastAliaNum) {
	
		 	if (RefFinderStr (recNum, ALIA_DLM, IndiRec.fields[indiAlias],	aliaXRef)) {
            TblSetRowUsable (table, row, true);
 	 	      TblMarkRowInvalid (table, row);
 	 	      TblSetItemStyle (table, row, 0, customTableItem);
            TblSetRowData (table, row, StrAToI (aliaXRef));
	      	row++;
	      	}
 	 	         
         recNum++;
      	}  // end while loop
      	
   	} // end if LastAliaNum > 0
   
   // Hide the rows that don't have any data.
   while (row < numRows) {
      TblSetRowUsable (table, row, false);
      row++;
      }

    // Update the scroll buttons.
   if (LastAliaNum == 0) {
   	scrollableU = scrollableD = false;
   	}
   else {
	   scrollableU = (Boolean) (TopVisAliaNum > 1);
   	scrollableD = (Boolean) ((TopVisAliaNum + (numRows - 1)) < LastAliaNum);
   	}
   
   UpdateScrollers (AliaListScrollUpRepeating, AliaListScrollDownRepeating,
   	scrollableU, scrollableD);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	AliaListHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Alias List Screen.
//
//						NOTE: we don't need to call AdjustScrollRate or ResetScrollRate
//						functions, as ScrollUnits is not used in AliaListScroll function.
//
// PARAMETERS:  	-> event - a pointer to an EventType structure.
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:	 	None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean AliaListHandleEvent (EventPtr event)
{
   Boolean  handled = false;

   switch (event->eType)
		{
		case tblSelectEvent:
         CurrentAliaRecN = TblGetRowData (event->data.tblSelect.pTable,
		      event->data.tblSelect.row);
		   FrmGotoForm (AliaDetailForm);
		   UpdateFrm = false;
         handled = true;
         break;
		
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case AliaListDoneButton:
               FrmGotoForm (IndiDetailForm);
               UpdateFrm = false;
               handled = true;
               break;

				case AliaListCloseButton:
					DetailViewQuickClose ();
   				handled = true;
               break;

            case AliaListInfoButton:
               FrmHelp (AliaListHelpString);
               handled = true;
               break;
       
            default:
               break;
         	}
         break;

   	case ctlRepeatEvent:
         switch (event->data.ctlRepeat.controlID)
         	{
            case AliaListScrollUpRepeating:
               AliaListScroll (NavUp);
               // leave unhandled so the buttons can repeat
               break;

            case AliaListScrollDownRepeating:
                 AliaListScroll (NavDn);
               // leave unhandled so the buttons can repeat
               break;

            default:
               break;
             }  

      case keyDownEvent:
      	handled = DetailViewHandleVirtual (event, AliaListAliaListTable);
       	SetNavFocusRing (AliaListDoneButton); // DTR 12-20-2005
      	
/*      	{
      	DirType navDir;
	      if (NavKeyHit (event, &navDir)) {
				switch (navDir) {
					case NavUp:
					case NavDn:
						AliaListScroll (navDir);
            		handled = true;
						break;
					
					default:
						break;
					}
         	}
      	} */
        	break;
      
     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
      
	   case frmOpenEvent:
		   DetailViewResizeForm (true);
		   WinSetDrawWindow (FrmGetWindowHandle (FrmGetActiveForm ()));
		   AliaListLoadTable ();
		   DrawForm ();
         DetailViewDrawXButton ();
         
         SetNavFocusRing (AliaListDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;

    	case frmUpdateEvent:
		   DrawForm ();
         DetailViewDrawXButton ();
         handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
			   DrawForm ();
   	      DetailViewDrawXButton ();

         	SetNavFocusRing (AliaListDoneButton); // DTR 12-20-2005   	      
   	      }
   		handled = true;
			break;

	   default:
		   break;
   	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	AliaViewInit
//
// DESCRIPTION: 	This routine initializes the "Record View" of the 
//              	Address application.  Most importantly it lays out the
//              	record and decides how the record is drawn.
//
// PARAMETERS:  	None.
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void AliaViewInit (void)
{
   UInt16  			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   FontID 			curFont;

	DetailViewInit ();  // initialize gadget array

	ErrFatalDisplayIf (AliaRecH, "AliaViewInit: handle should not be locked.");
	
	if (DbGetRecord (IndiDB, CurrentAliaRecN, &AliaRec, &AliaRecH) != 0) {
      DVLines[DetailViewLastLine].fieldNum = detailViewError;
   	DVLines[DetailViewLastLine].offset = 0;
   	DetailViewLastLine++;
   	return;
   	}
	
   // Check event record to see if there is a source.
  	if (AliaRec.fields[indiSouCNo] != NULL) // DTR 2003-Jan-27
		ShowObject (AliaDetailCiteButton, true);
      	 
   // Get width and height of current drawn window.
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID * 2; // subtract 4 to allow room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);
  
   // Put information in gadget array
   if (AliaRec.fields[indiFName])
      DetailViewAddField (AliaRec.fields[indiFName], indiFName, &width,
         maxWidth, MARGIN_WID);
         
   // Separate the last name from the first as long as they are on the same line.
   if (width > MARGIN_WID)
   	DetailViewAddSpaceForText (" ", &width);
   if (!AliaRec.fields[indiLName])
      AliaRec.fields[indiLName] = cUnknownStr;
   DetailViewAddField (AliaRec.fields[indiLName], indiLName, &width,
      maxWidth, MARGIN_WID);

   // If the line above isn't blank then add a blank line
   DetailViewNewLineIf (&width);
   DetailViewFirstPlainLine = DetailViewLastLine; // line where plain text starts
 
   AliaRec.fields[indiSex] = IndiRec.fields[indiSex];
   DetailViewPositionTextAt (&width);
   DetailViewAddField (AliaRec.fields[indiSex], indiSex, &width,
      maxWidth, HEAD_WID);
 
   // Check record to see if there is a note and set note title
   DetailViewAddNote (AliaRec.fields[indiNoteNo], AliaDetailNoteButton,
   	indiNoteNo, &width, maxWidth, "Alias");
   
   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AliaViewHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Event Viewer
//                of the Event List Window.
//
// PARAMETERS:    -> event	- a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean AliaViewHandleEvent (EventPtr event)
{
   Boolean handled = false;

   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case AliaDetailDoneButton:
              	ErrFatalDisplayIf (SouCListH, "AliaViewHandleEvent: SouCListH must be NULL");
               FrmGotoForm (AliaListForm);
               UpdateFrm = false;
               handled = true;
               break;
               
            case AliaDetailCiteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               DetailViewGetSouCList (AliaRec.fields[indiSouCNo]);
               // already made sure SouCList cannot be NULL
               FrmGotoForm (SouCDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
         
            case AliaDetailNoteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               FrmGotoForm (NoteDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
          
          	case AliaDetailCloseButton:
          		DetailViewQuickClose ();
         		DbMemHandleUnlock (&AliaRecH);
               handled = true;
               break;
          	
            case AliaDetailInfoButton:
               FrmHelp (IndiHelpString);
               handled = true;
               break;
          
            default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, AliaDetailAliaDetailGadget);
         break;

      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case AliaDetailScrollUpRepeating:
				case AliaDetailScrollDownRepeating:
					ResetScrollRate (); // reset scroll rate.
					// leave unhandled so the buttons can repeat
					break;
				}
         break;

      case ctlRepeatEvent:

	      switch (event->data.ctlRepeat.controlID)
         	{
            case AliaDetailScrollUpRepeating:
					DetailViewScroll (winUp, AliaDetailAliaDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case AliaDetailScrollDownRepeating:
					DetailViewScroll (winDown, AliaDetailAliaDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
      case menuEvent:
        return MenuDoCommand (event->data.menu.itemID);
	
	   case frmOpenEvent:
     		DetailViewResizeForm (true);
         AliaViewInit ();
         DrawForm ();
         
         // Load TopDetailViewLine, but only after calling AliaViewInit. We
         // use TopDetailViewLine when we return from SouCViewForm.
         TopDetailViewLine = PriorTopDetailViewLine;
         DetailViewDraw (AliaRec, Indi, AliaDetailAliaDetailGadget,
         	AliaDetailScrollUpRepeating, AliaDetailScrollDownRepeating);
         
         PriorSouCFormID = PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ());
         PriorTopDetailViewLine = 0;  // reset to 0
         
			SetNavFocusRing (AliaDetailDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;
		   
	   case frmUpdateEvent:
	      DrawForm ();
	      
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            DbMemHandleUnlock (&AliaRecH);
            AliaViewInit ();
            }
            
         DetailViewDraw (AliaRec, Indi, AliaDetailAliaDetailGadget,
          	AliaDetailScrollUpRepeating, AliaDetailScrollDownRepeating);
          	
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
      	
				DrawForm ();
				
			  	DetailViewDraw (AliaRec, Indi, AliaDetailAliaDetailGadget,
	          	AliaDetailScrollUpRepeating, AliaDetailScrollDownRepeating);
         	
         	SetNavFocusRing (AliaDetailDoneButton); // DTR 12-20-2005 	
      		}
      	handled = true;
			break;

      case frmCloseEvent:  
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&AliaRecH);
         DbMemHandleUnlock (&NoteRecH);
         break;
			
	   default:
		   break;
   	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RepCViewInit
//
// DESCRIPTION:   Initializes the Repository Citation  View form. It 
//                lays out the record and decides how the record is drawn.
//                RepoRec must be filled before calling this function.
//
//						The RepoCRecordH and RepoRecH handles are locked by this
//						routine and must be unlocked later on.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RepCViewInit (void)
{
   UInt16  			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   UInt16			fldNo;
   FontID 			curFont;
   
   DetailViewInit (); // initialize gadget array
    
   DetailViewLastLine = TopDetailViewLine = 0; // both updated in DetailViewAddField
 
   DbMemHandleUnlock (&RepCRecH);
   DbMemHandleUnlock (&RepoRecH);
 
   // Get the Repository Citation Record that was selected
   if (DbGetRecord (RepCDB, CurrentRepCRecN, &RepCRec, &RepCRecH) != 0)
   	goto RepCError;
   
   // Get the Repository Record 
	if (RepCRec.fields[repCRepoNo]) {
		CurrentRepoRecN = (UInt32) StrAToI (RepCRec.fields[repCRepoNo]);
      DbGetRecord (RepoDB, CurrentRepoRecN, &RepoRec, &RepoRecH);

	   // if no Repository record then hide button
		ShowObject (RepCDetailRepoButton, (Boolean) (RepoRecH != NULL));
		}

	// Add items to display to DVLines
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID*2; // subtract 2 to allow some room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);

  	// Put repository name on first line.
   if (RepCRec.fields[repCRepoNo] && RepoRec.fields[repoName]) {
      DetailViewAddField (RepoRec.fields[repoName], detailViewrepCDescD, &width,
         maxWidth, MARGIN_WID);
         }
 
   for (fldNo = repCCaln; fldNo <= repCMedi; fldNo++) {
   	if (RepCRec.fields[fldNo]) {
   		DetailViewNewLineIf (&width);
      	DetailViewPositionTextAt (&width);
      	DetailViewAddField (RepCRec.fields[fldNo], fldNo, &width,
         	maxWidth, SUB_HEAD_WID);
         }
      }

   // Check record to see if there is a note and set note title
   DetailViewAddNote (RepCRec.fields[repCNoteNo], RepCDetailNoteButton,
   	repCNoteNo, &width, maxWidth, "Srce-Repo Citation");

   FntSetFont (curFont);

   return;
   
   /////////
   RepCError:
   /////////
   
   DVLines[DetailViewLastLine].fieldNum = detailViewError;
   DVLines[DetailViewLastLine].offset = 0;
   DetailViewLastLine++;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RepCViewHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Repository 
//                Citation Detail Viewer
//                
// PARAMETERS:    -> event - a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean RepCViewHandleEvent (EventPtr event)
{
   Boolean     handled = false;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case RepCDetailDoneButton:
               FrmGotoForm (SourDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;

            case RepCDetailRepoButton:
               PriorRepCTopDetailViewLine = TopDetailViewLine; // save top line
               FrmGotoForm (RepoDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
               
            case RepCDetailNoteButton:
               PriorRepCTopDetailViewLine = TopDetailViewLine; // save top line
               FrmGotoForm (NoteDetailForm);
               UpdateFrm = false;
               handled = true;
               break;
            
				case RepCDetailCloseButton:
					DetailViewQuickClose ();
					DbMemHandleUnlock (&RepCRecH);
   				DbMemHandleUnlock (&RepoRecH);
               handled = true;
               break;
               
				case RepCDetailInfoButton:
               FrmHelp (RCitHelpString);
               handled = true;
               break;

            default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, RepCDetailDetailGadget);
         break;
      
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case RepCDetailScrollUpRepeating:
				case RepCDetailScrollDownRepeating:
					ResetScrollRate(); // reset scroll rate.
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

			switch (event->data.ctlRepeat.controlID)
         	{
            case RepCDetailScrollUpRepeating:
					DetailViewScroll (winUp, RepCDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case RepCDetailScrollDownRepeating:
					DetailViewScroll (winDown, RepCDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
      
      case frmOpenEvent:
     		DetailViewResizeForm (true);
	   	RepCViewInit ();
	   	DrawForm ();

         // Load TopDetailViewLine, but only after calling RepCViewInit. We
         // use TopDetailViewLine when we return from RepCViewForm.
         TopDetailViewLine = PriorRepCTopDetailViewLine;
         DetailViewDraw (RepCRec, RepC, RepCDetailDetailGadget,
         	RepCDetailScrollUpRepeating, RepCDetailScrollDownRepeating);
         
         PriorNoteFormID = FrmGetFormId (FrmGetActiveForm());
         PriorRepCTopDetailViewLine = 0; // reset to 0;
         
         SetNavFocusRing (RepCDetailDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;
      
	   case frmUpdateEvent:
	   	DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            RepCViewInit ();
            }
            
         DetailViewDraw (RepCRec, RepC, RepCDetailDetailGadget,
       		RepCDetailScrollUpRepeating, RepCDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
				DetailViewDraw (RepCRec, RepC, RepCDetailDetailGadget,
       			RepCDetailScrollUpRepeating, RepCDetailScrollDownRepeating);
       			
       		SetNavFocusRing (RepCDetailDoneButton); // DTR 12-20-2005	
      		}
      	handled = true;
			break;

      case frmCloseEvent:
      	MemHandleFree (DVLinesH);
      	DVLines = NULL;
			DbMemHandleUnlock (&RepCRecH);
   		DbMemHandleUnlock (&RepoRecH);
   		DbMemHandleUnlock (&NoteRecH);
         break;
		
	   default:
		   break;
   	}
   	
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    RepoViewInit
//
// DESCRIPTION: This routine initializes the Repo View Window of the GedWise
//              application.  It lays out the record on the form.
//
//              IMPORTANT: RepoRec must be loaded before calling this funtion.
//
// PARAMETERS:  None.
//
// RETURNED:    Nothing.
//
// REVISIONS:	 None.
////////////////////////////////////////////////////////////////////////////////////
static void RepoViewInit (void)
{
   UInt16  			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   FontID 			curFont;

   DetailViewInit ();  // initialize gadget array

	ErrFatalDisplayIf (RepoRecH, "RepoViewInit: handle should not be locked.");

   // Get Repo Record.  We already confirmed CurrentRepoRecN is valid in 	//
   // the RepoCitViewInit routine, so we don't need to much error catching	//
   if (DbGetRecord (RepoDB, CurrentRepoRecN, &RepoRec, &RepoRecH) != 0)
   	return;
      	
   // Add items to display to DVLines
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID*2; // subtract 2 for room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);

   if (RepoRec.fields[repoName]) {
      DetailViewAddField (RepoRec.fields[repoName], repoName, &width,
         maxWidth, MARGIN_WID);
      }

   if (RepoRec.fields[repoAddr]) {
      if (width > MARGIN_WID)
         DetailViewAddSpaceForText (cFieldSepStr, &width);
      DetailViewAddField (RepoRec.fields[repoAddr], repoAddr, &width,
         maxWidth, MARGIN_WID);
      }

   // Check record to see if there is a note and set note title
   DetailViewAddNote (RepoRec.fields[repoNoteNo], RepoDetailNoteButton, repoNoteNo,
		&width, maxWidth, "Repository");

   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	RepoViewHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Source Viewer
//              	of the Source View Form.
//                
// PARAMETERS:  	-> event -	a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean RepoViewHandleEvent (EventPtr event)
{
   Boolean handled = false;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case RepoDetailDoneButton:
               FrmGotoForm (RepCDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;
               
            case RepoDetailNoteButton:
               PriorRepoTopDetailViewLine = TopDetailViewLine; // hold top line
               FrmGotoForm (NoteDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;

				case RepoDetailCloseButton:
					DetailViewQuickClose ();
           		DbMemHandleUnlock (&RepoRecH);
               handled = true;
               break;
               
				case RepoDetailInfoButton:
               FrmHelp (RepoHelpString);
               handled = true;
               break;
            
           default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual(event, RepoDetailDetailGadget);
         break;
      
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case RepoDetailScrollUpRepeating:
				case RepoDetailScrollDownRepeating:
					ResetScrollRate (); // reset scroll rate.
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

		   switch (event->data.ctlRepeat.controlID)
         	{
            case RepoDetailScrollUpRepeating:
					DetailViewScroll (winUp, RepoDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case RepoDetailScrollDownRepeating:
					DetailViewScroll (winDown, RepoDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
     	
     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
      
	   case frmOpenEvent:
	   	DetailViewResizeForm (true);
	   	RepoViewInit ();
	   	DrawForm ();
         
         // Load TopDetailViewLine, but only after calling SouCViewInit. We
         // use TopDetailViewLine when we return from RepoViewForm.
         TopDetailViewLine = PriorRepoTopDetailViewLine;
         DetailViewDraw (RepoRec, Repo, RepoDetailDetailGadget,
         	RepoDetailScrollUpRepeating, RepoDetailScrollDownRepeating);
         
         PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ()); // for return from Note
         PriorRepoTopDetailViewLine = 0; // reset to 0
         
         SetNavFocusRing (RepoDetailDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;

	   case frmUpdateEvent:
	   	DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            DbMemHandleUnlock (&RepoRecH);
            RepoViewInit ();
            }
        	
        	DetailViewDraw (RepoRec, Repo, RepoDetailDetailGadget,
        		RepoDetailScrollUpRepeating, RepoDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
			  	DetailViewDraw (RepoRec, Repo, RepoDetailDetailGadget,
        			RepoDetailScrollUpRepeating, RepoDetailScrollDownRepeating);
        			
     		 	SetNavFocusRing (RepoDetailDoneButton); // DTR 12-20-2005	
      		}
      	handled = true;
			break;

      case frmCloseEvent:
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&RepoRecH);
         DbMemHandleUnlock (&NoteRecH);
         break;
	
	   default:
		   break;
   	}
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SouCListScroll
//
// DESCRIPTION:   Scrolls the Source Citation List in the direction specified.
//
// PARAMETERS:    -> direction - up or dowm
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void SouCListScroll (DirType direction)
{
	UInt16	newTopVisibleNum  = TopVisSouCNum;
	
	if (direction == NavR) { // scroll to next source citation record.
      if (TopVisSouCNum < LastSouCNum)
         newTopVisibleNum++;
      }
	else { // scroll to previous source citation record.
      if (TopVisSouCNum > 1)
         newTopVisibleNum--;
		}
		
	// Avoid redraw if no change
	if (TopVisSouCNum != newTopVisibleNum) {
		TopVisSouCNum = newTopVisibleNum;
      DrawForm ();
      SouCViewInit ();
      SouCNumberDraw ();
      DetailViewDraw (SouCRec, SouC, SouCDetailDetailGadget,
      	SouCDetailScrollUpRepeating, SouCDetailScrollDownRepeating);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SouCNumberDraw
//
// DESCRIPTION:   Draws information about the data items' total number
//                of sources citations as well as the number of the
//                current source citation being viewed.  The scroll buttons are
//						also updated.
//				
//						IMPORTANT: the LastSouCNum variable must be loaded prior to
//						calling this routine.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void SouCNumberDraw (void)
{
   UInt16 			y;
   FontID 			curFont;
   RectangleType 	rect;
   Char				string[25];
   Boolean 			scrollableL;
   Boolean 			scrollableR;

   GetObjectBounds (SouCDetailSCiteNumberGadget, &rect);
   curFont = FntSetFont (TITLE_FONT);
 		
   y = rect.topLeft.y;
   
   if (LastSouCNum > 0) {
   	StrPrintF (string, SCIT_HEAD_STR, TopVisSouCNum, LastSouCNum);
  		WinDrawChars (string, StrLen (string), MARGIN_WID, y);
      }

   FntSetFont (curFont);

  	// Update scroll buttons
   scrollableL = (Boolean) (TopVisSouCNum > 1);
   scrollableR = (Boolean) (TopVisSouCNum < LastSouCNum);
   UpdateLeftRightScrollers (SouCDetailScrollLeftButton, SouCDetailScrollRightButton,
   	scrollableL, scrollableR);
   
   // Draw line under source number information
   y+= FntLineHeight ();
   WinDrawGrayLine (0, y, 154, y); 
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SouCViewInit
//
// DESCRIPTION:   This routine initializes the Source View Window of the 
//                GedWise application.  It lays out the record on the form.
//                It is assumed that SourRec has a valid source record number
//                prior to calling this function.
//
//						NOTE : The SourRecH and SouCRecH handles are locked by this
//						routine and must be unlocked later.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void SouCViewInit (void)
{
   UInt16 			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   FontID 			curFont;
   UInt32         recN;
   UInt16			fldNo;
   Char           souCXRef[NREF_LEN+1];
   
   DetailViewLastLine = TopDetailViewLine = 0; // both updated in SouCViewAddField
   
   DbMemHandleUnlock (&SouCRecH);
   DbMemHandleUnlock (&SourRecH);
 
   // Get the Source Citation Record that was selected
	if (!RefFinderStr (TopVisSouCNum, SOUC_DLM, SouCList, souCXRef))
     goto SouCError;

   recN = (UInt32) StrAToI (souCXRef);
   
   if (DbGetRecord (SouCDB, recN, &SouCRec, &SouCRecH) != 0)
   	goto SouCError;

   // Get the Source Record reference number for the Source Citation
	if (SouCRec.fields[souCSourNo]) {
		CurrentSourRecN = (UInt32) StrAToI (SouCRec.fields[souCSourNo]);
      DbGetRecord (SourDB, CurrentSourRecN, &SourRec, &SourRecH);

	   // if Source record then show button
 //  	ShowObject (SouCDetailSourButton, (Boolean) SourRecH != NULL);
   	ShowObject (SouCDetailSourButton, true);
      }

   // Add items to display to DVLines
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID * 2; // subtract to allow room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);

   if (SouCRec.fields[souCSourNo] && SourRec.fields[sourTitl]) {
      DetailViewAddField (SourRec.fields[sourTitl], detailViewsouCDescD, &width,
         maxWidth, MARGIN_WID);
      }

   if (SouCRec.fields[souCPage]) {
      if (width > MARGIN_WID)
         DetailViewAddSpaceForText (cFieldSepStr, &width);
      DetailViewAddField(SouCRec.fields[souCPage], souCPage, &width,
         maxWidth, MARGIN_WID);
      }

	for (fldNo = souCEven; fldNo <= souCText; fldNo++) {
   	if (SouCRec.fields[fldNo]) {
      	DetailViewNewLineIf (&width);
      	DetailViewPositionTextAt (&width);
      	DetailViewAddField (SouCRec.fields[fldNo], fldNo, &width,
         	maxWidth, SUB_HEAD_WID);
      	}
		}

   // Check record to see if there is a note and set note title
   DetailViewAddNote (SouCRec.fields[souCNoteNo], SouCDetailNoteButton,
   	souCNoteNo,	&width, maxWidth, "Citation");
 
   FntSetFont (curFont);
   //SouCNumberDraw (); DTR: removed 9-13-2004
   return;
   
   /////////
   SouCError:
   /////////
   
   DVLines[DetailViewLastLine].fieldNum = detailViewError;
 	DVLines[DetailViewLastLine].offset = 0;
  	DetailViewLastLine++;
   //SouCNumberDraw (); DTR: removed 9-13-2004
 }

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	SouCViewHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Source Viewer
//              	of the Source View Form.
//                
// PARAMETERS:  	-> event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean SouCViewHandleEvent (EventPtr event)
{
   Boolean  handled = false;
  // DirType	navDir;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         
         switch (event->data.ctlSelect.controlID)
         	{
            case SouCDetailDoneButton:
              if (SouCListH) {
						MemHandleFree (SouCListH);
  						SouCListH = NULL;
  						}
               FrmGotoForm (PriorSouCFormID);  // set in prior form
       			UpdateFrm = false;
               handled = true;
               break;
               
            case SouCDetailSourButton:
               PriorSouCTopDetailViewLine = TopDetailViewLine; // hold top line
               SouCViewPriorSouCNumber = TopVisSouCNum; // used on return from repo
               FrmGotoForm (SourDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
         
            case SouCDetailNoteButton:
               PriorSouCTopDetailViewLine = TopDetailViewLine; // hold top line
               SouCViewPriorSouCNumber = TopVisSouCNum; // used on return from note 
               FrmGotoForm (NoteDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;
            
            case SouCDetailScrollRightButton:
               SouCListScroll (NavR);
               handled = true;
               break;
               
            case SouCDetailScrollLeftButton:
               SouCListScroll (NavL);
               handled = true;
               break;

				case SouCDetailCloseButton:
               DetailViewQuickClose ();
               DbMemHandleUnlock (&SouCRecH);
					DbMemHandleUnlock (&SourRecH);
               handled = true;
               break;
               
				case SouCDetailInfoButton:
               FrmHelp (SCitHelpString);
               handled = true;
               break;
               
            default:
               break;
         	}
         break;

      case keyDownEvent:
         
         handled = DetailViewHandleVirtual (event, SouCDetailDetailGadget);
       	SetNavFocusRing (SouCDetailDoneButton); // DTR 12-20-2005
         break;
         
         /*if (NavKeyHit (event, &navDir)) {
				
				switch (navDir) {
					case NavUp:
					case NavDn:
            		// Reset scroll rate if not auto repeating
		   			if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
	   					ResetScrollRate ();
	   		
						DetailViewScroll (navDir == NavUp ? winUp : winDown,
							SouCDetailDetailGadget);
            		handled = true;
						break;
					
					case NavL:
					case NavR:
						SouCListScroll (navDir);
						handled = true;
						break;
					
					default:
						break;
					}
         	}
         break; */
      
      case ctlEnterEvent:
			
			switch (event->data.ctlEnter.controlID)
				{
				case SouCDetailScrollUpRepeating:
				case SouCDetailScrollDownRepeating:
					ResetScrollRate (); // reset scroll rate.
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

         switch (event->data.ctlRepeat.controlID)
         	{
            case SouCDetailScrollUpRepeating:
					DetailViewScroll (winUp, SouCDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case SouCDetailScrollDownRepeating:
					DetailViewScroll (winDown, SouCDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
      
	   case frmOpenEvent:
	   	DetailViewResizeForm (true);
   		LastSouCNum = RefCounter (SOUC_DLM, SouCList);
         TopVisSouCNum = SouCViewPriorSouCNumber;
         DetailViewInit (); // initialize gadget array
         SouCViewInit ();
         DrawForm ();
	      
	      SouCNumberDraw ();
        
         // Load TopDetailViewLine, but only after calling SouCViewInit. We
         // use TopDetailViewLine when we return from RepCViewForm.
         TopDetailViewLine = PriorSouCTopDetailViewLine;
         DetailViewDraw (SouCRec, SouC, SouCDetailDetailGadget,
         	SouCDetailScrollUpRepeating, SouCDetailScrollDownRepeating);
         
         PriorNoteFormID = FrmGetFormId (FrmGetActiveForm()); // used for return from Note View
         PriorSouCTopDetailViewLine = 0; // reset to 0
         SouCViewPriorSouCNumber = 1;
         
         SetNavFocusRing (SouCDetailDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;

	   case frmUpdateEvent: 
	   	DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            DetailViewInit ();  // initialize gadget array
            SouCViewInit ();
            }
         
         SouCNumberDraw();
			DetailViewDraw (SouCRec, SouC, SouCDetailDetailGadget,
				SouCDetailScrollUpRepeating, SouCDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
				SouCNumberDraw();
			 	DetailViewDraw (SouCRec, SouC, SouCDetailDetailGadget,
	         	SouCDetailScrollUpRepeating, SouCDetailScrollDownRepeating);
	         	
	         SetNavFocusRing (SouCDetailDoneButton); // DTR 12-20-2005	
      		}
			handled = true;
			break;

      case frmCloseEvent:
      	MemHandleFree (DVLinesH);
      	DVLines = NULL;
      	DbMemHandleUnlock (&SouCRecH);
			DbMemHandleUnlock (&SourRecH);
			DbMemHandleUnlock (&NoteRecH);
         break;
	
	   default:
		   break;
   	}
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SourViewInit
//
// DESCRIPTION:   Initializes the Source View form.  It lays out the record on the
//                form.
//
//                IMPORTANT: SourRec must be loaded before calling this function.
//
//						The SourRecH handle is locked by this routine and must be
//						unlocked later on.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void SourViewInit (void)
{
   UInt16  			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
	UInt16			fldNo;
   FontID 			curFont;

   DetailViewInit(); // initialize gadget array
   
   ErrFatalDisplayIf (SourRecH, "SourViewInit: handle should not be locked.");
   
   // Get Source Record.  We already confirmed CurrentSourRecN is valid in
   // the SouCViewInit routine, so we don't need to much error catching
	if (DbGetRecord (SourDB, CurrentSourRecN, &SourRec, &SourRecH) != 0)
   	return;
  
	// Check Source Record to see if there is a Repository Citation
	if (SourRec.fields[sourRepCNo] != NULL)
		ShowObject (SourDetailCiteButton, true);
	
   // Add items to display to DVLines
   FrmGetFormBounds (FrmGetActiveForm(), &rect);
   maxWidth = rect.extent.x - MARGIN_WID*2; // subtract to allow room on edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);

   if (SourRec.fields[sourTitl]) {
      DetailViewAddField (SourRec.fields[sourTitl], sourTitl, &width,
         maxWidth, MARGIN_WID);
      }
  
	for (fldNo = sourEven; fldNo <= sourNumb; fldNo++) {
   	if (SourRec.fields[fldNo]) {
      	DetailViewNewLineIf (&width);
      	DetailViewPositionTextAt (&width);
      	DetailViewAddField (SourRec.fields[fldNo], fldNo, &width,
         	maxWidth, SUB_HEAD_WID);
      	}
      }

   // Check record to see if there is a note and set note title
   DetailViewAddNote (SourRec.fields[sourNoteNo], SourDetailNoteButton, sourNoteNo,
		&width, maxWidth, "Source");

   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	SourViewHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Source View Form.
//                
// PARAMETERS:  	-> event -	a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean SourViewHandleEvent (EventPtr event)
{
   Boolean handled = false;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case SourDetailDoneButton:
               FrmGotoForm (SouCDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
               
            case SourDetailCiteButton:
               PriorSourTopDetailViewLine = TopDetailViewLine; // hold top line
               CurrentRepCRecN = (UInt32) StrAToI (SourRec.fields[sourRepCNo]);
               FrmGotoForm (RepCDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;
         
            case SourDetailNoteButton:
               PriorSourTopDetailViewLine = TopDetailViewLine; // remember top line
               FrmGotoForm (NoteDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;
             
  				case SourDetailCloseButton:
					DetailViewQuickClose ();
         		DbMemHandleUnlock (&SourRecH);
               handled = true;
               break;
               
				case SourDetailInfoButton:
               FrmHelp (SourHelpString);
               handled = true;
               break;
                 
            default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, SourDetailDetailGadget);
         break;
      
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case SourDetailScrollUpRepeating:
				case SourDetailScrollDownRepeating:
					// Reset scroll rate
					ResetScrollRate();
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

         switch (event->data.ctlRepeat.controlID)
         	{
            case SourDetailScrollUpRepeating:
					DetailViewScroll (winUp, SourDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case SourDetailScrollDownRepeating:
					DetailViewScroll (winDown, SourDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);

		case frmOpenEvent:
			DetailViewResizeForm (true);
			SourViewInit ();
			DrawForm ();
        
         // Load TopDetailViewLine, but only after calling SouCViewInit. We
         // use TopDetailViewLine when we return from RepoViewForm.
         TopDetailViewLine = PriorSourTopDetailViewLine;
         DetailViewDraw (SourRec, Sour, SourDetailDetailGadget,
         	SourDetailScrollUpRepeating, SourDetailScrollDownRepeating);
         
         PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ()); // for return from Note
         PriorSourTopDetailViewLine = 0; // reset to 0
         
         SetNavFocusRing (SourDetailDoneButton); // DTR 12-20-2005
         
         handled = true;
         break;

	   case frmUpdateEvent:
	   	DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            DbMemHandleUnlock (&SourRecH);
            SourViewInit ();
            }
			DetailViewDraw (SourRec, Sour, SourDetailDetailGadget,
				SourDetailScrollUpRepeating, SourDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
			 	DetailViewDraw (SourRec, Sour, SourDetailDetailGadget,
					SourDetailScrollUpRepeating, SourDetailScrollDownRepeating);
					
				SetNavFocusRing (SourDetailDoneButton); // DTR 12-20-2005	
      		}
			handled = true;
			break;


      case frmCloseEvent:
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&SourRecH);
         DbMemHandleUnlock (&NoteRecH);
         break;
	
	   default:
		   break;
   	}
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      EvenDrawRecord
//
// DESCRIPTION:   Draws the event description and the date of the event
// 				   within the screen bounds passed.
//
// PARAMETERS:    -> record - record to draw
//                -> bounds - bounds of the draw region
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
//////////////////////////////////////////////////////////////////////////////////// 
static void EvenDrawRecord (DBRecordType* record, RectanglePtr bounds)
{
   Int16    		x, y;
   Char*				eventDate;
   Int16    		eventDateWidth;
   Int16    		eventDateLength;
   Int16    		eventDescWidth;
   Char*				eventDesc;
   Int16    		eventDescLength;
   Boolean  		ignored;
   UInt16   		descExtent;
   UInt16 			evenTypeN;
	MemHandle 		recH = NULL;
	DBRecordType	rec;

   Int16    shortenedFieldWidth = (FntCharWidth('.') * SHORTENED_FLD_LEN);

   x = bounds->topLeft.x;
   y = bounds->topLeft.y;
	
   // -- Get Event Date information. --
   eventDate = record->fields[evenDate];
   
	// Substitute the eventDate with event description (at users option) if no date.
	if (!eventDate && Prefs[UseEvenDesc]) {
		
		evenTypeN = (UInt16) StrAToI (record->fields[evenType]);
	
		// if this is a note, use note text.
		if (evenTypeN == EVEN_NOTE_POS) {
		
			if (NoteRecFinder (record->fields[evenNoteNo], &rec, &recH) == 0)
	   		eventDate = rec.fields[noteText];
	   	}
	
	   // if no date and event is not an event type then use description.
	   if (!eventDate && evenTypeN != EVEN_POS)
			eventDate = record->fields[evenDesc];
	
		// if no description then try place.
		if (!eventDate) 
			eventDate = record->fields[evenPlac];
		
		// if no place then try address.
		if (!eventDate) 
			eventDate = record->fields[evenAddr];
		}
  	   
   // If evenDate is empty, set pointer to "No Date" string.
   if (eventDate == NULL)
      eventDate = NO_DATE_STR;
   
   ErrFatalDisplayIf (eventDate == NULL, "EvenDrawRecord: eventDate cannot be NULL.");
   
   eventDateWidth = eventDateLength = bounds->extent.x; // more chrs than we can expect
   FntCharsInWidth (eventDate, &eventDateWidth, &eventDateLength, &ignored);

	// -- Get Event Description information. --
   eventDesc = EventDesc (record); // get ptr to event description

   ErrFatalDisplayIf (eventDesc == NULL, "EvenDrawRecord: eventDesc cannot be NULL.");

   eventDescWidth = eventDescLength = bounds->extent.x; // more chrs than we can expect
   FntCharsInWidth (eventDesc, &eventDescWidth, &eventDescLength, &ignored);

   // -- Draw event description and event date. --
   if (bounds->extent.x >= eventDescWidth + SPACE_BEF_LSPAN + eventDateWidth) {

      // We can draw it all!

      WinDrawChars (eventDesc, eventDescLength, x, y);
      
      WinDrawChars (eventDate, eventDateLength, bounds->topLeft.x +
      	bounds->extent.x - eventDateWidth, y);
      }
  
   else { // we cannot draw it all.

		// Set maximum allowable extent for event description.
		descExtent = bounds->extent.x - min (eventDateWidth, LSPAN_COL_WIDTH) - 
      	SPACE_BEF_LSPAN;
      
      // Add 3 periods for curtailed event description.
      if (eventDescWidth > descExtent) {
         
         eventDescWidth = descExtent - shortenedFieldWidth;
         FntCharsInWidth (eventDesc, &eventDescWidth, &eventDescLength, &ignored);
         WinDrawChars (eventDesc, eventDescLength, x, y);
         WinDrawChars (SHORTENED_FLD_STR, SHORTENED_FLD_LEN, x + eventDescWidth, y);
         x += shortenedFieldWidth;
         }

      else { // it fits
   
         WinDrawChars (eventDesc, eventDescLength, x, y);
         }

		x += eventDescWidth + SPACE_BEF_LSPAN;

		descExtent = x - bounds->topLeft.x;

      if (bounds->extent.x - descExtent >= eventDateWidth) { // it fits
            
      	WinDrawChars (eventDate, eventDateLength, bounds->topLeft.x +
         	bounds->extent.x - eventDateWidth, y);
      	}
 
     	else { // it doesn't fit
     		
      	eventDateWidth = bounds->extent.x - descExtent - shortenedFieldWidth;

			FntCharsInWidth (eventDate, &eventDateWidth, &eventDateLength, &ignored);

		   // Draw event date.  It must be right justified.
		   WinDrawChars (eventDate, eventDateLength, bounds->topLeft.x +
   		  	bounds->extent.x - eventDateWidth - shortenedFieldWidth, y);
		         
		   // Add 3 periods for curtailed date description.
			WinDrawChars (SHORTENED_FLD_STR, SHORTENED_FLD_LEN, bounds->topLeft.x +
   		  	bounds->extent.x - shortenedFieldWidth, y);
      	}
		}

	if (recH) DbMemHandleUnlock (&recH);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      EvenSummaryDrawRecord
//
// DESCRIPTION:   This routine draws an event record.  It is called as a callback
//                routine by the table object.
//
// PARAMETERS:    -> table  -	pointer to the address list table
//                -> row    -	row number, in the table, of the item to draw
//                -> column -	column number, in the table, of the item to draw
//                -> bounds -	bounds of the draw region
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
//////////////////////////////////////////////////////////////////////////////////// 
static void EvenSummaryDrawRecord (void* table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   UInt32			recN; // record in EvenDB
   DBRecordType	rec;
   MemHandle      recH = NULL;
   FontID 	      curFont;

   // Get the record number that corresponds to the table item to draw.
   recN = TblGetRowData (table, row);

	// Check if we are displaying no event data message.
   if (recN == NO_REC_LONG) {
  		curFont = FntSetFont (boldFont);
  		WinDrawChars (NO_EVEN_STR, NO_EVEN_STR_LEN, bounds->topLeft.x,
  			bounds->topLeft.y+1);
		FntSetFont (curFont);
		return;
		}
	
   if (DbGetRecord (EvenDB, recN, &rec, &recH) != 0) {
	   TblSetRowSelectable (table, row, false);
	   ErrNonFatalDisplay ("EventSummaryDrawRecord: Record not found");
   	return;
   	}

   curFont = FntSetFont (PrefFnts[IndiSummFont]);
   EvenDrawRecord (&rec, bounds);
	FntSetFont (curFont);

   DbMemHandleUnlock (&recH);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	EventListScroll
//
// DESCRIPTION: 	This routine scrolls the event list and dates in the direction
//						specified.  Scrolls one page at a time.
//
// PARAMETERS:  	-> direction -	up or dowm
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void EvenListScroll (DirType direction)
{
	TablePtr	table;
   UInt16	scrollUnits;
	UInt32 	newTopVisibleRecN; // record in EvenDB
	UInt32 	prevTopVisibleRecN = TopVisEvenRecN; // record in EvenDB

	table = GetObjectPtr (IndiSummEvenListTable);

 	// Check if any entries in Event List
 	if (LastEvenRecN == NO_REC_LONG)  // moved from IndiSummHandleEvent 8-3-04
		return; 

	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus (0);

	scrollUnits = TblGetLastUsableRow (table);

	newTopVisibleRecN = TopVisEvenRecN; 

	// Scroll the table down
	if (direction == NavDn || direction == NavR) {
		newTopVisibleRecN+= scrollUnits;
		if (newTopVisibleRecN + scrollUnits > LastEvenRecN) {
			if (LastEvenRecN > (FirstEvenRecN + scrollUnits))
				newTopVisibleRecN = LastEvenRecN - scrollUnits;
			else
				newTopVisibleRecN = FirstEvenRecN;
			}
		}
   else { // scroll the table up
		if (newTopVisibleRecN > FirstEvenRecN + scrollUnits)
      	newTopVisibleRecN -= scrollUnits;
    	else
  			newTopVisibleRecN = FirstEvenRecN;
      }

	// avoid redraw if no change
	if (TopVisEvenRecN != newTopVisibleRecN) {
		TopVisEvenRecN = newTopVisibleRecN;
		EvenListLoadTable ();
		
		// Compare previous top record to current one after EventListLoadTable 
		// as it will adjust TopVisEvenRecN if drawing from newTopVisibleRecN
		// does not fill the whole screen with items.
		if (TopVisEvenRecN != prevTopVisibleRecN)
			TblRedrawTable (table); 
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      EventListLoadTable
//
// DESCRIPTION:   This routine loads event database records into the Event List
//                table.
//
//						NOTE: The variables FirstEvenRecN and LastEvenRecN must be
// 					initialized before calling this function.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void EvenListLoadTable (void)
{
   Int16				row = 0; // init
   UInt16      	numRows;
   UInt32			recN;
   TablePtr 		table;
   Boolean     	scrollableU;
   Boolean 	   	scrollableD;
  	RectangleType	tblBounds;
    
  	table = (TablePtr) GetObjectPtr (IndiSummEvenListTable);
 	
 	TblGetBounds (table, &tblBounds); 

   // Get number of rows that will fit on screen (but limit it to total tbl rows)
   numRows = min ((tblBounds.extent.y / TblGetRowHeight (table, 0)),
		TblGetNumberOfRows (table)); 
         
   if (FirstEvenRecN != NO_REC_LONG) { // then there are events to display
      
		// reset TopVisEvenRecN if we have scrolled too far.
		if (TopVisEvenRecN + (numRows - 1) > LastEvenRecN)
			TopVisEvenRecN = (LastEvenRecN - FirstEvenRecN > (numRows - 1))
			 	? LastEvenRecN - (numRows - 1) : FirstEvenRecN;

	   recN = TopVisEvenRecN;

		ErrFatalDisplayIf (TopVisEvenRecN < FirstEvenRecN || TopVisEvenRecN > LastEvenRecN, 
			"EventListLoadTable: TopVisEvenRecN error");
		ErrFatalDisplayIf (!EventsVisible && FamiRec.fields[famiEvenNo] &&
			FirstEvenRecN != StrAToI (FamiRec.fields[famiEvenNo]), "EventListLoadTable: Bad FirstEvenRecN");
		ErrFatalDisplayIf (EventsVisible && EvenRec.fields[indiEvenNo] &&
			FirstEvenRecN != StrAToI (EvenRec.fields[indiEvenNo]), "EventListLoadTable: Bad FirstEvenRecN");
		
		// For each row in table, store event record number in data field.
   	while (row < numRows && recN <= LastEvenRecN) {
	     	TblSetRowUsable (table, row, true);
   	  	TblMarkRowInvalid (table, row);
      	TblSetRowSelectable (table, row, true);
      	TblSetRowData (table, row, recN);
      	row++;
      	recN++;
      	}
		} // if (FirstEvenRecN != NO_REC_LONG)
		
 	else { // so there are no Events
   	TblSetRowUsable (table, row, true);
     	TblMarkRowInvalid (table, row);
     	TblSetRowSelectable (table, row, false);
      TblSetRowData (table, row, NO_REC_LONG);
     	row++;
		}

	// Hide the items that don't have any data.
   while (row < numRows) {
     	TblSetRowUsable (table, row, false);
     	row++;
     	}

	// Update scroll buttons.
	if (FirstEvenRecN == NO_REC_LONG) {
  		scrollableU = scrollableD = false;
  		}
  	else {
  		scrollableU = (Boolean) (TopVisEvenRecN > FirstEvenRecN);
  		scrollableD = (Boolean) ((TopVisEvenRecN + (numRows - 1)) < LastEvenRecN);
   	}

   UpdateScrollers (IndiSummScrollUpRepeating, IndiSummScrollDownRepeating,
   	scrollableU, scrollableD);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	EvenLoadData
//
// DESCRIPTION:	Loads event information for either IndiRec or FamiRec
//              	depending on if global EventsVisible is true or false.  All 
//						event number variables are initialized.
//  
//						The IndiDB's and/or FamiDB's evenFld must be formated as the
//						first event record number for the individual/family + ' ' + 
//						the count of total events for the individual/family
//						(e.g. "1234 5").

//						IMPORTANT: an IndiRec must be loaded and locked prior to calling
//						this routine.  This routine will initialize the following global
//						variables:
//
//						FirstEvenRecN	-	Record number in EvenDB of the first
//												general/family event for the individual.
//						LastEvenRecN	-	Record number in EvenDB for the last
//												general/family event for the individual.
//						TopVisEvenRecN	-	Record nuber in EvenDB of the first event
//												record to show in the event list.
//
// PARAMETERS:		None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void EvenLoadData (void)
{
  Char*	evenFld = NULL;

	// Initialize global variables.
	FirstEvenRecN = LastEvenRecN = NO_REC_LONG;

   // Find the individual's first and last Event / Family records, if any.
   if (EventsVisible) { // event information is visible
      ErrFatalDisplayIf (!IndiRecH, "EvenLoadData: IndiRecH should not be NULL");
      evenFld = IndiRec.fields[indiEvenNo];
     	}
  
   else { // family information is visible.
      if (CurrentFamiRecN != NO_REC_LONG) { // check if family record loaded
      	ErrFatalDisplayIf (!FamiRecH, "EvenLoadData: FamiRecH should not be NULL");
        	evenFld = FamiRec.fields[famiEvenNo];
         }
      }

	// Set LastEvenRecN and TopVisEvenRecN.
	if (evenFld != NULL) {
   	
   	Char* cPos;
	   
	   FirstEvenRecN = (UInt32) StrAToI (evenFld); // ignores data after ' '
   
   	cPos = StrChr (evenFld, ' '); // search for space
   
   	if (cPos != NULL) {
   		cPos++;
   		LastEvenRecN =  FirstEvenRecN + StrAToI (cPos) - 1;
   		}
		}
		
	TopVisEvenRecN = FirstEvenRecN;	
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      EvenListInit
//
// DESCRIPTION:   This routine initializes the Event List Table on the 
//                Individual Summary Form.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void EvenListInit (void)
{
   Int16			row;
   Int16 		rowsInTable;
   TablePtr 	table;
   FontID		curFont;

	curFont = FntSetFont (IndiSummFont);
	table   = (TablePtr) GetObjectPtr (IndiSummEvenListTable);

	// Initialize the address list table.
   TblUnhighlightSelection (table); // this must be done here

   rowsInTable = TblGetNumberOfRows (table); // init all possible rows
   
   for (row = 0; row < rowsInTable; row++) {
      TblSetItemStyle (table, row, 0, customTableItem);
      TblSetRowHeight (table, row, FntLineHeight ());
      TblSetRowUsable (table, row, false);
      }

	FntSetFont (curFont);

   TblSetColumnUsable (table, 0, true);
   TblSetColumnUsable (table, 1, false);
   TblSetColumnWidth  (table, 0, 160);
      
   // Set the callback routine that will draw the records.
   TblSetCustomDrawProcedure (table, 0, EvenSummaryDrawRecord);

   // Load records into the event list.
   EvenListLoadTable ();
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	EvenViewInit
//
// DESCRIPTION: 	This routine initializes the Event View Window of the GedWise
// 					application.  Most importantly it lays out the record and decides
//						how the record is drawn.
//
// 					NOTE: CurrentEvenRecN must be loaded with a valid record
//					 	number before calling this function.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
////////////////////////////////////////////////////////////////////////////////////
static void EvenViewInit (void)
{
   UInt16  			width = MARGIN_WID;
   RectangleType	rect;
   UInt16 			maxWidth;
	UInt16			fldNo;
   FontID 			curFont;

   DetailViewInit(); // initialize gadget array

	ErrFatalDisplayIf (EvenRecH, "EvenViewInit: handle should not be locked.");

   // Get the event record that was selected.
   if (DbGetRecord (EvenDB, CurrentEvenRecN, &EvenRec, &EvenRecH) != 0) {
      DVLines[DetailViewLastLine].fieldNum = detailViewError;
   	DVLines[DetailViewLastLine].offset = 0;
   	DetailViewLastLine++;
   	return;
   	}
   	      
   // Check event record to see if there is a source citation.
   if (EvenRec.fields[evenSouCNo] != NULL) // DTR: 27 Jan 2003
		ShowObject (EvenDetailCiteButton, true);

   // Now start loading gadget array.
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID*2;  // subtract 2 for room on right edge
   curFont = FntSetFont (PrefFnts[DetailViewFont]);
  
  if (EvenRec.fields[evenDesc]) {
  		DetailViewAddField (EvenRec.fields[evenDesc], evenDesc, &width,
      maxWidth, MARGIN_WID);
      }
  
   // Copy long event description into EvenRec
   if (!EvenRec.fields[evenDesc])
   	EvenRec.fields[evenDesc] = EventDesc (&EvenRec);
     
	// Initialize fields in which we want "Unknown" to appear...but only
	// if the event is not a NOTE.
	if (StrAToI (EvenRec.fields[evenType]) != EVEN_NOTE_POS) {
		if (!EvenRec.fields[evenDate])
  			EvenRec.fields[evenDate] = cUnknownStr;
 			
	  	// only show evenPlac as "Unknown" if no evenAddr
	  	if (!EvenRec.fields[evenPlac] && !EvenRec.fields[evenAddr])	
	  		EvenRec.fields[evenPlac] = cUnknownStr;
	  	}
 
 	for (fldNo = evenDate; fldNo <= evenTempL; fldNo++) {
   	if (EvenRec.fields[fldNo]) {
      	DetailViewNewLineIf (&width);
      	DetailViewPositionTextAt (&width);
      	DetailViewAddField (EvenRec.fields[fldNo], fldNo, &width,
         	maxWidth, SUB_HEAD_WID);
      	}
		}
		
	// Check record to see if there is a Note
   DetailViewAddNote (EvenRec.fields[evenNoteNo], EvenDetailNoteButton,
   	evenNoteNo,	&width, maxWidth, NULL); // leave DataStr empty
   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	EvenViewHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Event Viewer
//              	of the Event List Window.
//              	It is assumed the EvenRec is filled before reaching
//              	here.
//                
// PARAMETERS:  	-> event - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean EvenViewHandleEvent (EventPtr event)
{
   Boolean handled = false;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
         	case EvenDetailCloseButton:
            case EvenDetailDoneButton:
              	DetailViewQuickClose ();   
               DbMemHandleUnlock (&EvenRecH);
               handled = true;
               break;
               
            case EvenDetailCiteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               DetailViewGetSouCList (EvenRec.fields[evenSouCNo]);
               // already made sure SouCList cannot be NULL
               FrmGotoForm (SouCDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;
         
            case EvenDetailNoteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               EvenTypeN = (UInt16) StrAToI (EvenRec.fields[evenType]);
					FrmGotoForm (NoteDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;

            case EvenDetailInfoButton:
               FrmHelp (EvenHelpString);
               handled = true;
               break;
               
            default:
               break;
         	}
         break;
      
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case EvenDetailScrollUpRepeating:
				case EvenDetailScrollDownRepeating:
					ResetScrollRate(); // reset scroll rate.
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

         switch (event->data.ctlRepeat.controlID)
         	{
            case EvenDetailScrollUpRepeating:
					DetailViewScroll (winUp, EvenDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case EvenDetailScrollDownRepeating:
					DetailViewScroll (winDown, EvenDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, EvenDetailDetailGadget);
         break;

     	case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
      
	   case frmOpenEvent:
	   	DetailViewResizeForm (true);
         DrawForm ();

         EvenViewInit ();
         
         // Load TopDetailViewLine, but only after calling EventViewInit. We
         // use TopDetailViewLine when we return from SouCViewForm.
         TopDetailViewLine = PriorTopDetailViewLine;
         DetailViewDraw (EvenRec, Even, EvenDetailDetailGadget,
         	EvenDetailScrollUpRepeating, EvenDetailScrollDownRepeating);
         
         PriorSouCFormID = PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ());
         PriorTopDetailViewLine = 0;  // init back to 0
         
         SetNavFocusRing (EvenDetailDoneButton); // DTR 12-13-2005
            
         handled = true;
         break;

	   case frmUpdateEvent:
	   	DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            DbMemHandleUnlock (&EvenRecH);
            EvenViewInit ();
            }
         DetailViewDraw (EvenRec, Even, EvenDetailDetailGadget,
         	EvenDetailScrollUpRepeating, EvenDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
			 	DetailViewDraw (EvenRec, Even, EvenDetailDetailGadget,
         		EvenDetailScrollUpRepeating, EvenDetailScrollDownRepeating);
      		
      		SetNavFocusRing (EvenDetailDoneButton); // DTR 12-13-2005
      		}
      		
			handled = true;
			break;
			
      case frmCloseEvent:
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&EvenRecH);
         DbMemHandleUnlock (&NoteRecH);
         break;
      
	   default:
		   break;
   	}

   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 		GetRefNumPos
//
// DESCRIPTION:	Finds the nth (counter) occurrence of the key string within
//                the field string, where the key represents a family XRef.
//
//						For example, if the key is "834" and the field is "@545@555@834"
//						then the counter returned is 3.
//
// PARAMETERS:    -> key   	- string to look for in field
//                -> field 	- a string that contains record reference numbers
//						-> delim		- delimiter in the field e.g. FAMI_DLM_CHR
//						<- counter 	- position of key in field
//
// RETURNS: 	   False if key not found in field.  This should never happen.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean GetRefNumPos (Char* key, Char* field, Char delim, UInt16* counter)
{
   UInt16  	i = 0;
   UInt16  	j = 0;
   UInt16   fieldLength;
   UInt16   keyLength;
   
   *counter = 0; // initialize
   
   ErrFatalDisplayIf (key == NULL || *key == '\0', "GetRefNumPos: key missing");
   
   // If there isn't a ref to search for return false (error).
   if (field == NULL)
      return false;
      
   fieldLength = StrLen (field);
   keyLength   = StrLen (key);
   
   while (i < fieldLength) {
      j = 0;
      if (delim == field[i]) {
         (*counter)++;
         i++;

         while (j < keyLength && field[i] == key[j]) {
            if (j == (keyLength-1) && // make key sure exact length of substring
            	(field[i+1] == delim || field[i+1] == chrNull))
               return true;
            i++;
            j++;
            }
         }
      else i++;
     }
      
   ErrFatalDisplay ("GetRefNumPos: key missing in field");
   return false; // key not found in field
}

////////////////////////////////////////////////////////////////////////////////////
//  FUNCTION: 		ParentFinder
//
//  DESCRIPTION:	Finds the parent of the current individual whose data is
//						loaded in the IndiRec.
//
//						NOTE: IndiRec must be loaded prior to calling this function.
//                
//                If successful, the following global variables are set:
//
//						CurrentIndiRecN - set to the father;
//						CurrentSpouRecN - set to the mother;
//						CurrentFamiRecN - set to the current family record number;
//               	FamiRec - loaded with the current family data;
//               	TopVisibleFamilyNum - set to current family num if swapIndi is true.
//
//						If there is no father in the FamiRec, CurrentIndiRecN is 
//						set to the mother and CurrentSpouRecN is set to NO_REC.
//
//  PARAMETERS:	-> swapIndi -	if true, when there is no husband record
//											number in the FamiRec, then the wife's
//											record number is put in CurrentIndiRecN.
//
//  RETURNS: 	   true if at least one parent is found, else false if either no 
//						parents are found or if it cannot load a FamiRec.
//
//  REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean ParentFinder (Boolean swapIndi)
{
 	ErrFatalDisplayIf (!IndiRecH, "ParentFinder: IndiRecH should be locked.");

   // find record location in FamiDB.
	if (!IndiRec.fields[indiFamCNo])
		return false;
		
	CurrentFamiRecN = (UInt32) StrAToI (IndiRec.fields[indiFamCNo]);
   
   // there may be family recorded loaded, so unlock it.
   DbMemHandleUnlock (&FamiRecH);
      
   // Get the family record
   if (DbGetRecord (FamiDB, CurrentFamiRecN, &FamiRec, &FamiRecH) != 0) {
   	ErrFatalDisplay ("ParentFinder: FamiRec not found");
   	return false;
   	}
     
 	// Catch database error where neither husband or wife are in family record.
 	if (!FamiRec.fields[famiHusbNo] && !FamiRec.fields[famiWifeNo]) {
 		DbMemHandleUnlock (&FamiRecH);
    	return false;
   	}
      
	CurrentIndiRecN = CurrentSpouRecN = NO_REC;  // must init here !!!

   // Find husband's record number in IndiDB.
   if (FamiRec.fields[famiHusbNo]) {
	   CurrentIndiRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);
      }
         
   // Find wife's record number in IndiDB.
   if (FamiRec.fields[famiWifeNo]) {
	   CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiWifeNo]);
      }
 
  	// We either have a husband, a wife, or both. If we have neither a husband or
  	// wife then we would have exited this function already above.
   if (CurrentIndiRecN == NO_REC && swapIndi) { // husband was not found
      CurrentIndiRecN = CurrentSpouRecN;
      CurrentSpouRecN = NO_REC;
      }
   
   // Find which family number the person belongs to.
   if (swapIndi) {
     	DBRecordType   rec;  // record in IndiDB
   	MemHandle      recH = NULL;
   	
   	ErrFatalDisplayIf (!IndiRecH, "ParentFinder: CurrentIndiRecN has no value.");
   	DbGetRecord (IndiDB, CurrentIndiRecN, &rec, &recH);

   	GetRefNumPos (IndiRec.fields[indiFamCNo], rec.fields[indiFamSNo],
   		FAMI_DLM, &TopVisFamiNum);
   
   	DbMemHandleUnlock (&recH);
   	}
   	
   return true;
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ChilListDrawRecord
//
// DESCRIPTION:   This routine draws an Child List entry into the 
//                the IndiSummEventListTable.  It is called as a callback
//                routine by the table object.
//
// PARAMETERS:    -> table  - pointer to the address list table
//                -> row    - row number, in the table, of the item to draw
//                -> column - column number, in the table, of the item to draw
//                -> bounds - bounds of the draw region
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ChilListDrawRecord (void* table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   UInt16 	      recN; // record number in IndiDB
   DBRecordType  	rec;
   MemHandle      recH = NULL;
   FontID 	      curFont;

   // Get the record number that corresponds to the table item to draw.
   recN = (UInt16) TblGetRowData (table, row);

   // Check if we are displaying no child data message.
   if (recN == NO_REC) {
   	if (column == 0) {
     		curFont = FntSetFont (boldFont);
     		WinDrawChars (NO_CHIL_STR, NO_CHIL_STR_LEN, bounds->topLeft.x,
     			bounds->topLeft.y+1);
			FntSetFont (curFont);
			}
		return;
      }
   	
   if (column == 0) {
      if (DbGetRecord (IndiDB, recN, &rec, &recH) != 0) {
		   ErrNonFatalDisplay ("ChilListDrawRecord: Record not found");
   		return;
   		}
   		
      if (rec.fields[indiChiFlg]) {
         BitmapPtr	childFlagP;
   		MemHandle  	childFlagH;
         childFlagH = DmGetResource (bitmapRsc, PlusBitmapFamily); // draw plus sign
         childFlagP = MemHandleLock (childFlagH);
         WinDrawBitmap (childFlagP, bounds->topLeft.x, bounds->topLeft.y+3);
         MemHandleUnlock (childFlagH);
         }
   		
      curFont = FntSetFont (PrefFnts[IndiSummFont]);
      // Only draw child's first name and lifespan. Move first name field 
      // to last name field so that DrawRecordNameAndLifspand draws name properly.
      rec.fields[indiLName] = rec.fields[indiFName];
      rec.fields[indiFName] = '\0';
		bounds->topLeft.x += 7;
		
      DrawNameLifespanColor (&rec, bounds, true, true);
      DbMemHandleUnlock (&recH);
		FntSetFont (curFont);
      }

   else { // column == 1
		// Draw a note symbol if the field has a note, source or other info.
		if (TblGetItemInt (table, row, 1) == 1) {
         RGBColorType 	drawColor = RGB_CT_BLUE;
         Char noteStr[] = {symbolNote, '\0'};
         curFont = FntSetFont (symbolFont); // note symbol
         DrawCharsColorI (noteStr, drawColor, bounds->topLeft.x, bounds->topLeft.y);
	      FntSetFont (curFont);
	      }
      }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ChilListScroll
//
// DESCRIPTION:   This routine scrolls the child list in the direction
//                specified.  Scrolls one page at a time.
//
// PARAMETERS:    -> direction -	up or dowm
//
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void ChilListScroll (DirType direction)
{
	TablePtr		table;
	UInt16 		scrollUnits;
	UInt32 		newTopVisibleRecN; // record in ChilDB

	table = GetObjectPtr (IndiSummEvenListTable);

	// Check if any children to scroll
	if (TopVisChilRecN == NO_REC_LONG) // moved from IndiSummHandleEvent 8-3-04
		return;

	MenuEraseStatus (0);	// be sure that the command bar has been closed

	scrollUnits = (UInt16) TblGetLastUsableRow (table);
	
	newTopVisibleRecN = TopVisChilRecN; 

	// Scroll the table down
	if (direction == NavDn || direction == NavR) {
		
		newTopVisibleRecN+= scrollUnits;
		
		if (newTopVisibleRecN + scrollUnits > LastChilRecN) {
			if (LastChilRecN > (FirstChilRecN + scrollUnits))
				newTopVisibleRecN = LastChilRecN - scrollUnits;
			else
				newTopVisibleRecN = FirstChilRecN;
			}
		}
   else { // scroll the table up
		
		if (newTopVisibleRecN > FirstChilRecN + scrollUnits)
      	newTopVisibleRecN -= scrollUnits;
    	else
  			newTopVisibleRecN = FirstChilRecN;
      }

	// avoid redraw if no change
	if (TopVisChilRecN != newTopVisibleRecN) {
		TopVisChilRecN = newTopVisibleRecN;
		ChilListLoadTable ();
		
		//if (TopVisChilRecN != prevTopVisibleRecN)
		TblRedrawTable (table); 
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ChilLoadData
//
// DESCRIPTION:	Gets the positions of the first and last child records in ChilDB
//              	for a given family.
//
//              	IMPORTANT: The FamiRec must be loaded prior to calling this
//              	routine. LastFamiNum must be loaded before calling this routine.
//
//						This routine sets the following variables:
//							
//						FirstChilRecN 	- 	record number in ChilDb of first child in family.
//
//						LastChilRecN 	- 	record number in ChilDb of last child in family.
//
//						TopVisChilRecN - 	record number in ChilDb of top visible child in
//												Child List on Individual Summary Screen
//
// PARAMETERS:  	Nothing.
//
// RETURNED:   	Nothing.
//
// REVISIONS:		None.
 ////////////////////////////////////////////////////////////////////////////////////
static void ChilLoadData (void)
{
   FirstChilRecN = LastChilRecN = TopVisChilRecN = NO_REC_LONG;
   
   // Make sure we have a family and children.
   if (LastFamiNum != 0 && IndiRec.fields[indiChiFlg]) {
		
		ErrFatalDisplayIf (!FamiRecH, "ChildLoadData: FamiRecH not locked");
		
		if (FamiRec.fields[famiChiRec]) {
			
			FirstChilRecN = (UInt32) StrAToI (FamiRec.fields[famiChiRec]);
         TopVisChilRecN = FirstChilRecN;
			LastChilRecN = TopVisChilRecN + StrAToI (FamiRec.fields[famiChiCnt]) - 1;
			
			ErrFatalDisplayIf (StrAToI (FamiRec.fields[famiChiCnt]) == 0,
				"ChildLoadData: invalid child count");
			}
   	}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ChilListLoadTable
//
// DESCRIPTION: 	This routine loads event database records into
//              	the list view form.  The ChilDB is ordered such that all children
//						in the same family are sequentially located.
//
// PARAMETERS:  	Nothing.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ////////////////////////////////////////////////////////////////////////////////////
static void ChilListLoadTable (void)
{
   Int16			  	row = 0;
   Int16         	numRows;
	UInt32		   recN; // record in ChilDB
   DBRecordType  	rec;
   MemHandle      recH = NULL;
   TablePtr 	   table;
   Boolean    		scrollableU;
   Boolean 	   	scrollableD;
 	RectangleType	tblBounds;
  
  	table   = (TablePtr) GetObjectPtr (IndiSummEvenListTable);

 	TblGetBounds (table, &tblBounds); 

	numRows = min ((tblBounds.extent.y / TblGetRowHeight (table, 0)),
		TblGetNumberOfRows (table));

   if (FirstChilRecN != NO_REC_LONG) { // FirstChilRecN set in ChildLoadData   
	   
	   // Reset TopVisChilRecN if we have scrolled too far.
		if (TopVisChilRecN + (numRows - 1) > LastChilRecN)
			TopVisChilRecN = (LastChilRecN - FirstChilRecN > (numRows - 1))
			 	? LastChilRecN - (numRows - 1) : FirstChilRecN;
	   
	   recN = TopVisChilRecN;

	   while (row < numRows) {
         
         TblSetRowUsable (table, row, true);
         TblMarkRowInvalid (table, row);
         TblSetRowSelectable (table, row, true);
     
         DbGetRecord (ChilDB, recN, &rec, &recH);

      	// Set child's record number in IndiDB
      	TblSetRowData (table, row, (UInt32) StrAToI (rec.fields[chilIndiNo]));
      	
      	// Save child number in family. We will need this later if there 
      	// is child-to-family link information.
		   TblSetItemInt (table, row, 0, (Int16) (recN - FirstChilRecN));
   
        	// Hide note symbol if we don't have any detail information.
        	if (rec.fields[chilPedi] || rec.fields[chilNoteNo] ||
        	 	rec.fields[chilSouCNo])
           	TblSetItemInt (table, row, 1, 1); // show symbol & make selectable
        	else
           	TblSetItemInt (table, row, 1, 0); // hide symbol & make unselectable

      	row++;
         recN++;
         DbMemHandleUnlock (&recH);

         if (recN > LastChilRecN)
         	break;
      	}  // end while loop
     
   	} // if FirstChilRecN != NO_REC_LONG
   	
   else { // use first row for No Child Data message
    	TblSetRowUsable (table, row, true);
      TblMarkRowInvalid (table, row);
      TblSetRowSelectable (table, row, false);
      TblSetRowData (table, row, NO_REC);
      row++;
    	}

   // Hide the item that don't have any data.
   while (row < numRows) {
      TblSetRowUsable (table, row, false);
      row++;
      }

   // Update the scroll buttons.
  	if (FirstChilRecN == NO_REC_LONG) {
  		scrollableU = scrollableD = false;
  		}
  	else {
  		scrollableU = (Boolean) (TopVisChilRecN > FirstChilRecN);
  		scrollableD = (Boolean) ((TopVisChilRecN + (numRows - 1)) < LastChilRecN);
   	}
   
   UpdateScrollers (IndiSummScrollUpRepeating, IndiSummScrollDownRepeating,
   	scrollableU, scrollableD);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ChilListInit
//
// DESCRIPTION:   This routine initializes the Child List on the 
//                Individual Summary form.
//
// PARAMETERS:    None.
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ChilListInit (void)
{
   Int16 		row, col;
   Int16			numRows;
   TablePtr    table;
   FontID		curFont = FntSetFont (IndiSummFont);

	table = (TablePtr) GetObjectPtr (IndiSummEvenListTable);

   TblUnhighlightSelection (table); // this must be done here
  
   numRows = TblGetNumberOfRows (table);
  
   for (row = 0; row < numRows; row++) {      
      TblSetItemStyle (table, row, 0, customTableItem);
      TblSetItemStyle (table, row, 1, customTableItem);
      TblSetRowHeight (table, row, FntLineHeight ());
      TblSetRowUsable (table, row, false);
      }

   // Reset column widths
   TblSetColumnWidth (table, 0, 152);
   
   for (col = 0; col < 2; col++) {
		TblSetColumnUsable (table, col, true);
		TblSetCustomDrawProcedure (table, col, ChilListDrawRecord);
		}

	FntSetFont (curFont);

   ChilListLoadTable (); // load the child records
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ChilViewInit
//
// DESCRIPTION: 	This routine initializes the Event View Window of the 
//              	GedWise application.  Most importantly it lays out the
//              	record and decides how the record is drawn.
//              	It is assumed that ChilRec is initialized before arriving
//              	at this function.
//
// PARAMETERS:		None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ChilViewInit (void)
{
   UInt16  			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   FontID 			curFont;

   DetailViewInit (); // initialize gadget array
  
   ErrFatalDisplayIf (FirstChilRecN + ChilNumInFam > LastChilRecN,
   	"ChildViewInit: Bad ChilNumInFam");
  
	ErrFatalDisplayIf (ChilRecH, "ChilViewInit: handle should not be locked.");
   
   if (DbGetRecord (ChilDB, FirstChilRecN + ChilNumInFam, &ChilRec,
   	&ChilRecH) != 0) {
   	ErrDisplay ("ChildViewInit: Record Not Found");
	
      DVLines[DetailViewLastLine].fieldNum = detailViewError;
   	DVLines[DetailViewLastLine].offset = 0;
   	DetailViewLastLine++;
   	return;
   	}

   // Check child record to see if there is a source citation.
   if (ChilRec.fields[chilSouCNo] != NULL) // DTR: Jan 27 2003
   	ShowObject (ChilDetailCiteButton, true);

   // Now load gadget array.
   FrmGetFormBounds (FrmGetActiveForm (), &rect);
   maxWidth = rect.extent.x - MARGIN_WID*2; // subtract 2 to allow room from edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);
 
    if (ChilRec.fields[chilPedi]) {
      DetailViewPositionTextAt (&width);
      DetailViewAddField (ChilRec.fields[chilPedi], chilPedi, &width,
         maxWidth, SUB_HEAD_WID);
      }
  
  	// Check record to see if there is a Note
	DetailViewAddNote (ChilRec.fields[chilNoteNo], ChilDetailNoteButton,
		chilNoteNo,	&width, maxWidth, "Child");

   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ChilViewHandleEvent
//
// DESCRIPTION:	This routine is the event handler for the Event Viewer
//              	of the Event List Window.
//              	It is assumed the EvenRec is filled before reaching
//              	here.
//                
// PARAMETERS:  	-> event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ChilViewHandleEvent (EventPtr event)
{
   Boolean handled = false;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
         	case ChilDetailCloseButton:
            case ChilDetailDoneButton:
               DetailViewQuickClose ();
               DbMemHandleUnlock (&ChilRecH); // locked in SeekChildRecordInFamily
               handled = true;
               break;
               
            case ChilDetailCiteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               DetailViewGetSouCList (ChilRec.fields[chilSouCNo]);
               // already made sure SouCList cannot be NULL
               FrmGotoForm (SouCDetailForm);
               UpdateFrm = false;
               handled = true;
               break;
         
            case ChilDetailNoteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               FrmGotoForm (NoteDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;
               
            case ChilDetailInfoButton:
               FrmHelp (ChilHelpString);
               handled = true;
               break;
   
            default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, ChilDetailDetailGadget);
         break;
         
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case ChilDetailScrollUpRepeating:
				case ChilDetailScrollDownRepeating:
					ResetScrollRate(); // reset scroll rate.
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

         switch (event->data.ctlRepeat.controlID)
         	{
            case ChilDetailScrollUpRepeating:
					DetailViewScroll(winUp, ChilDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case ChilDetailScrollDownRepeating:
					DetailViewScroll(winDown, ChilDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
     	case menuEvent:
			return MenuDoCommand (event->data.menu.itemID);
    
	  	case frmOpenEvent:
  			DetailViewResizeForm (true);
	   	DrawForm ();
         ChilViewInit ();
         
         // Load TopDetailViewLine, but only after calling ChilViewInit. We
         // use TopDetailViewLine when we return from SouCViewForm.
         TopDetailViewLine = PriorTopDetailViewLine;
         DetailViewDraw (ChilRec, Chil, ChilDetailDetailGadget,
         	ChilDetailScrollUpRepeating, ChilDetailScrollDownRepeating);

         PriorSouCFormID = PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ());
         PriorTopDetailViewLine = 0; // reset to 0
         
         SetNavFocusRing (ChilDetailDoneButton); // DTR 12-13-2005
         
         handled = true;
         break;
      
	   case frmUpdateEvent:
	   	DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
	         DbMemHandleUnlock (&ChilRecH); 
           	ChilViewInit ();
            }
            
         DetailViewDraw (ChilRec, Chil, ChilDetailDetailGadget,
         	ChilDetailScrollUpRepeating, ChilDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
			  	DetailViewDraw (ChilRec, Chil, ChilDetailDetailGadget,
         		ChilDetailScrollUpRepeating, ChilDetailScrollDownRepeating);
         		
         	SetNavFocusRing (ChilDetailDoneButton); // DTR 12-13-2005	
      		}
			handled = true;
			break;

      case frmCloseEvent:
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&ChilRecH); // locked in SeekChildRecordInFamily
         DbMemHandleUnlock (&NoteRecH);
         break;
	
	   default:
		   break;
   	}

   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FamiMarrDrawRecord
//
// DESCRIPTION:   This routine draws a marriage event record.  It is called as
//                a callback routine by the table object.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void FamiMarrDrawRecord (void)
{
   DBRecordType  	rec;
   MemHandle     	recH = NULL;
   FontID 	     	curFont;
   RectangleType	rect;
   Boolean			ctlEnabled;

   GetObjectBounds (IndiSummMarrListButton, &rect);
	WinEraseRectangle (&rect, 0);

	ctlEnabled = (Boolean) (CurrentMarrRecN != NO_REC_LONG);
   
   CtlSetEnabled (GetObjectPtr (IndiSummMarrListButton), ctlEnabled);
   
   if (!ctlEnabled)
   	return;
   
   if (DbGetRecord (EvenDB, CurrentMarrRecN, &rec, &recH) != 0) {
	   ErrNonFatalDisplay ("MarriageDrawRecord: Record not found");
   	return;
   	}

   curFont = FntSetFont (PrefFnts[IndiSummFont]);
   EvenDrawRecord (&rec, &rect);
	FntSetFont (curFont);
  
   DbMemHandleUnlock (&recH);

}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FamiSpouDrawRecord
//
// DESCRIPTION:   This routine draws a spouse name on the Individual
//						Summary Screen.  It is called as a callback routine
// 					by the table object.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing
//	
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void FamiSpouDrawRecord (void)
{
   DBRecordType  	rec;
   MemHandle      recH = NULL;
   FontID 	      curFont;
	RectangleType	rect;
	Boolean			ctlEnabled;

   GetObjectBounds (IndiSummSpouListButton, &rect);
	WinEraseRectangle (&rect, 0);

	ctlEnabled =  (Boolean) (LastFamiNum > 0);

	CtlSetEnabled (GetObjectPtr (IndiSummSpouListButton), ctlEnabled);

   if (!ctlEnabled)
   	return;

   curFont = FntSetFont (PrefFnts[IndiSummFont]);
   
   // Initialize
   rec.fields[indiFName] = NULL;
   rec.fields[indiLName] = NULL;
   rec.fields[indiTitle] = NULL;
   rec.fields[indiLspan] = NULL;

   if (CurrentSpouRecN != NO_REC) { // spouse not missing from family record.
      if (DbGetRecord (IndiDB, CurrentSpouRecN, &rec, &recH) != 0) {
	      ErrNonFatalDisplay ("FamiSpouDrawRecord: rec not found");
   	   return;
   	   }
   	}
   DrawRecordNameAndLifespan (&rec, &rect, true, false);

   if (CurrentSpouRecN != NO_REC)
	   DbMemHandleUnlock (&recH);
   
   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   	FamiNumberDraw
//
// DESCRIPTION: 	Draws information about the individual's number of families
//              	as well as the number of the current family being viewed.
//
//              	IMPORTANT: The TotalFamilyRecords, CurrentMarrNumber and
//						FirstChilRecN variables must be loaded prior to calling this
//						function.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing
//
// REVISIONS:		2003-01-11	-	DTR: added code for child count
////////////////////////////////////////////////////////////////////////////////////
static void FamiNumberDraw (Boolean updateScrollers)
{
   FontID 				curFont;
   RectangleType 		rect;
  	Char              string[25];
   Boolean				haveData = false;
   UInt16				x;
   Boolean 				scrollableL;
   Boolean 				scrollableR;
   
  	if ( FamiRecH && (FamiRec.fields[famiNChi] || FamiRec.fields[famiNoteNo]
  		||	FamiRec.fields[famiSouCNo]) )
 		haveData = true;
 	 			
	CtlSetUsable (GetObjectPtr (IndiSummFamiNoButton), haveData);
   
   GetObjectBounds (IndiSummFamiNoButton, &rect);
	WinEraseRectangle (&rect, 0);
   curFont = FntSetFont (TITLE_FONT);
 	
   if (LastFamiNum > 0) {
      StrPrintF (string, "Fam: %i of %i", TopVisFamiNum, LastFamiNum);
  		WinDrawChars (string, StrLen (string), 0, rect.topLeft.y);
 		x = FntCharsWidth (string, StrLen (string)) + 3;
 		if (haveData) {
		   RGBColorType 	drawColor = RGB_CT_BLUE;
 			Char noteStr[] = {symbolNote, '\0'};
   		FntSetFont (symbolFont); // note symbol
      	DrawCharsColorI (noteStr, drawColor, x, rect.topLeft.y);
	    	}
      }
   else
      WinDrawChars (NO_FAMI_STR, NO_FAMI_STR_LEN, 0, rect.topLeft.y);

   FntSetFont (PrefFnts[IndiSummFont]);
	
	// Show the child count
	if (FirstChilRecN != NO_REC_LONG) {
		UInt16	chilCnt = (LastChilRecN - FirstChilRecN) + 1;
		StrPrintF (string, "%i chil.", chilCnt);
		x = rect.extent.x - FntCharsWidth (string, StrLen (string));
		WinDrawChars (string, StrLen (string), x, rect.topLeft.y);
  			}

   FntSetFont (curFont);
 
 	if (updateScrollers) {  
	   // update the scroll buttons
   	scrollableL  = (Boolean) (TopVisFamiNum > 1);
		scrollableR  = (Boolean) (TopVisFamiNum < LastFamiNum);
  
   	UpdateLeftRightScrollers (IndiSummScrollLeftButton, IndiSummScrollRightButton,
   		scrollableL, scrollableR);
   	}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FamiListScroll
//
// DESCRIPTION:   This routine scrolls the Family number in the direction
//						specified.  Each family is represented by the spouse's name, 
//						or "Unknown" if spouse's name is not provided.
//
//  					IMPORTANT: This function loads the FamiRec, CurrentSpouRecN
//						and CurrentFamiRecN.
//						The IndiRec and LastFamiNum variables must be loaded prior
//						to calling this function.
//
// PARAMETERS:    -> direction -	up or dowm
//
// RETURNED:      True if list was scrolled, else false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean FamiListScroll (DirType direction)
{
	UInt16	newTopVisibleNumber;
   Char     famiXRef[NREF_LEN+1];  // record number as string

	ErrFatalDisplayIf (!IndiRecH, "FamilyListScroll: IndiRecH not locked");

	// Before scrolling, be sure that the command bar has been closed.
	MenuEraseStatus (0);
	
	newTopVisibleNumber = TopVisFamiNum;

	// Scroll to next family record.
	if (direction == NavDn || direction == NavR) {
      if (TopVisFamiNum < LastFamiNum)
         newTopVisibleNumber++;
      }
	else { // Scroll to previous family record.
      if (TopVisFamiNum > 1)
         newTopVisibleNumber--;
		}

	// Avoid redraw if no change
	if (TopVisFamiNum != newTopVisibleNumber) {
		TopVisFamiNum = newTopVisibleNumber;

      // Get the next family record in inidividuals family list string and      
      // load the Family Record, Spouse Record Number & Marriage Record Number. 
      if (RefFinderStr (TopVisFamiNum, FAMI_DLM, IndiRec.fields[indiFamSNo],
       	famiXRef)) {
    
         // There must be previous marriage record else we would not have reached 
         // here as there would be no reason to scroll.
         DbMemHandleUnlock (&FamiRecH); 
            
         // get the marriage record, if any.
         CurrentFamiRecN = (UInt32) StrAToI (famiXRef);
   		if (DbGetRecord (FamiDB, CurrentFamiRecN, &FamiRec, &FamiRecH) != 0) {
   			ErrNonFatalDisplay ("FamiListScroll: Record not found");
   			return false;
   			}
         }

		ErrFatalDisplayIf (!FamiRecH, "FamilyListScroll: FamiRecH not locked");

      // Get the spouse, if any.  Check first whether current individual is the
      // husband or wife in the current family.
  		CurrentSpouRecN = NO_REC; // initialize
      CurrentMarrRecN = NO_REC_LONG;
      
      if (FamiRec.fields[famiHusbNo]) {
      
	      UInt16 husbRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);
      
      	if (CurrentIndiRecN == husbRecN) {
     			if (FamiRec.fields[famiWifeNo])
         		CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiWifeNo]);
         	}
      	else
      		CurrentSpouRecN = husbRecN;
        	}
      
      // there could still be marriage record even if no spouse provided.
   	if (FamiRec.fields[famiEvenMNo])
   		CurrentMarrRecN = (UInt32) StrAToI (FamiRec.fields[famiEvenMNo]);
   
      FamiSpouDrawRecord ();
      FamiMarrDrawRecord ();
      return true;
		}

	return false; // list was not scrolled
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	FamiViewInit
//
// DESCRIPTION: 	This routine initializes the Family View Window of the 
//              	GedWise application.  It lays out the record and decides
//              	how the record is drawn.
//              	FamiRec must be initialized before calling this function.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void FamiViewInit (void)
{
   UInt16 			width = MARGIN_WID;
   RectangleType 	rect;
   UInt16 			maxWidth;
   FontID 			curFont;

	ErrFatalDisplayIf (!FamiRecH, "FamiViewInit: FamiRecH not locked");

   DetailViewInit (); // initialize gadget array

   // Check family record to see if there is a source.
   if (FamiRec.fields[famiSouCNo] != NULL) // DTR: 27 Jan 2003
		ShowObject (FamiDetailCiteButton, true);

   // Now load gadget array.
   FrmGetFormBounds (FrmGetActiveForm(), &rect);
   maxWidth = rect.extent.x - MARGIN_WID * 2; // subtract to allow room from edges
   curFont = FntSetFont (PrefFnts[DetailViewFont]);
 
   if (FamiRec.fields[famiNChi]) {
      DetailViewPositionTextAt (&width);
      DetailViewAddField (FamiRec.fields[famiNChi], famiNChi, &width,
         maxWidth, SUB_HEAD_WID);
      }
      
   // Check record to see if there is a Note
	DetailViewAddNote (FamiRec.fields[famiNoteNo], FamiDetailNoteButton,
		famiNoteNo, &width, maxWidth, "Family");
   FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FamiViewHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Family Viewer.
//                FamiRec must be filled before calling this routine.
//                
// PARAMETERS:    -> event  - a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean FamiViewHandleEvent (EventPtr event)
{
   Boolean handled = false;
   
   switch (event->eType)
		{
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
         	case FamiDetailCloseButton:
            case FamiDetailDoneButton:
            	DetailViewQuickClose ();
               handled = true;
               break;
               
            case FamiDetailCiteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               DetailViewGetSouCList (FamiRec.fields[famiSouCNo]);
               // already made sure SouCList cannot be NULL
               FrmGotoForm (SouCDetailForm);
        			UpdateFrm = false;
               handled = true;
               break;
         
            case FamiDetailNoteButton:
               PriorTopDetailViewLine = TopDetailViewLine;
               FrmGotoForm (NoteDetailForm);
       			UpdateFrm = false;
               handled = true;
               break;

            case FamiDetailInfoButton:
               FrmHelp (FamiHelpString);
               handled = true;
               break;

            default:
               break;
         	}
         break;

      case keyDownEvent:
         handled = DetailViewHandleVirtual (event, FamiDetailDetailGadget);
         break;
      
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case FamiDetailScrollUpRepeating:
				case FamiDetailScrollDownRepeating:
					ResetScrollRate(); // reset scroll rate
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:

         switch (event->data.ctlRepeat.controlID)
         	{
            case FamiDetailScrollUpRepeating:
					DetailViewScroll (winUp, FamiDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            case FamiDetailScrollDownRepeating:
					DetailViewScroll (winDown, FamiDetailDetailGadget);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
      
     	case menuEvent:
     	   return MenuDoCommand (event->data.menu.itemID);
      
  	   case frmOpenEvent:
	   	DetailViewResizeForm (true);
         FamiViewInit ();
         DrawForm ();
         
         // Load TopDetailViewLine, but only after calling ChilViewInit. We
         // use TopDetailViewLine when we return from SouCDetailForm.
         TopDetailViewLine = PriorTopDetailViewLine;
         DetailViewDraw (FamiRec, Fami, FamiDetailDetailGadget,
         	FamiDetailScrollUpRepeating, FamiDetailScrollDownRepeating);
         
         PriorSouCFormID = PriorNoteFormID = FrmGetFormId (FrmGetActiveForm ());
         PriorTopDetailViewLine = 0; // reset to 0
         
         SetNavFocusRing (FamiDetailDoneButton); // DTR 12-13-2005
         
         handled = true;
         break;
      
	   case frmUpdateEvent:
			DrawForm ();
         if (event->data.frmUpdate.updateCode == updateViewReInit) {
            MemHandleFree (DVLinesH);
            DVLines = NULL;
            FamiViewInit ();
            }
            
         DetailViewDraw (FamiRec, Fami, FamiDetailDetailGadget,
          	FamiDetailScrollUpRepeating, FamiDetailScrollDownRepeating);
   	   handled = true;
         break;

		case winDisplayChangedEvent:
      	if (DetailViewResizeForm (false)) {
				DrawForm ();
	         DetailViewDraw (FamiRec, Fami, FamiDetailDetailGadget,
   	       	FamiDetailScrollUpRepeating, FamiDetailScrollDownRepeating);
   	       	
   	      SetNavFocusRing (FamiDetailDoneButton); // DTR 12-13-2005 	
      		}
      	handled = true;
			break;

      case frmCloseEvent:
         MemHandleFree (DVLinesH);
         DVLines = NULL;
         DbMemHandleUnlock (&NoteRecH);
         break;
	
	   default:
		   break;
   	}

   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      NoteViewResizeForm
//
// DESCRIPTION:   This routine resizes and draws the Note View Form.
//
// PARAMETERS:   	newForm - true if drawing form for first time. 
//
// RETURNED:      True if form was updated, else false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean NoteViewResizeForm (Boolean newForm)
{
	Int16 			extentDelta;
	Int16				ctlDelta;
	UInt16 			numObjects, i;
	Coord 			x, y;
	RectangleType 	objBounds;  // bounds of object being set
	RectangleType 	clipBounds; // clipping bounds (needed for OS 3.3)
	RectangleType 	curBounds;  // bounds of active form
	RectangleType	disBounds; // bounds of screen
	RectangleType	smBounds = DV_FRM_BOUNDS;
	FormPtr			frm;
	FieldPtr			fld;
	UInt16			disSz;
	UInt16			curSz;
		
	frm  = FrmGetActiveForm ();
	fld  = (FieldPtr) GetObjectPtr (NoteDetailDetailField);

	// Get dimensions of current active form.
	WinGetWindowFrameRect (WinGetDrawWindow(), &curBounds);
	
	// Get dimensions of display window.
	WinGetWindowFrameRect (WinGetDisplayWindow(), &disBounds);

	RctCopyRectangle (&disBounds, &clipBounds); // for Palm III devices
	
	// Determine the amount of the change.
	if (!UseFullNoteScrn) {
		
		disSz = disBounds.extent.y + disBounds.topLeft.y;
		curSz = curBounds.extent.y + curBounds.topLeft.y;
		ctlDelta = 0;

		if (disSz == curSz) { // then screen size has not changed
			
			extentDelta = 	smBounds.extent.y - curBounds.extent.y;
			ctlDelta = extentDelta;
			smBounds.topLeft.y = disSz - smBounds.extent.y;
			}
			
		else {  // display size has changed
			extentDelta = disSz - curSz;

		 	if (disSz > curSz) // then move window down
				smBounds.topLeft.y = disSz - curBounds.extent.y;		 		
				//smBounds.topLeft.y += extentDelta; // DTR: 12-22-2005
			}
		
		RctCopyRectangle (&smBounds, &disBounds);
		}	
	
	else { // UseFullNoteScrn
		extentDelta = 	disBounds.extent.y - curBounds.extent.y;
		ctlDelta = extentDelta;
		}
	
	if (extentDelta == 0) // do not redraw if no changes
		goto ExitFunc;
		
	RctInsetRectangle (&disBounds, 2); // adjust for form border
		
	// Redraw the base form.
	if (!UseFullNoteScrn && (disSz >= curSz) && !newForm) {
		FrmSetActiveForm (FrmGetFormPtr (IndiSummForm));
		WinGetWindowFrameRect (WinGetDisplayWindow(), &curBounds);
		WinEraseRectangle (&curBounds, 0);
		IndiSummResizeForm ();
  		IndiSummDrawData (true);
		FrmSetActiveForm (frm);
		}
	
	WinSetBounds (FrmGetWindowHandle (frm), &disBounds); // set new form bounds

	WinSetDrawWindow (FrmGetWindowHandle (frm)); // do this instead of DrawFrm.
	
	// Reset clip area for Palm III devices
	if (Pre35Rom)
		WinSetClip (&clipBounds);

	// Iterate through objects and re-position them.
	numObjects = FrmGetNumberOfObjects (frm);

	for (i = 0; i < numObjects; i++) {
		
		switch (FrmGetObjectType (frm, i))
			{
			case frmControlObj:
			case frmBitmapObj:
				FrmGetObjectPosition (frm, i, &x, &y);
				FrmSetObjectPosition (frm, i, x, y + ctlDelta);
				break;
			
			case frmScrollBarObj:
			case frmFieldObj:
				FrmGetObjectBounds (frm, i, &objBounds);
				FldSetInsertionPoint (fld, 0);// must do this!!!
				objBounds.extent.y += ctlDelta;
				FrmSetObjectBounds (frm, i, &objBounds);
				break;
			} // switch
		}
	
	//FldRecalculateField (fld, true);
	FldSetScrollPosition (fld, FldGetScrollPosition (fld)); // correct over-scroll

	////////
	ExitFunc:
	////////

	ShowObject (NoteDetailScrnSzSmBitMap, !UseFullNoteScrn);
	ShowObject (NoteDetailScrnSzLgBitMap, UseFullNoteScrn);
	
	return (Boolean) (extentDelta != 0);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	NoteViewHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Note Detail Window.
//
//						NOTE: we don't need to call AdjustScrollRate or ResetScrollRate
//						functions, as ScrollUnits is not used in NoteViewPageScroll
// 					function.
//
// PARAMETERS:  	-> event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handled and should not be passed
//              	to a higher level handler.
//
// REVISIONS:	 	None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean NoteViewHandleEvent (EventType* event)
{
   FieldPtr fld;
   Boolean  handled = false;

   fld = (FieldPtr) GetObjectPtr (NoteDetailDetailField);
   
   switch (event->eType)
   	{
      case keyDownEvent:
      
  	   	handled = DetailViewHandleVirtual (event, NoteDetailDetailField); // DTR: 12-13-2005
      
       	/*if (EvtKeydownIsVirtual (event)) {
				if (event->data.keyDown.chr == vchrPageUp) {
					ScrollPage (winUp, NoteDetailDetailField, NoteDetailScrollScrollBar);
					handled = true;
					} 
				else if (event->data.keyDown.chr == vchrPageDown)  {
					ScrollPage (winDown, NoteDetailDetailField, NoteDetailScrollScrollBar);
					handled = true;
					}
				} */
         break;
         
      case ctlSelectEvent:
        
        switch (event->data.ctlSelect.controlID)
         	{
          	case NoteDetailDoneButton:
 					if (UseFullNoteScrn) {
 				 		RedrawBaseFrm = true; // redraw all if in full scrn
 						}
            	else
            		UpdateFrm = false;
            	FrmGotoForm (PriorNoteFormID);
      			handled = true;
            	break;
            
     	      case NoteDetailTopButton:
         		FldSetScrollPosition (fld, 0);
         		UpdateScrollBar (NoteDetailDetailField, NoteDetailScrollScrollBar);
         		SetNavFocusRing (NoteDetailDoneButton); // DTR 12-13-2005
         		handled = true;
         		break;

      		case NoteDetailBottomButton:
         		FldSetScrollPosition (fld, FldGetTextLength (fld));
         		UpdateScrollBar (NoteDetailDetailField, NoteDetailScrollScrollBar);
					SetNavFocusRing (NoteDetailDoneButton); // DTR 12-13-2005
         		handled = true;
         		break;              

				case NoteDetailScrnSzButton:
					UseFullNoteScrn = !UseFullNoteScrn;
					NoteViewResizeForm (false);
					UpdateScrollBar (NoteDetailDetailField, NoteDetailScrollScrollBar);
					DrawForm ();
					handled = true;
            	break;
				
     			default:
     				break;       
            }
         
         break;

      case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
      
      case sclRepeatEvent:
         ScrollField (event->data.sclRepeat.newValue - event->data.sclRepeat.value,
          	false, NoteDetailDetailField, NoteDetailScrollScrollBar);
         // don't set handled to true
         break;

      case frmOpenEvent:
   		WinSetDrawWindow (FrmGetWindowHandle (FrmGetActiveForm()));
   		
   		UseFullNoteScrn = Prefs[FullNoteScreen]; // set full/small screen mode
   		
   		NoteViewResizeForm (true); // resize the form
   		
			 // Get the Note record.
			NoteRecFinder (NoteXRef, &NoteRec, &NoteRecH);
		
			// Set a pointer to the Note field and redraw it.
			FldSetTextPtr (fld, NoteRec.fields[noteText]);
			FldRecalculateField (fld, true);
	
			UpdateScrollBar (NoteDetailDetailField, NoteDetailScrollScrollBar);
			DrawForm ();
			
			SetNavFocusRing (NoteDetailDoneButton); // DTR 12-13-2005
						
         handled = true;
         break;
			
      case frmUpdateEvent:
      	DrawForm ();
      	handled = true;
			break;

		case winDisplayChangedEvent:
      	if (NoteViewResizeForm (false)) {
      		UpdateScrollBar (NoteDetailDetailField, NoteDetailScrollScrollBar);
				DrawForm ();
        		}
      		
      	SetNavFocusRing (NoteDetailDoneButton); // DTR 12-13-2005	
      		
			handled = true;
			break;

      case frmCloseEvent:
         DbMemHandleUnlock (&NoteRecH);
         // don't set handled to true.
         break;

      default:
      	break;
   	}

   return (handled);
}

#pragma mark -
#pragma warn_a5_access on
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DetermineName
//
// DESCRIPTION:   Determines an Individual record's name parameters.
//						If surnameFirst is true, name1 is the surname and name2 is the
//						givenname. Else,if surnameFirst is false, name1 is the givenname.
//
// PARAMETERS:    <> name1, name2 	- names to draw
//                <> name1Length, name2Length - length of the names in chars
//                <> name1Width, name2Width 	 - width of the names when drawn
//                -> nameExtent 	- 	the space the names must be drawn in
//						-> surnameFirst- 	true if drawing the lastname before the
//											 	Givenname. Else false if drawing givenname
//												first.
// RETURNED:		Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
extern void DetermineName (DBRecordType* recordP, Int16* fieldSeparatorWidth,
   Char** name1, Int16* name1Length, Int16* name1Width, 
   Char** name2, Int16* name2Length, Int16* name2Width,
   Int16 nameExtent, Boolean surnameFirst)
{
    Boolean	ignored;

   if (surnameFirst) {
   	*fieldSeparatorWidth = FntCharsWidth (FLD_SEP_STR, FLD_SEP_LEN);
    	*name1 = recordP->fields[indiLName];
   	*name2 = recordP->fields[indiFName];
   	}
   else {
	   *fieldSeparatorWidth = FntCharsWidth (" ", 1);
    	*name1 = recordP->fields[indiFName];
   	*name2 = recordP->fields[indiLName];
   	}

   if (*name1) {
      *name1Length = *name1Width = nameExtent; // longer & wider than possible
      FntCharsInWidth (*name1, name1Width, name1Length, &ignored);
      }
   else { // set surname to "unknown" if blank
    	if (surnameFirst) { // name1 will be surname if surnameFirst
      	*name1 = UNK_STR; // unknown string is assumed to not need clipping
      	*name1Length = StrLen (*name1);
      	*name1Width = FntCharsWidth (*name1, *name1Length);
      	}
      else {
     		*name1Length = 0;
      	*name1Width = 0;
      	}
      }
      
   if (*name2) {
      *name2Length = *name2Width = nameExtent; // longer & wider than possible
      FntCharsInWidth (*name2, name2Width, name2Length, &ignored);
      }
   else { // set surname to "unknown" if blank
  		if (!surnameFirst) { // name2 will be surname if surnameFirst
      	*name2 = UNK_STR; // unknown string is assumed to not need clipping
      	*name2Length = StrLen (*name2);
      	*name2Width = FntCharsWidth (*name2, *name2Length);
      	}
		else {
      	*name2Length = 0;
      	*name2Width = 0;
      	}
		}
		
	if (!*name1 || !*name2) // only need seperator if we have both names
		*fieldSeparatorWidth = 0;

	return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DrawRecordName
//
// DESCRIPTION:   Draws an individual record name.  It is used only in the 
//                DrawRecordNameAndLifespan function.
//						Includes a variable ratio for	name1/name2 width allocation,
// 					a prioritization	parameter, and a word break search to allow
//						reclaiming of space from the low priority name.
//
// PARAMETERS:    name1 (surname), name2 (given name) - names to draw
//                name1Length, name2Length - length of the names in chars
//                name1Width, name2Width - width of the names when drawn
//                nameExtent - the space the names must be drawn in
//                *x, y - where the names are drawn
    //
// RETURNED:      x is set after the last char drawn
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
extern void DrawRecordName (
	Char* name1, Int16 name1Length, Int16 name1Width, 
	Char* name2, Int16 name2Length, Int16 name2Width,
	Int16 nameExtent, Int16* x, Int16 y, Int16 fieldSeparatorWidth,
	Boolean priorityIsName1, Boolean surnameFirst)
{
	Int16		name1MaxWidth;
	Int16		name2MaxWidth;
	Boolean	ignored;
	Char *	lowPriName;
	Int16		highPriNameWidth;
	Int16		lowPriNameWidth;
	Int16 	shortenedFieldWidth;

	shortenedFieldWidth = (FntCharWidth (periodChr) * SHORTENED_FLD_LEN);

	// Remove name separator width
	nameExtent -= fieldSeparatorWidth;
	
	// Test if both names fit 
	if ((name1Width + fieldSeparatorWidth + name2Width) <= nameExtent) {
		name1MaxWidth = name1Width;
		name2MaxWidth = name2Width;
		}
	else {
		// They dont fit. One or both needs truncation.
		// Establish name priorities and their allowed widths
		// Change this to alter the ratio of the low and high priority name spaces
		Int16	highPriMaxWidth = (nameExtent * 2) / 3;	// 1/3 to low and 2/3 to high
		Int16	lowPriMaxWidth = nameExtent - highPriMaxWidth;
		
		// Save working copies of names and widths based on priority
		if (priorityIsName1) { // Priority is name1
			highPriNameWidth = name1Width;
			lowPriName = name2;
			lowPriNameWidth = name2Width;
			}
		else { // Priority is name2
			highPriNameWidth = name2Width;
			lowPriName = name1;
			lowPriNameWidth = name1Width;
			}

		// Does high priority name fit in high priority max width?
		if (highPriNameWidth > highPriMaxWidth) {
			// Does not fit. Look for word break in low priority name
			if (lowPriName) {
				Char* 	spaceP = StrChr (lowPriName, spaceChr);
				if (spaceP != NULL) {
					// Found break. Set low priority name width to break width
					lowPriNameWidth = FntCharsWidth (lowPriName, spaceP - lowPriName);
					}
				}
			// Reclaim width from low pri name width to low pri max width, if smaller
			if (lowPriNameWidth < lowPriMaxWidth) { // DTR 5/10/02
				lowPriMaxWidth = lowPriNameWidth;
				// Set new high pri max width
				highPriMaxWidth = nameExtent - lowPriMaxWidth;
				}
			}
		else {
			// Does fit. Adjust maximum widths
			highPriMaxWidth = highPriNameWidth;
			lowPriMaxWidth = nameExtent - highPriMaxWidth;
			}
		
		// Convert priority widths back to name widths
		if (priorityIsName1) { // Priority is name1
			name1Width = highPriNameWidth;
			name2Width = lowPriNameWidth;
			name1MaxWidth = highPriMaxWidth;
			name2MaxWidth = lowPriMaxWidth;
			}
		else { // Priority is name2
			name1Width = lowPriNameWidth;
			name2Width = highPriNameWidth;
			name1MaxWidth = lowPriMaxWidth;
			name2MaxWidth = highPriMaxWidth;
			}
		}

	if (name1) {
		// Does name1 fit in its maximum width?
		if (name1Width > name1MaxWidth) {
			// No. Draw it to max width minus the ellipsis
			name1Width = name1MaxWidth - shortenedFieldWidth;
			FntCharsInWidth (name1, &name1Width, &name1Length, &ignored);
   		WinDrawChars (name1, name1Length, *x, y);
			*x += name1Width;
		
			// Draw ellipsis
			WinDrawChars (SHORTENED_FLD_STR, SHORTENED_FLD_LEN, *x, y);
			*x += shortenedFieldWidth;
			}
		else { // Yes. Draw name1 within its width
			FntCharsInWidth (name1, &name1Width, &name1Length, &ignored);
			WinDrawChars (name1, name1Length, *x, y);
			*x += name1Width;
			}
		}
	
	if (name2) {
		// Draw name separator, but only if there was a name1 drawn
		if (fieldSeparatorWidth) {
			if (surnameFirst)
				WinDrawChars (FLD_SEP_STR, FLD_SEP_LEN, *x, y);
			else
				WinDrawChars (" ", 1, *x, y);				
			*x += fieldSeparatorWidth;
			}
	
		// Does name2 fit in its maximum width?
		if (name2Width > name2MaxWidth && !surnameFirst) {
			// No. Draw it to max width minus the ellipsis
			name2Width = name2MaxWidth - shortenedFieldWidth;
			FntCharsInWidth (name2, &name2Width, &name2Length, &ignored);
  		 	WinDrawChars (name2, name2Length, *x, y);
			*x += name2Width;
			
			// Draw ellipsis
			WinDrawChars (SHORTENED_FLD_STR, SHORTENED_FLD_LEN, *x, y);
			*x += shortenedFieldWidth;
			}
		else { // Yes Draw name2 within its maximum width
			FntCharsInWidth (name2, &name2MaxWidth, &name2Length, &ignored);
			WinDrawChars (name2, name2Length, *x, y);
			*x += name2MaxWidth;
			}
		}
}
#pragma warn_a5_access off
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DrawRecordNameAndLifespan
//
// DESCRIPTION:   Draws the individual's name and lifespan information
// 				   within the screen bounds passed.
//
// PARAMETERS:    -> record - record to draw
//                -> bounds - bounds of the draw region
//                -> drawLifeSpan - if true, the person's lifespan data is drawn
//						-> surnameFirst - if true, draw surname before givenname
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DrawRecordNameAndLifespan (DBRecordType* record, RectanglePtr bounds,
 			Boolean drawLifeSpan, Boolean surnameFirst)
{
   Int16 	x, y;
   Int16 	fieldSeparatorWidth;
   Char*		name1; // first name to draw
   Char*		name2; // second name to draw
   Char*		cPos = NULL;
   Char*		sPos = NULL;
   Char		lifespanfldH[10];
   Char		lifespanfld[10] = "\0";
   Int16 	name1Length;
   Int16 	name2Length;
   Int16 	lifespanLength;
   Int16 	name1Width;
   Int16 	name2Width;
   Int16 	titWidth;
   Int16		titLength;
   Int16 	lifespanWidth;
   UInt16 	nameExtent;
   Boolean 	ignored;
   Boolean	priorityIsName1 = surnameFirst;
   char 		yearB[] = {0x97, 0x97, 0x96, 0xA0, '\0'}; // empty year filler
	char 		yearD[] = {0xA0, 0x97, 0x97, 0x96, '\0'}; // empty year filler
  
   x = bounds->topLeft.x;
   y = bounds->topLeft.y;
   
   DetermineName (record, &fieldSeparatorWidth,
    	&name1, &name1Length, &name1Width, &name2, &name2Length, &name2Width,
    	bounds->extent.x, surnameFirst);

   if (drawLifeSpan) {
   	if (!record->fields[indiLspan]) {
   		StrPrintF (lifespanfld, "%s/%s", yearB, yearD);
   		}
  		else {
	    	StrCopy (lifespanfldH, record->fields[indiLspan]); // size checked on PC
   		sPos = lifespanfldH;
  	   	cPos = StrChr (lifespanfldH, chrSolidus);
  	   	if (cPos) { // make sure "/" was found
   	   	*cPos = '\0';
      
	     	 	if ((cPos - sPos) == 0)
   	   		StrCopy (lifespanfld, yearB);
      		else
      			StrCopy (lifespanfld, sPos);
      	
	     		StrCat (lifespanfld, "/");
   	   	sPos = cPos + 1;
      
      		if (StrLen (sPos) == 0)
      			StrCat (lifespanfld, yearD);
     	 		else
 	    		 	StrCat (lifespanfld, sPos);
 	    		}
 	    	else { // no "/" found so copy what's there.  This should never happen.
 	    		StrCopy (lifespanfld, lifespanfldH);  // size checked on PC
 	    		}
 	    	}
      }
      
   if (lifespanfld && drawLifeSpan) {
      lifespanLength = lifespanWidth = bounds->extent.x; // longer/wider than possible
      FntCharsInWidth (lifespanfld, &lifespanWidth, &lifespanLength, &ignored);
      }
   else {
      lifespanLength = 0;
      lifespanWidth = 0;
      }
      
 	nameExtent = bounds->extent.x - bounds->topLeft.x - lifespanWidth;

   // Leave some space between names and lifespan if there is a lifespan string      
   if (lifespanfld && drawLifeSpan)
   	nameExtent -= SPACE_BEF_LSPAN;

   DrawRecordName (name1, name1Length, name1Width, name2, name2Length, name2Width,
   	nameExtent, &x, y, fieldSeparatorWidth, priorityIsName1, surnameFirst);
   
   // Now draw the title string if there is room // NEW DTR: 2 JUN 2003
   if (record->fields[indiTitle]) {
   	Int16 spaceWidth = FntCharWidth (chrSpace);
   	titLength = bounds->extent.x; // longer than possible
   	titWidth = bounds->extent.x - (x + SPACE_BEF_LSPAN + spaceWidth + lifespanWidth);
   	FntCharsInWidth (record->fields[indiTitle], &titWidth, &titLength, &ignored);
		if (titLength > 0) {
			x += spaceWidth;
			WinDrawChars (record->fields[indiTitle], titLength, x, y);
			}
      }

   // Now draw the lifespan string
   if (lifespanfld && drawLifeSpan) {
   	ErrFatalDisplayIf (bounds->extent.x - (x + SPACE_BEF_LSPAN) <
   		lifespanWidth, "DrawRecordNameAndLifespan: lifespanWidth too large");
		WinDrawChars (lifespanfld, lifespanLength, bounds->extent.x
		 	- lifespanWidth, y);
      }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DrawNameLifespanColor
//
// DESCRIPTION:   Selects a color and calls the routine to draw and individual's
//                name and lifespan.
//
// PARAMETERS:    -> record - record to draw
//                -> bounds - bounds of the draw region
//                -> drawLifeSpan - if true, the person's lifespan data is drawn
//						-> surnameFirst - if true, draw surname before givenname
///
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DrawNameLifespanColor (DBRecordType* rec, RectanglePtr bounds,
 			Boolean drawLifeSpan, Boolean surnameFirst)
{
	IndexedColorType 	ColorIndex;

   // Pick the color to be used to draw the record.
   if (!Pre35Rom) {
   
	   WinPushDrawState ();
   	if (rec->fields[indiChiFlg]) { // person has children
			ColorIndex = WinRGBToIndex (&Color1);
			WinSetTextColor (ColorIndex);
			}
		else if (rec->fields[indiFamSNo]) { // person married without children
			ColorIndex = WinRGBToIndex (&Color2);
			WinSetTextColor (ColorIndex);
			}
		//else {  DTR: Removed 5-14-2004
		//	RGBColorType	textColor = RGB_CT_BLACK;
		//	WinSetTextColor (WinRGBToIndex (&textColor));
		//	}
		}
	
   DrawRecordNameAndLifespan (rec, bounds, drawLifeSpan, surnameFirst);
	
	if (!Pre35Rom)
		WinPopDrawState();
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListViewSetButtonTxt
//
// DESCRIPTION:   Displays or hides the goFirstButton that allows the user to enter
//                and scroll to person's first name.
//
// PARAMETERS:    None.
//                      
// RETURNED: 		Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ListViewSetButtonTxt (void)
{
static Char	goFirstButtonText[] = "\0\0";

   if (!GetFirstName)
 		goFirstButtonText[0] = 0xBB;
	else	
		goFirstButtonText[0] = 0x8B;
	
   CtlSetLabel (GetObjectPtr (IndiListGoFirstButton), goFirstButtonText);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListViewLookupString
//
// DESCRIPTION:   Adds a character to IndiListSearchField, looks up the 
//                string in the database and selects the item that matches.
//
// PARAMETERS:    -> event	- 	EventPtr containing character to add.
//                      
// RETURNED:      true if the field handled the event
//
// REVISIONS:		12/22/04 - 	checks keydown character of '.' to make recognition
//										of 'x' character easier.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ListViewLookupStr (EventPtr event)
{
   FormPtr     frm;
   UInt16      fldIndex;
   FieldPtr    fldP;
   Char*			fldTextP;
   TablePtr    tableP;
   UInt16      foundRec;
   Boolean     completeMatch;
   Int16       length;
   Boolean		noDel = false;
      
   frm 		= FrmGetActiveForm ();
   fldIndex = FrmGetObjectIndex (frm, IndiListSearchField);
   FrmSetFocus (frm, fldIndex);

	fldP 		= (FieldPtr) FrmGetObjectPtr (frm, fldIndex);
      
   if (FldHandleEvent (fldP, event) || event->eType == fldChangedEvent) {

		// For devices using Grafitti 2, the small 'l' character is used to 
		// start certain letters (e.g. 'i', 't', 'k' and 'x'). So we cannot
		// delete if no surnames have this character next.
		//if (Graf2Device && event->data.keyDown.chr == 'l') {
		if (Graf2Device && (event->data.keyDown.chr == 'l'
		 	|| event->data.keyDown.chr == '.')) { // DTR: 12/22/2004
			noDel = true;
			}

      fldTextP = FldGetTextPtr (fldP);
      tableP = (TablePtr) GetObjectPtr (IndiListIndiListTable);

		if (!GetFirstName) {
      	if (!IndiLookupLName (fldTextP, &foundRec, &completeMatch)) {
         	// If the user deleted the lookup text remove the highlight.
  				CurrentIndiRecN = NO_REC;
         	TblUnhighlightSelection (tableP);
         	}
      	else {
	         ListViewSelectRec (foundRec, true);
	         }
         
      	// hide/show first name icon if an Individual is selected   
      	ShowObject (IndiListGoFirstButton, (CurrentIndiRecN != NO_REC));
      	if (CurrentIndiRecN != NO_REC)
      		ListViewSetButtonTxt ();
     		}
   	else { // GetFirstName
   		foundRec = CurrentIndiRecN; // start with found record to get indiLName
   		if (IndiLookupFName (fldTextP, &foundRec, &completeMatch))
         	ListViewSelectRec (foundRec, true);
         }
     	
      if (!completeMatch  && !noDel) { // delete the last character added.
         length = FldGetTextLength (fldP);
         FldDelete (fldP, length - 1, length);
         SndPlaySystemSound (sndError);
         }
         
      return true;
      }

   return false; // event not handled
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListViewDrawRecord
//
// DESCRIPTION:   Draws and individual record on the Individual List
//                form.  It is called as a callback routine by the table object.
//
// PARAMETERS:    -> table  - pointer to the Individual list table
//                -> row    - row number, in the table, of the item to draw
//                -> column - column number, in the table, of the item to draw
//                -> bounds - bounds of the draw region
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ListViewDrawRecord (void* table, Int16 row, Int16 column,
   RectanglePtr bounds)
{
   DBRecordType  	rec;
   MemHandle      recH = NULL;
   FontID 	      curFont;

   if (DbGetRecord (IndiDB, TblGetRowData (table, row), &rec, &recH) != 0) {
	   ErrNonFatalDisplay ("ListViewDrawRecord: Record not found");
   	return;
   	}
   
   curFont = FntSetFont (PrefFnts[IndiListFont]);
   DrawNameLifespanColor (&rec, bounds, true, true);
	FntSetFont (curFont);
   DbMemHandleUnlock (&recH);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListClearLookupString
//
// DESCRIPTION:   Clears the IndiListSearchField and hides/shows the 
//						IndiListGoFirstButton based on whether CurrentIndiRecN
//						equals NO_REC.
//						
//						NOTE: GetFirstName and CurrentIndiRecN must be set before 
//						calling this function.
//
// PARAMETERS:    None.
//                      
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ListClearLookupStr (void)
{
   FormPtr		frm;
   UInt16 		fldIndex;
   FieldPtr 	fldP;
   Int16 		length;
   Boolean		showButton;

	frm = FrmGetActiveForm ();

   // Hide or show first name icon if an Individual is selected   
   showButton = (Boolean) (CurrentIndiRecN != NO_REC);
   ShowObject (IndiListGoFirstButton, showButton);
   
   if (showButton)
     	ListViewSetButtonTxt ();
   
   SetFocus (noFocus);
   fldIndex = FrmGetObjectIndex (frm, IndiListSearchField);
   fldP 		= (FieldPtr) FrmGetObjectPtr (frm, fldIndex);
   FldSetSelection (fldP, 0, 0); // set this so we do not crash Palm III's

   length = FldGetTextLength (fldP);
   
	if (length > 0) {
  		// Clear it this way (not with FldDelete) to avoid sending a
  		// fldChangedEvent (which would undesirably unhighlight the item).
  		FldFreeMemory (fldP);
   	FldDrawField (fldP);
   	}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ListLoadTable
//
// DESCRIPTION: 	This routine loads IndiDB database records into the Individual
//              	List View form.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void ListViewLoadTable (void)
{
   Int16				row;
   UInt16      	numRows;
   UInt16			recN; // record in IndiDB
   TablePtr			table;
	RectangleType	tblBounds;
   Boolean     	scrollableU;
   Boolean			scrollableD;
   
   table = (TablePtr) GetObjectPtr (IndiListIndiListTable);
   
  	TblUnhighlightSelection (table); // must do this here

 	// Get the number of rows in the table.
 	TblGetBounds (table, &tblBounds); 
	
	numRows = min ((tblBounds.extent.y / TblGetRowHeight (table, 0)),
		TblGetNumberOfRows (table)); 

   // Make sure we haven't scrolled too far down the list of records
   // leaving blank lines in the table.
  	if (TopVisIndiRecN + (numRows - 1) >= IndiDBNumRecs)
     	TopVisIndiRecN = (IndiDBNumRecs > numRows) ? IndiDBNumRecs - numRows : 0;

   recN = TopVisIndiRecN;

	for (row = 0; row < numRows && recN < IndiDBNumRecs; row++) {
      TblSetRowUsable (table, row, true);
      TblMarkRowInvalid (table, row);
      TblSetRowData (table, row, recN);
      recN++;
      }

   // Hide the item that don't have any data.
   while (row < numRows) {      
      TblSetRowUsable (table, row, false);
      row++;
      }

   // Update the scroll buttons.
   scrollableU = (Boolean) (TopVisIndiRecN > 0);

	row = TblGetLastUsableRow (table); // find the record in last row of the table
	
	if (row != tblUnusableRow)
   	scrollableD = (Boolean) (TblGetRowData (table, row) < IndiDBNumRecs - 1);
   else
	   scrollableD = false;
   
   UpdateScrollers (IndiListScrollUpRepeating, IndiListScrollDownRepeating,
    	scrollableU, scrollableD); 
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ListViewScroll
//
// DESCRIPTION: 	This routine scrolls the list of names	in the direction
// 					specified.
//
// PARAMETERS:  	-> direction	- up or dowm.
//              	-> units			- unit amount to scroll.
//						-> forcePage	- true to force scroll by one full page of data.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	Nothing.
////////////////////////////////////////////////////////////////////////////////////
static void ListViewScroll (DirType direction, UInt16 units, Boolean forcePage)
{
	TablePtr			table;
	UInt16 			lastVisRow;
	Int16 			row, col;
	UInt16			scrollRows;
	UInt16 			newTopVisibleRecN;
	UInt16 			prevTopVisibleRecN = TopVisIndiRecN;
	Boolean			rowSelected;

	AdjustScrollRate (); // adjust scroll rate if necessary

	table = (TablePtr) GetObjectPtr (IndiListIndiListTable);

	MenuEraseStatus (0); // make sure that the command bar is closed
	
	GetFirstName = false; // DTR: 8-10-2003
	
   lastVisRow = TblGetLastUsableRow (table);

	// If user selects preference to scroll line by line using arrow buttons, then
	// set scroll amount to 1 line. However, allow accellerated page scrolling if
	// user holds down button (and units go above 1).
	if (Prefs[ScrollOneI] && units == 1)
		scrollRows = 1;
	else
		scrollRows = lastVisRow;

	// Scroll line by line in using hard up/down arrows. Check if we are scrolling
	// past the end of current page.
	rowSelected = TblGetSelection (table, &row, &col); // see if row is selected
	
	if (units == 1 && rowSelected && !forcePage) {

		if (direction == NavDn) // scroll to next line
			row++;
		else
			row--;
		
		scrollRows = 1; // remember, this assignment is made only if units == 1;
		
		if (row >= 0 && row <= TblGetLastUsableRow (table)) {
			CurrentIndiRecN = (UInt16) TblGetRowData (table, row);
		 	ListViewSelectRecN = CurrentIndiRecN;
			ListViewSelectRec (ListViewSelectRecN, true);
			ListClearLookupStr (); // clear lookup string for any scroll DTR: 08-Aug-2003
			return;
			}
		}

	newTopVisibleRecN = TopVisIndiRecN;

	if (direction == NavDn) { // scroll the table down
	   newTopVisibleRecN += (units * scrollRows);
	   
	   // make sure one full page is displayed
	   if (newTopVisibleRecN + lastVisRow >= IndiDBNumRecs) {
	   	if (IndiDBNumRecs > lastVisRow)
	     		newTopVisibleRecN = IndiDBNumRecs - (lastVisRow + 1);
	     	else
	     		newTopVisibleRecN = 0;
	     	}
      }
   else { // scroll the table up
		if (newTopVisibleRecN > units * scrollRows)
      	newTopVisibleRecN -= (units * scrollRows);
    	else
  			newTopVisibleRecN = 0;
      }

	// Avoid redraw if no change
	if (TopVisIndiRecN != newTopVisibleRecN) {
		TopVisIndiRecN = newTopVisibleRecN;
		CurrentIndiRecN = NO_REC; // scrolling always deselects current person
		ListViewLoadTable ();
		
		// Need to compare the previous top record to the current after ListLoadTable 
		// as it will adjust TopVisIndiRecN if drawing from newTopVisibleRecN will
		// not fill the whole screen with items.
		if (TopVisIndiRecN != prevTopVisibleRecN)
			TblRedrawTable (table);

	 	if (rowSelected && !forcePage) {
	 		
	 		ListViewSelectRecN = TopVisIndiRecN;
	 			 		
	 		if (direction == NavDn)
	 			ListViewSelectRecN += lastVisRow;
		 	
		 	CurrentIndiRecN = ListViewSelectRecN; // DTR: 08-Aug-2003
			ListViewSelectRec (ListViewSelectRecN, true); // DTR: 08-Aug-2003
			}
		}
		
	ListClearLookupStr (); // clear lookup string for any scroll DTR: 08-Aug-2003
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ListViewSelectRecord
//
// DESCRIPTION: 	Selects (highlights) a record on the table, scrolling
//              	the record if neccessary.  Also sets the CurrentIndiRecN.
//
// PARAMETERS:  	-> recN 			- 	record to select.
//						-> forceSelect - 	if true, unselect current person and then
//												reselect. This is for backwards compatibility.
//                      
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ListViewSelectRec (UInt16 recN, Boolean forceSelect)
{
   Int16 		row;
   Int16 		column;
   TablePtr		table;

	table = (TablePtr) GetObjectPtr (IndiListIndiListTable);

	if (recN == NO_REC)
		return;
	
   // Don't change anything if the same record is selected
   if ((TblGetSelection (table, &row, &column)) &&
   	 (recN == TblGetRowData (table, row)) && 
   	 (!forceSelect)) {
      return;
      }

   // See if the record is displayed in one of the rows in the table. If 
   // TblFindRowData fails, call it again to find the row in the reloaded table.
   while (!TblFindRowData (table, recN, &row)) {
		
		ErrFatalDisplayIf (recN >= IndiDBNumRecs, "Table Row Select Error");
		            
      // Scroll the view down placing the item on the top row
      TopVisIndiRecN = recN;
  
      ListViewLoadTable ();
      TblRedrawTable (table); // YES, we have to call this here.
      }
   
   // Select the item
   if (forceSelect) {
		TblUnhighlightSelection (table); // DTR: 22 May 2003
   	TblSelectItem (table, row, 0);
	 	}
    	
   CurrentIndiRecN = recN;
   return;
}   

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ListViewInit
//
// DESCRIPTION: 	This routine initializes the Individual List View of the
//						application.
//
// PARAMETERS:  	None.
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ListViewInit (void)
{
   Int16 		row;
   Int16 		rowsInTable;
   TablePtr		table;
   FontID		curFont = FntSetFont (IndiListFont);

	table = (TablePtr) GetObjectPtr (IndiListIndiListTable);

   // Initialize the Individual List table.
   rowsInTable = TblGetNumberOfRows (table); // set all rows unusable

   for (row = 0; row < rowsInTable; row++) {      
      TblSetItemStyle (table, row, 0, customTableItem);
      TblSetRowHeight (table, row, FntLineHeight ());
      TblSetRowUsable (table, row, false);
      }
   
   TblSetColumnUsable (table, 0, true);
   TblSetCustomDrawProcedure (table, 0, ListViewDrawRecord);
	FntSetFont (curFont);

   ListViewLoadTable ();
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListViewDrawForm
//
// DESCRIPTION:   This routine draws the List View Form.
//
// PARAMETERS:   	-> newForm	- 	true if drawing form for first time, else false
//											if just updating form. 
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ListViewDrawForm (Boolean newForm)
{
	FontID	curFont;
	FormPtr	frm;
	
	frm = FrmGetActiveForm ();
   
  	SetFocus (noFocus); // prevent white spot on screen when enlarging screen
  	
  	WinSetDrawWindow (FrmGetWindowHandle (frm)); // do this instead of DrawFrm.
    
   if (newForm) {
	   
	   ListViewInit ();

		// Show or hide certain buttons and labels depending if calling
		// this routine from the Relationship Calculator or List View.
		if (RelCalcEntry) { // selecting person in Relationship Calculator
			ShowObject (IndiListDBButton, 		 false);
			ShowObject (IndiListCancelButton, 	 true);
			ShowObject (IndiListSelectIndiLabel, true);
			}
		else {
			FrmSetMenu (frm, IndiListMenuBarMenuBar);
			ShowObject (IndiListHeaderLabel, true);
			}

		// Make sure the record to be selected is one of the table's rows or
		// else it reloads the table with the record at the top.
   	if (ListViewSelectRecN != NO_REC) {
   		ShowObject (IndiListGoFirstButton, true);
 			ListViewSelectRec (ListViewSelectRecN, false);
     		}
		}
		
	FrmDrawForm (frm);
			
	// Select the record.  This finds which row to select it and does it.
   ListViewSelectRec (ListViewSelectRecN, true);
       	
	// Draw Database Name at top of form.
	if (!RelCalcEntry) {
		RGBColorType	drawColor = RGB_CT_BLUE; // draw color is blue

		curFont = FntSetFont (stdFont);
   	
   	DrawCharsColorI (DbName, drawColor, 160 - FntCharsWidth (DbName,
   		StrLen (DbName)), 2);
		
		FntSetFont (curFont);
		}

	ListViewSetButtonTxt ();

	// Set focus in lookup field so user can easily bring up the keyboard.
	SetFocus (IndiListSearchField);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListViewResizeForm
//
// DESCRIPTION:   This routine resizes and draws the List View Form.
//
// PARAMETERS:   	None. 
//
// RETURNED:      True if form was resized, else false (no changed in size).
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ListViewResizeForm (void)
{
	Int16 			extentDelta;
	UInt16 			numObjects, i;
	Coord 			x, y;
	FormPtr			frmP;
	RectangleType 	objBounds; // bounds of object being set
	RectangleType 	curBounds; // bounds of active form
	RectangleType	displayBounds; // bounds of screen

	if (!DynInDevice)
		return false;

	frmP = FrmGetActiveForm();

	// Get dimensions of current active form.
	WinGetBounds (FrmGetWindowHandle (frmP), &curBounds);
	
	// Get the new display window bounds
	WinGetBounds (WinGetDisplayWindow (), &displayBounds);

	extentDelta = 	(displayBounds.extent.y + displayBounds.topLeft.y) -
						(curBounds.extent.y - curBounds.topLeft.y);
	
	if (extentDelta == 0)  // form has not changed in size, so do nothing
		return false;
		
	WinSetBounds (FrmGetWindowHandle (frmP), &displayBounds); // set new form bounds
	
	// Iterate through objects and re-position them.
	numObjects = FrmGetNumberOfObjects (frmP);

	for (i = 0; i < numObjects; i++) {
	
		switch (FrmGetObjectId (frmP, i))
			{
			case IndiListGoFirstButton:
			case IndiListSearchField:
			case IndiListSearchLabel:
			case IndiListScrollUpRepeating:
			case IndiListScrollDownRepeating:
				FrmGetObjectPosition (frmP, i, &x, &y);
				FrmSetObjectPosition (frmP, i, x, y + extentDelta);
				break;

			case IndiListIndiListTable:
				FrmGetObjectBounds (frmP, i, &objBounds);
				objBounds.extent.y += extentDelta;
				FrmSetObjectBounds (frmP, i, &objBounds);
				break;
			
			default:
				break;
			} // switch
		
		}  // for

	return true;
}


////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ListViewHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the "List View"
//                of the Address Book application.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure
//
// RETURNED:      True if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ListViewHandleEvent (EventPtr event)
{
   Boolean 		handled 	= false;
  	DirType 		navDir;
  	Int16			ignored;
	
   switch (event->eType)
   	{
      case tblSelectEvent:
         // An item in the list of names was selected, go to the individual view.
         CurrentIndiRecN = TblGetRowData (event->data.tblSelect.pTable, 
            event->data.tblSelect.row);
         FrmGotoForm (RelCalcEntry ?	RelCalcForm : IndiSummForm);
         handled = true;
         break;

		case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case IndiListScrollUpRepeating:
				case IndiListScrollDownRepeating:
					ResetScrollRate (); // reset scroll rate
					// leave unhandled so the buttons can repeat
					break;
					
				case IndiListDBButton: // open Database List
		      	// Save information about form to return to
					PriorFormID = IndiListForm;
   				ListViewSelectRecN = CurrentIndiRecN;
   				FrmCloseAllForms ();
		      	FrmGotoForm (DatabasesForm);
            	handled = true;
            	break;

				case IndiListMenuButton: // open the Menu Bar
					EvtEnqueueKey (vchrMenu, 0, commandKeyMask);
        			handled = true;
         		break;
            	
            default:
            	break;
				}
         break;

      case ctlRepeatEvent:

         switch (event->data.ctlRepeat.controlID)
         	{
            case IndiListScrollUpRepeating:
					ListViewScroll (NavUp, ScrollUnits, true);
					// leave unhandled so the buttons can repeat
               break;
               
            case IndiListScrollDownRepeating:
					ListViewScroll (NavDn, ScrollUnits, true);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;

		case ctlSelectEvent:
		
			switch (event->data.ctlSelect.controlID)
				{
				case IndiListGoFirstButton:
   				GetFirstName = !GetFirstName; // DTR: 8-10-2003
   				ListClearLookupStr (); // clear lookup string
            	SetFocus (IndiListSearchField);
        			handled = true;
         		break;

				case IndiListCancelButton:
					if (RelCalcGetRec1)
			      	CurrentIndiRecN = RelCalcRecN1;
			      else
			      	CurrentIndiRecN = RelCalcRecN2;
   		      FrmGotoForm (RelCalcForm);	
               handled = true;
               break;

				default:
					break;
					}
					
				break;

      case keyDownEvent:

      	if (NavKeyHit (event, &navDir)) {
				
				switch (navDir) {
					
					case NavUp:
					case NavDn:
					   // Reset scroll rate if not auto repeating
				   	if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
			   			ResetScrollRate ();

						ListViewScroll (navDir, ScrollUnits, false);
						handled = true;
						break;

					case NavL:
					case NavR:
						CtlHitControl (GetObjectPtr (IndiListGoFirstButton));
			  			handled = true;
						break;
						
					case NavSel:
					   if (TblGetSelection (GetObjectPtr (IndiListIndiListTable),
					   	&ignored, &ignored)) {
					   	FrmGotoForm (RelCalcEntry ?	RelCalcForm : IndiSummForm);
					   	}
					   else {
					   	ErrFatalDisplayIf (TopVisIndiRecN == NO_REC,
		     					"ListViewHandleEvent: TopVisIndiRecN should have value");
	 						ListViewSelectRecN = TopVisIndiRecN;
		 					CurrentIndiRecN = ListViewSelectRecN;
							ListViewSelectRec (ListViewSelectRecN, true); 
							ShowObject (IndiListGoFirstButton, true);
							ListViewSetButtonTxt ();
					   	}
				   	handled = true;
						break;
					
					default:
						break;
					}
				} // if NavKeyHit
				
			else if (event->data.keyDown.chr == linefeedChr) {
			   if (TblGetSelection (GetObjectPtr (IndiListIndiListTable),
			   	&ignored, &ignored))
			   	FrmGotoForm (RelCalcEntry ?	RelCalcForm : IndiSummForm);
			   handled = true;
			   }									
							
			else if (event->data.keyDown.chr == chrComma	
				&& CurrentIndiRecN != NO_REC) {
				CtlHitControl (GetObjectPtr (IndiListGoFirstButton));
			  	handled = true;
			  	}

	   	else { // must be letter for name
		     	ErrFatalDisplayIf (GetFirstName && (CurrentIndiRecN == NO_REC),
		     		"ListViewHandleEvent: CurrentIndiRecN should have value");
				handled = ListViewLookupStr (event);
				  }
         break; 

      case fldChangedEvent:
      	ErrFatalDisplayIf (GetFirstName && (CurrentIndiRecN == NO_REC),
      		"ListViewHandleEvent: CurrentIndiRecN should have value");
         ListViewLookupStr (event);
         handled = true;
         break;
            
     	case menuEvent:
     		ListViewSelectRecN = CurrentIndiRecN;
     		SetFocus (IndiListSearchField); // set focus in case user wants keyboard.
        	return MenuDoCommand (event->data.menu.itemID);
 
      case frmOpenEvent:
         GetFirstName = false; // initialize
      	ListViewResizeForm (); // resize before drawing
         ListViewDrawForm (true);
         handled = true;
         break;

     	case frmUpdateEvent:
    		ErrFatalDisplayIf (!UpdateFrm, "ListViewHandleEvent: UpdateFrm should be true");         
         ListViewSelectRecN = CurrentIndiRecN;
         if (ListViewResizeForm ())
	         ListViewDrawForm (true);
	      else
		   	ListViewDrawForm (false);
         handled = true;
         break;

		case winDisplayChangedEvent:
			ListViewSelectRecN = CurrentIndiRecN; // must do this !!!
			if (ListViewResizeForm ())
				ListViewDrawForm (true);
			handled = true;
			break;

      default:
      	break;
   	}
      
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ApplicationHandleEvent
//
// DESCRIPTION:   Load form resources and set the event handler for the
//                form loaded.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure
//
// RETURNED:      True if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ApplicationHandleEvent (EventPtr event)
{
   UInt16 		formId;
   FormPtr 		frm;
   WinHandle 	formWinH;

   if (event->eType == frmLoadEvent) {
      
      // Load the form resource.
      formId = event->data.frmLoad.formID;
      frm = FrmInitForm (formId);
      FrmSetActiveForm (frm);      
      
      // Set the Dynamic Input Area policy attribute and trigger state.
      if (DynInDevice) {
      	
      	formWinH = FrmGetWindowHandle (frm);   
     	
      	switch (formId)
      		{
      		case IndiListForm:
      		case IndiSummForm:
      		case NoteDetailForm:
      		case DescendancyForm:
   	      	FrmSetDIAPolicyAttr (frm, frmDIAPolicyCustom);
      			PINSetInputTriggerState (pinInputTriggerEnabled); 
      			PINSetInputAreaState (pinInputAreaUser);
      			WinSetConstraintsSize (formWinH, 160, 225, 225, 160, 160, 160);
      			break;
      		
      		case EvenDetailForm:
      		case IndiDetailForm:
      		case ChilDetailForm:
      		case FamiDetailForm:
      		case SourDetailForm:
      		case SouCDetailForm:
      		case RepoDetailForm:
      		case RepCDetailForm:
      		case AliaDetailForm:
      		case AliaListForm:
		      	FrmSetDIAPolicyAttr (frm, frmDIAPolicyCustom);
      			PINSetInputTriggerState (pinInputTriggerEnabled); 
      			PINSetInputAreaState (pinInputAreaUser);
      			WinSetConstraintsSize (formWinH, 95, 95, 95, 160, 160, 160);
      			break;
     		
	    		/*case Pref1Form:
         	case Pref2Form:
         	case SplashForm:
         	case SoundexForm:
	         	FrmSetDIAPolicyAttr (frm, frmDIAPolicyCustom);
	         	PINSetInputTriggerState (pinInputTriggerEnabled); 
      			PINSetInputAreaState (pinInputAreaUser);
					break;

    			case Pref1Form:
         	case Pref2Form:
         	case SplashForm:
	         	FrmSetDIAPolicyAttr (frm, frmDIAPolicyCustom);
	         	PINSetInputTriggerState (pinInputTriggerDisabled); 
					break; */

     		
      		default:
	      		FrmSetDIAPolicyAttr (frm, frmDIAPolicyStayOpen);
      			PINSetInputTriggerState (pinInputTriggerDisabled);
      			PINSetInputAreaState (pinInputAreaOpen);
      			break;
      		} // switch
         
          // Allow change in orientation.
			SysSetOrientationTriggerState (sysOrientationTriggerEnabled);
			
         } // if (DynInDevice)
      
      // Set the event handler for the form.
      switch (formId)
      	{
         case IndiListForm:
            FrmSetEventHandler (frm, ListViewHandleEvent);
            break;
      
         case IndiSummForm:
            FrmSetEventHandler (frm, IndiSummHandleEvent);
            break;

          case EvenDetailForm:
            FrmSetEventHandler (frm, EvenViewHandleEvent);
            break;
            
         case IndiDetailForm:
            FrmSetEventHandler (frm, IndiViewHandleEvent);
            break;

          case ChilDetailForm:
            FrmSetEventHandler (frm, ChilViewHandleEvent);
            break;

          case FamiDetailForm:
            FrmSetEventHandler (frm, FamiViewHandleEvent);
            break;
            
         case NoteDetailForm:
            FrmSetEventHandler (frm, NoteViewHandleEvent);
            break;

         case SourDetailForm:
            FrmSetEventHandler (frm, SourViewHandleEvent);
            break;

         case SouCDetailForm:
            FrmSetEventHandler (frm, SouCViewHandleEvent);
            break;
            
         case RepoDetailForm:
            FrmSetEventHandler (frm, RepoViewHandleEvent);
            break;

         case RepCDetailForm:
            FrmSetEventHandler (frm, RepCViewHandleEvent);
            break;
            
          case AliaListForm:
            FrmSetEventHandler (frm, AliaListHandleEvent);
            break;
            
          case AliaDetailForm:
            FrmSetEventHandler (frm, AliaViewHandleEvent);
            break;
               
         case Pref1Form:
         case Pref2Form:
            FrmSetEventHandler (frm, PreferencesHandleEvent);
            break;

         case SoundexForm:
            FrmSetEventHandler (frm, SoundexHandleEvent);
            break;

         case SearchForm:
            FrmSetEventHandler (frm, EvenSearchHandleEvent);
            break;

			case DatabasesForm:
            FrmSetEventHandler (frm, DBListHandleEvent);
            break;

			case RegisterForm:
            FrmSetEventHandler (frm, RegisterHandleEvent);
            break;
            
			case SplashForm:
            FrmSetEventHandler (frm, SplashHandleEvent);
            break;

			case AncestryForm:
            FrmSetEventHandler (frm, AncestryHandleEvent);
            break;

			case DescendancyForm:
            FrmSetEventHandler (frm, DescendancyHandleEvent);
            break;
         
			case CopyDBForm:
           	FrmSetEventHandler (frm, CopyDbHandleEvent);
            break;

			case DateCalcForm:
           	FrmSetEventHandler (frm, DateCalcHandleEvent);
            break;
               
			case RelCalcForm:
           	FrmSetEventHandler (frm, RelCalcHandleEvent);
            break;

			case JumpForm:
           	FrmSetEventHandler (frm, JumpListHandleEvent);
           	break;

			case FldSearchForm:
           	FrmSetEventHandler (frm, FldSrchHandleEvent);
            break;

			case ThisDayNewForm:
           	FrmSetEventHandler (frm, ThisDayNewHandleEvent);
            break;

			case FindPersonForm:
           	FrmSetEventHandler (frm, FindPersonHandleEvent);
            break;

			case ExportMemoForm:
           	FrmSetEventHandler (frm, ExportMemoHandleEvent);
            break;

         default:
            ErrNonFatalDisplayIf (true, "Invalid Form Load Event");
            break;
			}
   
      return (true);
   	}

   return (false);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ExportMemoPageScroll
//
// DESCRIPTION:   This routine scrolls the message a page up or down.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ExportMemoSaveMemo (void)
{
	Char*					ptr;
	Boolean 				empty;
	FieldPtr 			fld;
 	LocalID 				appID;
   UInt16				cardNo;
   DmSearchStateType searchState;
	UInt32				resultP;
 
 	if (!MemoDbRef) // make sure memo database is open (if not, error occured)
 		return;
 
	// Find out if the field has been modified or if it's empty.
	fld = (FieldPtr) GetObjectPtr (ExportMemoMemoField);
	ptr = FldGetTextPtr (fld);
	empty = (*ptr == 0);

	FldReleaseFocus (fld);
	
	FldCompactText (fld); // release any free space in the memo field.

	FldSetTextHandle (fld, 0); // clear field value (handle is unlocked as well)

	// If there's data in an existing record, mark it dirty if necessary
	// and release it.
	if (! empty) {
		
		DmReleaseRecord (MemoDbRef, ExportMemoIndex, true);

		DmCloseDatabase (MemoDbRef);
		
		// Move record to the correct sort position by silently sending a command
		// to the Memo application to sort database.
		if (DmGetNextDatabaseByTypeCreator (true, &searchState, sysFileTApplication,
   		sysFileCMemo, true, &cardNo, &appID))
   		return;
	
		SysAppLaunch (cardNo, appID, 0, sysAppLaunchCmdSyncNotify, NULL, &resultP);
		}
	
	else { // the record is empty so delete it.
		
		DmRemoveRecord (MemoDbRef, ExportMemoIndex); 
		
		DmCloseDatabase (MemoDbRef);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ExportMemoInit
//
// DESCRIPTION:   Exports currently selected individual to a Palm Memo.
//						Assumes - maximum name size  = 180
//									 maximum place size = 120
//									 maximum marriages  = 15
//						No need to size-check information added to "data" string
//						until we get to adding spouse information b/c there is
//						plenty of room given size assumptions above.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		DTR: Created 2001-07-16
//						DTR: Revised 2004-08-25 to use GedWise editor form.
////////////////////////////////////////////////////////////////////////////////////
static void ExportMemoInit (void)
{
	MemHandle  			exportDataH = NULL;
  	Char*					data; // holds data sent to memo			
	Err					err;
   LocalID				dbID 		= 0;
   LocalID 				appID 	= 0;
   UInt16				cardNo	= 0;
   UInt16				i;
   MemHandle			memoRecH = NULL;
   MemPtr				memoP		= NULL;
   MemHandle      	eRecH 	= NULL;
   DBRecordType   	eRec;
   MemHandle 	      indiRecH = NULL; // for spouse data
   DBRecordType 		indiRec; // for spouse data
   DmSearchStateType searchState;
	UInt32				resultP;
   Char           	famiXRef[NREF_LEN+1];
 	FieldPtr 			fld;
  
   // Get cardNo and ID for memo application
   if (DmGetNextDatabaseByTypeCreator (true, &searchState, sysFileTApplication,
   	sysFileCMemo, true, &cardNo, &appID) )
   	goto MemoErr;

   // Open the memo database (not the memo application).
   MemoDbRef = DmOpenDatabaseByTypeCreator ('DATA', sysFileCMemo, dmModeReadWrite);

	if (!MemoDbRef) { // if database does not exist then create it		
		
		// silently launch memo application with command to create database
		if (SysAppLaunch (cardNo, appID, 0, sysAppLaunchCmdInitDatabase, NULL,
			&resultP))
			goto MemoErr; 
   		// try again to open the memo database
   	MemoDbRef = DmOpenDatabaseByTypeCreator ('DATA', sysFileCMemo, dmModeReadWrite);
		}
				
	if (!MemoDbRef) goto MemoErr;
			
	// Get the ID and card number for memo database 
   if (DmOpenDatabaseInfo (MemoDbRef, &dbID, NULL, NULL, &cardNo, NULL) )
   	goto MemoErr;
	
	// Allocate data variable on dynamic heap and return a handle to it
	exportDataH = MemHandleNew (EXPORT_DATA_MAX_SZ+1);
   ErrFatalDisplayIf (!exportDataH, "ExportMemo: Out of memory");
   if (!exportDataH) goto MemoErr;
   
   data = (Char*) MemHandleLock (exportDataH);  

 	DbGetRecord (IndiDB, CurrentIndiRecN, &IndiRec, &IndiRecH);

	if (!IndiRec.fields[indiFName])
		IndiRec.fields[indiFName] = cUnknownStr;
     	
   if (!IndiRec.fields[indiLName])
   	IndiRec.fields[indiLName] = cUnknownStr;

   StrPrintF (data, "%s, %s (Id: %s)\n\n", IndiRec.fields[indiLName],
     	IndiRec.fields[indiFName], IndiRec.fields[indiNo]);

	// -- Get Birth and Death Information for Individual -- //
	for (i = 0; i < 2; i++) {
		switch (i)
			{
			case 0:
   			if (PrimaryBirtRecN != NO_REC_LONG)
      			DbGetRecord (EvenDB, PrimaryBirtRecN, &eRec, &eRecH);
      		else {
      			StrCat (data,"Bir: "); // leave blank field to enter data
      			}
      		break;
      		
      	case 1:
				if (PrimaryDeatRecN != NO_REC_LONG)
      			DbGetRecord (EvenDB, PrimaryDeatRecN, &eRec, &eRecH);
				else
	     			StrCat (data,"Dth: "); // leave blank field to enter data
				break;
			}

		if (eRecH != 0) {
					
 			StrCat (data, EvenDesc [StrAToI (eRec.fields[evenType])][1]);
			StrCat (data,": ");
			if (eRec.fields[evenDate]) {
				StrCat (data, eRec.fields[evenDate]);
				StrCat (data,"  ");
				}
			if (eRec.fields[evenPlac])
				StrCat (data, eRec.fields[evenPlac]);
      	DbMemHandleUnlock (&eRecH);
			}
		
		StrCat (data, "\n\n");
		}

	// -- Get Spouse(s) for Individual -- //
	if (LastFamiNum > 0) {
	
		for (i = 1; i <= LastFamiNum; i++) {
			
			// get the next family record in inidividuals family list string. 
      	if (!RefFinderStr (i, FAMI_DLM, IndiRec.fields[indiFamSNo], famiXRef))
      		break;
    
    		// get the marriage record, if any.
    		CurrentFamiRecN = (UInt32) StrAToI (famiXRef); 
         DbGetRecord (FamiDB, CurrentFamiRecN, &FamiRec,	&FamiRecH);

      	// Get the spouse, if any.  Check first whether current Individual
      	// is the husband or wife of the current family.  If there is not famiHusbNo
      	// then there is no spouse in database, so CurrentSpouRecN = NO_REC.
			CurrentSpouRecN = NO_REC;  // initialize
      	
      	if (FamiRec.fields[famiHusbNo]) {
      	
	      	UInt16 husbRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);
      	
      		if (CurrentIndiRecN == husbRecN) {
      	
      			if (FamiRec.fields[famiWifeNo])
      				CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiWifeNo]);
         		}
      		else
      			CurrentSpouRecN = husbRecN;
				}
				
			if (CurrentSpouRecN  != NO_REC) {

 				DbGetRecord (IndiDB, CurrentSpouRecN, &indiRec, &indiRecH);

				StrNCat (data,"Sp: ", EXPORT_DATA_MAX_SZ);
				if (indiRec.fields[indiLName])
					StrNCat (data, indiRec.fields[indiLName], EXPORT_DATA_MAX_SZ) ;
				if (indiRec.fields[indiFName]) {
					StrNCat (data, cFieldSepStr, EXPORT_DATA_MAX_SZ);
					StrNCat (data, indiRec.fields[indiFName], EXPORT_DATA_MAX_SZ);
					}
				StrNCat (data, "\n" , EXPORT_DATA_MAX_SZ);
				}

			DbMemHandleUnlock (&indiRecH);
			DbMemHandleUnlock (&FamiRecH);

			} // for loop
		}
		
	DbMemHandleUnlock (&IndiRecH);
	
	#ifdef GREMLINS
	if (DmNumRecords (MemoDbRef) > 20) { // max 20 memos during testing
   	if (MemoDbRef) DmCloseDatabase (MemoDbRef);
   	MemoDbRef = NULL;
	   MemHandleFree (exportDataH);
   	FrmGotoForm (IndiSummForm); // only do this during testing!!!
   	return;
   	}
   #endif
			
	// Add a new record at the end of the Memo database.
	ExportMemoIndex = DmNumRecords (MemoDbRef);
	memoRecH = DmNewRecord (MemoDbRef, &ExportMemoIndex, StrLen (data) + 1);

	// If the allocate failed, display a warning
	if (!memoRecH) {
		FrmAlert (DeviceFullAlert);
		goto MemoErr;
		}

	memoP = MemHandleLock (memoRecH);

	// Terminate new record with NULL (0) character.
	err = DmWrite (memoP, 0, data, StrLen (data) + 1);

	MemPtrUnlock (memoP);
	memoP = NULL;
	
	MemHandleFree (exportDataH);
	exportDataH = NULL;
 
 	if (err) goto MemoErr;
 
	DrawForm ();
	
	// Set a pointer to the Memo field and draw it.
	fld = (FieldPtr) GetObjectPtr (ExportMemoMemoField);
	FldSetTextHandle (fld, memoRecH);
	FldSetScrollPosition (fld, 0);

	UpdateScrollBar (ExportMemoMemoField, ExportMemoScrollScrollBar);

   SetFocus (ExportMemoMemoField);		

	ErrFatalDisplayIf (memoP || exportDataH || eRecH || indiRecH,
		"ExportMemoInit: Handle or Pointer left locked");
	return;
			
	////////
	MemoErr:
	////////
	
	if (memoRecH) DmRemoveRecord (MemoDbRef, ExportMemoIndex);
	if (MemoDbRef) DmCloseDatabase (MemoDbRef);
	MemoDbRef = NULL;
	
	if (exportDataH) MemHandleFree (exportDataH);
	exportDataH = NULL;
	
	FrmAlert (ExportMemoAlert);
	FrmGotoForm (IndiSummForm);
	
	
	ErrFatalDisplayIf (memoP || exportDataH || eRecH || indiRecH,
		"ExportMemoInit: Handle or Pointer left locked");
   return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ExportMemoHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Export Memo Form.
//
// PARAMETERS:  	-> event  - a pointer to an EventType structure
//
// RETURNED:    	True if the event has handled and should not be passed
//              	to a higher level handler.
//
// REVISIONS:	 	None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ExportMemoHandleEvent (EventType* event)
{
   Boolean  handled = false;

   switch (event->eType)
   	{
      case keyDownEvent:
      
      	if (EvtKeydownIsVirtual (event)) {
				if (event->data.keyDown.chr == vchrPageUp) {
					ScrollPage (winUp, ExportMemoMemoField, ExportMemoScrollScrollBar);
					handled = true;
					} 
				else if (event->data.keyDown.chr == vchrPageDown)  {
					ScrollPage (winDown, ExportMemoMemoField, ExportMemoScrollScrollBar);
					handled = true;
					}
				}
         break;
         
      case ctlSelectEvent:
        
        switch (event->data.ctlSelect.controlID)
         	{
          	case ExportMemoDoneButton:
   				FrmGotoForm (IndiSummForm);
         		handled = true;
            	break;

          	case ExportMemoCancelButton:
	      		if (FrmCustomAlert (ConfirmAlert, "", "delete this memo", "")==0) {
 						FieldPtr fld = GetObjectPtr (ExportMemoMemoField);
	         		FldDelete (fld, 0, FldGetTextLength (fld)); // erase the fld
	          		FrmGotoForm (IndiSummForm);
	          		}
         		handled = true;
            	break;
            				
     			default:
     				break;       
            }
         break;

      case fldChangedEvent:
         UpdateScrollBar (ExportMemoMemoField, ExportMemoScrollScrollBar);
         handled = true;
         break;
      
      case menuEvent:
         return false;
      
      case sclRepeatEvent:
         ScrollField (event->data.sclRepeat.newValue - event->data.sclRepeat.value,
         	false, ExportMemoMemoField, ExportMemoScrollScrollBar);
         // don't set handled to true
         break;

      case frmOpenEvent:
			ExportMemoInit ();
         handled = true;
         break;
			
      case frmUpdateEvent:
      	DrawForm ();
      	handled = true;
			break;
 
      case frmCloseEvent:
			ExportMemoSaveMemo ();
         // don't set handled to true.
         break;

      default:
      	break;
   	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      UpdateScrollBar
//
// DESCRIPTION:   Updates the scroll bar for the note viewer.
//
// PARAMETERS:    -> fldId			- 	id of field to scroll.
//
//						-> sclbarId		-	id of scroll bar for field to scroll.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void UpdateScrollBar (UInt16 fldId, UInt16 sclbarId)
{
   UInt16	scrollPos;
   UInt16	textHeight;
   UInt16	fieldHeight;
   Int16		maxValue;

   FldGetScrollValues (GetObjectPtr (fldId), &scrollPos, &textHeight, &fieldHeight);
   	
 	if (textHeight > fieldHeight)
		maxValue = textHeight - fieldHeight;
 	else      
	  	maxValue = scrollPos ? scrollPos : 0;
 
 	SclSetScrollBar (GetObjectPtr (sclbarId), scrollPos, 0, maxValue, fieldHeight-1);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ScrollField
//
// DESCRIPTION: 	This routine scrolls the Note View by the specified
//					 	number of lines.
//
// PARAMETERS:  	-> linesToScroll  -	the number of lines to scroll, positive for
//					 								down,	negative for up.
//
//					 	-> updateSclbar 	-	true to force a scrollbar update.
//
//						-> fldId				- 	id of field to scroll.
//
//						-> sclbarId			-	id of scroll bar for field to scroll.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ScrollField (Int16 linesToScroll, Boolean updateSclbar, UInt16 fldId,
 	UInt16 sclbarId)
{
	FieldPtr	fld = (FieldPtr) GetObjectPtr (fldId);

   if (linesToScroll < 0)
      FldScrollField (fld, -linesToScroll, winUp);
   else if (linesToScroll > 0)
      FldScrollField (fld, linesToScroll, winDown);

   if (updateSclbar)
   	UpdateScrollBar (fldId, sclbarId);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ScrollPage
//
// DESCRIPTION:   This routine scrolls the message a page up or down.
//
// PARAMETERS:    -> direction - up or down
//
//						-> fldId		 - id of field to scroll.
//
//						-> sclbarId	 -	id of scroll bar for field to scroll.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ScrollPage (WinDirectionType direction, UInt16 fldId,	UInt16 sclbarId)
{
   Int16 		linesToScroll;
   FieldPtr 	fld = (FieldPtr) GetObjectPtr (fldId);

   if (FldScrollable (fld, direction)) {
      linesToScroll = FldGetVisibleLines (fld) - 1;

		if (direction == winUp)
			linesToScroll = -linesToScroll;
		
		ScrollField (linesToScroll, true, fldId, sclbarId);
      }
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RomVersionCompatible
//
// DESCRIPTION:   This routine checks that a ROM version is meet your
//                minimum requirement.
//
// PARAMETERS:    requiredVersion - minimum rom version required
//                                  (see sysFtrNumROMVersion in SystemMgr.h 
//                                  for format)
//                launchFlags     - flags that indicate if the application 
//                                  UI is initialized.
//
// RETURNED:      Error code or zero if rom is compatible
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet (sysFtrCreator, sysFtrNumROMVersion, &romVersion);

	if (romVersion < requiredVersion) {
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {

			FrmAlert (RomIncompatibleAlert);
		
			// Palm OS 1.0 will continuously relaunch this app unless we switch to 
			// another safe one.
			if (romVersion < ROM_MIN_VER) {
				AppLaunchWithCommand (sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch,
					NULL);
				}
			}
		return sysErrRomIncompatible;
		}
	return errNone;
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      OpenDatabase
//
// DESCRIPTION:   Open the databases that have the given user database given.
//
// PARAMETERS:   	*userDBName - pointer to 6 character database name
//										  chosen by user.
//						mode			- mode in which to open the database
//						location 	- volume or card number where db is located
//						onExpCard 	- true if db is on an Expansion Card
//
// RETURNED:      Err - zero if no error, else 1
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Err OpenDatabase (Char* userName, UInt16 mode, UInt16 location, Boolean onExpCard)
{
   LocalID	dbID = NULL;
   UInt16	x;
	Char 		filePathName[OUR_FNAME_LEN+1];
	Char		uName[USER_DB_NAME_SZ+1];
	
	ErrFatalDisplayIf (!filePathName, "Out of memory");
	StrCopy (uName, userName);
	
 	if (OpenDbRef || OpenFileRef) { // close open database first
     	FrmCloseAllForms (); // NOTE: this will erase DatabaseList array!!!
     	CloseDatabase ();
     	}
	
	if (onExpCard) { // open file on Expansion Card
		// construct entire file name, including path and extension
		StrPrintF (filePathName, cFileNameStr, cPalmPath, uName, APP_CREATOR_STR);
		if (VFSFileOpen (location, filePathName, vfsModeRead, &OpenFileRef))
			goto BadExit;
		}
	else { // open file in RAM
		StrPrintF (filePathName, cDbNameStr, uName, APP_CREATOR_STR);
		dbID = DmFindDatabase (location, filePathName); // find the database
		if (!dbID) goto BadExit;
		OpenDbRef = DmOpenDatabase (location, dbID, mode);
		}

	if (!OpenDbRef && !OpenFileRef) goto BadExit;

	StrCopy (DbName, uName); // save the open database name
	FileLoc = location;
	Prefs[LastFileOnCrd] = onExpCard;
	IndiDBNumRecs = IndiDbNumRecords (); // save number of IndiDB Records

	// init certain global variables
	//ListViewSelectRecN = RelCalcRecN2 = NO_REC; REMOVED 12/22/04 as RelCalcRecN2 should not
	// be set to NO_REC upon starting up program.
		
	ListViewSelectRecN = NO_REC;
	TopVisIndiRecN 	 = 0;
		
	if (IndiDBNumRecs == 0)
		goto BadExit;
	
	// The following will initialize the Jump array to NO_REC in the situaion
	// where user transfers a database with same name as prior *open* database and
	// where that new database is smaller than the last database.
   for (x = 0; x < JUMP_MAX; x++) {
   	if (Jump[x] != NO_REC && Jump[x] >= IndiDBNumRecs) {
   		for (x = 0; x < JUMP_MAX; x++)
   			Jump[x] = NO_REC;
	   	break;
	   	}
   	}

 	return 0;

	///////
	BadExit:
	///////
	CloseDatabase ();
	*DbName = '\0';
	FileLoc = 0;
	Prefs[LastFileOnCrd] = false;
	ErrNonFatalDisplay ("OpenDatabase: Error openning file");
	return 1; // signal error
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      CloseDatabase
//
// DESCRIPTION:   Close the application's database.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void CloseDatabase (void)
{

	#ifdef ERROR_CHECK_FULL
	if (ChilRecH || FamiRecH || IndiRecH || SouCRecH || SourRecH ||
		RepCRecH || RepoRecH || NoteRecH || EvenRecH || DVLines)
      ErrDisplay ("Handle Left Open!!!");
	#endif

	if (OpenDbRef) {
		DmCloseDatabase (OpenDbRef);
		OpenDbRef = NULL;
		}
	else if (OpenFileRef) {
		VFSFileClose (OpenFileRef);
		OpenFileRef = NULL;
		}

	*DbName = '\0';
	
	// Reinit SrchArray (in case we got here after a Global Find)
	SrchArrayData = SrchNoData;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:     	StartApplication
//
// DESCRIPTION:	This routine loads the application's preferences and sets the 
//						screen depth and color mode.
//
// PARAMETERS:   	None.
//
// RETURNED:     	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void StartApplication (void)
{
   Err 				err;
   UInt32 			supportedDepth, currentDepth;
	UInt32 			osVersion;
	UInt32			grafValue;
	UInt32			valueP;
	UInt16			cardNo;
	LocalID			appID;

	// Init Jump array
	MemSet (Jump, sizeof (Jump), 0xFF);

	// Check if using Pre OS 3.5. If so we need to draw forms differently later.
	FtrGet (sysFtrCreator, sysFtrNumROMVersion, &osVersion);
	
	if (osVersion < ROM_35_VER)
		Pre35Rom = true;
	else if (osVersion < ROM_60_VER)
		Pre60Rom = true;

	// Check if Expansion Manager and VSF Manager is installed
	err = FtrGet (sysFileCExpansionMgr, expFtrIDVersion, &osVersion);
	if (!err) { // then Expansion Manager is installed
		err = FtrGet (sysFileCVFSMgr, vfsFtrIDVersion, &osVersion);
		if (!err) { // then VSF Manager is installed
			ExpCardCapable = true;
			
		 	// Register for Card Removal/Insertion notifications
	   	if (SysCurAppDatabase (&cardNo, &appID) == 0) {
   			SysNotifyRegister (cardNo, appID, sysNotifyVolumeMountedEvent,   NULL,
   				sysNotifyNormalPriority, NULL);
    			SysNotifyRegister (cardNo, appID, sysNotifyVolumeUnmountedEvent, NULL,
    				sysNotifyNormalPriority, NULL);
				}
      	}
		}

	// Get max supported screen depth
  	err = WinScreenMode (winScreenModeGetSupportedDepths, NULL, NULL,
  				&supportedDepth, NULL);
	ErrNonFatalDisplayIf (err, "WinScreenMode Error - Supported Depth");

	// Get current screen depth
   err = WinScreenMode (winScreenModeGet, NULL, NULL, &currentDepth, NULL);
	ErrNonFatalDisplayIf (err, "WinScreenMode Error - Current Depth");

	// Check if color is supported
  	err = WinScreenMode (winScreenModeGetSupportsColor, NULL, NULL, NULL,
  				&SupportsColor);
	ErrNonFatalDisplayIf (err, "WinScreenMode Error - Color Check");
   	
  	// Make sure screen depth is set to a minimum level based on supported depth
  	if (supportedDepth >= 0x808A && currentDepth < 16) {
  		currentDepth = 16;
  		err = WinScreenMode (winScreenModeSet, NULL, NULL, &currentDepth, NULL);
		ErrNonFatalDisplayIf (err, "WinScreenMode Error - Set Depth 1");
		}
  	else if (supportedDepth >= 0x8A && currentDepth < 8) {
  		currentDepth = 8;
  		err = WinScreenMode (winScreenModeSet, NULL, NULL, &currentDepth, NULL);
		ErrNonFatalDisplayIf (err, "WinScreenMode Error - Set Depth 2");
		}
	else if (supportedDepth >= 0x03 && currentDepth < 2) {
   	currentDepth = 2;
   	err = WinScreenMode (winScreenModeSet, NULL, NULL, &currentDepth, NULL);
		ErrNonFatalDisplayIf (err, "WinScreenMode Error - Set Depth 3");
		}
		
	// Check if device has dynamic Pen Input Area. DTR: 8/15/2004
	err = FtrGet (pinCreator, pinFtrAPIVersion, &osVersion);
	if (!err && osVersion) {
		
		DynInDevice = true;
		
		// Register for screen resize.
		if (SysCurAppDatabase (&cardNo, &appID) == 0)
			SysNotifyRegister (cardNo, appID, sysNotifyDisplayResizedEvent,   NULL,
   			sysNotifyNormalPriority, NULL);
		}

	// Check if device uses Graffiti 2  DTR: 10/25/2003
	if ((FtrGet ('grft', 1110, &grafValue) == errNone) || 
     	(FtrGet ('grf2', 1110, &grafValue) == errNone))
     	Graf2Device = true;	

	// Check if one-handed navigation is supported.
	err = FtrGet(sysFtrCreator, sysFtrNumFiveWayNavVersion, &valueP); // DTR: 12/20/2005
	if (err == errNone) 
   	FivWayDevice = true; // really just needed for focus rings around buttons


	// Get default draw color (do this before AppLoadPrefs).
	if (!Pre35Rom) {
		WinIndexToRGB (UIColorGetTableEntryIndex (UIObjectForeground), &Color1);
		Color2 = Color1;
		}

   // Load the application preferences
	AppLoadPrefs (false);

	// Mask off the key to avoid repeat keys causing clicking sounds
	KeySetMask (~KeyCurrentState());

	return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      StopApplication
//
// DESCRIPTION:   Close the application's database files and save the
//                current state of the application.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void StopApplication (void)
{
	LocalID		appID;
   UInt16		cardNo;

	// Write the preferences / saved-state information.
	AppSavePrefs();

	// Close all the open forms.
	FrmCloseAllForms ();

	// Close the application's data file.
	CloseDatabase();
	
	#ifdef ERROR_CHECK_FULL
	if (ChilRecH || FamiRecH || IndiRecH || SouCRecH || SourRecH ||
		RepCRecH || RepoRecH || NoteRecH || EvenRecH)
      ErrDisplay ("Handle Left Open!!!");
	#endif
	
	WinScreenMode (winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);

  	SysCurAppDatabase (&cardNo, &appID);
  	
	// Unregister for Card Removal/Insertions
  	if (ExpCardCapable) {
    	SysNotifyUnregister(cardNo, appID, sysNotifyVolumeMountedEvent,   sysNotifyNormalPriority);
    	SysNotifyUnregister(cardNo, appID, sysNotifyVolumeUnmountedEvent, sysNotifyNormalPriority);
    	}
    	
	// Unregister for screen resizes.
	if (DynInDevice) {
		SysNotifyUnregister(cardNo, appID, sysNotifyDisplayResizedEvent, sysNotifyNormalPriority);
		}
}

#pragma warn_a5_access on
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 		AppInfoGetPtr
//
// DESCRIPTION: 	Return a locked pointer to the AppInfo or NULL.
//  					This routine must not use global variables.
//
// PARAMETERS: 	-> dbP  - open database pointer
//						-> dbID - database ID of a closed database
//
// RETURNS: 		Locked ptr to the AppInfo block or NULL
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
DBInfoPtr AppInfoGetPtr (DmOpenRef dbP, LocalID dbID)
{
   UInt16   cardNo;
   LocalID  appInfoID;
   Err		err;
      
   if (dbID == NULL) {
   	// get database ID and card number.
   	if (DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL))
      	return NULL;
     	}

	// Get the application info block ID.
	for (cardNo = 0; cardNo < MemNumCards (); cardNo++) {
		err = DmDatabaseInfo (cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL,
  					NULL, &appInfoID, NULL, NULL, NULL);
		if (!err) break;
		}
 			
	if (err)
		return NULL;

   if (appInfoID == NULL)
      return NULL;
   else // lock the pointer to the database.
      return MemLocalIDToLockedPtr (appInfoID, cardNo);
}   
#pragma warn_a5_access off

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DbGetIndexes
//
// DESCRIPTION:   Get the beginning index and ending index for a mini-database
//						within the mega-database.  It opens the
//						Application Info Block to retreive the index information.
//						This routine cannot use global variables as it is called by
//						the Search function. This is why we pass the openDBRef 
//						parameter instead of using the global OpenDBbef.
//
// PARAMETERS:    openDbRef  - reference number for open database
// 					dbType  - the mini-database type
//            	   indexStart - the position of first record in a mini-db
//            	   indexEnd - one record past the last record in a mini-db
//
// RETURNS: 	   Nothing.
//
// REVISION:		None.
////////////////////////////////////////////////////////////////////////////////////
void DbGetIndexes (DmOpenRef openDbRef, CmbDBases dbType, UInt16 *indexStart,
	UInt16 *indexEnd)
{
	DBInfoPtr	dbInfoP = NULL;
	MemHandle	memH;

	// Get pointer to application information for the open database. If this function 
	// is called from the Search routine, then it must have an "openDbRef" so that 
	// no global variables will be accessed.
	if (openDbRef)
   	dbInfoP = AppInfoGetPtr (openDbRef, NULL);
	else
		dbInfoP = VfsAppInfoGetPtr (OpenFileRef, FileLoc, DbName);

	ErrFatalDisplayIf (dbInfoP == NULL, "Error: Null pointer in DbGetIndexes");

	*indexStart = dbInfoP->startDB[dbType];

	if (dbType == NoteDB) {
		if (openDbRef)
			*indexEnd = DmNumRecords (openDbRef);
		else // if (OpenFileRef)
			VFSFileDBInfo (OpenFileRef, NULL, NULL, NULL, NULL, NULL, NULL, 
				NULL, NULL, NULL, NULL, NULL, indexEnd);
		}
	else
		*indexEnd   = dbInfoP->startDB[dbType+1];

	if (openDbRef)
		MemPtrUnlock (dbInfoP);
	else {
		memH = MemPtrRecoverHandle (dbInfoP); // revised 10-08-2004
		MemHandleUnlock (memH); // revised 10-08-2004
		MemHandleFree (memH); // revised 10-08-2004
		}
}

#pragma mark-
#pragma warn_a5_access on
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      Search
//
// DESCRIPTION:   This routine searchs the the address database for records 
//                contains the string passed. 
//  					This routine must not use global variables.
//
// PARAMETERS:    <-> findParams	- parameters for Palm Find routine
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void Search (FindParamsPtr findparams)
{
   Boolean              done;
   Boolean              match;
   DmOpenRef            dbP = NULL;
   DmSearchStateType    searchState;
   Err                  err = 0;
   LocalID              dbID;
   RectangleType        rect;
   UInt16               cardNo;
   UInt16               recordNum;
   MemHandle            recordH = NULL;
   UInt16               i;
   UInt32               pos;
   UInt16					len;
 	Boolean					newSearch = true;
   Char						dbName[FULL_DB_NAME_LEN+1];
	Char						nHeader[20];
	Char*						cPos;
	Int16                prefsVersion;
	UInt16               prefsSize;
	AppPreferenceType    prefs;
	LocalID 					dbIdOld = 0;
	Boolean					cont;
	PackedDBRecord3   	*src;
	PackedDBRecord2   	*src2;
	UInt16					recPos;
	UInt16					recCnt;
	UInt16					megaRecSz;
	Char*						keyPos;
	UInt16					recSize;
   UInt16           		index; // mega-record number
   UInt16            	probe; // mini-record number
  	UInt16					indexStart, indexEnd;
   DBInfoPtr  				dbInfoP;
	UInt32   				flags; // field flags for IndiDBRec
	Char*						fldP; 

	#ifdef GREMLINS_NO_SEARCH
	return;
	#endif

   // Read the preferences / saved-state information.
	prefsSize = sizeof (AppPreferenceType);
	prefsVersion = PrefGetAppPreferences (APP_CREATOR, APP_PREF_ID,
		&prefs, &prefsSize, true);
	if (prefsVersion > APP_PREF_VERS_NO) {
		prefsVersion = noPreferenceFound;
	   }
	
	if (prefsVersion > noPreferenceFound)	{
		if (prefs.PrefFlds[ExcludeFind]) {
      	findparams->more = false;
      	return;
			}
		if (findparams->continuation)
			dbIdOld = prefs.dbIDOld;
		}
	else {
      findparams->more = false;
      return;
		}

	cont = findparams->continuation;
	
	while (!err) {

   	// Find the application's data file.
		do {
   		err = DmGetNextDatabaseByTypeCreator (newSearch, &searchState,
   			APP_DB_TYPE,	APP_CREATOR, false, &cardNo, &dbID);
 			newSearch = false;
   		if (err) break;
   		} while (dbIdOld != dbID && cont); 
   	
   	if (err) {
      	findparams->more = false;
      	return;
      	}

   	cont = false;
   	dbIdOld = dbID; 

		// Check if database is wrong version
		dbInfoP = AppInfoGetPtr (NULL, dbID);
		if (dbInfoP == NULL)
			continue;
		if (dbInfoP->dbVers != APP_DB_VERS) {
			MemPtrUnlock (dbInfoP);
			continue;
			}
		MemPtrUnlock (dbInfoP);

   	// Open the database.
    	ErrFatalDisplayIf (dbP, "Search: Database already open");
   	dbP = DmOpenDatabase (cardNo, dbID, findparams->dbAccesMode);
   	if (!dbP) {
   		findparams->recordNum = 0;
  			recordNum = 0;
   	 	continue; // then try next database
   	 	}
   		
   	// Display the heading line.
   	DmDatabaseInfo (cardNo, dbID, dbName, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL);
			
		// get user's file name for database
		cPos = StrChr (dbName, chrHyphenMinus);
		*cPos = '\0';
				
   	StrPrintF (nHeader, "GedWise [%s] ", dbName); // create header
   	
   	done = FindDrawHeader (findparams, nHeader);
   	if (done) 
      	goto Exit;
	
		// Get starting and ending index of mini-DB within mega-DB. The variable dbP
		// must have a value at this point or else we would never have gotten here.
		DbGetIndexes (dbP, IndiDB, &indexStart, &indexEnd);

   	// Search the description and note fields for the "find" string.
   	recordNum = findparams->recordNum;

   	while (true) { // read records from open database
  	   	
  	   	DBRecordType     record;
      	//	Stop the find if an event is pending.
      	// This stops if the user does something with the device. Because
      	// this call slows down the search we perform it every so many 
      	// records instead of every record.  The response time should still
      	// be short without introducing much extra work to the search.
      
      	//if ((recordNum & 0x000f) == 0 &&   // every 16th record
         if (EvtSysEventAvail (true)) {
         	// Stop the search process.
         	findparams->more = true;
         	goto Exit;
      		}

			index = indexStart + recordNum / DB_REC_MAX;
			probe = recordNum % DB_REC_MAX;
     
     		if (index >= indexEnd) {
     			recordH = NULL;
     			}
     		else {
        		recordH = DmQueryRecord (dbP, index);
      		if (recordH)
      			src = MemHandleLock (recordH);
      		}

      	// Have we run out of records?
      	if (!recordH) {
      		findparams->recordNum = 0;
  				recordNum = 0;
  				DmCloseDatabase(dbP);
      		dbP = NULL;
         	break;
      		} // if

			megaRecSz = (UInt16) MemHandleSize(recordH);
			keyPos = (char *) &src->recSize; // get ptr to mini-record size info
			recPos = 0; // initialize to include sortNo field
			recCnt = 0;

			// Locate recordNum we left off at.
			while (recPos < megaRecSz) {
				if (recCnt == probe)
					break; // success!
				recSize = *((UInt16*) keyPos);
				keyPos+= recSize;	// advance to next mini-record
				recCnt++;
				recPos+= recSize;
				} // while
			
   		// If we reach here and we went beyond current megarecord size
   		// then it must mean we are also at end of the database. YES.. this
   		// code is definitely needed...DTR 8/22/2001
   		if (recPos >= megaRecSz) {
       		findparams->recordNum = 0;
  				recordNum = 0;
   			MemHandleUnlock (recordH);
   			recordH = NULL;
  				DmCloseDatabase (dbP);
      		dbP = NULL;
    			break;
   			}

			// Search the rest of mega-record for a match //
			while (recPos < megaRecSz) {

      		// Search the name of the individual record.
				src2 = (PackedDBRecord2*) (keyPos);
      		
      		// Fill in the searchable fields of an IndiDBRec.  We must get all
      		// fields up to title, as we may need to display the name later.
     			flags = src2->flags.allBits;
   			fldP = &src2->firstField;
         
   			for (i = 0; i < indiDBSearchFlds; i++) {
      			if (GetBitMacro(flags, i) != 0) {
         			record.fields[i] = fldP;
         			fldP += StrLen (fldP) + 1;
         			}
      			else
         			record.fields[i] = NULL;
      			}
      		
      		match = false;
		      for (i = 0; i < indiDBSearchFlds; i++) {
         		if (i == indiSex || i == indiAlias) continue;
         		if (record.fields[i]) {
            		match = TxtFindString (record.fields[i], findparams->strToFind,
            		 			  &pos, &len);
            		if (match) break;
         			}
      			}
      
      		if (match) {
         		done = FindSaveMatch (findparams, recordNum, pos, i, 0, cardNo, dbID);
         		if (done) { // then can't display record info b/c screen is full.
            		MemHandleUnlock (recordH);
            		recordH = NULL;
            		goto Exit;
         			}

         		// Get the bounds of the region where we will draw the results.
         		FindGetLineBounds (findparams, &rect);

         		// Display the title of the description.
		   		FntSetFont (stdFont);
         		DrawRecordNameAndLifespan (&record, &rect, true, true);
         		findparams->lineNumber++;
	      		}

				recSize = *((UInt16*) keyPos);
				keyPos+= recSize;	// advance to next mini-record
				recPos+= recSize;
				if (recPos < megaRecSz)
					recordNum++;

				} // while (recPos < megaRecSz) loop

      	MemHandleUnlock (recordH);
      	recordH = NULL;
      	recordNum++;
      	
   		} // while (true) loop
   
	} // main while loop

	////
	Exit:
	////
	
   if (dbP) DmCloseDatabase (dbP);   
	prefs.dbIDOld = dbIdOld;
	// write the preferences.
	PrefSetAppPreferences (APP_CREATOR, APP_PREF_ID, APP_PREF_VERS_NO, &prefs, 
		sizeof (AppPreferenceType), true);

	ErrFatalDisplayIf (recordH, "Search: recordH is locked");

}
#pragma warn_a5_access off

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      GoToItem
//
// DESCRIPTION:   This routine is an entry point of this application.
//                It is generally call as the result of tapping on a
//                name in the Global Search dialog.
//
// PARAMETERS:    gotoParams - system defined parameter block
//						launchiApp - true if program is getting lauched so we
//										 know whether or not to close any open db.
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
   EventType 	event;
   Char			dbName[FULL_DB_NAME_LEN+1];
   Char*			cPos;

	// If database is already open then close it, as we don't know if 
	// record we are going to is in the open database.
	if (! launchingApp) {
		FrmCloseAllForms ();
		CloseDatabase ();
		}
		
	// Get the database name and open it.
	if (DmDatabaseInfo (goToParams->dbCardNo, goToParams->dbID, dbName, NULL,
		NULL, NULL, NULL, NULL, NULL,	NULL, NULL, NULL, NULL))
		return;
		
	// Get user's file name for database
	cPos = StrChr (dbName, chrHyphenMinus);
	*cPos = '\0';

   if (OpenDatabase (dbName, dmModeReadOnly, goToParams->dbCardNo, false)) {
      FrmAlert (DBOpenAlert);
      FrmGotoForm (DatabasesForm);
      return;
		}
		
   // Set global variables that keep track of the currently record.
   CurrentIndiRecN = goToParams->recordNum;
   
   MemSet (&event, sizeof (EventType), 0);

   // Send an event to load the form we want to goto.
   event.eType = frmLoadEvent;
   event.data.frmLoad.formID = IndiSummForm;
   EvtAddEventToQueue (&event);

   // Send an event to goto a form and select the matching text.
   event.eType = frmGotoEvent;
   event.data.frmGoto.formID = IndiSummForm;
   event.data.frmGoto.recordNum = goToParams->recordNum;
   event.data.frmGoto.matchPos = goToParams->matchPos;
   event.data.frmGoto.matchLen = goToParams->searchStrLen;
   event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
   EvtAddEventToQueue (&event);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    EventLoop
//
// DESCRIPTION: This routine is the event loop for the application.  
//
// PARAMETERS:  None.
//
// RETURNED:    Nothing.
//
// REVISIONS:	 None.
////////////////////////////////////////////////////////////////////////////////////
static void EventLoop (void)
{
  EventType 	event;
  UInt16 		error;
   do
      {
      EvtGetEvent (&event, evtWaitForever);
       if (! SysHandleEvent (&event))
         if (! MenuHandleEvent (0, &event, &error))
            if (! ApplicationHandleEvent (&event))
               FrmDispatchEvent (&event); 
      }
   while (event.eType != appStopEvent);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	AppPilotMain
//
// DESCRIPTION: 	This is the main entry point for the GedWise application.
//						Note: We need to create a branch island to PilotMain in order
// 					to successfully link this application for the device.
//
// PARAMETERS:  	-> cmd - launch command
//						-> cmdPBP -command information
//						-> launchFlags - launch flags from system.
//
// RETURNED:    	Error code in error
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static UInt32   AppPilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err 			error = errNone;
	EventType 	resizedEvent;

   // Check for minimum ROM version
   error = RomVersionCompatible (ROM_MIN_VER, launchFlags);
	if (error) return (error);

   switch (cmd)
   	{
      case sysAppLaunchCmdNormalLaunch:

         StartApplication ();
   		FrmGotoForm (SplashForm); 
   		
        EventLoop ();
         StopApplication ();         
         break;
		
      case sysAppLaunchCmdFind:
         Search ((FindParamsPtr) cmdPBP);
         break;
		
      // This action code could be sent to the app when it's already running.
      case sysAppLaunchCmdGoTo:
         {
         Boolean launched;
         
         // init in case we exit Relationship Calc via Global Find
         RelCalcEntry = false;
           
         launched = launchFlags & sysAppLaunchFlagNewGlobals;

         if (launched)
         	StartApplication();

         GoToItem ((GoToParamsPtr)cmdPBP, launched);

         if (launched) {
            EventLoop ();
            StopApplication ();   
         	}      
         }
         break;

		// This action code is sent to the app when an expansion card is removed
	  	case sysAppLaunchCmdNotify:
			switch (((SysNotifyParamType *)cmdPBP)->notifyType)
         	{
				case sysNotifyVolumeMountedEvent:
					
					// update the DatabasesForm if it is open.
					if (FrmGetActiveFormID () == DatabasesForm)
						FrmGotoForm (DatabasesForm);
						
               // make sure we don't call default application when inserting a card
               ((SysNotifyParamType *)cmdPBP)->handled = ((SysNotifyParamType *)cmdPBP)->handled | vfsHandledUIAppSwitch;
					break;
					
				case sysNotifyVolumeUnmountedEvent:
              	
              	if (OpenFileRef) { // if db on card is open, then close everything
						FrmCloseAllForms ();
						CloseDatabase ();
			   		FrmGotoForm (DatabasesForm);
						}
					else if (FrmGetActiveFormID () == DatabasesForm) {
						FrmGotoForm (DatabasesForm);
						}
               break;
				
				case sysNotifyDisplayResizedEvent:
					
					MemSet (&resizedEvent, sizeof (EventType), 0);
				
					// Add winDisplayChangedEvent to the event queue
					resizedEvent.eType = winDisplayChangedEvent;
					EvtAddUniqueEventToQueue (&resizedEvent, 0, true);
					break;
				}
			break; // case sysAppLaunchCmdNotify
					
		default:
			break;
		}
		
   return error;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AppLoadPrefs
//
// DESCRIPTION:   Load the application preferences.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void AppLoadPrefs (Boolean reloadPrefs)
{
	Int16              	prefsVersion;
	UInt16             	prefsSize;
	AppPreferenceType  	prefs;
	UInt16					x;

   // Read the preferences / saved-state information.
	prefsSize = sizeof (AppPreferenceType);
	prefsVersion = PrefGetAppPreferences (APP_CREATOR, APP_PREF_ID,
	 &prefs, &prefsSize, true);
	
	if (prefsVersion != APP_PREF_VERS_NO)
		prefsVersion = noPreferenceFound;
	
	if (prefsVersion > noPreferenceFound)	{
		// Values not set here are left at their default values
		// Write the preferences / saved-state information.
		InstallDate       = prefs.InstallDate;
		RelCalcRecN2 		= prefs.RelCalcRecN2;
		StrCopy (DbName, 	  prefs.LastFileName);
		FileLoc				= prefs.LastFileLoc;

		for (x = 0; x < TotPrefFnts; x++)
			PrefFnts[x] = prefs.PrefFonts[x];

		for (x = 0; x < TotPrefFlds; x++)
			Prefs[x] = prefs.PrefFlds[x];

		for (x = 0; x < JUMP_MAX; x++)
			Jump[x] = prefs.Jump[x];

		Color1	= prefs.Color1;
		Color2	= prefs.Color2;

		if (!reloadPrefs) // only reload CurrentIndiRecN when starting program
			CurrentIndiRecN  	= prefs.LastRecN;
		}
	else {
		AppSavePrefs (); // app must have just been installed
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:		AppSavePrefs
//
// DESCRIPTION:	Save the GedWise preferences.
//
// PARAMETERS:		None.
//
// RETURNED:		Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void AppSavePrefs (void)
{
	AppPreferenceType prefs;
	UInt16 				x;

	if (RelCalcEntry) // user must have exited app from Rel Calc indi list
		CurrentIndiRecN =	HldIndiRecN;

	// Write the preferences / saved-state information.
	prefs.LastRecN 			= Prefs[ReturnLast] ? CurrentIndiRecN : NO_REC;
	prefs.RelCalcRecN2		= RelCalcRecN2;
	prefs.dbIDOld				= 0;
	prefs.LastFileLoc			= FileLoc;
	StrCopy (prefs.LastFileName, DbName);

	for (x = 0; x < TotPrefFnts; x++)
		prefs.PrefFonts[x] = PrefFnts[x];

	for (x = 0; x < TotPrefFlds; x++)
		prefs.PrefFlds[x] = Prefs[x];

	for (x=0; x < JUMP_MAX; x++)
		prefs.Jump[x] = Jump[x];
		
	prefs.Color1	= Color1;
	prefs.Color2	= Color2;
	
	#ifndef REGISTERED
   if (InstallDate == 0)  // if just installed
      prefs.InstallDate    = TimGetSeconds();
   else
	#endif
      prefs.InstallDate    = InstallDate;
	
	// Write the state information.
	PrefSetAppPreferences (APP_CREATOR, APP_PREF_ID, APP_PREF_VERS_NO, &prefs, 
		sizeof (AppPreferenceType), true);
}
