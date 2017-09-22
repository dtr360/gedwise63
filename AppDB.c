////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise 6.0 & 6.1
//
// FILE:          AppDB.c
//
// AUTHOR:        Daniel T. Rencricca: July 15, 2004
//
// DESCRIPTION:   Application database routines.
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2004 Battery Park Software Corporation.  
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////
 
#include "Defines.h"
#include <PalmOS.h>
#include <VFSMgr.h>

#include "AppMain_res.h"
#include "AppMain.h"
#include "AppMisc.h"
#include "AppDB.h"

////////////////////////////////////////////////////////////////////////////////////
// Externally defined variables
////////////////////////////////////////////////////////////////////////////////////
extern Char				cErrorNoRec[18];         // MUST MATCH SIZE IN APPMAIN
extern Char*			EvenDesc[TOT_EVEN_L][2]; // MUST MATCH SIZE IN APPMAIN
extern Char*			Months[12]; 				 // MUST MATCH SIZE IN APPMAIN
extern DmOpenRef		OpenDbRef;
extern FileRef			OpenFileRef;
extern UInt16			FileLoc;
extern UInt16			IndiDBNumRecs;
extern UInt16			CurrentIndiRecN;
extern UInt32			CurrentEvenRecN;
extern UInt32			CurrentFamiRecN;
extern DbArrayType* 	DatabaseList;
extern UInt16			TotDatabases;
extern Boolean			Prefs[];
extern Char 			cUnknownStr[];
extern Char				DbName[];
extern Boolean			ExpCardCapable;
extern Char				cPalmPath[];
extern Char 			cDbNameStr[];
extern Char				cFileNameStr[];
extern UInt16			PriorFormID;
extern UInt16			Jump[JUMP_MAX];
extern Boolean			Pre35Rom;
extern UInt16			RelCalcRecN2;
extern Boolean			SupportsColor;
extern Boolean			DynInDevice;

////////////////////////////////////////////////////////////////////////////////////
// Local Variables
////////////////////////////////////////////////////////////////////////////////////
static UInt16	TotMemory;	// total memory usage by databases
static UInt16	FileRefNum[CPYDB_RN_AR_SZ];
static Char** 	ListArrayP = NULL;
static Char* 	LabelP;
static Char 	ListStr[CPYDB_LIST_SZ+1];
static UInt16  TopVisibleDBNum;
static Boolean ResetTopVisDBNum = true;
static UInt16 	UpdateCnt;
static UInt16 	UpdateTot;  // total updates that have been done to progress bar
static UInt16	DbNum; // position of database in DatabaseList array

static Char		SearchKey[SCH_KEY_LEN+1] = "\0"; // Use ONLY in DataSearch & FldSearch routines.
static Char		RecKeyD[SCH_REC_FLD_LEN+1];
static Boolean	ThisDaySearch 	 = true; // true if doing an "On This Day" search.

// Search options
static Boolean	SrchAll  = true;
static Boolean	SrchPlac = true;
static Boolean	SrchDate = false;
static Boolean	SrchAddr = false;
static Boolean SrchOpts[SrchOptTtl] = {false, false, false, false, false, false,
													false, false, false};
static UInt32	SrchArr[SCH_ARR_SZ] 	= {NO_REC_LONG};
static UInt16	TopVisSrchArrNum	 	= 0;	
static UInt16	SrchTotPages		 	= 0;	// total pages shown under current search
 		 SrchATp	SrchArrayData 			= SrchNoData;
		 Boolean	SrchShowEven 			= false;
		 Boolean SrchShowFamiEven 		= false;

// Local Constants
const  Char 	cHHStr[]					= DB_HH_STR;
const  Char		cBeamStr[]				= DB_BEAM_STR;
const	 Char		cCopyStr[]				= DB_COPY_STR;

////////////////////////////////////////////////////////////////////////////////////
//	Internal Function Prototypes																	 //
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	GetControlValue
//
// DESCRIPTION: 	This routine returns the value of a control.
//
// PARAMETERS:  	-> controlID - the ID of a control
//
// RETURNED:    	0 if control is off, else 1 if on.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Int16 GetControlValue (const UInt16 controlID)
{
   FormPtr	frm = FrmGetActiveForm ();
	return CtlGetValue (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm,	controlID)));
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
static void* GetObjectPtr (UInt16 objectID)
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
static void ShowObject (UInt16 objectID, Boolean showObj)
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

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
//  FUNCTION: 		StrCmpMatches
//
//  DESCRIPTION: 	Compares two strings and reports the number of matching characters
//						from the start of the strings.
//
//  PARAMETERS: 	s1, s2 - string pointers
//
//  RETURNS: 		number of matching characters between the two strings.
////////////////////////////////////////////////////////////////////////////////////
static Int16 StrCmpMatches (Char *s1, Char *s2)
{
   UInt16 matches = 0;

   ErrFatalDisplayIf (s1 == NULL, "StrCmpMatches: Error NULL string parameter"); 
   ErrFatalDisplayIf (s2 == NULL, "StrCmpMatches: Error NULL string parameter");
   TxtCaselessCompare(s1, StrLen(s1), &matches, s2, StrLen(s2), NULL);
      
   return matches;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiLookupFName
//
// DESCRIPTION:   Return the IndiDB record that contains the most of the string
//      		      passed in the "key" parameter.  This function is used do lookup
//						a given name once the user has already selected a surname.
//
// PARAMETERS:    -> key 				- 	string for we are searching.
//                <> foundRecN		- 	initially it contains the record with the 
//													surname in which we are seeking the given
//													name. Returned is the record number found
//													with a matching "key".
//                <- completeMatch	- 	true if a record contains all of the key and,
//													therefore, name should be highlighted.
//
// RETURNS: 	   True if complete or partial match found, else false. If true,
//						then a record should be highlighted in the individual list.
//
// REVISIONS:		Revised by DTR 5-13-2003
////////////////////////////////////////////////////////////////////////////////////
Boolean IndiLookupFName (Char *key, UInt16 *foundRecN, Boolean *completeMatch)
{
   MemHandle      rH1 = NULL;
   MemHandle      rH2 = NULL; // init
   DBRecordType	rec1, rec2;
   UInt16         kmin, i, probe, probe2;	// mini-record positions in database
   Int16          result; // result of comparing two mini-records
   Char*				lNameKey; // key from record at probe
   UInt16         matches1, matches2; 
	UInt32         remRecs;

   // If there isn't a record to search for the stop
	ErrNonFatalDisplayIf (*foundRecN == NO_REC, "IndiLookupFName: bad foundRecN");
	
   if (*foundRecN == NO_REC) {
      *completeMatch = true;
      return false;
      }

  	// Get the last name. We are only interested in finding records
	// with this same last name (lNameKey).
   DbGetRecord (IndiDB, *foundRecN, &rec1, &rH1);
   ErrNonFatalDisplayIf (!rH1, "IndiLookupFName: bad foundRecN");
   lNameKey = rec1.fields[indiLName];
   
   // If no lNameKey then error, so return false.
   if (lNameKey == NULL || *lNameKey == '\0') {
   	DbMemHandleUnlock (&rH1);
     	*completeMatch = true;
     	return false;
     	}

	kmin = probe = 0;	// init
	remRecs = IndiDBNumRecs; // init
	
	while (remRecs > 0) {

   	i = remRecs / 2;
    	probe = kmin + i;

	   DbMemHandleUnlock (&rH2);
      DbGetRecord (IndiDB, probe, &rec2, &rH2);
      ErrFatalDisplayIf (rH2 == NULL, "IndiLookupFName: bad probe #1");
            
      // compare the two keys
      if (rec2.fields[indiLName] == NULL) {
      	result = -1;
      	}
      else {
      	result = StrCaselessCompare (lNameKey, rec2.fields[indiLName]);

      	if (result == 0) {
      		if (key == NULL || *key == '\0') {
  		     		result = -1;
  		     		}
  		     	else if (rec2.fields[indiFName] == NULL)
         		result = 1;
				else      
        			result = StrCaselessCompare (key, rec2.fields[indiFName]);
				}
			}
			
       if (result <= 0)
         remRecs = i;
      else {
         kmin = probe + 1;
         remRecs = remRecs - i - 1;
         }
         
      }  // end of while loop

	if (result > 0) // make sure key <= record key at this point
      probe++;
	
   DbMemHandleUnlock (&rH2);

	// If user erased all prior letters then set foundRecN to current probe.
	if (key == NULL || *key == '\0') {
		*foundRecN = probe;
     	DbMemHandleUnlock (&rH1);
      *completeMatch = true;
      return true;
		}

	// Get the number of matches characters in the probe record
	if (probe >= IndiDBNumRecs) { // no matching letters
      matches1 = 0;
      }
   else { // count matching characters
  		DbGetRecord (IndiDB, probe, &rec2, &rH2);
		ErrFatalDisplayIf (rH2 == NULL, "IndiLookupFName: bad probe #2");

   	if (rec2.fields[indiFName] == NULL|| rec2.fields[indiLName] == NULL ||      
    		StrCaselessCompare (lNameKey, rec2.fields[indiLName]) != 0)
     		matches1 = 0;
   	else
      	matches1 = StrCmpMatches (key, rec2.fields[indiFName]);
		}

	// If probe is at record 0 then go no further
	if (probe == 0) {
		DbMemHandleUnlock (&rH1);
   	DbMemHandleUnlock (&rH2);
      if (matches1 > 0) { // go with probe as it has some letters in common
         *foundRecN = probe;
         *completeMatch = (matches1 == StrLen (key));
         return true;
         }
      else { // probe has no letters in common
         *completeMatch = false;
        	return false;
         }
      }

	DbMemHandleUnlock (&rH2);

  	// Check if the record before probe has more matching letters.
	probe2 = probe - 1; // note: probe must be > 0 if we got this far
  
   DbGetRecord (IndiDB, probe2, &rec2, &rH2);
   ErrFatalDisplayIf (rH2 == 0, "IndiLookupFName: bad probe #3");

   // Count the number of matching characters in probe2.
   if (rec2.fields[indiFName] == NULL || rec2.fields[indiLName] == NULL ||
     	StrCaselessCompare (lNameKey, rec2.fields[indiLName]) != 0)
      matches2 = 0;
   else
      matches2 = StrCmpMatches (key, rec2.fields[indiFName]);
         
  	DbMemHandleUnlock (&rH2);

   // Now, return the probe which has the most letters in common.
   if (matches1 > matches2) {
      *completeMatch = (matches1 == StrLen (key));
      *foundRecN = probe;
      }
   else { 
      if (matches1 == 0 && matches2 == 0) {
         *completeMatch = false; // no item with same first letter found
         }
      else { // first record matches as much as or more than the second
      	*foundRecN = probe2;
   		*completeMatch = (matches2 == StrLen (key));    
   		}
		}
		
	DbMemHandleUnlock (&rH1);
	
   return (*completeMatch);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiLookupLName
//
// DESCRIPTION:   Return the IndiDB record that contains the most of the string
//      		      passed in the "key" parameter. If no string is passed or there
//  			      aren't any records that match then false is returned.
//
// PARAMETERS:    -> key 				- 	string for we are searching.
//                <- recordP 			- 	to contain number of the record found.
//                <- completeMatch 	- 	true if a record contains all of the key and,
//													therefore, name should be highlighted.
//
// RETURNS: 	   True if complete or partial match found, else false. If true,
//						then a record should be highlighted in the individual list.
//
// REVISIONS:		Revised by DTR 5-13-2003
////////////////////////////////////////////////////////////////////////////////////
Boolean IndiLookupLName (Char* key, UInt16* recordP, Boolean* completeMatch)
{
   MemHandle      recH = NULL; // must init
   DBRecordType	rec;
   UInt16         kmin, probe, probe2, i;// all positions in the database.
   Int16          result;                // result of comparing two records.
   Char*          recordKey;				  // key from record at probe.
   UInt16         matches1, matches2;	  // number of maching characters
   UInt32         remRecs;
  
   // If there isn't a key to search with stop the with the first record.
   if (key == NULL || *key == '\0') {
      *completeMatch = true;
      return false;
      }
      
   remRecs = IndiDBNumRecs; // init
   kmin = probe = 0; // init
   
   while (remRecs > 0) {

      i = remRecs / 2;
      probe = kmin + i;

      DbMemHandleUnlock (&recH);        
      DbGetRecord (IndiDB, probe, &rec, &recH);
   	recordKey = rec.fields[indiLName];
      
      ErrFatalDisplayIf (!recH, "IndiLookupLName: Invalid probe record #1");
            
      // Compare the two keys.
      if (recordKey == NULL)
         result = 1;
      else
         result = StrCaselessCompare (key, recordKey);

      if (result <= 0)
         remRecs = i;
      else {
         kmin = probe + 1;
         remRecs = remRecs - i - 1;
         }
      }  // end of while loop

   if (result > 0) // make sure key <= recordKey at this point
      probe++;
      
   DbMemHandleUnlock (&recH);
   
   // The probe is now at the position where the string could be
   // inserted. It is in between two entries. Neither the record
   // before or after may have ANY letters in common. Go with the
   // record that has the most letters in common.
   
	if (probe >= IndiDBNumRecs) { // no matching letters
      matches1 = 0;
      }
   else {
		DbGetRecord (IndiDB, probe, &rec, &recH);
   	recordKey = rec.fields[indiLName];
		ErrFatalDisplayIf (!recH, "IndiLookupLName: Invalid probe record #2");
      
		// Count the number of matching characters in probe record. //
		if (recordKey == NULL)
     		matches1 = 0;
   	else
     		matches1 = StrCmpMatches (key, recordKey);
 
  		DbMemHandleUnlock (&recH);
     	} // else statement
   
   if (probe == 0) {
     	if (matches1 > 0) { // go with probe as it has some letters in common
        	*recordP = probe;
        	*completeMatch = (matches1 == StrLen (key));
        	return true;
        	}
     	else { // probe has no letters in common
        	*completeMatch = false;
        	return false;
        	}
     	}

	// Check if record before probe has more matching letters.
	probe2 = probe - 1;  // probe must be > 0 if we got this far
	
	DbGetRecord (IndiDB, probe2, &rec, &recH);
   recordKey = rec.fields[indiLName];
   ErrFatalDisplayIf (!recH, "IndiLookupLName: Invalid probe record #3");
   
   // Count the number of matching characters in probe2 record
   if (recordKey == NULL)
      matches2 = 0;
   else
      matches2 = StrCmpMatches (key, recordKey);
   
  	DbMemHandleUnlock (&recH);

   // Now, return the probe which has the most letters in common.
   if (matches1 > matches2) {
      *completeMatch = (Boolean) (matches1 == StrLen (key));
      *recordP = probe;
      }
   else {
      if (matches1 == 0 && matches2 == 0) {
         *completeMatch = false; // no item with same first letter found
         //return false; 
         }
      else { // first record matches as much as or more than the second
         *recordP = probe2;
	     	*completeMatch = (Boolean) (matches2 == StrLen (key));
      	}
		}
	
	return (*completeMatch);

}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      IndiDbNumRecords
//
// DESCRIPTION:   Get a count of the total name records in the IndiDB.
//
// PARAMETERS:    None.
//
// RETURNS: 	   Total count of name records, 0 if no records or unsuccessfull.
//
// REVISION:		None.
////////////////////////////////////////////////////////////////////////////////////
UInt16 IndiDbNumRecords (void)
{
	MemHandle 			recH;
   PackedDBRecord3*	rec;
	UInt16				recPos = 0;
	UInt16				recCnt = 0;
	UInt16				megaRecSz;
	Char*					keyPos;
	UInt16				recSize;
	UInt16				lastRecNum;
	UInt16				indexStart, indexEnd;

	// Get starting and ending index of mini-DB within mega-DB
	DbGetIndexes (OpenDbRef, IndiDB, &indexStart, &indexEnd);

	lastRecNum = indexEnd - 1;

   recH = DbQueryRecord (lastRecNum);
   rec = MemHandleLock (recH);
	ErrFatalDisplayIf (!rec, "IndiDbNumRecords: record missing"); 

	megaRecSz =  (UInt16) MemHandleSize(recH);
	keyPos = (char *) &rec->recSize; // get ptr to mini-record size info
	recPos = 0; // initialize to include sortNo field
	
	while (recPos < megaRecSz) {
		recSize = *((UInt16*) keyPos);
		keyPos+= recSize;	// advance to next mini-record
		recCnt++;
		recPos+= recSize;
		}

	recCnt = recCnt + (lastRecNum - indexStart)*DB_REC_MAX;
	DbMemHandleUnlock (&recH);
 	return recCnt;
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DbQueryRecord
//
// DESCRIPTION:   This routine gets a handle to a record in the open database. This
//						handle must be freed if the open database is on an Expansion Card.
//                
//
// PARAMETERS:    recIndex - index of record to get
//
// RETURNED:      The handle to the record to be locked.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
MemHandle DbQueryRecord (UInt16 recIndex)
{
	MemHandle 	recH = NULL;
	Err		 	err;

	ErrFatalDisplayIf (OpenDbRef && OpenFileRef, "Error in DbNumRecords");
	ErrFatalDisplayIf (!OpenDbRef && !OpenFileRef, "Error in DbNumRecords");

	if (OpenDbRef)
		recH = DmQueryRecord (OpenDbRef, recIndex);
	else // if (OpenFileRef)
		err = VFSFileDBGetRecord (OpenFileRef, recIndex, &recH, NULL, NULL);

	ErrFatalDisplayIf (!recH || (!OpenDbRef && err), "DbQueryRecord: Bad Record Handle");
	return recH;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DbMemHandleUnlock
//
// DESCRIPTION:   This routine unlocks a handle to a record in the open database.
//						It also frees the handle if the open database is on an Expansion
// 					Card.  The recH handle is also set to NULL.
//
// PARAMETERS:    recH 	- the handle to be freed and set to NULL.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DbMemHandleUnlock (MemHandle* recH)
{
	if (*recH) {
		MemHandleUnlock (*recH);
		if (OpenFileRef)
			MemHandleFree (*recH);
		*recH = NULL;
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DbGetRecord
//
// DESCRIPTION:   Get a mini-record from application database.  Each
//						mega-record in the database consists of a 2-byte sort
//						number followed by up to DB_REC_MAX mini-records, except 
//						for the NoteDB database, which has as many mini-records
//						as will fit in a 65,000-byte mega-record.  Each mini-record
//						consists of a 2-byte number representing the size of that 
//						mini-record (incl. this size number) followed by a
//						2-byte flags field followed by the record data.  We have made
//						sure each mini-record is aligned to start on a WORD via the
//						application conduit.
//
//						This routines gets a record for the following databases:
//						EvenDB, SouCDB, RepCDB, FamiDB, ChilDB, SourDB and RepoDB
//
// PARAMETERS:    dbType  - the database type to pull record from 
//            	   index   - index of record to lock
//            	   rec     - pointer event structure
//            	   recH 	  - handle to unlock when done
//
// RETURNS: 	   0 if successful, error code if not
//    			   The record's handle is locked so that the pointer to 
//  			      strings within the record remain pointing to valid chunk
//  			      versus the record randomly moving.
//
// REVISION:		None.
////////////////////////////////////////////////////////////////////////////////////
Err DbGetRecord (CmbDBases dbType, UInt32 index, DBRecordType *rec, MemHandle *recH)
{
   PackedDBRecord3   *src;
	PackedDBRecord2   *src2;
   UInt16				rProbe;  // mega index record number
   UInt16				probe;   // number of mini record within mega record
	UInt16				recPos = 0;
	UInt16				recCnt = 0;
	UInt16				megaRecSz;
	Char*					keyPos;
	UInt16				recSize;
	UInt16				indexStart, indexEnd;


	ErrFatalDisplayIf (*recH, "DbGetRecord: recH should be NULL");

 	// Get starting and ending index of mini-DB within mega-DB
	DbGetIndexes (OpenDbRef, dbType, &indexStart, &indexEnd);
	
	rProbe = indexStart + (index / DB_REC_MAX);
	probe  = index % DB_REC_MAX;

	ErrFatalDisplayIf (rProbe>=indexEnd, "DbGetRecord: rProbe out of range");

   // Check if probe is in a mega-record beyond dbType database
   if (rProbe >= indexEnd)		
      return dmErrIndexOutOfRange;

   *recH = DbQueryRecord (rProbe);
   src = MemHandleLock (*recH); // lock a chunk of memory
   ErrFatalDisplayIf (!src, "DbGetRecord: Search data missing");

  	megaRecSz = (UInt16) MemHandleSize (*recH);
	keyPos 	 = (char *) &src->recSize; // get ptr to mini-record size info

	recPos = 0; // initialize to include sortNo field
	
	while (recPos < megaRecSz) {
		if (recCnt == probe)
			break; // success!
		recSize = *((UInt16*) keyPos);
		keyPos+= recSize;	// advance to next mini-record
		recCnt++;
		recPos+= recSize;
		}

   if (recPos >= megaRecSz) {
   	DbMemHandleUnlock (recH);
   	ErrFatalDisplay ("DbGetRecord: recPos out of range");
      return dmErrIndexOutOfRange;
		}
		
	src2 = (PackedDBRecord2*) (keyPos);
 	DbUnpackRecord (src2, rec);
 	
 	#ifdef GREMLINS_MEM_CHK
  	MemHeapScramble (MemHeapID (0,1));
 	#endif	
 	
 	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 	   DbUnpackRecord
//
// DESCRIPTION:   Fills in the DBRecord structure
//
// PARAMETERS:    src   - the Database Record to unpack
//                dest  - the Database Record to unpack into
//
// RETURNS: 	   The Database Record unpacked
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DbUnpackRecord (PackedDBRecord2* src, DBRecordType* dest)
{
   Int16  	index;
   UInt16 	flags;
   char  	*p;
   
   flags = src->flags.allBits;
   p = &src->firstField;
         
   for (index = 0; index < 16; index++) {
      // If the flag is set point to the string else NULL
      if (GetBitMacro (flags, index) != 0) {
         dest->fields[index] = p;
         p += StrLen(p) + 1;
         }
      else
         dest->fields[index] = NULL;
      }
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 		NoteRecFinder
//
// DESCRIPTION:	Finds the first Record in the Cmb2DB database that has
//                a record key field that matches the given "key" string 
//						passed.  The record key field is always the first field
//						for each record.
//						By default, the we always seek the first record the ChilDB, 
//						which can have numerous records with the same record key
//      		      value. If no string is passed or there aren't any records
//                then false is returned.  A return of false is only allowed
//						in the ChilDB.  It should never happen in the other db's.
//
//						This routine handles all databases except for the IndxDB,
//						IndiDB and NoteDB.
//
// PARAMETERS:   	-> key       - an xRef string to lookup record with.
//                -> recordNum - to contain the record found.
//						<> recH		 - record handle of Note record found.			 
//
// RETURNS: 	   The record in recordNum or false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Err NoteRecFinder (Char *key, DBRecordType *rec, MemHandle *recH)
{
   Int32            	numOfRecords;
   PackedDBRecord3* 	r;
   PackedDBRecord2* 	r2;
   UInt32           	i, kmin; // all positions in the database.
   UInt32				rProbe;  // mega record number
   Int16            	result;  // result of comparing two records
	UInt16				recPos;
	UInt16				megaRecSz;
	Char*					keyPos;
	UInt16				recSize;
	UInt16				indexStart; // start position of mini-database
	UInt16 				indexEnd;	// end position of mini-database
		
   *recH = NULL;  // initialize

   // If no key to search with then return error
   if (key == NULL || *key == '\0')
      return dmErrInvalidParam;
 
 	// Get starting and ending index of mini-DB within mega-DB
	DbGetIndexes (OpenDbRef, NoteDB, &indexStart, &indexEnd);
 
   numOfRecords = indexEnd - indexStart;
	if (numOfRecords == 0)  // then mini database is empty
		return dmErrIndexOutOfRange;

   result = 0;
   kmin = rProbe = indexStart;
   
   while (numOfRecords > 0) {
      
      i = numOfRecords / 2;
      rProbe = kmin + i;

      DbMemHandleUnlock (recH);
      *recH = DbQueryRecord ( (UInt16) rProbe);
   	r = MemHandleLock (*recH);
   	ErrFatalDisplayIf (!r, "NoteRecFinder: bad rProbe record #1");

      // compare the two keys.
      if (&r->firstField == NULL)
         result = 1; 
      else
         result = StrCompare (key, &r->firstField);

      if (result == 0)
			break;
     
      if (result < 0)
         numOfRecords = i;
      else {
         kmin = rProbe + 1;
         numOfRecords = numOfRecords - i - 1;
         }
         
      } // while loop

	// Make sure "key" is greater than the record key of current rProbe
	// record. If not then go back one record.
	if (result < 0) {
		DbMemHandleUnlock (recH);	
	  	if (rProbe <= indexStart) {
     		ErrFatalDisplay ("NoteRecFinder: record missing #1");
     		return dmErrIndexOutOfRange;
			}
		rProbe--;
  		*recH = DbQueryRecord ((UInt16) rProbe);
     	if (*recH == NULL)	return dmErrMemError;
  		r = MemHandleLock (*recH);
  		ErrFatalDisplayIf (!r, "NoteRecFinder: bad rProbe record #2");
       } // if

	megaRecSz = (UInt16) MemHandleSize (*recH); // get size of current mega-record
	keyPos = (char*) &r->recSize;  // get ptr to mini-record size info
	recPos = 0; // initialize to include sortNo field

	// Search through current mega-record to find matching mini-record.
	while (true) {
		
		r2 = (PackedDBRecord2*) (keyPos);

     	if (&r2->firstField != NULL)  {
     	
         result = StrCompare (key, &r2->firstField); 
	
			if (result == 0) { // then we have a match
				goto GetRecord;
				}
			else if (result < 0) {// then record we are seeking is not there
				DbMemHandleUnlock (recH);
      		//ErrFatalDisplay("NoteRecFinder: record missing");
				return dmErrIndexOutOfRange;
				}
			// else continue searching
			}
			
		recSize = *((UInt16*) keyPos);
		keyPos+= recSize;	// advance position in mega-record
		recPos+= recSize;
		ErrFatalDisplayIf (recPos > megaRecSz, "NoteRecFinder: recPos too high");
		if (recPos >= megaRecSz) {
			DbMemHandleUnlock (recH);
			return dmErrIndexOutOfRange;
			}
		
		} // while loop

	/////////
	GetRecord:
	/////////
	
	r2 = (PackedDBRecord2*) (keyPos);
 	DbUnpackRecord (r2, rec);
 	return 0;
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ClearField
//
// DESCRIPTION:   Clears a fields in the displayed Form.
//
// PARAMETERS:    frmID - field to clear
//                      
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void ClearField (UInt16 frmID)
{
   FieldPtr fldP = GetObjectPtr (frmID);
   
   FldSetSelection (fldP, 0 ,0);  // set so we do not crash Palm III's
  	
	if (FldGetTextLength (fldP) > 0) {
      FldFreeMemory (fldP);
      FldDrawField (fldP);
      }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ProgressBarUpdate
//
// DESCRIPTION:	This routine updates the Progress Bar using the global variables
//              	the list view form.
//
// PARAMETERS:  	->	progBarID -	id of progress bar
//					 	-> updateBar -	true if we want to increment the UpdateCnt, else 
//											false.  It would typically be false if this routine
//											is called during a frmUpdateEvent.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ////////////////////////////////////////////////////////////////////////////////////
static void ProgressBarUpdate (UInt16 progBarID, Boolean updateBar)
{
	FormPtr				frm = FrmGetActiveForm ();
	RectangleType 		rectP;
	float					pctCmp;
	UInt16				extentX;
	Char					statusStr[7]; // eg.  " 100% " (space chr bef/aft number)
	FontID		   	curFont;
	UInt16 				xPos;
  	CustomPatternType	regAppt = {0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55}; // 50% gray pattern

	FrmGetObjectBounds (frm, FrmGetObjectIndex (frm, progBarID), &rectP);
	WinDrawRectangleFrame (rectangleFrame, &rectP);
  	extentX = rectP.extent.x;
  	
	if (updateBar)
		UpdateCnt++;
	
	WinSetPattern (&regAppt);
		
	pctCmp = (float) UpdateCnt / (float) UpdateTot;
	pctCmp = min (pctCmp, 1); // make sure we don't go above 100%
  	rectP.extent.x = pctCmp * (float) extentX;

	ErrNonFatalDisplayIf (rectP.extent.x > extentX, "ProgressBarUpdate: Too many updates");
  	
  	WinFillRectangle (&rectP, rectangleFrame);
  	
  	// draw status message
	curFont = FntSetFont (stdFont);
	pctCmp = (float) pctCmp * 100;
	StrPrintF (statusStr, " %i%% ", (UInt16) pctCmp);
	xPos = rectP.topLeft.x + extentX / 2  - FntCharsWidth (statusStr,
		StrLen (statusStr)) / 2;
	WinDrawChars (statusStr, StrLen (statusStr), xPos, rectP.topLeft.y+1); 
	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 		SearchCanceled
//
// DESCRIPTION:	Checks the control for a cancel event.
//
// PARAMETERS:   	-> ctlId - 	control ID of button on which to check for an event
//
// RETURNS: 	   true if cancel hit, else false
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean SearchCanceled (UInt16 ctlId)
{
	EventType	event;
	Boolean 		canceled = false;
	
	// Check for low-level events
	if (EvtSysEventAvail (false) || EvtEventAvail ()) {

		EvtGetEvent (&event, 0);

		if (!SysHandleEvent (&event)) {
			RectangleType	rectP;
			GetObjectBounds (ctlId, &rectP);
			
			if (RctPtInRectangle (event.screenX, event.screenY, &rectP)) {
				while (CtlHandleEvent (GetObjectPtr (ctlId), &event))
					EvtGetEvent (&event, 0);
				}
						
			if (event.eType == ctlSelectEvent) {
				if (event.data.ctlSelect.controlID == ctlId)
					canceled = true;
				}
			else if (event.eType == appStopEvent) {
				canceled = true;
				EvtAddEventToQueue (&event); // add event to queue again
				}
				
			if (canceled)
  	     		SndPlaySystemSound (sndConfirmation);
			}
		else { // !SysHandleEvent (&event)
			canceled = true; // cancel on pressing of any silkscreen buttons
			}
		
		}
	return canceled;
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 		FindPersonSearch
//
// DESCRIPTION:	Searches the Individual Database for an IDNO or REFN
//                field that matches the given key string passed. 
//      		      If no string is passed or there aren't any records then false
//                is returned.
//
// PARAMETERS:   	srchKey   - key to search for
//						srchIdNo	 - true to search IDNO field, false for REFN field
//                recordP   - to contain the record found
//
// RETURNS: 	   The record in recordP or false
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean FindPerson (Char *srchKey, Boolean srchIdNo, UInt16 *recordP)
{
   MemHandle         recH = NULL;
   PackedDBRecord3* 	r1;
   PackedDBRecord2* 	r2;
	DBRecordType		IndiRec;
   Int16             result = 1; // result of comparing two records
   Char*             recordKey;
	UInt16				recSize;
  	UInt16				rProbe; 		// mega index record number
	UInt16				recPos;
	UInt16				recCnt;
  	UInt16				megaRecSz;
	Char*					keyPos;
   FormPtr 				frm;
	FontID				curFont;
	Boolean				canceled = false;
	UInt16				indexStart, indexEnd;

   // If there isn't a key to search with stop
  	if (srchKey == NULL || *srchKey == '\0') {
   	ErrDisplay ("FindPerson: No key provided.");
   	return false;
   	}

	*recordP = NO_REC;

	// Get starting and ending index of mini-DB within mega-DB
	DbGetIndexes (OpenDbRef, IndiDB, &indexStart, &indexEnd);

	rProbe = indexStart;
	
	// Display searching message
	curFont = FntSetFont (boldFont);	
	frm = FrmInitForm (SearchingForm);
	FrmDrawForm (frm);
	FrmSetActiveForm (frm);
   
 	// Initialize variables for progress bar
 	UpdateTot = indexEnd - indexStart;
	UpdateCnt = 0;
  
   while (rProbe < indexEnd) {

		// Check if 'Canceled' button was hit
		if (SearchCanceled (SearchingCancelButton)) {
			canceled = true;
			break;
			}
  
		ProgressBarUpdate (SearchingStatusGadget, true);  // update progress bar

      recH = DbQueryRecord (rProbe);
   	r1 = MemHandleLock (recH);
      ErrFatalDisplayIf (!r1, "FindPerson: Search data missing");

		megaRecSz =  (UInt16) MemHandleSize (recH);
		keyPos = (char*) &r1->recSize; // get ptr to mini-record size info
		recPos = recCnt = 0; // initialize to include sortNo field
		
		while (recPos < megaRecSz) {
   		
   		r2 = (PackedDBRecord2*) (keyPos);
		 	DbUnpackRecord (r2, &IndiRec);
 		
 			recordKey = IndiRec.fields[srchIdNo ? indiNo : indiRefn];
 
      	if (recordKey == NULL)
         	result = 1; 
      	else
	      	result = StrCompare (srchKey, recordKey);
      	
      	if (result == 0) { // if equal stop here!
      		*recordP = (rProbe - indexStart)*DB_REC_MAX + recCnt;
        		break;
        		}

			recSize = *((UInt16*) keyPos);
			keyPos+= recSize;	// advance to next mini-record
			recCnt++;
			recPos+= recSize;
			} // end of inner while loop


		DbMemHandleUnlock (&recH);
		if (result == 0) break;
     	rProbe++;
      
      } // end of outer while loop

	// draw "Record Not Found" message and pause
	if (!canceled && result != 0) {
		FrmEraseForm (frm);
		FrmDrawForm (frm);
		ShowObject (SearchingCancelButton, false);
		WinDrawChars (cErrorNoRec, sizeof (cErrorNoRec) - 1, 21, 16);
  		SndPlaySystemSound (sndConfirmation);
 		SysTaskDelay (SysTicksPerSecond());
		}
	
	FntSetFont (curFont);
	
	FrmReturnToForm (0);
	
	return (Boolean) (result == 0); // no matching records were found.
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FindPersonHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Find Person Form.
//
// PARAMETERS:    event  - a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean FindPersonHandleEvent (EventPtr event)
{
	//FormPtr 	frm = FrmGetActiveForm ();
   Boolean 	handled = false;
   	
   switch (event->eType)
   	{
  		case ctlEnterEvent:
			
			switch (event->data.ctlEnter.controlID)
				{
				case FindPersonCancelButton:
					FrmReturnToForm (0);
               handled = true;
               break;
               
            case FindPersonFindButton:
               {
  					FieldPtr	fldP = GetObjectPtr (FindPersonTextField);

				   // make sure text is entered
   				if (FldGetTextLength (fldP) > 0) {

   					Char*		txtP = FldGetTextPtr (fldP);
						UInt16	recN;
						
						if (FindPerson (txtP, GetControlValue (FindPersonIDNOCheckbox),
							&recN)) {
							CurrentIndiRecN = recN;
				 			FrmCloseAllForms ();
            	   	FrmGotoForm (IndiSummForm);
 							} 
 						else { // search key not found
 							FrmUpdateForm (FrmGetActiveFormID (), updateRedrawAll);
 							}
            		}
            	}
               handled = true;
               break;
               
            case FindPersonClearButton:
              	ClearField (FindPersonTextField);
               handled = true;
               break;
            	
            case FindPersonInfoButton:
               FrmHelp (FindPersonHelpString);
               handled = true;
               break;
               
            default:
               break;
				}
         break;

		case fldChangedEvent:
      case keyDownEvent:
	      FldHandleEvent (GetObjectPtr (FindPersonTextField), event);
		   handled = true;
         break;
			
      case frmOpenEvent:
      case frmUpdateEvent:
      	FrmDrawForm (FrmGetActiveForm ());
      	SetNavFocusRing (FindPersonTextField);
			//FrmSetFocus (frm, FrmGetObjectIndex (frm, FindPersonTextField));
         handled = true;
         break;

      default:
      	break;
   	}
      
   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	EvenSearchInit
//
// DESCRIPTION: 	This routine initializes the buttons on the Data Search Form.
//
// PARAMETERS:  	-> getSel	-	true if retrieving data from form.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void EvenSearchInit (const Boolean getSel)
{
	UInt16	optN;

	if (getSel) {
		SrchPlac = (Boolean) GetControlValue (SearchPlaceCheckbox);
		SrchAddr = (Boolean) GetControlValue (SearchAddressCheckbox);
		SrchDate = (Boolean) GetControlValue (SearchDateCheckbox);
		SrchAll  = (Boolean) GetControlValue (SearchAllCheckbox);
		}
   
   for (optN = SearchBirCheckbox; optN <= SearchResCheckbox; optN++)
   	ShowObject (optN, (Boolean) !SrchAll);

   if (getSel && !SrchAll) {
   	for (optN = 0; optN < SrchOptTtl; optN++)
   		SrchOpts[optN] = (Boolean) GetControlValue (SearchBirCheckbox+optN);
		}
	else {
		SetControlValue (SearchPlaceCheckbox, 	 SrchPlac);
		SetControlValue (SearchAddressCheckbox, SrchAddr);
		SetControlValue (SearchDateCheckbox, 	 SrchDate);
  		SetControlValue (SearchAllCheckbox, 	 SrchAll);
  		SetControlValue (SearchSelCheckbox, 	(Boolean) !SrchAll);
  		
		for (optN = 0; optN < SrchOptTtl; optN++)
			SetControlValue (SearchBirCheckbox + optN, SrchOpts[optN]);
  		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      EvenSearchHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Search Form.
//
// PARAMETERS:    event  - a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean EvenSearchHandleEvent (EventPtr event)
{
   Boolean 	handled = false;
   //FormPtr	frm  = FrmGetActiveForm ();
   FieldPtr	fldP = (FieldPtr) GetObjectPtr (SearchTextField);
   	
   switch (event->eType)
   	{
  		case ctlSelectEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case SearchCancelButton:
					ThisDaySearch = true; // re-init
      	 		if (SrchArrayData == SrchNewSrch)
			 			SrchArrayData = SrchPlacEven;
					FrmGotoForm (PriorFormID);
               handled = true;
               break;
               
         	case SearchSearchButton:
         		// make sure text is entered and options choses correctly
   				if ((FldGetTextLength (fldP) > 0) && 
   					(SrchPlac || SrchAddr || SrchDate) &&
   				 	(SrchAll ||
   				 	(!SrchAll &&
   				 	 (SrchOpts[SrchBir] || SrchOpts[SrchBap] || SrchOpts[SrchChr] ||
   				 	  SrchOpts[SrchDth] || SrchOpts[SrchBur] || SrchOpts[SrchCrm] ||
   				 	  SrchOpts[SrchMar] || SrchOpts[SrchCen] || SrchOpts[SrchRes])))) {
				      StrCopy (SearchKey, FldGetTextPtr (fldP));
            	   FrmGotoForm (FldSearchForm);
						}
					else { // no characters entered or no search opts selected
						SndPlaySystemSound (sndWarning);
						}
               handled = true;
               break;
               
            case SearchClearButton:
              	SrchArrayData = SrchNoData;
               *SearchKey = '\0'; // init
               ClearField (SearchTextField);
               SetNavFocusRing (SearchTextField);
               //FrmSetFocus (frm, FrmGetObjectIndex (frm, SearchTextField));
               handled = true;
               break;
            	
            case SearchInfoButton:
               FrmHelp (SearchHelpString);
               handled = true;
               break;
               
            default: // if switching betw. All Evens and Select Events
            	EvenSearchInit (true);
               break;
				}
         break;

		case frmUpdateEvent:
      case frmOpenEvent:
      
      	ThisDaySearch = false;  // we must do this first here.
      	
      	// Check if SrchArr already has data from a prior search.
      	if (SrchArrayData == SrchPlacEven && SrchArr[0] < NO_REC_LONG)	{
	       	FrmGotoForm (FldSearchForm);
       	   }
       	else {  // no existing data
      		EvenSearchInit (false);
	      	FrmDrawForm (FrmGetActiveForm ());
	      
	      	// Copy last SearchKey text back to search field.
	    		if (SearchKey != NULL && *SearchKey &&
	    		 	(SrchArrayData == SrchPlacEven || SrchArrayData == SrchNewSrch))
	    			FldInsert (fldP, SearchKey, StrLen (SearchKey));
	      
         	SetNavFocusRing (SearchTextField);
         	//FrmSetFocus (frm, FrmGetObjectIndex (frm, SearchTextField));
         	}
         	
         handled = true;
         break;

		case fldChangedEvent:
      case keyDownEvent:
	      FldHandleEvent (GetObjectPtr (SearchTextField), event);
		   handled = true;
         break;

      default:
      	break;
   	}
      
   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FldSrchListDrawRec
//
// DESCRIPTION:   This routine draws a FldSearch entry into the FldSearchTable.
//  					It is called as a callback routine by the table object.
//
//						NOTE: TblGetItemInt should be set with will get the event number
// 					if in column 0, or the event year if in column 1.
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
static void FldSrchListDrawRec (void *table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   UInt32		      eRecN; // record number in EventDB
	DBRecordType  		eRec;  // holds unpacked EvenDB record
   MemHandle         eRecH = NULL; // EvenDB record handle
   UInt16		      iRecN; // record number in IndiDB
	DBRecordType  		iRec;  // holds unpacked IndiDB record
   MemHandle         iRecH = NULL; // IndiDB record handle
	DBRecordType  		fRec;  // holds unpacked Fami record
   MemHandle         fRecH = NULL; // family record handle
   UInt32		      aRecN; // record number in IndiDB or FamiDB
  	Char* 				sPos;  // pointer to start of year in evenDate
	Char* 				ePos;  // pointer to end of year in evenDate
  	Char* 				mPos;  // pointer to month in evenDate
	UInt16				yearN; // year of event
 	Char					tmpStr[5]; // holds etype and year of event
   FldSrchRows 		rowType;
   FontID 	      	curFont;
   UInt16 				eTypeN; // event type number
    
   eRecN   = TblGetRowData (table, row); // get event record number
   rowType = (FldSrchRows) TblGetItemInt (table, row, 0); // get type of row displayed

   curFont = FntSetFont (stdFont);

  	// Display message if no records found.
	if (rowType == RowNoRecFnd) {
		WinDrawChars (SCH_NO_EVEN, SCH_NO_EVEN_LEN, 2, bounds->topLeft.y);
		TblSetRowSelectable (table, row, false);
		goto ExitFunc;
		}

   DbGetRecord (EvenDB, eRecN, &eRec, &eRecH); // get event record.
  
	// Get related Individual or Family record.
	aRecN  = (UInt32) StrAToI (&eRec.fields[evenOrgNo][1]); // skip initial 'I' / 'F'
	    
	// -- Draw title of event --
   if (rowType != RowWife) { // all rows except for wife get title
		
		eTypeN  = (UInt16) StrAToI (eRec.fields[evenType]); // event type

   	ErrFatalDisplayIf (eTypeN >= TOT_EVEN_L, "FldSrchListDrawRec: bad Event Type");
	
		StrPrintF (tmpStr, "%s:", EvenDesc[eTypeN][1]); // abbrevated event name
   	WinDrawChars (tmpStr, StrLen (tmpStr), bounds->topLeft.x, bounds->topLeft.y);
     	}
     	
	//  -- Draw individual name --
   bounds->topLeft.x = SCH_ETYPE_WID;
   bounds->extent.x 	= SCH_EXT_WID;
 
	if (rowType == RowIndi) {
		iRecN = (UInt16) aRecN;
		}
			
	else { // must be rowType == RowHusb || rowType == RowWife
	  
	  UInt16 fldN = (rowType == RowHusb) ? famiHusbNo : famiWifeNo;
	  
	  if (DbGetRecord (FamiDB, aRecN, &fRec, &fRecH) != 0) {
	  		ErrNonFatalDisplay ("FldSearchListDrawRec: FamiDB Record not found");
	  		goto ExitFunc;
	  		}
	  		
   	// Prepare individual table entry
   	if (fRec.fields[fldN])
			iRecN = (UInt16) StrAToI (fRec.fields[fldN]);
		else
			iRecN = NO_REC;
		
  		DbMemHandleUnlock (&fRecH);		
		}
	
	if (iRecN != NO_REC) {
		
		if (DbGetRecord (IndiDB, iRecN, &iRec, &iRecH) != 0) {
	  	 	ErrNonFatalDisplay ("FldSearchListDrawRec: IndiDB Record not found");
   		goto ExitFunc;
  			}
		}
		
	else { // draw "Unknown"
     	
     	iRec.fields[indiFName] = iRec.fields[indiLName] = iRec.fields[indiTitle] = NULL;
  		TblSetRowSelectable (table, row, false);
  		}

  	DrawRecordNameAndLifespan (&iRec, bounds, false, false);
   
  	DbMemHandleUnlock (&iRecH);
     	
  	// -- Draw the year of the event --
	if (rowType !=  RowHusb) { // year on all lines except for husband
  		
  		// Get the event year (assumed date format is DD MMM YYYY).
		yearN = 0; // init
		sPos = eRec.fields[evenDate];

		if (sPos) {
		
			while ((*sPos < '1' || *sPos > '9') && *sPos != '\0')
				sPos++; // remove '0' and/or other text (eg ABT)

				mPos = StrChr (sPos, chrSpace);

				if (mPos == NULL) { // in case of just "YYYY"
					if (StrLen (sPos) == 4) {
						StrCopy (tmpStr, sPos);
						yearN = (UInt16) StrAToI (tmpStr);
						}
					}
					
				else { // if (mPos == NULL)
	    			mPos++; // skip space
    				sPos = StrChr (mPos, chrSpace);

					if (sPos != NULL) {
						UInt16 	sLen;
								
						sPos++; // skip space
						ePos = sPos;
						while (*ePos >= '0' && *ePos <= '9') // remove ending non-digits
							ePos++;
									
						// copy year from date string
						sLen = min (ePos - sPos, 4);
						StrNCopy (tmpStr, sPos, sLen);	
						tmpStr[sLen] = '\0';
						yearN = (UInt16) StrAToI (tmpStr);	
						} // if (sPos != NULL)
					} // else
			} // if (sPos)

  			if (yearN == 0)
  				StrCopy (tmpStr, SCH_NOT_AVAIL);
  			else
  				StrIToA (tmpStr, yearN);
    		WinDrawChars (tmpStr, StrLen (tmpStr), 160-FntCharsWidth (tmpStr,
    			StrLen (tmpStr)),	bounds->topLeft.y);
    			
    	} // if (rowType !=  RowHusb)

  	////////
  	ExitFunc:
  	////////
  	
  	DbMemHandleUnlock (&eRecH);
	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FldSrchDrawRecordAtRow
//
// DESCRIPTION:   This routine draws a This Day row entry into the Search
//                Table.
//
// PARAMETERS:    row    - row number in the table of the item to draw
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void FldSrchDrawRecAtRow (Int16 row)
{
	RectangleType 	bounds;
  	TablePtr 	   table;
	
	table = (TablePtr) GetObjectPtr (FldSearchListTable);
	TblGetItemBounds (table, row, 0, &bounds);
  	FldSrchListDrawRec (table, row, 0,  &bounds);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FldSrchListInit
//
// DESCRIPTION:   This routine initializes the event list table on the Search
//                Table Form.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void FldSrchListInit (void)
{
   Int16			row;
   TablePtr 	table;

	table = (TablePtr) GetObjectPtr (FldSearchListTable);

   // Set the callback routine that will draw the records
   for (row = 0; row < TblGetNumberOfRows (table); row++) {
      TblSetItemStyle (table, row, 0, customTableItem);
      TblSetRowUsable (table, row, false);
      }
      
	TblSetColumnUsable (table, 0, true);
	TblSetCustomDrawProcedure (table, 0, FldSrchListDrawRec);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	FldSrchUpdScrollers
//
// DESCRIPTION: 	This routine updates the scroller on the Search Form.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ///////////////////////////////////////////////////////////////////////////////////
static void FldSrchUpdScrollers (void)
{
	Boolean		scrollableL;
	Boolean		scrollableR;
	UInt16		topPage;
	UInt16		priorPageTot = 0; // must init
	
 	if (SrchTotPages > SCH_ARR_SZ/SCH_PAGE_ROWS)
 		priorPageTot = (SrchTotPages - SCH_ARR_SZ/SCH_PAGE_ROWS);

	topPage = TopVisSrchArrNum/SCH_PAGE_ROWS + priorPageTot + 1;
	
	scrollableL = (Boolean) (topPage > priorPageTot + 1);
   scrollableR = (Boolean) (topPage < SrchTotPages);
   UpdateLeftRightScrollers (FldSearchScrollLeftButton, FldSearchScrollRightButton,
   	scrollableL, scrollableR);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	FldSrchScroll
//
// DESCRIPTION: 	This routine is called to page forward or back.
//
// PARAMETERS:  	-> direction - direction to scroll (NavL or NavR).
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ///////////////////////////////////////////////////////////////////////////////////
static void FldSrchScroll (DirType direction)
{
	UInt16	newTopVisSrchArrNum;
	UInt16	topPage;
	UInt16	priorPageTot = 0;

 	if (SrchTotPages > SCH_ARR_SZ/SCH_PAGE_ROWS)
 		priorPageTot = (SrchTotPages - SCH_ARR_SZ/SCH_PAGE_ROWS);

	topPage = TopVisSrchArrNum/SCH_PAGE_ROWS + priorPageTot + 1;

	newTopVisSrchArrNum = TopVisSrchArrNum;

	if (direction == NavR) { // scroll to next search page
		if (topPage < SrchTotPages)
         newTopVisSrchArrNum+= SCH_PAGE_ROWS;
      }
	else if (direction == NavL){ // scroll to previous search page
      if (topPage > priorPageTot + 1)
         newTopVisSrchArrNum-= SCH_PAGE_ROWS;
		}

	// Avoid redraw if no change
	if (TopVisSrchArrNum != newTopVisSrchArrNum) {
		TopVisSrchArrNum = newTopVisSrchArrNum;
		FldSrchRedrawTable ();
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FldSrchCompRec
//
// DESCRIPTION:   Searches a given recKey string for the srchKey string. This 
//						function can only be used to find a event date or event place.
//						If searching for an event date the StrNCompare function is 
//						used.
//
// PARAMETERS:   	-> srchKey		- key to search for
//						-> srchkeyLen	- length of searchKeyP
//						-> recKey		- search key from record
//
// RETURNED:      True if key is found, else false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean FldSrchCompRec (Char* srchKey, UInt16 srchKeyLen, Char* recKey)
{
	if (srchKey == NULL || *srchKey == '\0')
		return false;

	if (ThisDaySearch) {
		// remove any '0' and/or other text (eg ABT) in front of date
		while ((*recKey < '1' || *recKey > '9') && *recKey != '\0')
 			recKey++; 
 
		StrNCopy (RecKeyD, recKey, srchKeyLen);
		RecKeyD[srchKeyLen] = '\0';
		StrToLower (RecKeyD, RecKeyD);
		return (StrNCompare (srchKey, RecKeyD, srchKeyLen) == 0);
		}
			
	else {	
		StrCopy (RecKeyD, recKey);
		StrToLower (RecKeyD, RecKeyD);
		return (StrStr (RecKeyD, srchKey) != NULL);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	FldSrchListLoadTable
//
// DESCRIPTION: 	This routine loads records the event record numbers into the 
//						FldSearch table.  The event records sent to this function will
// 					only be those with matching search data. The 'row' number in
//						the table is not incremented if an error occurs.
//
//						NOTE: row will always be incremented by 1 during this
//						routine.
//
// PARAMETERS:  	-> table		-	pointer to FldSearch table.
//						<> row 		-  current row in the table.
//						-> eRec		-	pointer to event record.
//						-> eRecN 	- 	record number for event.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ///////////////////////////////////////////////////////////////////////////////////
static void FldSrchListLoadTable (TablePtr table, Int16 *row,
	DBRecordType eRec, UInt32 eRecN)
{
	ErrFatalDisplayIf (eRec.fields[evenOrgNo][0] != 'I' &&
		eRec.fields[evenOrgNo][0] != 'F', "FldSearchListLoadTable: Bad OrgNo.");

	ErrFatalDisplayIf (*row >= TblGetNumberOfRows (table),
	 	"FldSearchListLoadTable: Bad row.");

	// exclude if no OrgNo xref or if unknown event. Should never happen.
	if (!eRec.fields[evenOrgNo] || !*eRec.fields[evenOrgNo] ||
		 !eRec.fields[evenType]  || !*eRec.fields[evenType]) {
		ErrNonFatalDisplay ("FldSrchListLoadTable: Bad evenOrgNo or evenType");
		(*row)++; // skip to next row
		return;
		}
	
	TblSetRowSelectable (table, *row, true);
	TblSetRowUsable (table, *row, true);
   TblSetRowData (table, *row, eRecN);
	
	if (eRec.fields[evenOrgNo][0] == 'I') { // Individual Event
  		TblSetItemInt (table, *row, 0, RowIndi);
		}
		
	else { // Family Event
		TblSetItemInt (table, *row, 0, RowHusb); // set husband
		FldSrchDrawRecAtRow (*row);
		SrchArr[TopVisSrchArrNum + (*row)] = eRecN;
		(*row)++;
		TblSetRowSelectable (table, *row, true);
		TblSetRowUsable (table, *row, true);
		TblSetRowData (table, *row, eRecN);
		TblSetItemInt (table, *row, 0, RowWife); // set wife
		}
						
	FldSrchDrawRecAtRow (*row);
	SrchArr[TopVisSrchArrNum + (*row)] = eRecN;
	(*row)++;
	
	return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	FldSrchSearchRecs
//
// DESCRIPTION: 	This routine loads records into the ThisDay table.
//
//						NOTE: ThisDaySearch must be set before calling this function
//
// PARAMETERS:  	-> contSearch	-	true if continuing a search, else false if new 
//												search. First call to this function must have
//												this value as false.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ///////////////////////////////////////////////////////////////////////////////////
static void FldSrchSearchRecs (Boolean contSearch)
{
   Int16			      row = 0;
   TablePtr 	      table;
   UInt16            numRows;
   MemHandle         fRecH = NULL;
   DBRecordType  		eRec;  // holds unpacked Event record
   PackedDBRecord2* 	sRec;  // holds packed mini-record
  	PackedDBRecord3* 	mRec;  // holds packed mega record
  	MemHandle         mRecH = NULL;
	UInt16				recSize;
  	UInt16				megaRecSz;
  	Boolean				cont = true;
	UInt16				indexStart, indexEnd;
   UInt16				keyLen;
  	Char					srchKey[SCH_KEY_LEN+1];
  	Char*					srchKeyP = srchKey;
	Char*					keyPos; // pointer to mini-record.
	static UInt16		recPos;
	static UInt16		recCnt;
	static UInt16     mRecN;  // mega record number
  	Boolean				chkMatch;
  	Boolean 				loadRec = false;
  	UInt16				ctlId;

 	// Hide or show buttons.
 	for (ctlId = FldSearchDoneButton; ctlId <= FldSearchScrollRightButton; ctlId++)
 		ShowObject (ctlId, false);
	ShowObject (FldSearchCancelButton, true);

   // Initialize some variables
   table 	= (TablePtr) GetObjectPtr (FldSearchListTable);
   numRows 	= TblGetNumberOfRows (table);

 	FldSrchListInit (); // hide lines without data
 	TblEraseTable (table);
	DbGetIndexes (OpenDbRef, EvenDB, &indexStart, &indexEnd);

	// Set the type of data we are searching for.
	SrchArrayData = ThisDaySearch ? SrchDateEven: SrchPlacEven;

  	if (!contSearch) { // then we are doing a new search
  	
	  	// Get total times the progress bar will be updated. We will add 1 to
	  	// UpdateTot b/c ProgressBarUpdate will be called one last time after
	  	// we have run out of records to search.
		UpdateTot = (indexEnd - indexStart) + 1;  		
  		
		MemSet (SrchArr, sizeof (SrchArr), 0xFF); // init
		TopVisSrchArrNum 	= UpdateCnt = 0; 	// init
		SrchTotPages 	  	= 1; 					// init
  		mRecN 				= indexStart; 		// init

		ErrFatalDisplayIf (mRecN >= indexEnd, "FldSrchSearchRecs: Bad mRecN");
	
		// erase progress bar data
		EraseRectangleObject (FldSearchStatusGadget);
  		}
	
	ErrFatalDisplayIf (TopVisSrchArrNum >= SCH_ARR_SZ, "TopVisScrhArrNum out of bounds.");

	// Draw title and page number.
	FldSrchDrawForm (); // this MUST be done in this function. Do not move it.

	// Prepare the search key.
	StrToLower (srchKey, SearchKey);
	if (ThisDaySearch && srchKeyP[0] == '0') // remove any starting '0'
  		srchKeyP++;
  	keyLen = StrLen (srchKeyP);
	
	//  -- Search event records. --
	while (mRecN < indexEnd) {

		ProgressBarUpdate (FldSearchStatusGadget, !contSearch); // update progress bar

      // retrieve the mega record
      mRecH = DbQueryRecord (mRecN);
   	mRec = MemHandleLock (mRecH);
      ErrFatalDisplayIf (!mRec, "FldSearchSearchRecs: Search data missing");

		megaRecSz =  (UInt16) MemHandleSize (mRecH);
		
		if (!contSearch)
			recPos = recCnt = 0; // re-init
			
		keyPos = (char*) &mRec->recSize + recPos; // get ptr to where srch left off
			
		contSearch = false;  // re-init

		while (recPos < megaRecSz) {

   		sRec = (PackedDBRecord2*) (keyPos); // load mini-record
			DbUnpackRecord (sRec, &eRec);

			chkMatch = true; // init
				
			// Filter search results
			if (!SrchAll && !ThisDaySearch) { // items must match fields in EvenDesc array
				
				switch (StrAToI (eRec.fields[evenType]))
					{
					case  1: // Birth
						if (!SrchOpts[SrchBir]) chkMatch = false;
						break;
					case  2: // Baptism
						if (!SrchOpts[SrchBap]) chkMatch = false;
						break;
					case  3: // Christening
						if (!SrchOpts[SrchChr]) chkMatch = false;
						break;
					case  4: // Death
						if (!SrchOpts[SrchDth]) chkMatch = false;
						break;
					case  5: // Burial
						if (!SrchOpts[SrchBur]) chkMatch = false;
						break;
					case  6: // Cremation
						if (!SrchOpts[SrchCrm]) chkMatch = false;
						break;
					case  8: // Marriage
						if (!SrchOpts[SrchMar]) chkMatch = false;
						break;
					case 14: // Census
						if (!SrchOpts[SrchCen]) chkMatch = false;
						break;
					case 28: // Residence
						if (!SrchOpts[SrchRes]) chkMatch = false;
						break;
					default:  // ignore all other event types
						chkMatch = false;
						break;
						}  // switch
				} // else

			if (chkMatch) {
			
				if (ThisDaySearch) {
					
					if (eRec.fields[evenDate])
						loadRec = FldSrchCompRec (srchKeyP, keyLen, eRec.fields[evenDate]);
					}
					
				else {
		
					if (SrchPlac && eRec.fields[evenPlac])
						loadRec = FldSrchCompRec (srchKeyP, keyLen, eRec.fields[evenPlac]);

					if (!loadRec && SrchAddr && eRec.fields[evenAddr])
						loadRec = FldSrchCompRec (srchKeyP, keyLen, eRec.fields[evenAddr]);
					
					if (!loadRec && SrchDate && eRec.fields[evenDate])
						loadRec = FldSrchCompRec (srchKeyP, keyLen, eRec.fields[evenDate]);
					}
				
				if (loadRec) {
					FldSrchListLoadTable (table, &row, eRec,
					 	(mRecN - indexStart)*DB_REC_MAX + recCnt);
					loadRec = false;
					}
					
				// check if enough lines for another record (2 lines)
				if (row >= numRows - 1) {
 	   	      ShowObject (FldSearchMoreButton, true);
					cont = false;
					}

				} // if (chkMatch)

			// Advance to next mini-record.
			recSize = *((UInt16*) keyPos);
			keyPos+= recSize;	
			recCnt++;
			recPos+= recSize;

			// Check (every 16 records) if 'Cancel' button was hit.
			if (!(recCnt &  0x000F) && SearchCanceled (FldSearchCancelButton)) {
				SrchArrayData = SrchNoData;
				cont = false;
				break;
				}
		
			if (!cont) break;	
			
			} // end of inner (mini record) while loop


		DbMemHandleUnlock (&mRecH);

		if (!cont) break; // break before advancing mRecN !!

	  	mRecN++;
	  	
		if (mRecN >= indexEnd) {
			ProgressBarUpdate (FldSearchStatusGadget, true); // final update
	  		break;
			}

		EvtResetAutoOffTimer (); // stop auto-off timer

 		ErrFatalDisplayIf (cont && row >= numRows - 1,"FldSrchSearchRecs: Too many rows.");
      } // end of outer (mega record) while loop

	// If no matches found on last page, init first row for 'Not Found' message.
	if (UpdateCnt == UpdateTot && row == 0) {
		TblSetRowUsable (table, row, true);
		TblSetItemInt (table, row, 0, RowNoRecFnd);
		FldSrchDrawRecAtRow (row);
		SrchTotPages--; // set back to prior page
		}

	// If no matches found, set SrchArrayData bact to SrchNoData.
	if (TopVisSrchArrNum == 0 && row == 0)
		SrchArrayData = SrchNoData;

	// Reset the form buttons.
	ShowObject (FldSearchCancelButton,  false);
 	ShowObject (FldSearchDoneButton,    true);
	ShowObject (FldSearchRestartButton,	ThisDaySearch);
 	ShowObject (ThisDaySearch ? FldSearchPickButton : FldSearchSearchButton, true);

	FldSrchUpdScrollers ();

 	ErrFatalDisplayIf (cont && UpdateCnt != UpdateTot,
 		"FldSrchSearchRecs: Progress Bar not updated correctly");
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	FldSrchRedrawTable
//
// DESCRIPTION: 	This routine re-loads records into the Search table from the 
//						SrchArr Array.
//
//						NOTE: The following variables must be loaded prior to calling this 
//						routine:
//
//						SrchArr
//						ThisDaySearch
//						TopVisSrchArrNum
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:	 	None.
 ///////////////////////////////////////////////////////////////////////////////////
static void FldSrchRedrawTable (void)
{
   Int16		   	row 	= 0;
   TablePtr			table; 	   
   MemHandle      eRecH = NULL;
   DBRecordType  	eRec; // holds unpacked Event record
   
   table = (TablePtr) GetObjectPtr (FldSearchListTable);
   
  	FldSrchListInit (); // hide lines without data
  	FldSrchDrawForm (); // draw title, page number and lines
  	
	if (UpdateCnt != UpdateTot) // then no more recs to search
		ShowObject (FldSearchMoreButton, true);

	// Hide or show buttons
 	ShowObject (FldSearchDoneButton, 	true);
	ShowObject (FldSearchRestartButton,	ThisDaySearch);
	ShowObject (FldSearchPickButton, 	ThisDaySearch);
	ShowObject (FldSearchSearchButton, 	!ThisDaySearch);

 	TblEraseTable (table); // erase data in table
	EraseRectangleObject (FldSearchStatusGadget); // erase old progress bar
	ProgressBarUpdate (FldSearchStatusGadget, false); // redraw progress bar

	// Load the table rows with Event Records.
	while (row < TblGetNumberOfRows (table)) {
		
		if (SrchArr[TopVisSrchArrNum + row] == NO_REC_LONG) {
			row++;
			}
		else {
			DbGetRecord (EvenDB, SrchArr[TopVisSrchArrNum + row], &eRec, &eRecH);
			FldSrchListLoadTable (table, &row, eRec, SrchArr[TopVisSrchArrNum + row]);
			DbMemHandleUnlock (&eRecH);
			}
		} // while

	FldSrchUpdScrollers ();
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FldSrchDrawForm
//
// DESCRIPTION:   This routine draws the Field Search Form.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void FldSrchDrawForm (void)
{
   FontID		   curFont;
	Char				headStr[SCH_HDR_LEN+1]; // eg "Events on 12 Jul" or "Srch: xxxx"
	RectangleType	rect;
	Int16 			len, wid; // length & width of header
	Boolean 			ignored;
	UInt16			topPage;
	UInt16			priorPageTot = 0;

	// Draw the form title.
	GetObjectBounds (FldSearchTitleGadget, &rect);
	WinEraseRectangle (&rect, 0); // erase old title
	
	if (ThisDaySearch) { // create the title string for "On This Day" search
		StrPrintF (headStr, SCH_HEAD_EVEN, SearchKey); // eg "Events On 12 JUL"
		}
	else { // create the title string for Event search
		StrCopy (headStr, SCH_HEAD_SCH); // "Srch:"
		StrNCat (headStr, SearchKey, SCH_HDR_LEN);
		}

	curFont 	= FntSetFont (boldFont);
	wid = rect.extent.x;
	len = (Int16) StrLen (headStr);
	FntCharsInWidth (headStr, &wid, &len, &ignored); // draw title on the screen
	WinDrawChars (headStr, len, rect.extent.x/2 - wid/2, rect.topLeft.y); 
 
	// Draw page number.
	if (SrchTotPages > SCH_ARR_SZ/SCH_PAGE_ROWS)
 		priorPageTot = (SrchTotPages - SCH_ARR_SZ/SCH_PAGE_ROWS);
	topPage = TopVisSrchArrNum/SCH_PAGE_ROWS + priorPageTot + 1;
	
	// Erase page number & Draw new page number
	GetObjectBounds (FldSearchPageGadget, &rect);
	WinEraseRectangle (&rect, 0);
	
	FntSetFont (stdFont);

	StrPrintF (headStr, "%i", topPage);
	len = (Int16) StrLen (headStr);
	wid = FntCharsWidth (headStr, len);
	WinDrawChars (headStr, len, rect.topLeft.x + rect.extent.x - wid, rect.topLeft.y);

	FntSetFont (curFont);	

 	// Draw screen divider lines.
 	DrawScreenLines (FldSearchForm);
  }

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      FldSrchHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the This Day Form.
//
//						NOTE: PriorFormID must be set before calling this function. This
//						is the form to return to after selecting the "Done" button.
//
// PARAMETERS:    event  - a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean FldSrchHandleEvent (EventPtr event)
{
   Boolean 	handled = false;
   DirType	navDir;
   	
   switch (event->eType)
   	{
   	case tblSelectEvent:
   		{
   		UInt32		   aRecN; // record number in IndiDB or FamiDB
			DBRecordType  	aRec;  // holds unpacked EvenDB or FamiDB record
   		MemHandle      aRecH = NULL; // handle for EvenDB or FamiDB record
   		FldSrchRows		rowType;	// type of row (Event, Husb, or Wife);
   	
   		// Get the EvenDB Record number
   		CurrentEvenRecN = TblGetRowData (event->data.tblSelect.pTable,
   			event->data.tblSelect.row);
  			
  			ErrFatalDisplayIf (CurrentEvenRecN == NO_REC_LONG,
  				"FldSrchHandleEvent: Bad CurrentEvenRecN.");
  			
  		  	// Get the type of event records: indi, fami (husb) or fami (wife)
   		rowType = (FldSrchRows) TblGetItemInt (event->data.tblSelect.pTable,
				event->data.tblSelect.row, 0);
   
  		 	// Get the Event Record selected.
  		 	DbGetRecord (EvenDB, CurrentEvenRecN, &aRec, &aRecH);
			aRecN  = (UInt32) StrAToI (&aRec.fields[evenOrgNo][1]); // skip starting 'I'/'F'
			DbMemHandleUnlock (&aRecH);
			
			if (rowType == RowIndi) {
				CurrentIndiRecN = (UInt16) aRecN;
				SrchShowEven = true;
				}
				
			else { // must be RowHusb or RowWife
	  			UInt16 fldN = (rowType == RowHusb) ? famiHusbNo : famiWifeNo;
	  
	  			// Get the FamiDB Record information
	  			DbGetRecord (FamiDB, aRecN, &aRec, &aRecH);
				CurrentIndiRecN = (UInt16) StrAToI (aRec.fields[fldN]); // get husb/wife rec. no
				CurrentFamiRecN = aRecN;
				DbMemHandleUnlock (&aRecH);
				SrchShowFamiEven = true;
  				}
  			}
  			
        	FrmGotoForm (IndiSummForm);
         handled = true;
         break;

		case keyDownEvent:
 			
 			if (NavKeyHit (event, &navDir)) {
				
				switch (navDir)
					{
					case NavL:
					case NavR:
						FldSrchScroll (navDir);
						handled = true;
						break;
					
					default:
						break;
					}
         	}
   	
  		case ctlSelectEvent:
			
			switch (event->data.ctlEnter.controlID)
				{
				case FldSearchDoneButton:
					// If no records found for last page, then go back one page
					if (SrchArr[TopVisSrchArrNum] == NO_REC_LONG &&
						 TopVisSrchArrNum >= SCH_PAGE_ROWS)
						TopVisSrchArrNum -= SCH_PAGE_ROWS;
					FrmGotoForm (PriorFormID);
               handled = true;
               break;
            
            case FldSearchSearchButton:
            	SrchArrayData = SrchNewSrch;
            	FrmGotoForm (SearchForm);
               handled = true;
               break;
               
            case FldSearchMoreButton:
				   
				   ErrFatalDisplayIf (SrchTotPages == 0,
				   	"FldSrchHandleEvent: SrchTotPages should not be 0");
				   
				   // Put TopVisSrchArrNum back to last page (in case user paged back)
				   if (SrchTotPages > SCH_ARR_SZ/SCH_PAGE_ROWS)
 						TopVisSrchArrNum = SCH_ARR_SZ - SCH_PAGE_ROWS;
					else
						TopVisSrchArrNum = (SrchTotPages-1) * SCH_PAGE_ROWS;	
				   
				   // Shift array entries if we have no room for more.
					if (TopVisSrchArrNum + SCH_PAGE_ROWS == SCH_ARR_SZ) {
						UInt16 x, y;
						x = 0;
						for (y = SCH_PAGE_ROWS; y < SCH_ARR_SZ; y++) {
							SrchArr[x] = SrchArr[y];
							x++;
							}
							
						// Initialize next page of data in array.
						MemSet (&SrchArr[TopVisSrchArrNum],
							SCH_PAGE_ROWS * sizeof (UInt32), 0xFF);
						}
				   else { // yes, there is still room for more entries
					   TopVisSrchArrNum+= SCH_PAGE_ROWS;
					   }
				   
				   SrchTotPages++;
				   FldSrchSearchRecs (true);
               handled = true;
               break;

            case FldSearchRestartButton:
 	            FldSrchSearchRecs (false);
               handled = true;
               break;

				case FldSearchPickButton:
					FrmPopupForm (ThisDayNewForm);
               handled = true;
               break;

				case FldSearchScrollLeftButton:
					FldSrchScroll (NavL);
               handled = true;
               break;

				case FldSearchScrollRightButton:
					FldSrchScroll (NavR);
               handled = true;
               break;

            default:
               break;
				}
         break;
         
		case frmOpenEvent:
      	
      	if (ThisDaySearch && SrchArrayData != SrchDateEven) {
				// Get search string w/ today day & month. GEDCOM files are nearly 
				// always in English. But some devices are international versions and
				// will return the local month when using DateToAscii function. This
				// causes the search to fail given the local month string will not
				// match the English month string in the GEDCOM file. Therefore, we
				// will always use the Months array to get the month string.
   		   DateTimeType	dateTimeP;
  				TimSecondsToDateTime (TimGetSeconds (), &dateTimeP); 
		  		StrPrintF (SearchKey, "%i %s", dateTimeP.day, Months[dateTimeP.month-1]);
				}
	
			FrmDrawForm (FrmGetActiveForm ());

			// We want to redraw the previous Search table data as long as:
			// (a) we are initiating a "Event Search" and the existing table data
			// is from an "Event Search", or (b) we are initiating an "On This Day"
			// search and the existing table data is from an "On This Day"
			// search; and (c) the first item in the SrchArr is not NO_REC_LONG.
			if (((!ThisDaySearch && SrchArrayData == SrchPlacEven) ||
 				  (ThisDaySearch && SrchArrayData == SrchDateEven))  &&
 				  (SrchArr[0] < NO_REC_LONG))
				FldSrchRedrawTable ();
			else
				FldSrchSearchRecs (false);
	     		
         handled = true;
         break;

		case frmUpdateEvent:
			FrmDrawForm (FrmGetActiveForm ());
   		FldSrchDrawForm ();
   		ProgressBarUpdate (FldSearchStatusGadget, false);
			handled = true;
			break;

		case frmCloseEvent:
			ThisDaySearch = true; // re-init
			break;

      default:
      	break;
   	}
      
   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ThisDayNewCheckDate
//
// DESCRIPTION: 	This routine validates the date entered by the user.
//
// PARAMETERS:  	None.
//
// RETURNED:    	True if good date, else false
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ThisDayNewCheckDate (void)
{
	Int16 		day, mon;
	FieldPtr 	fldP;
	ListPtr 		lstP;
	FormPtr		frm;
	
	frm = FrmGetActiveForm ();

	// check if the date entered is valid
	FrmHideObject (frm, FrmGetObjectIndex (frm, ThisDayNewInvalidLabel));

	// get Start Month from selected list item
	lstP = (ListPtr) GetObjectPtr (ThisDayNewMonthList);
	mon = LstGetSelection (lstP) + 1;

	// get Start Day from its field
	fldP = (FieldPtr) GetObjectPtr (ThisDayNewDayField);
	
	if (FldGetTextLength (fldP) > 0)  // then data has been entered
   	day = (Int16) StrAToI (FldGetTextPtr (fldP));
	else return;
			
  	if (day > DaysInMon (mon, 2000)) { // use year with 29 days in Feb
	  	FrmShowObject (frm, FrmGetObjectIndex (frm, ThisDayNewInvalidLabel));
		SndPlaySystemSound (sndWarning);
		return;
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      ThisDayNewHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the ThisDay Form.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure
//
// RETURNED:      True if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean ThisDayNewHandleEvent (EventPtr event)
{
   Boolean 		handled = false;
   ListPtr 		lstMonP = (ListPtr) GetObjectPtr (ThisDayNewMonthList);
   FieldPtr 	fldP = (FieldPtr) GetObjectPtr (ThisDayNewDayField);
  	FormPtr		frm = FrmGetActiveForm ();
   
   switch (event->eType)
   	{
  		case ctlSelectEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case ThisDayNewStartButton:
					// Get search string w/ today day & month. GEDCOM files are nearly 
					// always in English. But some devices are international versions
					// and will return the local month when using DateToAscii function.
					// This causes the search to fail given the local month string will
					// not match the English month string in the GEDCOM file. Therefore,
					// we will always use the Months array to get the month string.
					if (FldGetTextLength (fldP) > 0) {
				  		Char* sPtr;
				  		StrCopy (SearchKey, FldGetTextPtr (fldP));
				  		sPtr = StrChr (SearchKey, chrFullStop); // remove '.' character
					  	if (sPtr != NULL)
					  		*sPtr = chrNull;
				  		StrCat (SearchKey, " "); // should always fit
				  		StrNCat (SearchKey, Months[LstGetSelection (lstMonP)], SCH_KEY_LEN+1);

						FrmReturnToForm (0);
						FldSrchSearchRecs (false);
      				}
		  			else
						FrmReturnToForm (0);
               handled = true;
               break;
               
            case ThisDayNewCancelButton:
	            FrmReturnToForm (0);
               handled = true;
               break;
            	
            default:
               break;
				}
         break;

   	case popSelectEvent: // called on popup list selection
			ThisDayNewCheckDate ();
  			// do not set handled = true;
   		break;

   	case fldChangedEvent:
      case keyDownEvent:
		   {
  			Int16 	day;
	 		UInt16	length;

  			// check for valid day of month
  			if (FldHandleEvent (fldP, event) || event->eType == fldChangedEvent) {
  				ThisDayNewCheckDate ();
				length =   FldGetTextLength (fldP);
				if (length > 0) {
					day = (Int16) StrAToI (FldGetTextPtr (fldP));
					if (day > 31 || day < 1) {
   		   		FldDelete (fldP, length-1, length);
     		 			SndPlaySystemSound (sndError);
      				}
      			}
		  		}
		   }
		   handled = true;
         break;

      case frmOpenEvent:
      	FrmDrawForm (frm);
   
			// Set up pick list for months.
			LstSetListChoices (lstMonP, Months, 12);
  			LstSetSelection (lstMonP, 0);
  			CtlSetLabel (GetObjectPtr (ThisDayNewMonthPopTrigger), 
  				LstGetSelectionText (lstMonP, 0));

			SetNavFocusRing (ThisDayNewDayField);
			//FrmSetFocus (frm, FrmGetObjectIndex (frm, ThisDayNewDayField));
         handled= true;
         break;

		case frmUpdateEvent:
			FrmDrawForm (frm);
		   handled = true;
         break;
  
      default:
      	break;
   	}
      
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:		WriteDBData
//
// DESCRIPTION:	Callback for ExgDBWrite to send data with exchange manager
//
// PARAMETERS:		-> dataP - 		buffer containing data to send
//						-> sizeP - 		number of bytes to send
//						-> userDataP - app defined buffer for context
//											(holds exgSocket when using ExgManager)
//
// RETURNED:    	error if non-zero
////////////////////////////////////////////////////////////////////////////////////
static Err WriteDBData (const void* dataP, UInt32* sizeP, void* userDataP)
{
	Err	err;

	// Try to send as many bytes as were requested by the caller
	*sizeP = ExgSend ((ExgSocketPtr) userDataP, dataP, *sizeP, &err);
	return err;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:		SendDatabase
//
// DESCRIPTION:	Beamss a database to another handheld device using the Exg API.
//
// PARAMETERS:		-> dbNum - 	position in DatabaseList containing file we want
//										to beam.
//
// RETURNED:		error code or zero for no error.
////////////////////////////////////////////////////////////////////////////////////
static Err SendDatabase (const UInt16 dbNum)
{
	ExgSocketType	exgSocket;
	Err				err = 0;
  	LocalID     	dbID = 0;
   Char				fName[FULL_DB_NAME_LEN+1+4]; // make room for ".pdb"
   Char 	 			descript[USER_DB_NAME_SZ + 10];
   Char				*appName = DB_APPNAME; // "GedWise"

	StrPrintF (fName, cDbNameStr, DatabaseList[dbNum].dbName, APP_CREATOR_STR);
	dbID = DmFindDatabase (DatabaseList[dbNum].location, fName);
					 	
	if (!dbID)
		return dmErrCantFind;

	StrPrintF (descript, DB_DBCNT_STR, DatabaseList[dbNum].dbName);
	StrCat (fName, ".pdb");

	// Create exgSocket structure
	MemSet (&exgSocket, sizeof (exgSocket), 0);	
	exgSocket.description = descript;
	exgSocket.name = fName;

	#ifdef GREMLINS
	return 0;
	#endif

	// Start and exchange put operation
	err = ExgPut (&exgSocket);
	if (err) return err;

	// Convert Palm OS database into its external (public) format.
   err = ExgDBWrite (WriteDBData, &exgSocket, appName, dbID, 
   			DatabaseList[dbNum].location);

	err = ExgDisconnect (&exgSocket, err);

	return err;	
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	CopyDbToCard
//
// DESCRIPTION: 	This routine is the event handler for the database list.
//
// PARAMETERS:  	-> dbNum  	 -	position in DbListArray
//						-> volRefNum -	volume reference number where we want to copy
//											the file to.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Err CopyDbToCard (UInt16	dbNum, UInt16 volRefNum)
{
	Err		err 			= 0;
	Boolean	foundPath	= false;
	FormPtr	frm;
	UInt16	cardNo;
	LocalID	dbID;
	Char 		filePathName[OUR_FNAME_LEN+1];
	FileRef 	dirRef 		= NULL;
	UInt8*	scrLocked 	= NULL;
	Int16		deleteOrig;
	
	deleteOrig 	= GetControlValue (CopyDBDeleteCheckbox);
	cardNo 		= DatabaseList[DbNum].location;
	dbID 			= DatabaseList[DbNum].dbID;

	ErrFatalDisplayIf (Pre35Rom, "CopyDBToCard: wrong OS version for WinScreenLock");

	// Construct entire file name, including path and extension.
	StrPrintF (filePathName, cFileNameStr, cPalmPath, DatabaseList[DbNum].dbName,
		APP_CREATOR_STR);

	// Open the directory to make sure it is there.
	err = VFSFileOpen (volRefNum, cPalmPath, vfsModeRead, &dirRef);

	VFSFileClose (dirRef);  // close file reference
	if (err != errNone)
		goto ExitFunc;
	foundPath = true;

	// Display the Wait message form and Export the file
	frm = FrmInitForm (WaitForm);
	FrmSetActiveForm (frm);
	FrmDrawForm (frm);

	err = VFSExportDatabaseToFile (volRefNum, filePathName, cardNo, dbID);
	if (err == vfsErrFileAlreadyExists) {

		FrmReturnToForm (CopyDBForm);

		if (FrmCustomAlert (DBExistsAlert, DatabaseList[DbNum].dbName, DB_CARD_STR,
			"") == 1)
			goto ExitFunc;

		// Display the Wait message form again
		frm = FrmInitForm (WaitForm);
		FrmSetActiveForm (frm);
		FrmDrawForm (frm);
	
		// check if we are deleting the database that is open
		if (OpenFileRef && StrCompare (DatabaseList[dbNum].dbName, DbName) == 0) {
			scrLocked = WinScreenLock (winLockDontCare); // freeze screen
			FrmCloseAllForms (); // if deleting open db, close all forms first
			CloseDatabase ();
			}
      
      err = VFSFileDelete (volRefNum, filePathName);
		
		if (!err)
			err = VFSExportDatabaseToFile (volRefNum, filePathName, cardNo, dbID);
		}
	
	if (!err && deleteOrig) {
		// check if we are deleting the database that is open
		if (OpenDbRef && StrCompare (DatabaseList[dbNum].dbName, DbName) == 0) {
			FrmCloseAllForms (); // if deleting open db, close all forms first
			CloseDatabase ();
			}
			
		#ifndef GREMLINS
		DmDeleteDatabase (cardNo, dbID);
		#endif
		}

	// erase Wait message form and return to CopyDBForm
	if (FrmGetActiveForm () != NULL)
		FrmReturnToForm (CopyDBForm);
	
	// display error if not enough space to copy file
	if (err == vfsErrVolumeFull)
		FrmAlert (DBMemoryErrAlert);
	
	////////
	ExitFunc:
	////////
	if (scrLocked) WinScreenUnlock ();
	if (!foundPath) {
		FrmAlert (NoFolderAlert);
	 	err = vfsErrFileNotFound;
	 	}
	return err;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	CopyDbToHH
//
// DESCRIPTION: 	This routine is the event handler for the database list.
//
// PARAMETERS:  	-> dbNum  - 	position in DbListArray
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Err CopyDbToHH (UInt16	dbNum)
{
	Err		err = 0;
	FormPtr	frm;
	UInt16	cardNo;
	LocalID	dbID;
	UInt8*	scrLocked = NULL;
	UInt16	volRefNum;
	Char 		filePathName[OUR_FNAME_LEN+1];
	Boolean	deleteOrig;
	
	volRefNum = DatabaseList[DbNum].location;
	deleteOrig = (Boolean) GetControlValue (CopyDBDeleteCheckbox);
	
	ErrFatalDisplayIf (Pre35Rom, "CopyDbToHH: wrong OS version for WinScreenLock");
	
	// construct entire file name, including path and extension
	StrPrintF (filePathName, cFileNameStr, cPalmPath, DatabaseList[DbNum].dbName,
		APP_CREATOR_STR);

	// Display the Wait message form and Export the file
	frm = FrmInitForm (WaitForm);
	FrmSetActiveForm (frm);
	FrmDrawForm (frm);

	err = VFSImportDatabaseFromFile (volRefNum, filePathName, &cardNo, &dbID);
	
	if (err == dmErrAlreadyExists) {
	
		FrmReturnToForm (CopyDBForm);
		
		if (FrmCustomAlert (DBExistsAlert, DatabaseList[DbNum].dbName, cHHStr,
			 "") == 1)
			goto ExitFunc;

		// Display the Wait message form again
		frm = FrmInitForm (WaitForm);
		FrmSetActiveForm (frm);
		FrmDrawForm (frm);

		// check if we are deleting the database that is open
		if (OpenDbRef && StrCompare (DatabaseList[dbNum].dbName, DbName) == 0) {
			scrLocked = WinScreenLock (winLockDontCare); // freeze screen
			FrmCloseAllForms (); // if deleting open db, close all forms first
			CloseDatabase ();
			}
		err = DmDeleteDatabase (cardNo, dbID);
		if (!err)
			err = VFSImportDatabaseFromFile (volRefNum, filePathName, &cardNo, &dbID);
		}	
		
	if (!err && deleteOrig) {
		// check if we are deleting the database that is open
		if (OpenFileRef && StrCompare (DatabaseList[dbNum].dbName, DbName) == 0) {
			FrmCloseAllForms (); // if deleting open db, close all forms first
			CloseDatabase ();
			}
		#ifndef GREMLINS
		VFSFileDelete (volRefNum, filePathName);
		#endif
		}

	// erase Wait message form and return to CopyDBForm
	if (FrmGetActiveForm () != NULL)
		FrmReturnToForm (CopyDBForm);

	// display error if not enough space to copy file
	if (err == dmErrMemError)
		FrmAlert (DBMemoryErrAlert);
	
	/////////
	ExitFunc:
	////////
	if (scrLocked) WinScreenUnlock ();
	return err;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	CopyDbGetList
//
// DESCRIPTION: 	This routine constructs a list of choices for the database Copy
// 					drop down list.
//
// PARAMETERS:		-> fileOnHH	- 	true if file to copy is on handheld, else false
// 					<> itemTot 	-	number of items in the list
//
// RETURNED:    	error code or zero if no error.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Err CopyDbGetList (Boolean fileOnHH, UInt16* itemTot)
{
	Err		err = 0;
	UInt16 	volRefNum;
	UInt32 	volIterator = vfsIteratorStart;
	Char* 	volName 		= MemPtrNew (MAX_FNAME_SZ+1);
	Char*		sPos 			= (Char*) ListStr;

	// initialize variables
	*itemTot = 0;
	*ListStr = '\0';

	ErrFatalDisplayIf (!volName, "CopyDbGetList: Out of memory (volName)");

	if (fileOnHH) { // copy from Handheld to Expansion Card
		
		if (ExpCardCapable) {
		
			while (volIterator != vfsIteratorStop) { // search all volumes
				
				err = VFSVolumeEnumerate (&volRefNum, &volIterator);
				if (err != errNone) continue;
				
				err = VFSVolumeGetLabel (volRefNum, volName, MAX_FNAME_SZ);
				if (err != errNone) continue;
				
				if (! *volName)
					StrPrintF (volName, DB_CARDNUM_STR, (*itemTot)+1);
				if (StrLen (volName) > MAX_LNAME_SZ) { // limit name size to MAX_LNAME_SZ
					volName[MAX_LNAME_SZ-1] = chrEllipsis;
					volName[MAX_LNAME_SZ] = '\0';
					}
				
				StrCopy (sPos, volName);
				sPos += StrLen (sPos) + 1;
				FileRefNum[*itemTot] = volRefNum;
				(*itemTot) ++;
				
				if (*itemTot >= CPYDB_RN_AR_SZ - 1) // leave 1 for Beam
					break;
				} // while
				
			}
		else { // file is on handheld but not expansion card capable.
		
			ShowObject (CopyDBDeleteCheckbox, false);
			}

		// add the Beam option to the list
		StrCopy (sPos, cBeamStr);
		FileRefNum[*itemTot] = COPY_BEAM;
		(*itemTot) ++;
		}
	else { // copy from Expansion Card to Handheld 
		StrCopy (sPos, cHHStr);
		sPos += StrLen (sPos) + 1;
		FileRefNum[*itemTot] = COPY_HH;
		(*itemTot) ++;
		}

	MemPtrFree (volName);
	return err;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	CopyDbFreeData
//
// DESCRIPTION: 	This routine frees the memory used by the CopyDB functions.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void CopyDbFreeData (void)
{
	if (ListArrayP) {
	 	MemHandleUnlock (MemPtrRecoverHandle (ListArrayP));
 		MemPtrFree (ListArrayP); // DTR:DO NOT REMOVE. It is needed on older OS's
 	   ListArrayP = NULL;
 	   }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	CopyDBHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the database Copy function.
//
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean CopyDbHandleEvent (EventPtr event)
{
   Boolean  handled = false;
   ListPtr 	lstP = (ListPtr) GetObjectPtr (CopyDBCopyDBList);
   
   switch (event->eType)
		{
     	case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case CopyDBCancelButton:
 	           	CopyDbFreeData ();
               FrmReturnToForm (0);
               handled = true;
               break;

            case CopyDBCopyButton:
 					{
 					Boolean 	opCanceled 	= true;
           	
            	if (FileRefNum[LstGetSelection (lstP)] == COPY_HH) {
            		// Copy from Card to Handheld
						if (FrmCustomAlert (ConfirmAlert, cCopyStr,
						 	DatabaseList[DbNum].dbName, "") == 0)
							opCanceled = (CopyDbToHH (DbNum) != 0);
            		}
					else if (FileRefNum[LstGetSelection (lstP)] == COPY_BEAM) {
						// Beam
						if (FrmCustomAlert (ConfirmAlert, cBeamStr,
						 	DatabaseList[DbNum].dbName, "") == 0) {
            			SendDatabase (DbNum);
            			}
						}
					else  {
						// Copy from Handheld to Card
						if (FrmCustomAlert (ConfirmAlert, cCopyStr,
						 	DatabaseList[DbNum].dbName, "") == 0)
							opCanceled = (CopyDbToCard (DbNum,
								FileRefNum[LstGetSelection (lstP)]) != 0);
						}
               	
              	CopyDbFreeData ();

               if (FrmGetActiveForm () != NULL)
               	FrmReturnToForm (0);

               if (!opCanceled) {
	               ResetTopVisDBNum = false;
						FrmGotoForm (DatabasesForm);
						}
					
               handled = true;
               break;
               }

            default:
               break;
         	}
         break;

		case popSelectEvent:
			// hide delete original file option if beaming a database
			ShowObject (CopyDBDeleteCheckbox, (Boolean) FileRefNum[LstGetSelection (lstP)]
				!= COPY_BEAM);
			// do not set handled = true;
			break;

  		case menuEvent:
         return MenuDoCommand(event->data.menu.itemID);
		   // don't set handled to true; this event must fall through to the system.
		   break;

	   case frmUpdateEvent:
		   FrmDrawForm (FrmGetActiveForm());
         handled= true;
         break;
	   
      case frmOpenEvent:
			{
			MemHandle 	listArrayH;
			UInt16		itemTot;

	   	FrmDrawForm (FrmGetActiveForm());
			CopyDbGetList (DatabaseList[DbNum].dbID != 0, &itemTot);

			listArrayH = SysFormPointerArrayToStrings (ListStr, itemTot);
			
			if (listArrayH == NULL) {
				ErrDisplay ("CopyDbHandleEvent: Unable to init listArrayH");
				FrmReturnToForm (0);
				}
			else {	// set up Copy To destination list
				ListArrayP = MemHandleLock (listArrayH);
				LstSetHeight (lstP, min (3, itemTot));  // allow 3 list options
				LstSetListChoices (lstP, ListArrayP, itemTot);
		   	LstSetSelection (lstP, 0);
   	   	LabelP = LstGetSelectionText (lstP, 0);
     			CtlSetLabel (GetObjectPtr (CopyDBCopySelectPopTrigger), LabelP);
     			}

         handled= true;
         break;
			}

      case frmCloseEvent:
      	CopyDbFreeData ();
         break;
			
	   default:
		   break;
   	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DBCompareRecords
//
// DESCRIPTION:   Compare two array elements.
//
// PARAMETERS:    -> db1 -	database element 1.
//                -> db2 - database element 2.
//
// RETURNS:       negative number if record one is less.
//                positive number if record two is less.
//						0 if equal
////////////////////////////////////////////////////////////////////////////////////
static Int16 DBCompareRecords(DbArrayType *db1, DbArrayType *db2, UInt16) 
{
  return StrCaselessCompare (db1->dbName, db2->dbName);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      VfsAppInfoGetPtr
//
// DESCRIPTION:   This routine gets a locked handle to the application info block
//						of a pdb database located on an expansion card.                 
//
// PARAMETERS:    -> fileRef -	file reference number for open database. This can
//											be NULL if database is closed.
//						-> volRefNum - volume on which the database is located.
//                -> userName - 	user name of database we are interested in.
//
// RETURNED:      A locked handle to a DBInfoPtr, or NULL if error.  This handle
//						must be unlocked by the calling routine.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
DBInfoPtr VfsAppInfoGetPtr (FileRef fileRef, const UInt16 volRefNum,
 				 const Char* userName)
{
	Err 			err 		 = errNone;
	MemHandle	appInfoHP = NULL;
	Boolean		closeFile = false;
	DBInfoPtr	dbInfoPtr;
	
	if (!fileRef) {
		Char filePathName[OUR_FNAME_LEN+1];
		ErrFatalDisplayIf (!filePathName, "Out of memory");

		// construct entire file name, including path and extension
		StrPrintF (filePathName, cFileNameStr, cPalmPath, userName, APP_CREATOR_STR);
		ErrFatalDisplayIf (StrLen (filePathName) > OUR_FNAME_LEN, "File name error");
	
		// open the file
		err = VFSFileOpen (volRefNum, filePathName, vfsModeRead, &fileRef);
		closeFile = true; // remember we have to close file if we opened it
		}
		
	if (err == errNone) {
		// get file name and application info pointer
		err = VFSFileDBInfo (fileRef, NULL, NULL, NULL, NULL, NULL, NULL, 
					NULL, &appInfoHP, NULL, NULL, NULL, NULL);
		
		dbInfoPtr = MemHandleLock (appInfoHP);
		
		if (closeFile)
			VFSFileClose (fileRef);  // close file reference
		
		return dbInfoPtr;
		}
	
 	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      VfsEnumeratVolumes
//
// DESCRIPTION:   This routine checks for databases for this applicatoin on an
//						expansion card.  It also constructs a list of databases found.
//                
// PARAMETERS:    createList -	true if we are creating the DatabaseList array or
//											else it is false if just checking if a database
//											is on the expansion card.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void VfsGetDatabaseList (const Boolean createList)
{
	Err 				err;
	UInt32			fileSize;
	UInt16 			volRefNum;
	UInt32 			volIterator = vfsIteratorStart;
	FileRef 			dirRef;
	FileRef			fileRef;
	UInt32 			fileIterator;
	FileInfoType 	fileInfo;
	UInt32			dbType;
	Char*				cPos;
	Char 				fileName[MAX_FNAME_SZ+1];
	Char 				filePathName[OUR_FNAME_LEN+1];

	// Open the directory and iterate through the files in it.
	while (volIterator != vfsIteratorStop) {
		
		err = VFSVolumeEnumerate (&volRefNum, &volIterator);
		if (err != errNone) break;
		
		err = VFSFileOpen (volRefNum, cPalmPath, vfsModeRead, &dirRef);
		//if (err != errNone) break; // DTR removed 6-6-2005
		if (err != errNone) {
			VFSFileClose (dirRef); // close directory reference  // DTR 6-6-2005
			continue;  // DTR 6-6-2005
			}
		
		// Iterate through all the files in the open directory
		fileInfo.nameP = fileName; // point to local buffer
		fileInfo.nameBufLen = MAX_FNAME_SZ;
		fileIterator = expIteratorStart;
		
		while (fileIterator != expIteratorStop) {
			// get the next file and put file's information in fileInfo
			err = VFSDirEntryEnumerate (dirRef, &fileIterator, &fileInfo);
			if (err == vfsErrIsADirectory) continue;
			if (err != errNone) break;

			// eliminate files with names too long to be our app's database.
			if (StrLen (fileName) > FULL_DB_NAME_LEN+4)
				continue;

			StrCopy (filePathName, cPalmPath);
			StrNCat (filePathName, "/", OUR_FNAME_LEN+1);// DTR: 6-6-2005
			StrNCat (filePathName, fileInfo.nameP, OUR_FNAME_LEN+1);

			// open the file
			err = VFSFileOpen (volRefNum, filePathName, vfsModeRead, &fileRef);
			if (err != errNone) break;

			// get file name and file type
			err = VFSFileDBInfo (fileRef, NULL, NULL, NULL, NULL, NULL, NULL, 
						NULL, NULL, NULL, &dbType, NULL, NULL);

			if (err != errNone || dbType != APP_DB_TYPE) {
			   VFSFileClose (fileRef); // close file reference
				continue; // we only want files for our application
				}
					
			if (createList) {
				cPos = StrChr (fileInfo.nameP, chrHyphenMinus); // get user name for db
				*cPos = '\0';
				StrCopy (DatabaseList[TotDatabases].dbName, fileInfo.nameP);
				DatabaseList[TotDatabases].dbID = 0; // flag db is on exp card
				DatabaseList[TotDatabases].location = volRefNum;
				VFSFileSize(fileRef, &fileSize);
				DatabaseList[TotDatabases].size = fileSize / 1024;
				TotMemory+= DatabaseList[TotDatabases].size;
				}
			
			VFSFileClose (fileRef);  // close file reference
			TotDatabases++;
								
			if (!createList) break;
			if (TotDatabases == DBaseCountMax) break;
			} // while

		VFSFileClose (dirRef); // close the directory reference
			
		} // while
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      GetDatabaseList
//
// DESCRIPTION:   This routine counts the databases (TotDatabases) on the HH.
//                It also calculates the total memory used by the databases
//						(TotMemory). It will also build a list of databases if 
//						createList is true and it puts the database info in the 
//						DatabaseList array.
//
// PARAMETERS:    -> createList - 	true if building list of databases for the
//												DatabaseList array.
//
// RETURNED:      Err - zero if no error, else the error.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Err GetDatabaseList (const Boolean createList)
{
   DmSearchStateType    searchState;
   Err                  err = 0;
   LocalID              dbID;
   UInt16               cardNo;
   Char						dbName[FULL_DB_NAME_LEN+1];
   Boolean					newSearch = true;
	Char*						cPos;
	UInt32					dbSize;
	
	TotMemory = TotDatabases = 0;

	while (!err) {
   	// get database ID for db file.
   	err = DmGetNextDatabaseByTypeCreator(newSearch, &searchState, APP_DB_TYPE,
         	   APP_CREATOR, false, &cardNo, &dbID);
		if (err) break; // reached last database (or no databases found).
		
		// get database name.
		err = DmDatabaseInfo(cardNo, dbID, dbName, NULL, NULL, NULL, NULL,
		 			NULL, NULL,	NULL, NULL, NULL, NULL);			
		if (err) break;
					 
		if (createList) {		
			cPos = StrChr (dbName, chrHyphenMinus); // get user's file name for db
			*cPos = '\0';
			*DatabaseList[TotDatabases].dbName = '\0';
			StrCat (DatabaseList[TotDatabases].dbName, dbName);
			DatabaseList[TotDatabases].dbID = dbID;
			DatabaseList[TotDatabases].location = cardNo;
			DmDatabaseSize (cardNo, dbID, NULL, &dbSize, NULL);
			DatabaseList[TotDatabases].size = dbSize / 1024; //convert to KB
			TotMemory+= DatabaseList[TotDatabases].size;
			}
		
		newSearch = false;
		TotDatabases++;

		if (!createList) break; // we just needed to make sure > 1 database on HH

		if (TotDatabases == DBaseCountMax) break;
		}

	// check if any files on expansion card
	if (ExpCardCapable)
		VfsGetDatabaseList(createList);
		
	if (createList && TotDatabases > 1) {
		SysQSort (DatabaseList, TotDatabases, sizeof(DbArrayType),
			(CmpFuncPtr) &DBCompareRecords, 0);
		}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DeleteDatabase
//
// DESCRIPTION:   Delete the database that has the given position in the 
//						DatabaseList array.
//
// PARAMETERS:    -> dbNum - 	the position in DatabaseList which has infor on the
//										database that is to be deleted.
//
// RETURNED:      zero if no error, else 1
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Err DeleteDatabase (UInt16 dbNum)
{
   LocalID	dbID;
   Boolean	closeFirst = false;
   Boolean	onExpCrd;
   UInt16	location;
	Char		filePathName[OUR_FNAME_LEN+1];
	
	ErrFatalDisplayIf (!filePathName, "Out of memory");

	onExpCrd = !DatabaseList[dbNum].dbID;
 	location = DatabaseList[dbNum].location;

	if (DatabaseList[dbNum].dbName == NULL) {
		return 1; // error
		}
		
	if (onExpCrd) {
		// construct entire file name, including path and extension
		StrPrintF (filePathName, cFileNameStr, cPalmPath, DatabaseList[dbNum].dbName,
	 		APP_CREATOR_STR);
		ErrFatalDisplayIf (StrLen (filePathName) > OUR_FNAME_LEN, "File name error");
		}
	else {
		StrPrintF (filePathName, cDbNameStr, DatabaseList[dbNum].dbName,
			APP_CREATOR_STR);	
		dbID = DmFindDatabase (location, filePathName);
		if (!dbID) {
			return 1; // error
			}
		}
		
	// check if we are deleting the database that is open
	if (OpenDbRef || OpenFileRef) { // if so then a database is open
		
		// set flag to close open database before deleting
		closeFirst = (StrCompare(DatabaseList[dbNum].dbName, DbName) == 0 &&
					    ((DatabaseList[dbNum].dbID && !Prefs[LastFileOnCrd]) ||
				  		  (!DatabaseList[dbNum].dbID && Prefs[LastFileOnCrd]) ));

		if (closeFirst) {
			FrmCloseAllForms (); // if deleting open db, close all forms first
			CloseDatabase ();
			}
		}

	#ifndef GREMLINS
	if (onExpCrd)
		VFSFileDelete (location, filePathName);
	else
		DmDeleteDatabase (location, dbID);
	#endif  		

	return 0; // no error
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DBListDrawRecord
//
// DESCRIPTION:   This routine draws an Database List entry into the 
//                the DatabasesTable.  It is called as a callback
//                routine by the table object.
//
// PARAMETERS:    -> table  - pointer to the address list table
//                -> row    - row number, in the table, of the item to draw
//                -> column - column number, in the table, of the item to draw
//                -> bounds - bounds of the draw region
//
// RETURNS:      	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DBListDrawRecord (void *table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   Int16 			y;
   UInt16			dbNum;
   FontID 	   	curFont;
   Char				numStr[8];
   Boolean			match;
   UInt16			totPeople = 0;
   DBInfoPtr		dbInfoP;
   MemHandle 		memH;
   RGBColorType 	drawColor = RGB_CT_BLACK;  // default draw color is black

   dbNum = (UInt16) TblGetRowData (table, row);
   y = bounds->topLeft.y;

	// Get pointer to database on Expansion Card or in RAM.
	if (DatabaseList[dbNum].dbID == 0)
		dbInfoP = VfsAppInfoGetPtr (NULL, DatabaseList[dbNum].location,
 				 		 DatabaseList[dbNum].dbName);
 	else
		dbInfoP = AppInfoGetPtr (NULL, DatabaseList[dbNum].dbID);

	if (dbInfoP) {
		totPeople = dbInfoP->peopleCnt;
		if (DatabaseList[dbNum].dbID == 0) {
			memH = MemPtrRecoverHandle (dbInfoP); // revised 10-08-2004
   		MemHandleUnlock (memH); // revised 10-08-2004
   		MemHandleFree (memH); // revised 10-08-2004
  			}
		else
			MemPtrUnlock (dbInfoP);
		}
	
	// Determine if db name being displayed is the open database so it can be drawn
	// in a different color.  Take special care b/c it is possible there could be
	// more than one expansion card installed.
	match = ( StrCompare (DatabaseList[dbNum].dbName, DbName) == 0 &&
				 DatabaseList[dbNum].location == FileLoc &&
				( (DatabaseList[dbNum].dbID != 0 && OpenDbRef != 0) || 
				  (DatabaseList[dbNum].dbID == 0 && OpenFileRef != 0) ) );

   curFont = FntSetFont (DBLIST_FONT);
	
	if (match && SupportsColor) {
		drawColor = (RGBColorType) RGB_CT_GREEN; // set color to green
		}
	
	// Draw database name.
	DrawCharsColorI (DatabaseList[dbNum].dbName, drawColor, DB_NAME_X, y);
	
	// Draw total names in database.
	StrPrintF (numStr, "%u", totPeople);
   DrawCharsColorI (numStr, drawColor, DB_NAMS_X - FntCharsWidth (numStr,
   	StrLen (numStr)), y);
   
	// Draw Size of database.
	StrPrintF (numStr, "%dK", DatabaseList[dbNum].size);
  	DrawCharsColorI (numStr, drawColor, DB_SIZE_X - FntCharsWidth (numStr,
  		StrLen (numStr)), y);

	FntSetFont (stdFont);
	
	// Draw the 'Arrow' next to name of open database.
	if (match) {
		Char	arrowStr[2] = {0xBB, '\0'}; // for non-color displays
		DrawCharsColorI (arrowStr, drawColor, DB_OPEN_X, y-1); // draw ">>"
		}

   // Draw picture of expansion card for db that is on expansion card. Do this
   // last as it changes the draw color.
	if (DatabaseList[dbNum].dbID == 0) {
		Char	cardIconStr[2] = {chrCardIcon, '\0'};
		drawColor = (RGBColorType) RGB_CT_BLUE; // set color to blue
		DrawCharsColorI (cardIconStr, drawColor, DB_CARD_X, y+1);
   	}
   
	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DBListScroll
//
// DESCRIPTION:   This routine scrolls the datbase list in the direction
//                specified.
//
// PARAMETERS:    -> direction	- up or dowm
//
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void DBListScroll (WinDirectionType direction)
{
	TablePtr		table;
	UInt16		lastRow;
	UInt16 		newTopVisibleRecN;
 
 	table = (TablePtr) GetObjectPtr (DatabasesDBListTable);
 
   lastRow = (UInt16) TblGetLastUsableRow (table);
   
	// There must be at least one row in the table.
	newTopVisibleRecN = TopVisibleDBNum;

	if (direction == winDown) { // scroll the table down
	   newTopVisibleRecN += lastRow;  // scroll one page at a time.
	   if (newTopVisibleRecN + lastRow + 1 > TotDatabases) {
	   	if (TotDatabases > lastRow + 1)
	     		newTopVisibleRecN = TotDatabases - lastRow - 1;
	     	else
	     		newTopVisibleRecN = 0;
	     	}
      }
   else { // scroll the table up
		if (newTopVisibleRecN > lastRow)
      	newTopVisibleRecN -= lastRow;
    	else
  			newTopVisibleRecN = 0;
      }
       
	// Avoid redraw if no change
	if (TopVisibleDBNum != newTopVisibleRecN) {
		TopVisibleDBNum = newTopVisibleRecN;
		DBListLoadTable ();
		DBListShowInfo (false);	
		TblRedrawTable (table);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DBListLoadTable
//
// DESCRIPTION: 	This routine loads Databases names and related information
//              	the list view form.
//
// PARAMETERS:  	None.
//
// RETURNS:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DBListLoadTable (void)
{
   Int16			row;
   Int16			numRows;
   UInt16		dbNum;  // position of database in DBListArray (zero based)
   TablePtr 	table;
   Boolean  	scrollableU;
   Boolean 		scrollableD; 
   
   table = (TablePtr) GetObjectPtr (DatabasesDBListTable);
   
   TblUnhighlightSelection (table); // DTR: Added 6-28-03
   
   numRows = TblGetNumberOfRows (table);

   if (TotDatabases > 0) { // check that there is at least one database.
   
	   dbNum = TopVisibleDBNum;
   
	   for (row = 0; row < numRows; row++) {
         TblSetRowUsable (table, row, true);
         TblMarkRowInvalid (table, row);
         TblSetRowSelectable (table, row, true);
         TblSetRowData (table, row, dbNum);
         
        	dbNum++;

         if (dbNum >= TotDatabases) {
            break;
         	}
      	}  // end for loop
      
   } // if TotDatabases > 0

   // Update the scroll buttons.
  	if (TotDatabases == 0) {
  		scrollableU = scrollableD = false;
  		}
  	else {
     scrollableU = (Boolean) (TopVisibleDBNum > 0);
	  row = TblGetLastUsableRow ((TablePtr) GetObjectPtr (DatabasesDBListTable));
     scrollableD = (Boolean) ((TopVisibleDBNum + row + 1) < TotDatabases);
     }
     
   UpdateScrollers (DatabasesScrollUpRepeating, DatabasesScrollDownRepeating,
   	scrollableU, scrollableD);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DBListInit
//
// DESCRIPTION:   This routine initializes the Database List for the
//                application.
//
// PARAMETERS:    None.
//
// RETURNS:      	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DBListInit (void)
{
   Int16 	   row;
   TablePtr    table;
   
   table = (TablePtr) GetObjectPtr (DatabasesDBListTable);
   
   // Initialize the database list table.
   for (row = 0; row < TblGetNumberOfRows (table); row++) {      
      TblSetItemStyle (table, row, 0, customTableItem);
      TblSetRowUsable (table, row, false);
      }

   TblSetColumnUsable (table, 0, true);
   TblSetCustomDrawProcedure (table, 0, DBListDrawRecord);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DBListShowInfo
//
// DESCRIPTION:   Called to load GEDCOM file information into form.
//     
// PARAMETERS:    None.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DBListShowInfo (Boolean show)
{
  	UInt16 	objID;
  	
  	// Hide or show the following:
  	// DatabasesFileLabel, DatabasesFileField, DatabasesCreatedLabel,
  	// DatabasesCreatedField, DatabasesLoadedLabel, DatabasesLoadedField,
  	// DatabasesSelectButton, DatabasesDeleteButton, DatabasesCopyButton
  	for (objID = DatabasesFileLabel; objID <= DatabasesCopyButton; objID++)
	  	ShowObject (objID, show);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DBListGetInfo
//
// DESCRIPTION:   Called to load GEDCOM file informational form. It will also check
//						that the selected database is the correct version.  If it is an
//						old version, an error message is displayed and only the "Delete"
//						button is displayed.
//     
// PARAMETERS:    dbNum -	the position in DatabaseList array of database.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Err DBListGetInfo (UInt16 dbNum)
{
   FieldPtr    fldP;
   DBInfoPtr  	dbInfoP;
   Char*			sPtr;
   MemHandle	memH;

	// get pointer to application information for the database.
	if (DatabaseList[dbNum].dbID == 0 && ExpCardCapable)
		dbInfoP = VfsAppInfoGetPtr (NULL, DatabaseList[dbNum].location, 
					 	 DatabaseList[dbNum].dbName);
	else
		dbInfoP = AppInfoGetPtr (NULL, DatabaseList[dbNum].dbID);
   
    // check if database is wrong version
   if (dbInfoP == NULL || dbInfoP->dbVers != APP_DB_VERS) {
   	DBListShowInfo (false);
   	ShowObject (DatabasesDeleteButton, true);
		if (dbInfoP)
			if (DatabaseList[dbNum].dbID == 0) { // then db on expansion card
   			memH = MemPtrRecoverHandle (dbInfoP); // revised 10-08-2004
   			MemHandleUnlock (memH); // revised 10-08-2004
   			MemHandleFree (memH); // revised 10-08-2004
   			}
			else 
				MemPtrUnlock (dbInfoP);
		return dmErrCantOpen;
   	}
   
   fldP = (FieldPtr) GetObjectPtr (DatabasesFileField);
   FldSetTextPtr (fldP, dbInfoP->gedFile);
   FldDrawField (fldP);    

   fldP = (FieldPtr) GetObjectPtr (DatabasesCreatedField);
   
   if (!*dbInfoP->gedDate)
   	sPtr = cUnknownStr;
   else	
   	sPtr = dbInfoP->gedDate;
   FldSetTextPtr (fldP, sPtr);
   FldDrawField (fldP);    
	
   fldP = (FieldPtr) GetObjectPtr (DatabasesLoadedField);

   if (!*dbInfoP->loadDate)
   	sPtr = cUnknownStr;
   else	
   	sPtr = dbInfoP->loadDate;
  	FldSetTextPtr (fldP, sPtr);
   FldDrawField (fldP);
             
   if (DatabaseList[dbNum].dbID == 0) { // then db on expansion card
   	memH = MemPtrRecoverHandle (dbInfoP); // revised 10-08-2004
   	MemHandleUnlock (memH); // revised 10-08-2004
   	MemHandleFree (memH);   // revised 10-08-2004
   	}
	else   
   	MemPtrUnlock (dbInfoP);
   
	DBListShowInfo (true);
	return 0;
   }

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DBListScreenInit
//
// DESCRIPTION: 	This routine initializes the screen for the database form.
//
// PARAMETERS:  	None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DBListScreenInit (void)
{
	char		fldStr[NREF_LEN] = "\0";
	FontID 	curFont;
	UInt16	xPos;
	const 	Char labelDBCnt[] 	= "DB Tot:";
	const 	Char labelMemory[] 	= "Mem:";
	const 	Char labelDBName[] 	= "Database";
	const 	Char labelNames[] 	= "People";
	const 	Char labelSize[] 		= "Size";
	#define  Y_HEAD 18
	#define  Y_SUMM 88
	
	DrawScreenLines (DatabasesForm);

	// Draw column labels at top of table
	curFont = FntSetFont (boldFont);
	WinSetUnderlineMode (solidUnderline);
	WinDrawChars (labelDBName, sizeof (labelDBName) - 1, DB_NAME_X, Y_HEAD);
	WinDrawChars (labelNames, sizeof (labelNames)-1,
	 	DB_NAMS_X - FntCharsWidth (labelNames, sizeof (labelNames) - 1), Y_HEAD);
	WinDrawChars (labelSize, sizeof (labelSize)-1,
	 	DB_SIZE_X - FntCharsWidth (labelSize, sizeof (labelSize) - 1), Y_HEAD);
	WinSetUnderlineMode (noUnderline);
		
	xPos = DB_NAME_X + FntCharsWidth (labelDBCnt, 7) + 4;
	FntSetFont (curFont);
	
	// Draw summary information at bottom of table
	StrIToA (fldStr, TotDatabases);
	WinDrawChars (fldStr, StrLen (fldStr), xPos , Y_SUMM); 

	StrPrintF (fldStr, "%dK", TotMemory);
	xPos = FntCharsWidth (fldStr, StrLen (fldStr)); 
	WinDrawChars (fldStr, StrLen (fldStr), DB_SIZE_X - xPos, Y_SUMM);
		
	FntSetFont (boldFont);
	xPos+= FntCharsWidth (labelMemory, sizeof (labelMemory) - 1);
	WinDrawChars (labelDBCnt, sizeof (labelDBCnt) - 1, DB_NAME_X, Y_SUMM);
	WinDrawChars (labelMemory, sizeof (labelMemory) - 1, DB_SIZE_X - xPos - 4, Y_SUMM);
	FntSetFont (curFont);
	
	ResetTopVisDBNum = true;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DBListHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the database list.
//
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean DBListHandleEvent (EventPtr event)
{
   Boolean  	handled = false;
	TablePtr 	table = (TablePtr) GetObjectPtr (DatabasesDBListTable);
	Int16			row, col;
	DirType 		navDir;
	MemHandle 	DatabaseH;
  
   switch (event->eType)
		{
		case tblSelectEvent:
		   DbNum = TblGetRowData (event->data.tblSelect.pTable,
        		event->data.tblSelect.row);
        	if (DBListGetInfo (DbNum) == dmErrCantOpen)
	        	FrmAlert (DBVersionAlert);
         
         handled = true;
         break;
		
		case ctlEnterEvent:
			if (event->data.ctlEnter.controlID == DatabasesMenuButton) {
				EvtEnqueueKey (vchrMenu, 0, commandKeyMask); // open the Menu Bar
        		handled = true;
        		}
         	break;
		
      case ctlSelectEvent:
			TblGetSelection (table, &row, &col);
         DbNum = TblGetRowData (table, row);

         switch (event->data.ctlSelect.controlID)
         	{
            case DatabasesDoneButton:
            	if (OpenDbRef || OpenFileRef)
           	      FrmGotoForm (PriorFormID);
               else
               	SndPlaySystemSound (sndWarning);
               handled = true;
               break;
               
            case DatabasesSelectButton:
					// Open database, 0 returned if no problem
					if (!OpenDatabase (DatabaseList[DbNum].dbName, dmModeReadOnly,
        			  	DatabaseList[DbNum].location, !DatabaseList[DbNum].dbID)) {
						

						// Added RelCalcRecN2 back here as it was incorrectly setting
						// it to NOREC in OpenDatabase function. DTR 12/22/04
						CurrentIndiRecN = RelCalcRecN2 = NO_REC;
				
						// init Jump array when selecting new database
						MemSet (Jump, sizeof (Jump), 0xFF);

                  FrmGotoForm (IndiListForm);
                  }
               else {
                	FrmAlert (DBOpenAlert);
                	FrmGotoForm (DatabasesForm);
                	}
               handled = true;
               break;

            case DatabasesDeleteButton:
 					if (FrmCustomAlert (ConfirmAlert, DB_DEL_STR,
 						DatabaseList[DbNum].dbName, "") == 0) {
            		DeleteDatabase (DbNum);
            		if (TopVisibleDBNum > 0) // make sure we don't leave
            			TopVisibleDBNum--;	 // blank line at bottom of db list
            		ResetTopVisDBNum = false;
     		        	FrmGotoForm (DatabasesForm);
         		   }
              	handled = true;
               break;

            case DatabasesCopyButton:
					FrmPopupForm (CopyDBForm);
               handled = true;
               break;

            default:
               break;
         	}
         
         break;

   	case ctlRepeatEvent:
         switch (event->data.ctlRepeat.controlID)
         	{
            case DatabasesScrollUpRepeating:
               DBListScroll (winUp);
               // leave unhandled so the buttons can repeat
               break;

            case DatabasesScrollDownRepeating:
                 DBListScroll (winDown);
               // leave unhandled so the buttons can repeat
               break;

            default:
               break;
             }  

      case keyDownEvent:
	      
	      if (NavKeyHit (event, &navDir)) {
	      
				switch (navDir) {
			
					case NavUp:
					case NavDn:
						DBListScroll (navDir == NavUp ? winUp : winDown);
            		handled = true;
					
					case NavL:
					case NavR:
						handled = true;
						break;
				
					case NavSel:
						handled = true; // prevent highlighting other buttons
						break;
					
					default:
						break;
					}
         	}
      	break;
         
      /*   
         
         if (EvtKeydownIsVirtual (event))
            {
          	switch (event->data.keyDown.chr)
         		{
            	case vchrPageUp:
               	DBListScroll (winUp);
               	handled = true;
               	break;
               
            	case vchrPageDown:
                  DBListScroll (winDown);
               	handled = true;
               	break;
               	
               default:
                  break;
               }
         	}
         break; */

  		case menuEvent:
         return MenuDoCommand (event->data.menu.itemID);
		   // don't set handled to true; event must fall through to the system.
		   break;

     	case fldEnterEvent:
			// This must be here so user cannot select a field!
         handled = true;
         break;

	   case frmOpenEvent:
		
  			// Allocate the database list array (dynamic heap).
			DatabaseH = MemHandleNew (sizeof (DbArrayType) * DBaseCountMax);
   		ErrFatalDisplayIf (!DatabaseH, "Out of memory");
   		DatabaseList = MemHandleLock (DatabaseH);
			
			if (ResetTopVisDBNum)
				TopVisibleDBNum = 0;

			// Draw empty database form
      	DBListInit ();
      	FrmDrawForm (FrmGetActiveForm ());
			DBListShowInfo (false);
			DrawScreenLines (DatabasesForm);

			// Display the Wait message
  	     	ShowObject (DatabasesWaitLabel, true);
	
         GetDatabaseList (true);
   
         // Remove the Wait message
   		ShowObject (DatabasesWaitLabel, false);
   		
			DBListLoadTable ();
			DBListScreenInit ();
			TblDrawTable (table);

 			if (TotDatabases == 0) // no databases on HH
 				FrmAlert (NoDatabaseAlert);
	       
	      handled= true;
         break;

	   case frmUpdateEvent:
			DBListShowInfo (false);
			FrmDrawForm (FrmGetActiveForm ());

			// Reset row selection
			if (TblGetSelection (table, &row, &col)) {
				TblEraseTable (table);  // for backward compatibility
				TblDrawTable (table);
				FrmDrawForm (FrmGetActiveForm ()); // for backward compatibilty
				TblSelectItem (table, row, 0);
				DbNum = TblGetRowData (table, row);
				DBListGetInfo (DbNum);
				}
				
			DBListScreenInit ();
			handled = true;
			break;

      case frmCloseEvent: 
        	MemHandleFree (MemPtrRecoverHandle (DatabaseList));
			DatabaseList = NULL;	
         break;

	   default:
		   break;
   	}

   return (handled);
}