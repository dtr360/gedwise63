////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:  	GedWise 6.0 & 6.1
//
// FILE:		AppMisc.c
//
// AUTHOR:  	Daniel T. Rencricca: October 15, 2004
//
// DESCRIPTION:	This is the application's module for various
//              utility functions
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2005 Battery Park Software Corporation.
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
extern UInt32		InstallDate;
extern UInt16		CurrentIndiRecN;
extern UInt16  		CurrentSpouRecN; 
extern MemHandle 	FamiRecH;
extern MemHandle 	IndiRecH;
extern DBRecordType  	IndiRec;
extern DBRecordType	FamiRec;
extern UInt16		TotDatabases;
extern UInt16		FileLoc;
extern UInt16		ListViewSelectRecN;
extern Boolean		SupportsColor;
extern Boolean		Pre35Rom;
extern Boolean		Pre60Rom;
extern Char 		cUnknownStr[];
extern Char			DbName[];
extern Boolean		RelCalcEntry;
extern UInt16		RelCalcRecN2;// rec num of indiv 2 in Rel Calc.
extern UInt16		RelCalcGen;  // num of generations to search in Rel Calc.
extern Boolean		RelCalcGetRec1;
extern UInt16		PriorFormID;
extern UInt16		HldIndiRecN;
extern UInt16		Jump[];
extern UInt16		IndiDBNumRecs;
extern RGBColorType	Color1;
extern RGBColorType	Color2;
extern Boolean		UpdateFrm;
extern Boolean		Prefs[]; 	// user preferences
extern FontID		PrefFnts[]; // user font preferences
extern Boolean		DynInDevice;
extern Boolean		AboutMode;

////////////////////////////////////////////////////////////////////////////////////
// Local Variables and Defines
////////////////////////////////////////////////////////////////////////////////////

// Descendancy Chart & Ancestry Chart variables
static 	QueueHistType*	QueueHist = 0;
static 	UInt16 			HistPos;
static 	DescListType*	DescList = 0;
static 	UInt16			TopDescPos;
static 	UInt16			DescPos;

// Date Calculator variables
Char*					Months[12] = {	"Jan","Feb","Mar","Apr","May","Jun",
		 				"Jul","Aug","Sep","Oct","Nov","Dec" };
static 	Char*			MonNum[12] = {	"0", "1", "2","3", "4", "5", "6",
						"7", "8","9", "10","11"};
static 	Char			DateCalcStr[20]; // holds date calculation results

// Relationship Calculator variables
static Char*	cSibling	= "sibling";
static Char* 	cGreat	= "gr-";
static Char* 	cGrand	= "grand";
static Char* 	cChild	= "child";
static Char* 	cParent	= "parent";
static Char* 	cNephew	= "nephew";
static Char* 	cNiece	= "niece";
static Char* 	cCousin	= "cousin";
static Char* 	cUncle	= "uncle";
static Char* 	cAunt	= "aunt";
static Char* 	cNst	= "st";
static Char*	cNnd	= "nd";
static Char*	cNrd	= "rd";
static Char*	cNth	= "th";

static Char				RCStr[32]; // holds relationship calculation results
static RCListType*		RCList1;
static RCListType*		RCList2;
static RCHistType*		RCHist;
static UInt16			RCHistLast;
static UInt16*			RCAncArr;
static Boolean			RCFastSrch = true; // init
		 UInt16			RelCalcRecN1;
static UInt16			RCArrSz  = RC_ARR_SZ;
static UInt16			RCArrGen = RC_ARR_GEN;
static UInt16			ProgRecN1;
static UInt16			ProgRecN2;

////////////////////////////////////////////////////////////////////////////////////
//	Internal Function Definitions																	 //
////////////////////////////////////////////////////////////////////////////////////

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
//					-> showObj 	-	true to show object, else false to hide
//
// RETURNED:    	Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void ShowObject (const UInt16 objectID, const Boolean showObj)
{
   FormPtr 	frm;
   UInt16	objIndx; 
   
   frm 	  = FrmGetActiveForm ();
   objIndx = FrmGetObjectIndex (frm, objectID);
   
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
//					-> rect		- rectangle
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
// FUNCTION:    	SetControlValue
//
// DESCRIPTION: 	This routine sets the value of a control.
//
// PARAMETERS:  	-> controlID - the ID of a control
//					-> value 	 -	true for on, false for off
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void SetControlValue (const UInt16 controlID, const Boolean value)
{
   FormPtr	frm = FrmGetActiveForm ();
	CtlSetValue (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm,	controlID)), value);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      EraseRectangleObject
//
// DESCRIPTION:   Erases a rectangular object (eg. a gadget) in a form
//
// PARAMETERS:    -> objectId  - Id of object to erase
//
// RETURNS:       Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
void EraseRectangleObject (const UInt16 objectId)
{
   RectangleType 	r;
   
   GetObjectBounds (objectId, &r);
   WinEraseRectangle (&r, 0);
}


////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      	ShiftForm
//
// DESCRIPTION:		This routine resizes and draws the Individual Summary Form.
//
// PARAMETERS:   	None.
//
// RETURNED:      	True if form was resized, else false (no changed in size).
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
/*Boolean ShiftForm (void)
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
} */

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      	DrawScreenLines
//
// DESCRIPTION:   	Draws the lines on screen for the form ID passed in
//				  	in the whichForm parameter.
//
// PARAMETERS:   	-> whichForm - the form ID on which the lines are to be drawn.
//
// RETURNS:      	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DrawScreenLines (const UInt16 whichForm)
{
	if (SupportsColor) {
		RGBColorType 	RGB = RGB_CT_BLUE;
   	WinPushDrawState ();
		WinSetForeColor (WinRGBToIndex (&RGB));	
		}

	switch (whichForm)
		{
		case IndiSummForm:
			// draw two horizontal lines
	 		WinDrawGrayLine (0,61,160,61); 
    		WinDrawGrayLine (0,99,160,99);
    		WinDrawGrayLine (0,100,160,100);
    		break;

    	case FldSearchForm:
    		// draw two horizontal lines
    		WinDrawLine (0, 12, 159, 12);
			WinDrawLine (0, 13, 159, 13);
			WinDrawLine (0, 142, 159, 142);
			WinDrawLine (0, 143, 159, 143);
    		break;

    	case AncestryForm:
    	case DescendancyForm:
    		// draw two horizontal lines
    		WinDrawLine (0, 15, 159, 15);
			WinDrawLine (0, 16, 159, 16);
    		break;
    		
   	case DatabasesForm:
			{
			RectangleType rect = {3, 17, 154, 83};
    		WinDrawRectangleFrame (roundFrame, &rect);
			WinDrawLine (3, 87, 156, 87);
    		break;
			}
			
    	default:
    		break;
    	}

	if (SupportsColor)
		WinPopDrawState();
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      	DrawObjectFrame
//
// DESCRIPTION:   	Draws a rectangular around an object in a form
//
// PARAMETERS:		-> objectId  - Id of object
//					-> adjXAmt	 -	number of pixels to adjust width of frame
//					-> adjXAmt	 -	number of pixels to adjust height of frame
//					-> grayFrame - true if drawing gray frame, else false to 
//											draw black frame.
// RETURNS:      	Nothing.
//
// REVISIONS:     	None.
////////////////////////////////////////////////////////////////////////////////////
static void DrawObjectFrame (const UInt16 objectId, const Int16 adjXAmt,
							 const Int16 setYAmt, const Boolean grayFrame)
{
   RectangleType 	rect;

	GetObjectBounds (objectId, &rect);
	rect.topLeft.x -= adjXAmt;
	rect.extent.x += (2*adjXAmt);
	
	if (setYAmt)
		rect.extent.y = setYAmt;
	
	if (grayFrame)
		WinDrawGrayRectangleFrame (rectangleFrame, &rect);
	else
		WinDrawRectangleFrame (rectangleFrame, &rect);
	}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DrawCharsColorI
//
// DESCRIPTION: 	This routine draws text to the screen in the color specified by
// 					the colorRGB parameter.  The current draw state is pushed
//					before any changes are made.  For black & white screens, no
//					color change is attempted and the text is simply drawn.
//
// PARAMETERS:		-> chars 		- characters to draw.
//					-> colorRGB 	- color to draw text.
//					-> x			- x coordinate position to begin drawing text.
//					-> y			- y coordinate position to begin drawing text.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
void DrawCharsColorI (const Char* chars, RGBColorType colorRGB, Coord x, Coord y)
{
	IndexedColorType 	colorIndex;

	if (!Pre35Rom) {
	   WinPushDrawState ();
		colorIndex = WinRGBToIndex (&colorRGB);
		WinSetTextColor (colorIndex);
		WinDrawChars (chars, StrLen (chars), x, y);
		WinPopDrawState ();
		return;
		}
	
	// devide does not support color, so draw in b&w
	WinDrawChars (chars, StrLen (chars), x, y);
}

////////////////////////////////////////////////////////////////////////////////////
//  FUNCTION: 		RefFinderStr
//
//  DESCRIPTION:	Finds the nth (keyNumber) occurrence of the key string within
//                	the field string, where the key represents the delimiter of an
//                	souC (S) or Fami (@).
//      		      
//  PARAMETERS:   	-> keyNumber - nth occurence of key within field.
//                	-> key       - one character string to look in field for.
//                	-> field     - string in which to look for key
//                	<- xRefStr   - string containing the nth key
//
//  RETURNS: 	   	if unsuccessful a false is returned.
//
//  REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean RefFinderStr (UInt16 keyNumber, Char key, Char *field, Char *xRefStr)
{
   UInt16    i = 0;
   UInt16    j = 0;
   UInt16    counter = 1;
   UInt16    fieldLength;

   *xRefStr = '\0';
   
	// If there isn't a key to search for flag error
   ErrFatalDisplayIf (key == NULL || key == '\0', "RefFinderStr: key missing");

   // If there isn't a ref to search for stop return false (error).
   if (field == NULL)
      return false;
      
   fieldLength = StrLen (field);
   
   while (i < fieldLength && counter <= keyNumber) {
      if (key == field[i]) {
         if (counter == keyNumber) {
            i++;
            while (field[i] != key && i < fieldLength) {
               xRefStr[j] = field[i];
               i++; j++;
               }
            xRefStr[j] = '\0';
            return true;
            }
         counter++;            
         }
      i++;
      }

   return false; 
}

////////////////////////////////////////////////////////////////////////////////////
//  FUNCTION: 		RefCounter
//
//  DESCRIPTION:	Count the number of keys in the field where each key
//                represents the reference for an event (E#), family (F#).
//      		      etc.
//
//  PARAMETERS:   key   - string to lookup record with ("E", "F", etc.);
//                field - string in which to look for items
//
//  RETURNS: 	   if unsuccessful a false is returned, else true.
//
//  REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
UInt16 RefCounter (Char key, Char *field)
{
   UInt16      i = 0;
   UInt16      counter = 0;

   // If there isn't a key to search for flag error
   ErrFatalDisplayIf (key == NULL || key == '\0', "RefCounter: key missing");
   
   if (field == NULL || *field == '\0')
      return 0;
      
   while (i <= StrLen (field)) {
      if (key == field[i])
         counter++;
      i++;
      }
      
   return counter; 
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RegisterCodeEntry
//
// DESCRIPTION:   Loads the Registration Code Entry Form.
//
// PARAMETERS:    None.
//                      
// RETURNS:      	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RegisterCodeEntry (void)
{
	FieldPtr   	fldP;
   	Char		*code;
   	Int16		num1, num2, num3, num4, num5;
	Int16		numTot;
	Char		hld_str[3] = "\0\0\0"; // must init!!
   	Boolean		valid = false;

   fldP = GetObjectPtr (RegisterCodeField);

	// check field length is correct
	if (FldGetTextLength (fldP) == CHK_TOT_CHARS) {

   	code = FldGetTextPtr (fldP); 

		// check each character in code
		if ((code[0] != CHK_CHR_0a  || code[1] != CHK_CHR_1a  ||
		 	  code[2] != CHK_CHR_2a  || code[3] != CHK_CHR_3a  ||
		 	  code[5] != CHK_CHR_5a))
		 	  if ((code[0] != CHK_CHR_0b || code[1] != CHK_CHR_1b  ||
			  		 code[2] != CHK_CHR_2b || code[3] != CHK_CHR_3b  ||
			  		 code[5] != CHK_CHR_5b))
					 goto Check;
					
		// check for valid numbers
		hld_str[0] = code[4];
		num1 = (Int16) StrAToI (hld_str);

		hld_str[0] = code[6];
		num2 = (Int16) StrAToI (hld_str);
		
		hld_str[0] = code[9];
		num3 = (Int16) StrAToI (hld_str);

		hld_str[0] = code[10];
		num4 = (Int16) StrAToI (hld_str);

		hld_str[0] = code[7];
		hld_str[1] = code[8];
		num5 = (Int16) StrAToI (hld_str);

		numTot = (num1 * num2) - (num1 + num2) + (num3 - num4);
		 
		if ((Int16) numTot != num5)
			goto Check; 
		
		valid = true; // we have a valid code
		}
	
	/////
	Check:
	/////
	
	if (valid) {
		Prefs[Registered] = true;
		AppSavePrefs ();
		FrmAlert (RegisteredAlert);
		}
	else
		FrmAlert (InvalidCodeAlert);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    RegisterHandleEvent
//
// DESCRIPTION:	This routine is the event handler for the Registration
//              Code Entry Form.
//
// PARAMETERS:	event  - a pointer to an EventType structure
//
// RETURNED:    true if the event has handle and should not be passed
//              to a higher level handler.
//
// REVISIONS:	None.
////////////////////////////////////////////////////////////////////////////////////
Boolean RegisterHandleEvent (EventPtr event)
{
   Boolean 	handled = false;
   	
   switch (event->eType)
   	{
  		case ctlSelectEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case RegisterOKButton:
					RegisterCodeEntry ();
			      FrmReturnToForm (0);
			      if (FrmGetActiveFormID () == SplashForm)
						FrmUpdateForm (SplashForm, updateViewReInit);
               handled = true;
               break;
              
           /* case RegisterKeyboardButton:
               SysKeyboardDialog (kbdDefault);
					handled = true;
               break;*/
              
            default:
               break;
				}
         break;

      case frmUpdateEvent:
      case frmOpenEvent:
		   FrmDrawForm (FrmGetActiveForm ());
		   SetNavFocusRing (RegisterCodeField); // DTR 2-13-2006
		   SetFocus (RegisterCodeField);
         handled = true;
         break;

      default:
      	break;
   	}
      
   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    ShowSplash
//
// DESCRIPTION: This routine displays the application's splash screen.
//
// PARAMETERS: 	->	doDelay - true if we want to delay splash screen.
//
// RETURNED: 	true if registered, otherwise false
//
// REVISIONS:	None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean ShowSplash (Boolean doDelay)
{
 	float  	splashDelay		= ShortSplashDelay; // init
   	FontID	currFont;
   	UInt32	startSecs 		= TimGetSeconds ();
  	Char    dayRemStr[25] 	= TRLDAYS_STR;

	#ifdef REGISTERED
	Prefs[Registered] = true;	
	#endif	
	
	
	if (Prefs[Registered]) { // Registered version
	
		if (!Prefs[HideSplash]) {
				
			ShowObject (SplashRegisteredLabel, true); // display REGISTERED label
	
			FrmDrawForm (FrmGetActiveForm()); // display Splash Screen
	
			SysTaskDelay ((Int32) SysTicksPerSecond() * splashDelay);
	
			}
			
		return true;
		
		}

	else { // !Registered version

		Int32			daysRemaining; // days left in trial period;
	   	Char     		holdStr[3];
   		UInt32   		ElapsedSecs;   
		Int32 			remSecs;
		Int32			remSecsLast;
		UInt32			timeSec;
		RectangleType 	rect;
		
		currFont = FntSetFont (boldFont);	
			
		// Calculate remaining days in trial period
	   if (InstallDate == 0)
      	InstallDate = startSecs;
  	 	
  	 	ElapsedSecs = startSecs - InstallDate;
   	daysRemaining = (TrialPeriod - ElapsedSecs / SecondsPerDay);
   	
   	if (daysRemaining < 0)
   		daysRemaining = 0;  

   	// display UNREGISTERED or BETA in field
		#ifdef BETA
	   
	   FrmDrawForm (FrmGetActiveForm ()); // display Splash Screen
	   WinDrawChars ("       BETA VERSION", 19, 28, 56); // display BETA
		
		#else // BETA
		
		ShowObject (SplashUnregisteredLabel, true); // display UNREGISTERED
		FrmDrawForm (FrmGetActiveForm ()); // display Splash Screen
		
		#endif // BETA


		// -- Display days remaining message --
		
   		StrIToA (holdStr, daysRemaining);
   		StrCat (dayRemStr, holdStr);
		WinDrawChars (dayRemStr, StrLen (dayRemStr), 19, 76);

   	// -- handle if user is past trial period --
   	if (daysRemaining <= 0) {
   		
   		#ifndef BETA
   		ShowObject (SplashRegisterButton, true); // display Register button
   		#endif // ifndef BETA
   		
   		SndPlaySystemSound (sndWarning);
     		return false;
     		}
   	
	 
		// -- Display seconds remaining message --
		
	  	if (!doDelay)
   		return false;
	
		FntSetFont (currFont);
	
		ShowObject (SplashDelayLabel, true);
		
		// get rectangle were seconds counter will go
		GetObjectBounds (SplashDelayLabel, &rect);
		rect.topLeft.x += rect.extent.x; 
		rect.extent.x = 15; // assume 15 pixels at most for needed space 

		splashDelay = LongSplashDelay;
		
		remSecsLast = splashDelay + 1;
		
		while (true) {

			timeSec = TimGetSeconds ();
		
			if ((timeSec - startSecs) > splashDelay)
				remSecs = 0;
			else
				remSecs =  splashDelay - (timeSec - startSecs);
     
     		// only update screen once per second
     		if ((remSecsLast - remSecs) >= 1) {
     
    		  	StrPrintF (holdStr, "%i", (Int16) remSecs);
  				WinEraseRectangle (&rect, 0);	
      	
  				WinDrawChars (holdStr, StrLen (holdStr), rect.topLeft.x, rect.topLeft.y);
  					
  				remSecsLast = remSecs;
  				}	

  			if (remSecs <= 0)
  				break;
  		   }
	   	
	   	#ifdef BETA
			return true;
			#endif
	   	
	   	WinEraseRectangle (&rect, 0);	
	   	ShowObject (SplashDelayLabel, false);
	   	ShowObject (SplashRegisterButton, true);
		ShowObject (SplashContinueButton, true);
			
	  } // else

	return false;
}
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: 	SplashGotoForm
//
// DESCRIPTION: This routine calls the correct form after the splash screen is
//						displayed.
//
// PARAMETERS:  None.
//
// RETURNED:    Nothing.
//
// REVISIONS:	None.
////////////////////////////////////////////////////////////////////////////////////
void SplashGotoForm (void)
{
	// Get total number of databases on HH.
   GetDatabaseList (false);  // init TotDatabases variable
         		
 	if (*DbName && TotDatabases > 0) {
		// At this point we either have one database with the name
		// of that database in DbName or we have multiple databases
		// with the name of the last database viewed in DbName.

	if (OpenDatabase (DbName, dmModeReadOnly, FileLoc, Prefs[LastFileOnCrd]))
		FrmGotoForm (DatabasesForm);
        				
   else if (CurrentIndiRecN != NO_REC && Prefs[ReturnLast]) {
        				
   	if (CurrentIndiRecN >= IndiDBNumRecs) {
      	CurrentIndiRecN = NO_REC;
        	FrmGotoForm (IndiListForm);
        	}
        						
		else {
      	
      	ErrFatalDisplayIf (CurrentIndiRecN >= IndiDBNumRecs &&
				CurrentIndiRecN != NO_REC,	"SplashHandleEvent: CurrentIndiRelN too high");
        		FrmGotoForm (IndiSummForm);
				}        				
    	}
      
      else {
      	ErrFatalDisplayIf (CurrentIndiRecN >= IndiDBNumRecs &&
			CurrentIndiRecN != NO_REC,	"SplashHandleEvent: CurrentIndiRelN too high");
			FrmGotoForm (IndiListForm);		
			}
		}

	else // no database selected in prior user session
	
		FrmGotoForm (DatabasesForm); 
}


////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	SplashHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Splash Screen.
//
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean SplashHandleEvent (EventPtr event)
{
   Boolean	handled 	= false;

	switch (event->eType)
   	{
  		case ctlSelectEvent:
      	
      	switch (event->data.ctlSelect.controlID)
         	{
         	case SplashRegisterButton:
      			FrmPopupForm (RegisterForm);
            	handled = true;
            	break;
            	
            case SplashContinueButton:
	            SplashGotoForm ();
            	handled = true;
            	break;	
            
            case SplashOKButton:
    		     	FrmReturnToForm (0);
            	handled = true;
            	break;
            
         	default:
            	break;
         	}
     		break;
     
     	case frmUpdateEvent:
     		
     		if (!AboutMode) {
	     		// following will be called after registration code entered
				if (!Prefs[Registered])
					ShowSplash (false);
				else
					FrmGotoForm (SplashForm);
				}
				
	      handled = true;
         break;
     
   	case frmOpenEvent:
         
         if (!AboutMode) { // then show Splash Screen
         	
         	Boolean regUser =  ShowSplash (true);

				while (EvtSysEventAvail (false)) // removed any pen taps from queue
					EvtGetEvent (event, 0);
            
         	if (regUser)
	         	SplashGotoForm ();
	      	}

     		else { // AboutMode
    		
     	 		ShowObject (SplashVersionLabel, 		false);
         	ShowObject (SplashVerFullLabel, 		true);
         	ShowObject (SplashEmailLabel, 		true);
         	ShowObject (SplashDeveloperLabel,	true);
         	ShowObject (SplashOKButton, 			true);
         
         	if (Prefs[Registered])
					ShowObject (SplashRegisteredLabel, true);
				else
         		ShowObject (SplashUnregisteredLabel, true);
         
   	      FrmDrawForm (FrmGetActiveForm ());
     			}
     	
         handled = true;
         break;

      default:
      	break;
   	}

   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      JumpListDrawRecord
//
// DESCRIPTION:   This routine draws an Alias List entry into the 
//                the AliasListTable.  It is called as a callback
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
 static void JumpListDrawRecord (void *table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   UInt16 	  		recN; // holds and IndiDB record
   DBRecordType  	rec;
   MemHandle      recH = NULL;
   FontID 	      curFont;

   recN = TblGetRowData (table, row);
   
	if (recN == NO_REC)
		return;

   if (DbGetRecord (IndiDB, recN, &rec, &recH) != 0) {
	   ErrNonFatalDisplay ("JumpListDrawRecord: Record not found");
   	return;
   	}
 
   curFont = FntSetFont (PrefFnts[IndiSummFont]);
   DrawNameLifespanColor (&rec, bounds, true, true);
   DbMemHandleUnlock (&recH);
	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	JumpListHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Jump List Window.
//
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:	 	None.
////////////////////////////////////////////////////////////////////////////////////
Boolean JumpListHandleEvent (EventPtr event)
{
   Boolean  handled = false;

   switch (event->eType)
		{
		case tblSelectEvent:
         CurrentIndiRecN = TblGetRowData (event->data.tblSelect.pTable,
		      event->data.tblSelect.row);
		   FrmGotoForm (IndiSummForm);
         handled = true;
         break;
		
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case JumpCancelButton:
               FrmGotoForm (PriorFormID);
               handled = true;
               break;

            default:
               break;
         	}
         break;

    	case frmUpdateEvent:
	   case frmOpenEvent:
         {
         UInt16 	row, rowsInTable;
  			TablePtr	table = GetObjectPtr (JumpJumpTable);
  
   		rowsInTable = TblGetNumberOfRows (table);
   
		   for (row = 0; row < rowsInTable; row++) {      
      		TblSetItemStyle (table, row, 0, customTableItem);
		      TblSetRowData (table, row, Jump[row]);
		      if (Jump[row] != NO_REC)
	   	   	TblSetRowUsable (table, row, true);
	   		else {
      			TblSetRowSelectable (table, row, false);
      			TblSetRowUsable (table, row, false);
      			}
      		TblMarkRowInvalid (table, row);
      		}

		   TblSetColumnUsable (table, 0, true);
		   TblSetCustomDrawProcedure (table, 0, JumpListDrawRecord);
         }
         FrmDrawForm (FrmGetActiveForm());
         handled = true;
         break;

	   default:
		   break;
   	}

   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      PreferencesSave
//
// DESCRIPTION:	Write the new user preferences.
//
//					 	NOTE: the order of the PrefFlds MUST MATCH the order of the
//						CheckBoxes on the Prefrences 1 Screen.  Otherwise, this function
//						will not work properly. Also, the CheckBoxes must be numbered
//						sequentially.
//
//					 	NOTE: the order of the PrefFnts MUST MATCH the order of the
//						Buttons on the Prefrences 2 Screen.  Otherwise, this function
//						will not work properly. Also, these Buttons must be numbered
//						sequentially.
//
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void PreferencesSave (void)
{
  	UInt16 x;
  		
  	if (FrmGetActiveFormID () == Pref1Form) {
		
		for (x = 0; x <= LAST_PREF1_CHKBX; x++)
			Prefs[x]	= GetControlValue (FIRST_PREF1_RSC + x);
  		}

   else { // FrmGetActiveFormID () == Pref2Form
	   
	   PrefFnts[IndiSummNameFont] = PrefFnts[IndiListFont] 
	   	= PrefFnts[IndiSummFont] = PrefFnts[DetailViewFont] = boldFont;

	   if (GetControlValue (Pref2IndiNamestdFontPushButton))
  	   	PrefFnts[IndiSummNameFont] = stdFont;
		else if (GetControlValue (Pref2IndiNamelargeBoldFontPushButton))
      	PrefFnts[IndiSummNameFont] = largeBoldFont;

		for (x = 0; x <= LAST_PREF2_FNT; x++) {
			if (GetControlValue (FIRST_PREF2_FNT + x))
				PrefFnts[x]	= stdFont;
				}
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   	PreferencesInit
//
// DESCRIPTION:	Initialize the Preferences Form.
//
//					 	NOTE: the order of the PrefFlds MUST MATCH the order of the
//						checkboxes on the Prefrences 1 Screen.  Otherwise, this function
//						will not work properly. Also, the CheckBoxes must be numbered
//						sequentially.
//
// PARAMETERS:		None.
//
// RETURNED:    	Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void PreferencesInit (void)
{
 	if (FrmGetActiveFormID () == Pref1Form) {
		
		UInt16 x;

		for (x = 0; x <= LAST_PREF1_CHKBX; x++)
			SetControlValue (FIRST_PREF1_RSC + x, Prefs[x]);
   	}
   else {
   	UInt16	controlID;
   
     	if (PrefFnts[IndiSummNameFont] == stdFont )
      	controlID = Pref2IndiNamestdFontPushButton;
   	else if (PrefFnts[IndiSummNameFont] == boldFont )
      	controlID = Pref2IndiNameboldFontPushButton;
   	else
      	controlID = Pref2IndiNamelargeBoldFontPushButton;
		SetControlValue (controlID, true);

   	if (PrefFnts[IndiListFont] == stdFont)
   		controlID = Pref2IndiListstdFontPushButton;
   	else
   		controlID = Pref2IndiListboldFontPushButton;
   	SetControlValue (controlID, true);

  		if (PrefFnts[IndiSummFont] == stdFont)
      	controlID = Pref2IndiSummstdFontPushButton;
   	else
      	controlID = Pref2IndiSummboldFontPushButton;
   	SetControlValue (controlID, true);

   	if (PrefFnts[DetailViewFont] == stdFont)
      	controlID = Pref2DetailViewstdFontPushButton;
   	else
      	controlID = Pref2DetailViewboldFontPushButton;
   	SetControlValue (controlID, true);
		}
	
  	FrmDrawForm (FrmGetActiveForm ());

	if (FrmGetActiveFormID () == Pref2Form && !Pre35Rom) {

   	RectangleType 	rect;

		FntSetFont (stdFont);
		GetObjectBounds (Pref2Pick1Button, &rect);
		DrawCharsColorI (PERSONS_W_CHIL, Color1, 4, rect.topLeft.y);

		GetObjectBounds (Pref2Pick2Button, &rect);
		DrawCharsColorI (MARRIED_NO_CHIL, Color2, 4, rect.topLeft.y);
		ShowObject (Pref2ColorsLabel, true);
		ShowObject (Pref2Pick1Button, true);
		ShowObject (Pref2Pick2Button, true);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	PreferencesHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Preferences
//              	Form.
//
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean PreferencesHandleEvent (EventPtr event)
{
   Boolean 	handled = false;

   switch (event->eType)
		{
     	case ctlSelectEvent:
        	switch (event->data.ctlSelect.controlID)
        		{
	         case Pref1CancelButton:
  	       	case Pref2CancelButton:
         		AppLoadPrefs (true); // get original preferences
            	FrmReturnToForm (0);
					if (Pre35Rom || !Pre60Rom)
						IndiSummUpdForms (true);
            	handled = true;
            	break;
         
         	case Pref1SaveButton:
         	case Pref2SaveButton:
            	PreferencesSave ();
            	AppSavePrefs ();
	      		FrmReturnToForm (0);
  	         	IndiSummUpdForms (true);  	         	  	         	
      	    	handled = true;
            	break;
         
        		case Pref2Pick1Button:
        			if (!Pre35Rom) {
        				IndexedColorType	PickIndex  = WinRGBToIndex (&Color1);
						UIPickColor (&PickIndex, NULL, UIPickColorStartPalette,
							NULL, NULL);
						WinIndexToRGB (PickIndex, &Color1);
						FrmUpdateForm (FrmGetActiveFormID (), updateViewReInit);
						}
	      		handled = true;
            	break;
        
        		case Pref2Pick2Button:
        			if (!Pre35Rom)	{
        				IndexedColorType	PickIndex = WinRGBToIndex (&Color2);
						UIPickColor (&PickIndex, NULL, UIPickColorStartPalette,
							NULL, NULL);
						WinIndexToRGB (PickIndex, &Color2);
						FrmUpdateForm (FrmGetActiveFormID (), updateViewReInit);
						}
 	      		handled = true;
            	break;
    
        		default:
            	break;
         	}
         break;
         
  		case frmOpenEvent:
  			AppSavePrefs ();  // must call this to save dbname and current rec no.
   		//ResizeForm ();
   		PreferencesInit ();
      	handled = true;
      	break;
      
   	case frmUpdateEvent:
			PreferencesInit ();
        	handled = true;
        	break;

  	 	/*case winDisplayChangedEvent:    
			if (ResizeForm ())
				PreferencesInit ();
      	handled = true;
       	break; */

	   default:
		   break;
      }

   return (handled);
}

#pragma mark -
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AncestryDrawRecord
//
// DESCRIPTION:   This routine draws an Ancestry Chart entry into the 
//                the Ancestry Chart Table.  It is called as a callback
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
static void AncestryDrawRecord (void *table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
	UInt16 	      recN; // record in IndiDB
	DBRecordType  	rec;
	MemHandle      recH = NULL;
	FontID 	      curFont;
   	Int16 			x, y;
	Char				dash[] = "   ";  // must be 3 spaces

	#ifdef GREMLINS_MEM_CHK
  	MemHeapScramble (MemHeapID (0,1));
 	#endif

	x = bounds->topLeft.x;
	y = bounds->topLeft.y;
	recN = TblGetRowData (table, row);

	if (recN == NO_REC)
	  	TblSetRowSelectable (table, row, false);
	else  // get individual record to draw
   	DbGetRecord (IndiDB, recN, &rec, &recH);
 		
	curFont = FntSetFont (CHART_FONT);
	switch (row)
   		{
		case 0:
		case 2:
		case 6:
		case 8:
			dash[0] = 0x09;
			dash[1] = 0x97;
				WinDrawChars (dash, 3, x, y);
			bounds->topLeft.x+= FntCharsWidth (dash, 3);
			break;

		case 3:
		case 5:
			// ignore the two blank lines.
				return;

		case 1:
		case 7:
			dash[0] = 0x96;
			dash[2] = '\0';
				WinDrawChars (dash, 2, x, y);
			bounds->topLeft.x+= FntCharsWidth (dash, 2);
			break;

			case 4:
				dash[0] = 0x2D;
			dash[2] = '\0';
				WinDrawChars (dash, 2, x, y);
			bounds->topLeft.x+= FntCharsWidth (dash, 2);
			break;

		default:
   			break;
		}   	

	if (recN != NO_REC) { 
   	DrawRecordNameAndLifespan (&rec, bounds, true, true);
   	DbMemHandleUnlock (&recH);
   	}
   else
   	WinDrawChars (cUnknownStr, 7, bounds->topLeft.x, y);

	WinDrawLine (2, 30, 2, 54);
	WinDrawLine (2, 102, 2, 126);
	WinDrawLine (0, 42, 0, 114);
   	
	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AncestrySetRow
//
// DESCRIPTION:   This routine loads individuals into the rows in the
//                Ancestry Chart table.  
//
// PARAMETERS:    table - the Ancestry table;
//						row1  - the row for the father's record number
//						row2  - the row for the mother's record number
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
 ////////////////////////////////////////////////////////////////////////////////////
static void AncestrySetRow (TablePtr table, Int16 row1, Int16 row2)
{
	if (CurrentIndiRecN == NO_REC) 
		return;

 	#ifdef GREMLINS_MEM_CHK
  	MemHeapScramble (MemHeapID (0,1));
 	#endif

	ErrFatalDisplayIf (IndiRecH, "AncestrySetRow: IndiRecH should be unlocked.");
		
	// Get IndiRec for person whose parents we are looking for.
	DbGetRecord (IndiDB, CurrentIndiRecN, &IndiRec, &IndiRecH);
	
	if (ParentFinder (false)) { // returns false if no parents found 
		// If true, then we must have a father or mother or both.
	   TblSetRowData (table, row1, CurrentIndiRecN);
	   TblSetRowData (table, row2, CurrentSpouRecN);
		}
	else {
		CurrentIndiRecN = CurrentSpouRecN = NO_REC;
		}

	DbMemHandleUnlock (&IndiRecH);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      AncestryLoadTable
//
// DESCRIPTION:   This routine loads Ancestry View database records into
//                the table view form.
// 					IndiRecH is unlocked at this point via FrmCloseEvent 
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void AncestryLoadTable (void)
{
	Int16		row;
	UInt16		holdMotherRecord;
	UInt16      origIndiRecN = CurrentIndiRecN;
	TablePtr 	table;
	
	table = GetObjectPtr (AncestryAncestryTable);

	ShowObject (AncestryBackButton, (Boolean) (HistPos > 0)); // hide/show Back Button

	// initialize rows
	for (row = 0; row < 9; row++) {
		TblSetItemStyle (table, row, 0, customTableItem);
		TblSetRowUsable (table, row, true);
   		TblMarkRowInvalid (table, row);
   		TblSetRowData (table, row, NO_REC);  // initialize
		TblSetRowHeight (table, row, ANCST_FONT_SZ);
 		TblSetRowSelectable (table, row, true);
		}

	// initialize columns
	TblSetColumnUsable (table, 0, true);
	TblSetCustomDrawProcedure (table, 0, AncestryDrawRecord);

	// root individual
	TblSetRowData (table, 4, CurrentIndiRecN);
  	TblSetRowSelectable (table, 4, false); // can't select root	person
	
	// father and mother
	AncestrySetRow (table, 1, 7);
	
	holdMotherRecord = CurrentSpouRecN;
	
	// father's parents
	AncestrySetRow (table, 0, 2);

	CurrentIndiRecN = holdMotherRecord;
	
	// mother's parents
	AncestrySetRow (table, 6, 8);			

  	// set blank rows before and after root individual
  	TblSetRowSelectable (table, 3, false);
  	TblSetRowSelectable (table, 5, false);

   CurrentIndiRecN = origIndiRecN;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	AncestryHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Ancestry Chart.
//					 	IndiRec will be loaded and locked prior to calling this
//					 	routine.  It will be loaded and locked upon exit as well.
//	
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean AncestryHandleEvent (EventPtr event)
{
   Boolean  	handled = false;
   MemHandle	QueueHistH;

   switch (event->eType)
		{
		case tblSelectEvent:
         CurrentIndiRecN = TblGetRowData (event->data.tblSelect.pTable,
		      event->data.tblSelect.row);
		   if (HistPos < QHIST_MAX)
		   	HistPos++;
		   QueueHist[HistPos].person = CurrentIndiRecN;
			TblUnhighlightSelection(event->data.tblSelect.pTable); // backward compatibility
			FrmUpdateForm (AncestryForm, updateRedrawAll);
         handled = true;
         break;
		
      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{
            case AncestryDoneButton: // go back to person we started with
            	CurrentIndiRecN = QueueHist[0].person;
               FrmGotoForm (IndiSummForm);
               handled = true;
               break;

            case AncestryGoToButton: // go to current root person
            	CurrentIndiRecN = QueueHist[HistPos].person;
               FrmGotoForm (IndiSummForm);
               handled = true;
               break;

				case AncestryBackButton:
					HistPos--;
					CurrentIndiRecN = QueueHist[HistPos].person;
					FrmUpdateForm (AncestryForm, updateRedrawAll);
               handled = true;
               break;

				case AncestryInfoButton:
               FrmHelp (AncestryHelpString);
               handled = true;
               break;

            default:
               break;
         	}
         break;

	   case frmOpenEvent:
         ErrFatalDisplayIf (IndiRecH, "AncestryHandleEvent: IndiRecH should be unlocked");

         QueueHistH = MemHandleNew (sizeof (QueueHistType) * QHIST_MAX);
   		ErrFatalDisplayIf (!QueueHistH, "AncestryHandleEvent: Out of memory");
   		
   		if (!QueueHistH) {
            FrmGotoForm (IndiSummForm);
            }
         else {
   			QueueHist = MemHandleLock (QueueHistH);  
         	HistPos = 0;
        		QueueHist[HistPos].person = CurrentIndiRecN;
     	   	AncestryLoadTable ();
     	   	FrmDrawForm (FrmGetActiveForm ());
  				DrawScreenLines (AncestryForm);
  				WinDrawLine (0, 42, 0, 114);
         	handled = true;
         	break;
				}
			
    	case frmUpdateEvent:
	   	CurrentIndiRecN = QueueHist[HistPos].person;
 	 		AncestryLoadTable ();
      	DrawForm ();
			DrawScreenLines (AncestryForm);
			WinDrawLine (0, 42, 0, 114);
			handled = true;
			break;

		case frmCloseEvent:
			if (QueueHist) {
     			MemHandleFree (MemPtrRecoverHandle (QueueHist));
 				QueueHist = NULL;
 				}
   	   ErrFatalDisplayIf (IndiRecH, "AncestryHandleEvent: IndiRecH should be unlocked");
	   	DbMemHandleUnlock (&FamiRecH);
			break;
      
	   default:
		   break;
   	}

   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendancyDrawRecord
//
// DESCRIPTION:   This routine draws a Descendancy person entry into the 
//                the Descendancy Chart Table. It is called as a callback
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
 static void DescendancyDrawRecord (void *table, Int16 row, Int16 column, 
   RectanglePtr bounds)
{
   UInt16 	      recN; // record in IndiDB
   DBRecordType	rec;
   MemHandle      recH = NULL;
   FontID 	      curFont;
   Int16 			x, y;
	Char				dash[] = {0x96, '\0'}; // must be 1 space
	UInt16			rowId;
 	
 	#ifdef GREMLINS_MEM_CHK
  	MemHeapScramble (MemHeapID (0,1));
 	#endif
 	
 	x = bounds->topLeft.x;
   y = bounds->topLeft.y;
   recN = TblGetRowData (table, row);

	// Spouse information may be missing and recordNum will be NO_REC
	if (recN != NO_REC)
		DbGetRecord (IndiDB, recN, &rec, &recH);
 	
 	rowId = TblGetRowID (table, row);
 		
   curFont = FntSetFont (CHART_FONT);
   switch (rowId)
   	{
   	case 0:  // root person
   		break;

   	case 1: // level 1 child
   	case 2: // level 1 child at end of list
     		WinDrawChars (dash, 1, x+1, y);
     		WinDrawLine (0, y, 0, (rowId == 2) ? y+5 : y+11);
   		bounds->topLeft.x+= FntCharsWidth (dash, 1) + 2;
   		break;

   	case 3: // level 2 child
   	case 4: // level 2 children at end of list
     		WinDrawChars (dash, 1, x+4, y);
     		if (rowId == 3)
     			WinDrawLine (0, y, 0, y+11);
     		WinDrawLine (3, y-5, 3, y+5);
   		bounds->topLeft.x+= FntCharsWidth (dash, 1) + 7;
   		
   		// draw fancy arrow next to child that has children
			if (rec.fields[indiChiFlg]) {
				dash[0] = 0x9B;
				WinDrawChars (dash, 1, bounds->extent.x-3, y-1);
				WinDrawLine (bounds->extent.x-3, y+5, bounds->extent.x-3, y+5);
				}
   		break;
 
 		case 5:  // level 0 spouse
   		dash[0] = '+';
     		WinDrawChars (dash, 1, x, y);
   		bounds->topLeft.x+= FntCharsWidth (dash, 1);
   		break;
   		
		case 6: // level 1 spouse (level 1&2 vertical lines)
		case 7: // level 1 spouse (level 1 vertical line)
		case 8: // level 1 spouse (level 1 vertical line + 1 term horizontal)
		case 9: // level 1 spouse (level 1&2 vertical lines + 1 term horizontal)
   		dash[0] = '+';
     		WinDrawChars (dash, 1, x+5, y);
 			WinDrawLine (3, y-5, 3, y+11);
     		if (rowId == 6) {
     			WinDrawLine (0, y, 0, y+11);
     			}
     		else if (rowId == 8) {
				WinDrawLine (3, y+10, 7, y+10);
     			}
     		else if (rowId == 9) {
     		   WinDrawLine (0, y, 0, y+11);
     			WinDrawLine (3, y+10, 7, y+10);
     			}		
   		bounds->topLeft.x+= FntCharsWidth (dash, 1) + 5;
   		break;
   		   	   	
   	default:
   		break;
		}
	
	bounds->extent.x -=4;
		
	if (recN != NO_REC) {
   	DrawRecordNameAndLifespan (&rec, bounds, true, true);
   	DbMemHandleUnlock (&recH);
   	}
   else {
   	WinDrawChars (cUnknownStr, 7, bounds->topLeft.x, y);
   	TblSetRowSelectable(table, row, false);
   	}

	FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendancyScroll
//
// DESCRIPTION:   This routine scrolls the Descendancy Chart in the direction
//						specified.
//
// PARAMETERS:    direction	- up or dowm
//
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void DescendancyScroll (WinDirectionType direction)
{
	TablePtr	table;
	UInt16	lastVisRow;
	UInt16	scrollRows;
	UInt16 	newTopDescendancyPos;

   table = (TablePtr) GetObjectPtr (DescendancyDescendancyTable);
   
   lastVisRow = TblGetLastUsableRow (table);

   if (Prefs[ScrollOneD])
   	scrollRows = 1;
   else
		scrollRows = lastVisRow;

	newTopDescendancyPos = TopDescPos;
	
	// Scroll the table down.
	if (direction == winDown) {
	   newTopDescendancyPos += scrollRows;
	   if (newTopDescendancyPos + lastVisRow > DescPos) {
	   	if (DescPos > lastVisRow)
	   		newTopDescendancyPos = DescPos - lastVisRow;
	   	else
	   		newTopDescendancyPos = 0;
	    	}
      }
   else { // Scroll the table up
      if (TopDescPos > scrollRows)
  	      newTopDescendancyPos -= scrollRows;
      else
      	newTopDescendancyPos = 0;
      }
	
	// Avoid redraw if no change
	if (TopDescPos != newTopDescendancyPos) {
		TopDescPos = newTopDescendancyPos;
		DescendancyLoadTable ();
		TblRedrawTable (table);
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendencyLoadTable
//
// DESCRIPTION:   This routine loads record numbers (from IndiDB) into the
//						Descendancy Chart table.  It also loads the level indicator
//                into the table.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DescendancyLoadTable (void)
{
   Int16				row = 0;
   Int16   			numRows;
   UInt16			recordCnt;
   TablePtr 		table;
   RectangleType	tblBounds;
   Boolean     	scrollableU;
   Boolean 	   	scrollableD;

   table = (TablePtr) GetObjectPtr (DescendancyDescendancyTable);

	// Get number of rows in the table.
	TblGetBounds (table, &tblBounds); 
	//numRows = tblBounds.extent.y / TblGetRowHeight (table, 0); 

	numRows = min ((tblBounds.extent.y / TblGetRowHeight (table, 0)),
		TblGetNumberOfRows (table)); 

  	// Check that we have not scrolled to far (when screen sz change)
	if (TopDescPos + (numRows-1) > DescPos)
		TopDescPos = DescPos > (numRows-1) ? DescPos - (numRows-1) : 0;

	 recordCnt = TopDescPos;

	// Load data into table rows
   while (row < numRows) {
      TblSetRowUsable (table, row, true);
      TblMarkRowInvalid (table, row);
      TblSetRowData (table, row, DescList[recordCnt].person);
		TblSetRowID (table, row, DescList[recordCnt].level);
   	TblSetRowSelectable (table, row, true);
	   row++;
	   recordCnt++;
     
      if (recordCnt > DescPos)
         break; 

      }  // end while loop

   // Hide the item that don't have any data.
   while (row < numRows) {
      TblSetRowUsable (table, row, false);
      row++;
      }

   // Update the scroll buttons.
   scrollableU = (Boolean) (TopDescPos > 0);
	scrollableD = (Boolean) (TopDescPos + TblGetLastUsableRow (table) < DescPos);
   
   UpdateScrollers (DescendancyScrollUpRepeating, DescendancyScrollDownRepeating,
   	scrollableU, scrollableD);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendancyInit
//
// DESCRIPTION:   Initializes the Descendancy Chart
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DescendancyInit (void)
{
   UInt16 	   row;
   TablePtr    table;
   FontID		curFont = FntSetFont (stdFont);

   table = (TablePtr) GetObjectPtr (DescendancyDescendancyTable);
	
	ShowObject (DescendancyBackButton, (Boolean) (HistPos > 0)); // hide/show back button
	
	DescList[0].person = CurrentIndiRecN;
 
   for (row = 0; row < TblGetNumberOfRows (table); row++) {
      TblSetItemStyle (table, row, 0, customTableItem);
      TblSetRowHeight (table, row, FntLineHeight ());
      TblSetRowUsable (table, row, false);
      }
      
   TblSetColumnUsable (table, 0, true);
   TblSetCustomDrawProcedure (table, 0, DescendancyDrawRecord);
	FntSetFont (curFont);

   DescendancyLoadTable ();
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendancyChildsChildren
//
// DESCRIPTION:   Finds the children of the individual whose record
//						number is passed in the indiRecordNum paremeter.
//						Looks for children in all families of the individual
//						and stores IndiDB record number in DescenantsList
//						array.
//
// PARAMETERS:    indiRecNum -	record number of individual for which to
//												check for children.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DescendancyChildsChildren (UInt16 indiRecN)
{
	UInt16			famiCnt = 1;
   Char        	famiXRef[NREF_LEN+1];
   DBRecordType	indiRec;
   MemHandle		indiRecH = NULL;
   UInt32			chilRecN; // record in ChilDB
   DBRecordType  	chilRec;   
   MemHandle   	chilRecH = NULL;
   UInt16			lastFamiNum;
   UInt32			lastChilRecN;
  
  	if (DbGetRecord (IndiDB, indiRecN, &indiRec, &indiRecH)) {
   	ErrNonFatalDisplay ("DescendanyChildsChildren: Record not found");
  		return;
  		}

	if (!indiRec.fields[indiFamSNo])
		goto ExitFunc;

   lastFamiNum = RefCounter (FAMI_DLM, indiRec.fields[indiFamSNo]);

   if (lastFamiNum == 0)
    	goto ExitFunc;
		
	while (famiCnt <= lastFamiNum) {
	
     	// Find first family of individual in the FamiDB
     	if (!RefFinderStr (famiCnt, FAMI_DLM, indiRec.fields[indiFamSNo], famiXRef))
        	goto ExitFunc;
        	
		// We could not have gotten this far if there was not a Family Record
		// loaded already.  So we need to unlock the handle to it.
		DbMemHandleUnlock (&FamiRecH);
      DbGetRecord (FamiDB, (UInt32) StrAToI (famiXRef), &FamiRec,	&FamiRecH);
    
      // Look for the spouse associated with the FamiRec
     	CurrentSpouRecN = NO_REC; // initialize
     	
     	if (FamiRec.fields[famiHusbNo]) {
     	
	     	UInt16 husbRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);

     		if (indiRecN == husbRecN) {
     			if (FamiRec.fields[famiWifeNo])
     				CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiWifeNo]);
         	}
      	else
      		CurrentSpouRecN = husbRecN;
      	}

		// Enter spouse record number into Descendant List array
		DescPos++;
		DescList[DescPos].person = CurrentSpouRecN;
		DescList[DescPos].level  = 6; 			
     	
     	chilRecN = lastChilRecN = NO_REC_LONG;
   
	   // Make sure we have children
		if (FamiRec.fields[famiChiRec]) {
			chilRecN = (UInt32) StrAToI (FamiRec.fields[famiChiRec]);
			lastChilRecN = chilRecN + (UInt32) StrAToI (FamiRec.fields[famiChiCnt]) - 1;
			}
     	
     	if (chilRecN == NO_REC_LONG && famiCnt == lastFamiNum)
     		DescList[DescPos].level = 9; 			
     	
     	if (chilRecN != NO_REC_LONG) {
     		// must have at least one child to continue	
   	
     		while (chilRecN <= lastChilRecN) {
     			DbGetRecord (ChilDB, chilRecN, &chilRec, &chilRecH);
				DescPos++;
				DescList[DescPos].person = (UInt16) StrAToI (chilRec.fields[chilIndiNo]);
				DescList[DescPos].level = 3; 
        		chilRecN++;
       		DbMemHandleUnlock (&chilRecH);
				}
      	}
     	
     	famiCnt++;
		}

	////////
	ExitFunc:
	////////
	
	DbMemHandleUnlock (&indiRecH);
	return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendancyLoadData
//
// DESCRIPTION:   Initializes the Descendancy View.
//						IndiRec must be loaded prior to calling this function.
//
// PARAMETERS:    None.
//
// RETURNED:      true if arrays successfully allocated on heap, else false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean DescendancyLoadData (void)
{
	UInt16			famiCnt = 1;
   Char        	famiXRef[XREF_LEN+1];
   UInt32			chilRecN; // record in ChilDB
   DBRecordType  	chilRec;
   MemHandle   	chilRecH = NULL;
   UInt32			lastChilRecN;
   UInt16			lastFamiNum;
   UInt16			indiRecN; // record in IndiDB
   MemHandle		DescenantListH;
   MemHandle   	QueueHistH;
 
	DbMemHandleUnlock (&IndiRecH);
	DbGetRecord (IndiDB, CurrentIndiRecN,	&IndiRec, &IndiRecH);

	// Allocate the Decendancy List and Queue History arrays (on dynamic heap).
	// This routine can be called on a form open event or form update event. If
	// it is form update we do not want to establish the arrays again.
	if (!DescList) {
		DescenantListH = MemHandleNew (sizeof (DescListType) * DL_MAX);
  		ErrFatalDisplayIf (!DescenantListH, "Out of memory");
  		if (!DescenantListH)
  			return false;
  		DescList = MemHandleLock (DescenantListH);  
		}
		
	if (!QueueHist) {
		QueueHistH = MemHandleNew (sizeof (QueueHistType) * QHIST_MAX);
   	ErrFatalDisplayIf (!QueueHistH, "Out of memory");
 		if (!QueueHistH)
	   	return false;
   	QueueHist = MemHandleLock (QueueHistH);  
		}
		
   TopDescPos = 0;
	DescPos = 0;
	
	DescList[DescPos].person = CurrentIndiRecN;
	DescList[DescPos].level = 0;

   // get number of families for root individual.
   lastFamiNum = RefCounter (FAMI_DLM, IndiRec.fields[indiFamSNo]);

   if (lastFamiNum == 0)
     	return true;
		      
	while (famiCnt <= lastFamiNum) {
     	// find family of individual in the FamiDB, if any
     	if (!RefFinderStr (famiCnt, FAMI_DLM, IndiRec.fields[indiFamSNo],	famiXRef))
        	goto ExitFunc;

		DbMemHandleUnlock (&FamiRecH);
		DbGetRecord (FamiDB, (UInt32) StrAToI (famiXRef), &FamiRec, &FamiRecH);   
      	
      // Look for the spouse associated with the FamiRec
		CurrentSpouRecN = NO_REC; // initialize
      
      if (FamiRec.fields[famiHusbNo]) {
      
	      UInt16 husbRecN = (UInt16) StrAToI (FamiRec.fields[famiHusbNo]);
      
      	if (CurrentIndiRecN == husbRecN) {
      		if (FamiRec.fields[famiWifeNo])
      			CurrentSpouRecN = (UInt16) StrAToI (FamiRec.fields[famiWifeNo]);
         	}
      	else
      		CurrentSpouRecN = husbRecN;
			}

		// Enter spouse record number into Descendant List array
		DescPos++;
		DescList[DescPos].person = CurrentSpouRecN;
		DescList[DescPos].level = 5; 			

		// get first and last child record numbers for family
     	chilRecN = lastChilRecN = NO_REC_LONG;
		
		if (FamiRec.fields[famiChiRec]) {
			chilRecN = (UInt32) StrAToI (FamiRec.fields[famiChiRec]);
			lastChilRecN = chilRecN + StrAToI (FamiRec.fields[famiChiCnt]) - 1;
			}

     	// If at least one child found then get each child's children
     	if (chilRecN != NO_REC_LONG) {

     		while (chilRecN <= lastChilRecN) {
        		DbGetRecord (ChilDB, chilRecN, &chilRec, &chilRecH);
        		indiRecN = (UInt16) StrAToI (chilRec.fields[chilIndiNo]);
        		DbMemHandleUnlock (&chilRecH);	
				DescPos++;
				DescList[DescPos].person = indiRecN;
				DescList[DescPos].level = 1; 			
				DescendancyChildsChildren (indiRecN);
        		chilRecN++;
				}
     		}
     	famiCnt++;
		}

	// Flag the last rows for special line drawings in DecendancyDrawRecord  //
	if (DescList[DescPos].level == 6)
		DescList[DescPos].level = 8;
	
	for (famiCnt = DescPos; famiCnt > 0; famiCnt--) {
		if (DescList[famiCnt].level == 1) {
			DescList[famiCnt].level = 2;
			break;
			}
		if (DescList[famiCnt].level == 3)
			DescList[famiCnt].level = 4;
		if (DescList[famiCnt].level == 6)
			DescList[famiCnt].level = 7;
		if (DescList[famiCnt].level == 9)
			DescList[famiCnt].level = 8;
		}
	
	////////
	ExitFunc:
	////////

	//FrmReturnToForm (DescendancyForm);
	return true;
}


////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DescendancyResizeForm
//
// DESCRIPTION:   This routine resizes and draws the Descendancy Form.
//
// PARAMETERS:   	None. 
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean DescendancyResizeForm (void)
{
	Int16 			extentDelta;
	UInt16 			numObjects, i;
	Coord 			x, y;
	RectangleType 	objBounds;  // bounds of object being set
	RectangleType 	curBounds;  // bounds of active form
	RectangleType	displayBounds; // bounds of screen
	FormPtr			frmP;

	frmP = FrmGetActiveForm();

	// Get dimensions of current active form.
	WinGetBounds (FrmGetWindowHandle (frmP), &curBounds);
	
	// Get the new display window bounds
	WinGetBounds (WinGetDisplayWindow (), &displayBounds);

	extentDelta = 	(displayBounds.extent.y - displayBounds.topLeft.y) -
						(curBounds.extent.y - curBounds.topLeft.y);
	
	if (extentDelta == 0)  // form has not changed in size, so do nothing
		return false;
		
	WinSetBounds (FrmGetWindowHandle (frmP), &displayBounds); // set new form bounds
	
	// Iterate through objects and re-position them.
	numObjects = FrmGetNumberOfObjects (frmP);

	for (i = 0; i < numObjects; i++) {
	
		switch (FrmGetObjectId (frmP, i))
			{
			case DescendancyDoneButton:
			case DescendancyBackButton:
			case DescendancyGoToButton:
			case DescendancyInfoButton:
			case DescendancyTopButton:
			case DescendancyScrollUpRepeating:
			case DescendancyScrollDownRepeating:
				FrmGetObjectPosition (frmP, i, &x, &y);
				FrmSetObjectPosition (frmP, i, x, y + extentDelta);
				break;

			case DescendancyDescendancyTable:
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
// FUNCTION:    	DescendancyHandleEvent
//
// DESCRIPTION: 	This routine is the event handler for the Descendancy Chart.
//					 	IndiRec will be loaded and locked prior to calling this
//					 	routine.  It will be loaded and locked upon exit as well.
//                
// PARAMETERS:  	event  - a pointer to an EventType structure
//
// RETURNED:    	true if the event has handle and should not be passed
//              	to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean DescendancyHandleEvent (EventPtr event)
{
   Boolean 	handled = false;
   
   switch (event->eType)
		{
		case tblSelectEvent:
			{
			UInt16 selectedRec;
			selectedRec = TblGetRowData (event->data.tblSelect.pTable,
		      event->data.tblSelect.row);
		   if (CurrentIndiRecN != selectedRec) {
         	CurrentIndiRecN = selectedRec;		   	
		   	if (HistPos < QHIST_MAX)
		   		HistPos++;
		   	QueueHist[HistPos].person = CurrentIndiRecN;
		   	DescendancyLoadData (); // won't re-allocate arrays DTR 6-11-02
				FrmUpdateForm (DescendancyForm, updateRedrawAll);
				}
			// can't select top individual so unhighlight it
			TblUnhighlightSelection (event->data.tblSelect.pTable);
         handled = true;
         }
         break;

      case ctlSelectEvent:
         switch (event->data.ctlSelect.controlID)
         	{               
            case DescendancyDoneButton: // go back to person we started with
            	CurrentIndiRecN = QueueHist[0].person;
               FrmGotoForm (IndiSummForm);
               handled = true;
               break;

            case DescendancyGoToButton: // go to current root person
            	CurrentIndiRecN = QueueHist[HistPos].person;
               FrmGotoForm (IndiSummForm);
               handled = true;
               break;

				case DescendancyBackButton:
					HistPos--;
					CurrentIndiRecN = QueueHist[HistPos].person;
					DescendancyLoadData (); // won't re-allocate arrays DTR 6-11-02
               FrmUpdateForm (DescendancyForm, updateRedrawAll);
               handled = true;
               break;

				case DescendancyTopButton:
					TopDescPos = 0;
               FrmUpdateForm (DescendancyForm, updateRedrawAll);
               handled = true;
               break;
               
				case DescendancyInfoButton:
               FrmHelp (DescendancyHelpString);
               handled = true;
               break;
            
           default:
               break;
         	}
         break;

      case keyDownEvent:
      	if (EvtKeydownIsVirtual(event)) {
				
				WinDirectionType	direction;
				
				switch (event->data.keyDown.chr)
					{
				   case vchrPageUp:
				   case vchrPageDown:
				   	// Reset scroll rate if not auto repeating
				   	if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
				   		ResetScrollRate();
						AdjustScrollRate();
						if (event->data.keyDown.chr == vchrPageUp)
							direction = winUp;
						else
							direction = winDown;
				      DescendancyScroll (direction);
				      handled = true;
				      break;
					
					default:
						break;
					}
				}
         break;
      
      case ctlEnterEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case DescendancyScrollUpRepeating:
				case DescendancyScrollDownRepeating:
					ResetScrollRate();
					// leave unhandled so the buttons can repeat
					break;
				}
         break;
      
      case ctlRepeatEvent:
			AdjustScrollRate();
		   switch (event->data.ctlRepeat.controlID)
         	{
            case DescendancyScrollUpRepeating:
					DescendancyScroll (winUp);
					// leave unhandled so the buttons can repeat
               break;
               
            case DescendancyScrollDownRepeating:
					DescendancyScroll (winDown);
					// leave unhandled so the buttons can repeat
               break;
               
            default:
               break;
	         }
         break;
     	     
	   case frmOpenEvent:
         HistPos = 0;
 			
 			if (DynInDevice)
	      	DescendancyResizeForm (); // resize before drawing
 			 
 			// Load Descendancy array
			if (!DescendancyLoadData ()) { 
         	FrmGotoForm (IndiSummForm); // couldn't allocate required memory
				}
			else {
 				DescendancyInit ();
         	QueueHist[0].person = CurrentIndiRecN;
      		FrmDrawForm (FrmGetActiveForm ());
				DrawScreenLines (DescendancyForm);
				}
         handled = true;
         break;

	   case frmUpdateEvent:
 			DescendancyInit ();
         DrawForm ();
  			DrawScreenLines (DescendancyForm);
			handled = true;
			break;

		case winDisplayChangedEvent:
			if (DescendancyResizeForm ()) {
	 			DescendancyInit ();
   	      DrawForm ();
  				DrawScreenLines (DescendancyForm);
				}
			handled = true;
			break;

		case frmCloseEvent:
      	// if user Exits program or hits Done or Goto Button 
      	if (DescList) {
      		MemHandleFree (MemPtrRecoverHandle (DescList));
				DescList = 0;
				}
			if (QueueHist) {   
	     		MemHandleFree (MemPtrRecoverHandle (QueueHist));
 				QueueHist = 0;
 				}
	   	DbMemHandleUnlock (&IndiRecH);
	   	DbMemHandleUnlock (&FamiRecH);
      	break;
	
	   default:
		   break;
   	}
   return (handled);
}

#pragma mark-
////////////////////////////////////////////////////////////////////////////////////
// Function:		DaysInMonth
//
// Description: 	Calculates the days in the given month.  It factors in leapyears
//						in order to determine if the month of February has 28 or 29 days
//						A leapyear is evenly divisible by 4.  However, years that are
//						evenly divisible by 100 are not leapyears unless they are also
//						evenly divisible by 400. February has 29 days in a leapyear.
//						Example: 1900, not a leapyear. 2000 a leapyear
//
// Parameters:		-> month -	The month which we are interested in.
//						-> year  - 	The year which contains the given month.
//
// Returns:			The number of days in the month.
////////////////////////////////////////////////////////////////////////////////////
UInt16 DaysInMon (UInt16 month, UInt16 year)
{
	switch (month)
		{
		case 4: // April
		case 6: // June
		case 9: // September
		case 11:// November
			return 30;
			break;
		
		case 2: // February
			if (year % 4 == 0) {
				if (year % 100 != 0)
					return 29;
				else if (year % 400 == 0)
			  		return 29;
				}
			return 28; // not a leapyear
			break;
		
		default: // January, March, May, July, August, October, December
			return 31;
			break;
			}
		
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	ConvertDateToNumber
//
// DESCRIPTION: 	Calculates the date given the fields that have been entered in
//						the DateCalc form.
//
// PARAMETERS:  	None.
//
// RETURNED:    	True if successful in calculating date, else false
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
/*static Boolean ConvertDateToNumber (char *date, char *year)
{
	char	*token;
	char	dateStr[DATE_SZ+1]= "\0";
	char	delim[]				= " ,-./"; // delimiters for date string.
	char	dayVal[]				= "00";
	char	monthVal[]			= "00";
	char	yearVal[]			= "0000";	
	char  dateVal[]			= "00000000";
	int	holdInt;
	
	strcpy(year, "\0"); // initialize

	if (*date == '\0' || date == NULL)
		return 0;

	strncat(dateStr, _strupr(date), DATE_SZ);  // make sure string is upper case.

	if (dateStr[0] == '(' )
		return 0; // date phrases start with "(" and are not parsable.

	// first make sure date is not split (eg 1 JAN 1990-6 JUL 1991)
	if (strstr(dateStr,"-") || strstr(dateStr,"/")) {
		for (UINT x = 0; x < strlen(dateStr); x++)
			if (dateStr[x] == '-' || dateStr[x] == '/') {
				dateStr[x] = '\0';  // terminate the string
				break;
				}
		}

	token = strtok (dateStr, delim);

	while (token != NULL) {
	
		holdInt = atoi(token); // first set value of token to holdInt
		
		if (strstr("ABT,ABOUT,CALCULATED,ESTIMATED,BEFORE,AFTER,BTW,BETWEEN,FROM,INT", token)) {
			// do nothing;

		} else if (strstr("AND,/-",token)) { // if = one of these then ignore rest of string.
				break; 
		
		} else if (strstr("TO",token)) { // if token = TO then ignore rest of the string...
			if (atoi (yearVal) != 0)		  // ..but only if we already have a year value.
				break;

		} else if (strstr("JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC", token)) {
			// Get English Month //
			strcpy(monthVal, ConvertMonthToNumber(token, 0));

		} else if (strstr("VEND,BRUM,FRIM,NIVO,PLUV,VENT,GERM,FLOR,PRAI,MESS,THER,FRUC,COMP", token)) {
			// Get French Month //
			strcpy(monthVal, ConvertMonthToNumber(token, 1));

		} else if (strstr("TSH,CSH,KSL,TVT,SHV,ADR,ADS,NSN,IYR,SVN,TMZ,AAV,ELL", token)) {
			// Get Hebrew Month //
			strcpy (monthVal, ConvertMonthToNumber(token, 2));

		} else if (holdInt > 0 && holdInt <= 31) {  // get the day of month
			itoa(holdInt, dayVal, 10);
			if (strlen(dayVal) < 2) {
				strcat(dayVal, "0");
				_strrev(dayVal);
				}

		} else if (holdInt > 31 && holdInt < 9999) {  // get the year
			itoa(holdInt, yearVal, 10);
			if (strlen(yearVal) < 4) {
				_strrev(yearVal);
				strncat(yearVal, "00", 4 - strlen(yearVal));
				_strrev(yearVal);
				}
		}

		token = strtok (NULL, delim);
		
	}  // main while loop.
	
	if (atoi (yearVal) == 0) {
			return 0;
			}

	strcpy(year, yearVal);
	strcpy(dateVal, yearVal);

	if (*monthVal) {
		strncat(dateVal, monthVal, 2);
		strncat(dateVal, dayVal, 2);
		}
	return atoi(dateVal);
} 

*/

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    	DateCalc
//
// DESCRIPTION: 	Calculates the date given the fields that have been entered in
//						the DateCalc form.
//
// PARAMETERS:  	None.
//
// RETURNED:    	True if successful in calculating date, else false
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean DateCalc (void)
{
  	Int16 		dayStart, dayChg, dayCalc;
	Int16 		monStart, monChg, monCalc;
	Int16 		yrStart,  yrChg,  yrCalc;
	Int16			direction;
	Char			circStr[7] = "\0"; // hold word "circ" for End Date string
   FieldPtr 	fldP;
  	ListPtr 		lstP;

	// check which calendar and add/subtract operation we are using
	if (GetControlValue (DateCalcAddPushButton))
		direction = 1;
	else
		direction = -1;

	// get Start Month from selected list item
	lstP		= (ListPtr) GetObjectPtr (DateCalcMonth1List);
  	monStart = LstGetSelection (lstP) + 1;

	// get Start Day from its field
   fldP = (FieldPtr) GetObjectPtr (DateCalcDay1Field);
   if (FldGetTextLength (fldP) > 0)  // then data not entered yet
   	dayStart = (Int16) StrAToI (FldGetTextPtr (fldP));
	else
		goto ErrExit;
		
	// get Start Year from its field
   fldP = (FieldPtr) GetObjectPtr (DateCalcYear1Field);
   if (FldGetTextLength (fldP) > 0)
   	yrStart = (Int16) StrAToI (FldGetTextPtr (fldP));
	else {
		//if year has not been entered yet, don't bother going any further
		yrStart =0;
		goto ErrExit;
		}

	// check if the date entered is valid
	ShowObject (DateCalcInvalidDateLabel, false); // initialize
  	if (dayStart > DaysInMon (monStart, yrStart)) {
		ShowObject (DateCalcInvalidDateLabel, true);
		SndPlaySystemSound(sndWarning);
		goto ErrExit;
		}
	else
		ShowObject (DateCalcInvalidDateLabel, false);

	// get Change Months from the selected list item
	lstP	 = (ListPtr) GetObjectPtr (DateCalcMonthsList);
  	monChg = LstGetSelection (lstP);

	// get Change Days from its field	
   fldP = (FieldPtr) GetObjectPtr (DateCalcDaysField);
   if (FldGetTextLength (fldP) > 0)
   	dayChg = (Int16) StrAToI (FldGetTextPtr (fldP));
	else
		dayChg = 0;
  
	// get Change Years from its field		
   fldP = (FieldPtr) GetObjectPtr (DateCalcYearsField);
   if (FldGetTextLength (fldP) > 0)
   	yrChg = (Int16) StrAToI (FldGetTextPtr (fldP));
	else
		yrChg = 0;

	//  \/ \/		Finally, do the date change calculations 		\/ \/
	yrCalc  = yrStart  + (yrChg*direction);
	monCalc = monStart + (monChg*direction);
	dayCalc = dayStart + (dayChg*direction);
	
	// Recalculate months. The change at this point cannot be by more than
	// 11 months as it is limited by the popup list choices.
	if (monCalc < 1) {
		monCalc = 12 + monCalc;
		yrCalc--;
		}
		
	if (monCalc > 12) {
		monCalc = monCalc - 12;
		yrCalc++;
		}

	// Check for amiguous dates (e.g. 31 DEC 2000 + 2 months).  If the month
	// calculated at this point has fewer days than dayStart, then subtract
	// the difference in the number of days in each month from dayCalc.
	if (dayStart > DaysInMon (monCalc, yrCalc)) {
		StrCopy (circStr, "circa ");
		dayCalc = dayCalc - (dayStart - DaysInMon (monCalc, yrCalc));
		}

	// Recalculate days.
	while (dayCalc < 1) {
		monCalc--;
		if (monCalc < 1) {
			monCalc = 12;
			yrCalc--;
			}
		dayCalc = dayCalc + DaysInMon (monCalc, yrCalc);
		}

	while (dayCalc > DaysInMon (monCalc, yrCalc)) {
		dayCalc = dayCalc - DaysInMon (monCalc, yrCalc);
		monCalc++;
		if (monCalc > 12) {
			monCalc = 1;
			yrCalc++;
			}
		}

   // put calculated date in Result field
   fldP = (FieldPtr) GetObjectPtr (DateCalcResultField);
	StrPrintF (DateCalcStr, "%s%i %s %i", circStr, dayCalc, Months[monCalc-1],
		yrCalc);
   FldSetTextPtr (fldP, DateCalcStr);
   FldDrawField (fldP);


	return true; // success

	///////
	ErrExit:
	///////
	
  	// clear the End Date Field
  	fldP = (FieldPtr) GetObjectPtr (DateCalcResultField);
  	*DateCalcStr = '\0';
   FldSetTextPtr (fldP, DateCalcStr);
   FldDrawField (fldP);
	
	return false; // error in date

	}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DateCalcInitForm
//
// DESCRIPTION:   Initializes the fields and lists in the Date Calc Form.
//
// PARAMETERS:    None.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DateCalcInitForm (Boolean openInit)
{
	if (openInit) {
	
		ControlPtr	trigMon1P = (ControlPtr) GetObjectPtr (DateCalcMonth1PopTrigger);
		ControlPtr	trigMonsP = (ControlPtr) GetObjectPtr (DateCalcMonthsPopTrigger);
  		Char* 		labelMon1P;
  		Char* 		labelMonsP;
  		ListPtr 		lstMon1P;
  		ListPtr 		lstMonsP;
   
		lstMon1P= (ListPtr) GetObjectPtr (DateCalcMonth1List);
		lstMonsP= (ListPtr) GetObjectPtr (DateCalcMonthsList);
		LstSetListChoices (lstMon1P, Months, 12);
		LstSetListChoices (lstMonsP, MonNum, 12);
  		LstSetSelection (lstMon1P, 0);
  		LstSetSelection (lstMonsP, 0);
  		labelMon1P = LstGetSelectionText (lstMon1P, 0);
  		labelMonsP = LstGetSelectionText (lstMonsP, 0);
  		CtlSetLabel (trigMon1P, labelMon1P);
  		CtlSetLabel (trigMonsP, labelMonsP);
      	
		SetControlValue (DateCalcSubtractPushButton, true);
		SetControlValue (DateCalcAddPushButton, false);
		ShowObject (DateCalcInvalidDateLabel, false);

  		// erase any existing data in the fields
  		ClearField (DateCalcDay1Field);
		ClearField (DateCalcDaysField);
		ClearField (DateCalcYear1Field);
		ClearField (DateCalcYearsField);
   	}
    	
	// draw frames around input fields
	DrawObjectFrame (DateCalcDay1Field, 1, 0, true);
	DrawObjectFrame (DateCalcDaysField, 1, 0, true);
	DrawObjectFrame (DateCalcYear1Field, 1, 0, true);
	DrawObjectFrame (DateCalcYearsField, 1, 0, true);
	DrawObjectFrame (DateCalcMonth1List, 2, 12, true);
	DrawObjectFrame (DateCalcMonthsList, 2, 12, true);
	DrawObjectFrame (DateCalcResultField, 1, 0, false); // black frame
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DateCalcCheck
//
// DESCRIPTION:   Checks numbers entered in the day fields of the Date Calc Form.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void DateCalcCheck (EventPtr event)
{
   FieldPtr 	fldP;
   FormPtr  	frm; 
   UInt16		length;
   UInt16		objectID;
   UInt16		day;
   Boolean		err = false;
      
   frm = FrmGetActiveForm();
   
	objectID = FrmGetObjectId (frm, FrmGetFocus (frm));
	
	fldP = (FieldPtr) GetObjectPtr (objectID);

  	if (FldHandleEvent (fldP, event) || event->eType == fldChangedEvent) {
		length = FldGetTextLength (fldP);

		if (event->data.keyDown.chr == '.') // remove any period characters
			err = true;

		if (length == 0)
			return;

		day = (UInt16) StrAToI (FldGetTextPtr (fldP));

		if (objectID == DateCalcDay1Field) {
			if (day > 31 || day < 1)
				err = true;
			}
		else
			FldSetSelection (fldP,0,0); // set so this function does not crash Palm III's
	
		if (err) {
			FldDelete (fldP, length-1, length);
			SndPlaySystemSound (sndError);
			}
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      DateCalcHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the DateCalc Form.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure.
//
// RETURNED:      True if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean DateCalcHandleEvent (EventPtr event)
{
  	Boolean 	handled = false;
   	
   switch (event->eType)
   	{
  		case ctlSelectEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case DateCalcDoneButton:
					FrmGotoForm (PriorFormID);
               handled = true;
               break;
               
            case DateCalcClearButton:
					DateCalcInitForm (true);
					{
            	FieldPtr fldP = (FieldPtr) GetObjectPtr (DateCalcResultField);
           	  	*DateCalcStr = '\0';
   				FldSetTextPtr (fldP, DateCalcStr);  // clear result field
           	   FldDrawField (fldP);
               SetFocus (DateCalcDay1Field);
               }
               handled = true;
               break;
            	
            case DateCalcInfoButton:
               FrmHelp (DateCalcHelpString);
               handled = true;
             	break;
   
   			case DateCalcAddPushButton:
   			case DateCalcSubtractPushButton:
	  	   		DateCalc ();
	  	   		// do not set handled = true;
	  	   		break;
    
            default:
               break;
				}
         break;

   	case popSelectEvent: // called on popup list selection
   		DateCalc ();
   		FrmUpdateForm (DateCalcForm, updateViewReInit);
   		
   		if (event->data.ctlEnter.controlID == DateCalcMonth1PopTrigger)
         	SetFocus (DateCalcYear1Field);
         else if (event->data.ctlEnter.controlID == DateCalcMonthsPopTrigger)
        		SetFocus (DateCalcYearsField);
   		// do not set handled = true;
   		break;
   			
   	case fldChangedEvent:
      case keyDownEvent:
		  	DateCalcCheck (event);
		   DateCalc ();
		   handled = true;
         break;

   	case frmOpenEvent:
      	FrmDrawForm (FrmGetActiveForm ());
			DateCalcInitForm (true);
			SetFocus (DateCalcDay1Field);
         handled= true;
         break;

		case frmUpdateEvent:
      	// redraw frames around popup lists
      	if (event->data.frmUpdate.updateCode != updateViewReInit) {
	      	FrmDrawForm (FrmGetActiveForm());
      		DateCalcInitForm (false);
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
// FUNCTION:      RelFinderGetParents
//
// DESCRIPTION:   
//
// PARAMETERS:    
//
// RETURNED:      
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Char*	RetNumEnd (UInt16	number)
{
	if (number >= 11 && number <= 13)
		return cNth;
		
	number = number % 10;
	
	switch (number)
		{
		case 1:
			return cNst;
		case 2:
			return cNnd;
		case 3:
			return cNrd;
		default:
			return cNth;
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcGetParents
//
// DESCRIPTION:   Retieves parents for a given person.
//
// PARAMETERS:    -> indiRecN	-	record number of person to find.
//						<- husbRecN	-	record number of father of person to find.
//						<- wifeRecN -	record number of mother of person to find.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RelCalcGetParents (UInt16 indiRecN, UInt16 *husbRecN, UInt16 *wifeRecN)
{
   UInt32			famiRecN; // record in FamiDB
   DBRecordType   famiRec, indiRec;
   MemHandle      indiRecH = NULL;
   MemHandle      famiRecH = NULL;

	*husbRecN = NO_REC;
	*wifeRecN = NO_REC;	
	
 	// get the individual record
   DbGetRecord (IndiDB, indiRecN, &indiRec, &indiRecH);
	
	if (indiRec.fields[indiFamCNo])
		famiRecN = (UInt32) StrAToI (indiRec.fields[indiFamCNo]);
	else {
		DbMemHandleUnlock (&indiRecH);
		return;
		}
   
   ErrFatalDisplayIf (famiRecN == NO_REC_LONG, "RelCalcGetParents: famiRecN invalid");
   
   // get individual's birth family record
   DbGetRecord (FamiDB, famiRecN, &famiRec, &famiRecH);
 
  	// catch when husband or wife are not in family record
 	if (famiRec.fields[famiHusbNo])
 		*husbRecN = (UInt16) StrAToI (famiRec.fields[famiHusbNo]);
 	if (famiRec.fields[famiWifeNo])
 		*wifeRecN = (UInt16) StrAToI (famiRec.fields[famiWifeNo]);

	DbMemHandleUnlock (&indiRecH);
	DbMemHandleUnlock (&famiRecH);
		
  	return;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcShow
//
// DESCRIPTION:   Constructs a string that shows the relationship of 2 individuals.
//
// PARAMETERS:    -> i, j - positions of 2 individuals relative to common ancestor.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RelCalcShow (UInt16 i, UInt16 j, UInt16 sex)
{
	Char*		cFrmStr1 = "is the %i%s %s%s%s";
	Char*		cFrmStr2 = "is the %s%s%s";
	UInt16 	relaN;
	UInt16 	remvN;
	UInt16 	num;
	Char*		hldStr;
		
	if (i >= 2 && j >= 2) { // cousins
		num = (i > j ? j : i);
		relaN = num - 1;
		remvN = (i > j ? i-j : j-i);
		if (remvN)
			StrPrintF (RCStr, "is the %i%s %s %ix rem.", relaN,
				RetNumEnd (relaN), cCousin, remvN); 
		else
			StrPrintF (RCStr, "is the %i%s %s", relaN, RetNumEnd (relaN), cCousin);
		}
	else if ( (i >= 1 && j >= 1)) { // niece, nephew, uncle, aunt, sibling
		num = (i == 1 ? j : i);
		if (sex == 1)
			hldStr = (i < j ?  cNephew : cUncle);
		else
			hldStr = (i < j ?  cNiece : cAunt);
			
		if (num >= 5) {
			relaN = num - 3;
			StrPrintF (RCStr, cFrmStr1, relaN,  RetNumEnd (relaN),
				cGreat, cGrand, hldStr);
			}
		else if (num >= 2) {
			StrPrintF (RCStr, cFrmStr2, (num == 4 ? cGreat : ""),
				(num >= 3 ? cGrand : ""), hldStr);
			} 
		else { // if (num == 1 || num == 0) {
			StrPrintF (RCStr, cFrmStr2, num == 1 ? cSibling : cChild, "", "");
			}
		}
	else if (i == 0 ||  j == 0) { // great- grand- father/child
		num = (i == 0 ? j : i);
		hldStr = (i < j ? cChild : cParent);

		if (num >= 4) {
			relaN = num - 2;
			StrPrintF (RCStr, cFrmStr1, relaN, RetNumEnd (relaN),
				cGreat, cGrand, hldStr); 
			}
		else if (num >= 1) {
			StrPrintF (RCStr, cFrmStr2, (num >= 3 ? cGreat : ""),
				(num >= 2 ? cGrand : ""), hldStr);
			}
		else
			StrPrintF (RCStr, "is the same person as");
		}
	if (! (i == 0 && j == 0))	
		StrCat (RCStr, " of");
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcInitArr
//
// DESCRIPTION:   This routine loads the RCAncArr given a starting the person (indiN)
//						record number.
//
// PARAMETERS:    -> indiN  	- Individual Record number
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean RelCalcInitArr (UInt16 indiN)
{
	Int16		x = 0; // number of generations from indiN
	UInt16	pos;
	
	if (indiN == NO_REC)
		return false;

	// initialize array
	for (pos = 0; pos < RCArrSz; pos++)
		RCAncArr[pos] = NO_REC;
		
	pos = 1;
	RCList1[x].chilRecN = indiN;
	RCList1[x].parent = 0;
	
	while (x >= 0) {
	
		if (RCList1[x].parent == 2) {
				x--;
				if (x < 0) break;
				pos = pos / 2;
				}
		else { 
			RelCalcGetParents (RCList1[x].chilRecN, &RCList1[x].husbRecN,
				&RCList1[x].wifeRecN);
			RCAncArr[pos-1] = RCList1[x].chilRecN;
			}
		
		if (x >= RCArrGen) {
			RCList1[x].parent = 2;
			continue;
			}

		// decide which line to go up (mother or father)
		if (RelCalcIncr (RCList1, &x)) {
			if (RCList1[x-1].parent == 1) // mother
				pos = pos*2;
			else // father
				pos = (pos*2) + 1;
			}
		
		// pos can equal RCArrSz b/c whenever we access RCAncArr, we
		// subtract 1 from pos
		ErrFatalDisplayIf (pos > RCArrSz, "RelCalcInitArr: pos too high");
		
 		// check if user pressed "Cancel" button.
 		if ((x % 4) == 0 && SearchCanceled (RelCalcSearchCancelButton))
			return false;
		
		}
	return true;
	}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcIncr
//
// DESCRIPTION:   This routine decides which parent line to move up in the family
//						record at position pos.
//
// PARAMETERS:    ->  RCList  -	a pointer the RCList array structure.
//						<-> pos	   -	the current (last) position in the RCList.
//
// RETURNED:      True if able to advance to a parent, else false.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean RelCalcIncr (RCListType* RCList, Int16* pos)
{
	if (RCList[*pos].wifeRecN == NO_REC && RCList[*pos].parent == 0)
		RCList[*pos].parent = 1; 

	if (RCList[*pos].husbRecN == NO_REC && RCList[*pos].parent == 1)
		RCList[*pos].parent = 2;

	if (RCList[*pos].parent == 2)
		return false;

	if (RCList[*pos].parent == 0) {
		RCList[*pos+1].chilRecN = RCList[*pos].wifeRecN;
		RCList[*pos].parent = 1;
		}
	else if (RCList[*pos].parent == 1) {
		RCList[*pos+1].chilRecN = RCList[*pos].husbRecN;
		RCList[*pos].parent = 2;
		}
			
	(*pos)++;
	RCList[*pos].parent = 0; // initialize
	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalc
//
// DESCRIPTION:   Searches for a relationship between 2 individuals given the record
//						numbers of each individual.  The methodology is as follows:
//						(1) Set up an array in memory that will hold the record numbers of
//						the first RCArrGen generations of individuals of indiN1.  This
//						will drastically reduce search time, given accessing records from
//						a Palm db multiple times will be very very slow.
//						(2) Search all the ancestors of indiN1 to see if indiN2 is a blood
//						relationship.  If not, then one by one increment through indiN2's
//						ancestors to see if indiN1 has a common relationship.
//						The i and j variables hold the number of generations between the
//						given individuals and a common ancestor. The search will be
//						performed for relationships withing maxI and maxJ generations.
//
//						The RCList1 and RCList2 arrays must be created in memory before
//						calling this function.  Each element of this array holds a child
//						and his/her parents.
//
// PARAMETERS:    -> indiN1-	the record numbers of individual 1.
//						-> indiN2-	the record numbers of individual 2.
//						<> i		- 	A position in the RCList1 array.  If a common
//										ancestor is found, this will represent the number
//										of generations between indiN1 and the common ancestor
//										of indiN2.
//						<> j		- 	A position in the RCList2 array.  If a common
//										ancestor is found, this will represent the number
//										of generations between indiN2 and the common ancestor
//										of indiN1.
//						-> maxI	-	The number of generations to search.
//						<> maxJ 	-	The adjusted maxLvls for the "j" loop.  If a
//										common ancestor is found, we can adjust the 
//										maximum number of generations to search based on
//										the where the relationship was found.
//						<> y 		- 	position in the RCAncArr array. This array contains an
//										ahnentafel format of first "RCArrGen" generations of 
//										individual indiN1. Females will have even numbers and
// 									males odd numbers. Note that y will always have
//										1 subtracted from when accessing the desired persion
//										the RCAncArr array.
//
// RETURNED:      True if relationship found, else false
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static Boolean RelCalc (UInt16 indiN1, UInt16 indiN2, Int16* i, Int16* j, UInt16* maxI,
	UInt16* maxJ, Int16 *y, Boolean* cont, Boolean swapped)
{
	Boolean 			found = false;
	Int16				x;
	UInt32			counter = 0;
	Boolean			endLoop = true;
	static Boolean	inArr;
	UInt16			indiRecN;
	Boolean			doCompare;
	UInt16			hldLvls;
	UInt16			hldI;

	if (indiN1 == NO_REC || indiN2 == NO_REC)
		goto EndExit;

	if (*i > 0 || *j > 0)
		goto ContinuePoint;

	inArr = true;
	endLoop = false;
	RCList2[0].chilRecN = indiN2;
	RCList2[0].parent = 0;

	if (!RelCalcInitArr (indiN1)) {
		*cont = false;
		return false;
		}
		
	*j = 0;
	while (*j >= 0) {
	
		if (RCList2[*j].parent == 2) {
			(*j)--;
			if (*j < 0)	break;
			}
		else {
			RelCalcGetParents (RCList2[*j].chilRecN, &RCList2[*j].husbRecN,
				&RCList2[*j].wifeRecN);
				
			endLoop = false; // initialize				
			*y = 1; // initialize

			while (*y > 0) {
					
				doCompare = true;
				
				if (!inArr) {
					if (RCList1[*i].parent == 2 || endLoop) {
						if (*i == 0) {
							inArr = true;
							endLoop = true;
							continue;
							}
						else {
							(*i)--;
							}
						doCompare = false;
						}
					else { // 
						ErrFatalDisplayIf (*i <=0, "i too low");
						ErrFatalDisplayIf (*i > *maxI-RCArrGen, "RelCalc: *i too high #1");
						ErrFatalDisplayIf (*y < RCArrSz/2 || *y > RCArrSz, "RelCalc: bad DB Access");
						
						RelCalcGetParents (RCList1[*i].chilRecN, &RCList1[*i].husbRecN,
							&RCList1[*i].wifeRecN);
						}
					indiRecN = RCList1[*i].chilRecN;
					}
					
				else { // inArr
					if (RCAncArr[(*y)-1] == NO_REC || endLoop) {

						while (odd (*y)) {
							*y = ((*y)-1) / 2;
							if (*y == 1) {
								*y = 0;
								break;
								}
							}

							if (*y == 0) break;
							(*y)+= 1;
							}
								
						if (RCAncArr[(*y)-1] == NO_REC) continue;
						indiRecN = RCAncArr[(*y)-1];
						}

				endLoop = false; // initialize

				if (doCompare) {

					// Check if common ancestor match was found using this algorythm:
					// The RCList2 will hold the trail of individual that the "j" path
					// has followed. The "x" variable loops through the RCList2 to search
					// for a match w/ the current individual at the "i" / "y" position.
					// If a match is found then we want to reverse the "i" / "y" trail,
					// as we have already covered that trail with the "j" path.
					// The exceptions to this is if the match is found at position "j"
					// (ie. where x ==*j) as this is the only time we have a valid match.
					// Also, if match is found at an individual (at "x") below position
					// "j" in RCList2 AND this individual is a direct ancestor or the
					// parents of a direct ancestor (eg x <= 1 || *y <=3) then
					// increment"x" to see if a match is found beyond the parents.
					// This is done as it is possible to have a common ancestor reached
					// via each parent,so we don't want to reverse the "i" / "y" path
					// prematurely if it is taking a different path than "j".
					for (x = 0; x <=*j; x++) {
						if (indiRecN == RCList2[x].chilRecN) {
							if (!inArr)
								RCList1[*i].parent = 2;
							if ((*j-x) > 1 && (x <= 1 || *y <= 3)) continue;
							if (x != *j)
								endLoop = true;
							else
								found = true;
							break;
							}
						} // for loop

					if (found) {
						
						// save Progenitor record number
						if (x>0) {
							ProgRecN1 = RCList2[x-1].husbRecN;
							ProgRecN2 = RCList2[x-1].wifeRecN;
							}
						else
							ProgRecN1 = ProgRecN2 = indiRecN;
						
						// convert y to a generation number
						if (inArr) {
							x = *y;
							*i =0;
							while (x > 1) {
								x = x/2;
								(*i)++;
								}
							}
							
						// make sure we don't duplicate relationship
						for (x = 0; x < RCHistLast; x++)
							if (RCHist[x].iPos == *i + (inArr ? 0 : RCArrGen))
								if (RCHist[x].jPos == *j) {
									endLoop = true;
									found = false;
									break;
									}
	
						if (endLoop) continue;
									
						// store i and j in relationship history array
						if (RCHistLast < RC_HIST_MAX) {
							RCHist[RCHistLast].iPos = *i + (inArr ? 0 : RCArrGen);
							RCHist[RCHistLast].jPos = *j;
							RCHistLast++;
							}

						// adjust maximum i or j value based on found relationship
						if (*maxJ == *maxI ) {
							hldI = *i + (inArr ? 0 : RCArrGen);
							if (*j > hldI+3) {
								hldLvls = (*maxJ - *j) + hldI+2; // add extra 2 gen.
	 							*maxI = hldLvls;
	 							if (*maxI < RCArrGen) *maxI = RCArrGen;
			 					}
							else if (hldI > *j+3) {
								hldLvls = (*maxI - hldI) + *j+2; // add extra 2 gen.
								*maxJ = hldLvls;
								}
							}
			 			}
								
					if (found) break;
	
					} // if (doCompare)
	
		 		// check if 'Canceled' button was hit
				counter++;
	 			if ((counter & 0x0000000f) == 0 &&
					SearchCanceled (RelCalcSearchCancelButton)) {
					*cont = false;
					return false;
					}
			
				/////////////
				ContinuePoint:
				/////////////
				
				if (!inArr) {
					if (*i >= *maxI - RCArrGen) {
						endLoop = true;
					 	continue;
					 	}
					if (!RelCalcIncr (RCList1, i))
						continue;
					}
				else { // inArr
					if (endLoop) continue;
					*y = (*y) * 2;
					if (*y > RCArrSz) {
						ErrFatalDisplayIf (*y > (RCArrSz*2)+1, "RelCalc: y too high");
						*y = (*y) / 2;
						if (*maxI == RCArrGen) {
							endLoop = true;
							continue;
							}
						RCList1[0].chilRecN = RCAncArr[(*y)-1];
						RCList1[0].parent = 0;
						*i = 0;
						RelCalcGetParents (RCList1[*i].chilRecN, &RCList1[*i].husbRecN,
							&RCList1[*i].wifeRecN);
						if (!RelCalcIncr (RCList1, i)) {
							endLoop = true;
							continue;
							}
						else 
							inArr = false;
						}
					}
					
 				} // while (y > 0) loop

			} // else

		if (found) break;

		if (*j >= *maxJ) {
			RCList2[*j].parent = 2;
		 	continue;
		 	}
		if (!RelCalcIncr (RCList2, j))
			continue;
		
		EvtResetAutoOffTimer (); // stop auto-off timer
		
		} // while (*j >= 0) loop

	///////
	EndExit:
	///////

	if (found) {
		DBRecordType	indiRec;
		UInt16			sex;
		MemHandle		indiRecH = NULL;
		UInt16			chilRecN;

		if (swapped)
			chilRecN = indiN2;
		else
			chilRecN = indiN1;
		
   	// get individual's sex
   	DbGetRecord (IndiDB, chilRecN, &indiRec, &indiRecH);
		if (indiRec.fields[indiSex])
		if (StrCompare (indiRec.fields[indiSex], "M") == 0)
			sex = 1;
		else
		 	sex = 0;
		DbMemHandleUnlock (&indiRecH);

		#ifdef GREMLINS  // for debugging only
		{
		UInt16 	t = *y;
		UInt16	indiHoldN;
		
		while (t > 0) {
			indiHoldN = RCAncArr[t-1];
			t = t/2; // put breakpoint here to view indiHoldN
			}
		}
		#endif

		if (swapped) 
			RelCalcShow (*i + (inArr ? 0 : RCArrGen), *j, sex);
		else
			RelCalcShow (*j, *i + (inArr ? 0 : RCArrGen), sex);
		
		return true;
		} // if found
	else {
		if (RCHistLast == 0)
			StrCopy (RCStr, "has no blood relationship to");
		return false;
		}
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcInitForm
//
// DESCRIPTION:   Looks up the given individual record number in the IndiDB and 
//						write that individual's name in the buttonID area of the screen.
//						This routine firsts checks to make sure an Alias name was not
//						selected. If so, the primary name record is retrieved.
//						We have already made sure RelCalcRecN2 and CurrentIndiRecN do
//						not exceed size of IndiDB.
//
// PARAMETERS:    <- indiRecN - 	record number of individual to look up in IndiDB.
//						-> buttonID -	button that holds the persons name.
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RelCalcInitIndi (UInt16* indiRecN, UInt16 buttonID)
{
	DBRecordType	rec;
	MemHandle		recH = NULL;
	RectangleType	rect;
	FontID			curFont = FntSetFont (stdFont);
	
	GetObjectBounds (buttonID, &rect);

	if (*indiRecN != NO_REC) {
		
		DbGetRecord (IndiDB, *indiRecN, &rec, &recH);
		// next check if individual selected is an alias name
 		if (rec.fields[indiAlias] && !StrCompare (rec.fields[indiAlias], "A")) { 
  			*indiRecN = (UInt16) StrAToI (rec.fields[indiNo]);
  			DbMemHandleUnlock (&recH);
     		DbGetRecord (IndiDB, *indiRecN, &rec, &recH);
     		}
     	DrawRecordNameAndLifespan (&rec, &rect, false, false);
		DbMemHandleUnlock (&recH);
		}
 	else {
 		WinDrawChars (SELECT_IND, SELECT_IND_LEN, rect.topLeft.x, rect.topLeft.y);
 		}

	rect.topLeft.x -=1;
	WinDrawGrayRectangleFrame (simpleFrame, &rect);
	
	curFont = FntSetFont (curFont);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcSearch
//
// DESCRIPTION:   This routines initiates the search for the relationship between
//						the two individiuals entered.  It also creates the large array
//						structures needed (on the heap) to carry out the search.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure
//
// RETURNED:      Nothing.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RelCalcSearch (EventPtr	event)
{
	Boolean 				found;
	MemHandle			RelCalc1H = NULL;
	MemHandle 			RelCalc2H = NULL;
	MemHandle 			RelCalcSH = NULL;
	MemHandle 			RCHistH;
	Boolean				cont = true;
	Int16					i = 0;
	Int16					j = 0;
	Int16 				y = 1; // must always start at 1!!! (not 0)
	FormPtr				frm;
	UInt16				maxJ;
	UInt16				maxI;
	UInt16				hldMax;
	Boolean				firstTry = true;
	Boolean				firstN1 = true;

	// get maximum generations to search based on user selection
	if (GetControlValue (RelCalcFastCheckbox))
		hldMax = RC_MAX_GEN_FAST; // 15 generations (incl. starting individual)
	else
		hldMax = RC_MAX_GEN_SLOW; // 25 generations (incl. starting individual)

	maxJ = maxI = hldMax; // init
	RCHistLast = 0; // init
	StrCopy (RCStr, "and");
	
	RelCalcInit ();

	// create room on heap for RC array. If not enough room make it smaller.
	RelCalcSH = MemHandleNew (sizeof (UInt16) * RCArrSz);
	while (!RelCalcSH && RCArrGen > RC_ARR_GEN-4) {
		RCArrSz = RCArrSz/2;
		RCArrGen--;
		RelCalcSH = MemHandleNew (sizeof (UInt16) * RCArrSz);
		}
	ErrFatalDisplayIf (!RelCalcSH, "Out of memory");
	if (!RelCalcSH) goto EndExit;
	RCAncArr = MemHandleLock (RelCalcSH);

	RCHistH = MemHandleNew (sizeof (RCHistType) * RC_HIST_MAX);
	ErrFatalDisplayIf (!RCHistH, "Out of memory");
	if (!RCHistH) goto EndExit;
	RCHist = MemHandleLock (RCHistH);  
				
	RelCalc1H = MemHandleNew (sizeof (RCListType) * (maxI+2));
	ErrFatalDisplayIf (!RelCalc1H, "Out of memory");
	if (!RelCalc1H) goto EndExit;
	RCList1 = MemHandleLock (RelCalc1H);

	RelCalc2H = MemHandleNew (sizeof (RCListType) * (maxI+2));
	ErrFatalDisplayIf (!RelCalc2H, "Out of memory");
	if (!RelCalc2H) goto EndExit;
	RCList2 = MemHandleLock (RelCalc2H);  
	
	// display searching message
	frm = FrmInitForm (RelCalcSearchForm);
	FrmDrawForm (frm);
	FrmSetActiveForm (frm);

	while (cont) {
					
		ShowObject (RelCalcSearchContinueLabel, false);
		ShowObject (RelCalcSearchNoButton, false);
		ShowObject (RelCalcSearchYesButton, false);
		ShowObject (RelCalcSearchSearchingLabel, true);
		ShowObject (RelCalcSearchCancelButton, true);

		if (firstTry) {
			maxJ = 1; // search within 2 generations of RelCaclRecN1 person
			found = RelCalc (RelCalcRecN1, RelCalcRecN2, &i, &j, &maxI, &maxJ, &y,
				&cont, false);
			maxJ = hldMax;
			if (! found && cont) {
				StrCopy (RCStr, "and");
				j= i = 0;
				y = 1;
				firstN1 = false;
				found = RelCalc (RelCalcRecN2, RelCalcRecN1, &i, &j, &maxI, &maxJ, &y,
					&cont, true);
				}
			firstTry = false;
			}
		else { // not firstTry
			if (firstN1)
				found = RelCalc (RelCalcRecN1, RelCalcRecN2, &i, &j, &maxI, &maxJ, &y,
					&cont, false);
			else
				found = RelCalc (RelCalcRecN2, RelCalcRecN1, &i, &j, &maxI, &maxJ, &y,
					&cont, true);
			}

		// display relationship and beep
		if ((found || RCHistLast == 0) && cont) {
			FrmReturnToForm (RelCalcForm);
			RelCalcInit ();  // display relationship in RCStr
			
			// redisplay Searching form
			frm = FrmInitForm (RelCalcSearchForm);
			FrmDrawForm (frm);
			FrmSetActiveForm (frm);
						
			// ask user if he wants to continue and wait for response
			ShowObject (RelCalcSearchCancelButton, false);
			ShowObject (RelCalcSearchSearchingLabel, false);
			ShowObject (RelCalcSearchNoButton, true);
			ShowObject (RelCalcSearchYesButton, true);
			ShowObject (RelCalcSearchContinueLabel, true);
			}

		if (cont)
			SndPlaySystemSound (sndConfirmation);

		if (!found) {
			cont = false;
			}
		else { // if (found)
		
			RectangleType	rectP1, rectP2;
			GetObjectBounds (RelCalcSearchNoButton, &rectP1);
			GetObjectBounds (RelCalcSearchYesButton, &rectP2);
			
			while (cont) {

				EvtGetEvent (event, evtWaitForever);

				if (!SysHandleEvent (event)) {					
	
					if (RctPtInRectangle (event->screenX, event->screenY, &rectP1))
						CtlHandleEvent (GetObjectPtr (RelCalcSearchNoButton), event);
					else if (RctPtInRectangle (event->screenX, event->screenY, &rectP2))
						CtlHandleEvent (GetObjectPtr (RelCalcSearchYesButton), event);

					if (event->eType == ctlSelectEvent) {
						if (event->data.ctlSelect.controlID == RelCalcSearchNoButton) {
							cont = false;
							break;
							}
						else if (event->data.ctlSelect.controlID == RelCalcSearchYesButton) {
							// cont is already true
							break;
							}
						}
					else if (event->eType == appStopEvent) {
						EvtAddEventToQueue (event); // add event to queue again
						goto EndExit;
						}
					}
					
				} // while (cont)
				
			} // if (found)
					
		if (!cont) break;	
		
		} // while cont

	// erase searching message
	FrmReturnToForm (RelCalcForm);

	///////
	EndExit:
	///////
	
	if (RelCalc1H) MemHandleFree (RelCalc1H);
	if (RelCalc2H) MemHandleFree (RelCalc2H);
	if (RCHistH)   MemHandleFree (RCHistH);
	if (RelCalcSH) MemHandleFree (RelCalcSH);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcInit
//
// DESCRIPTION:   This routine initializes the Relatonship Calculator form.
//						RCStr must be initialized prior to calling this function.
//
// PARAMETERS:    None.
//
// RETURNED:      Nothing
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
static void RelCalcInit (void)
{
  	RectangleType	rect;
	FontID			curFont = FntSetFont (boldFont);
	char 				relCntStr[5];
	DBRecordType  	rec;
  	MemHandle      recH = NULL;
	
 	EraseRectangleObject (RelCalcResultGadget);
	EraseRectangleObject (RelCalcProgenitor1Button);
	EraseRectangleObject (RelCalcProgenitor2Button);
	
	ShowObject (RelCalcProgenitor1Button, false);
	ShowObject (RelCalcProgenitor2Button, false);

	if (RCHistLast != 0) { // then there are relationships to show
	
		// draw relationship counter information
		ShowObject (RelCalcFindLabel, false); // hide 'Find' label
		EraseRectangleObject (RelCalcFindLabel);
		GetObjectBounds (RelCalcFindLabel, &rect);
		WinDrawChars (RC_REL_STR, RC_REL_STR_LEN, rect.topLeft.x, rect.topLeft.y);
		StrPrintF (relCntStr, "%i", RCHistLast);
		WinDrawChars (relCntStr, StrLen (relCntStr), 78, rect.topLeft.y);
		FntSetFont (curFont);		
		
		// draw progenitor names, except when progenitor is the direct ancestor
		if (RelCalcRecN1 != ProgRecN1 && RelCalcRecN2 != ProgRecN1 &&
			 RelCalcRecN1 != ProgRecN2 && RelCalcRecN2 != ProgRecN2) {

			ShowObject (RelCalcProgenitorsLabel, true); // show "Progenitors:" label

 			GetObjectBounds (RelCalcProgenitor1Button, &rect);

			if (ProgRecN1 != NO_REC) {
      		
      		if (DbGetRecord (IndiDB, ProgRecN1, &rec, &recH) != 0) {
	      		ErrNonFatalDisplay ("RelCalcInit: Progenitor Record not found");
   	   		}
   			else {
      			DrawRecordNameAndLifespan (&rec, &rect, true, false);
      			DbMemHandleUnlock (&recH);
	   			ShowObject (RelCalcProgenitor1Button, true);
      			}
				}

 			GetObjectBounds (RelCalcProgenitor2Button, &rect);
     	
			if (ProgRecN2 != NO_REC) {
	      	
	      	if (DbGetRecord (IndiDB, ProgRecN2, &rec, &recH) != 0) {
	      		ErrNonFatalDisplay ("RelCalcInit: Progenitor Record not found");
   	   		}
   			else {
      			DrawRecordNameAndLifespan (&rec, &rect, true, false);
      			DbMemHandleUnlock (&recH);
      			ShowObject (RelCalcProgenitor2Button, true);
      			}
				}
			}
		}
	else { // no relationships found
		ShowObject (RelCalcFindLabel, true);
		ShowObject (RelCalcProgenitorsLabel, false); // hide "Progenitors:" label
		}
	
	// draw relationship string or the word "and"
	if (RCStr || *RCStr) {
		FntSetFont (boldFont);
		GetObjectBounds (RelCalcResultGadget,&rect);
		WinDrawChars (RCStr, StrLen (RCStr), rect.topLeft.x, rect.topLeft.y);
		}

	FntSetFont (curFont);
	
	ErrFatalDisplayIf (RelCalcRecN2 >= IndiDBNumRecs && RelCalcRecN2 != NO_REC,
 		"RelCalcInit: RelCalcRecN2 too high");
	ErrFatalDisplayIf (RelCalcRecN1 >= IndiDBNumRecs && RelCalcRecN1 != NO_REC,
 		"RelCalcInit: RelCalcRecN1 too high");

	// display names of individuals to search
	RelCalcInitIndi (&RelCalcRecN1, RelCalcPerson1Button);
	RelCalcInitIndi (&RelCalcRecN2, RelCalcPerson2Button);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      RelCalcHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Relationship Calculator
//						Form.
//
// PARAMETERS:    -> event  - a pointer to an EventType structure.
//
// RETURNED:      True if the event has handle and should not be passed to a higher
//                level handler.
//
// REVISIONS:		None.
////////////////////////////////////////////////////////////////////////////////////
Boolean RelCalcHandleEvent (EventPtr event)
{
   Boolean 	handled = false;
   	
   switch (event->eType)
   	{
  		case ctlSelectEvent:
			switch (event->data.ctlEnter.controlID)
				{
				case RelCalcDoneButton:
					FrmGotoForm (PriorFormID);
					*RCStr = '\0';
               handled = true;
	            break;

				case RelCalcCalculateButton:
               RelCalcSearch (event);
               handled = true;
	            break;
					
				case RelCalcPerson1Button:
      			RelCalcEntry = true;
      			RelCalcGetRec1 = true;
		      	ListViewSelectRecN = RelCalcRecN1;
      		   FrmGotoForm (IndiListForm);
               handled = true;
	            break;

				case RelCalcPerson2Button:
      			RelCalcEntry = true;
	     			RelCalcGetRec1 = false;
		      	ListViewSelectRecN = RelCalcRecN2;
      		   FrmGotoForm (IndiListForm);
               handled = true;
	            break;

				case RelCalcProgenitor1Button:
      			CurrentIndiRecN = ProgRecN1;
        			FrmGotoForm (IndiSummForm);
               handled = true;
	            break;

				case RelCalcProgenitor2Button:
      			CurrentIndiRecN = ProgRecN2;
        			FrmGotoForm (IndiSummForm);
               handled = true;
	            break;

            case RelCalcInfoButton:
               FrmHelp (RelCalcHelpString);
               handled = true;
             	break;
               
            default:
               break;
				}
         break;

      case frmOpenEvent:
      
        	//if (CurrentIndiRecN >= IndiDBNumRecs && CurrentIndiRecN != NO_REC) { revised 10-14-2004
        	if (CurrentIndiRecN >= IndiDBNumRecs) {
      		ErrNonFatalDisplayIf (CurrentIndiRecN != NO_REC, "RelCalcHandleEvent: CurrentIndiRecN too high");
      		CurrentIndiRecN = NO_REC;
      		}
      
			if (!RelCalcEntry) {
				RelCalcRecN1 = HldIndiRecN = CurrentIndiRecN;
				// if user loaded a new database, make sure saved record number
				// is not greater than size of new database.
				if (RelCalcRecN2 >= IndiDBNumRecs)
					RelCalcRecN2 = NO_REC;
				}
			else { // RelCalcEntry
				if (RelCalcGetRec1)
					RelCalcRecN1 = CurrentIndiRecN;
				else
					RelCalcRecN2 = CurrentIndiRecN;
					
				SetControlValue (RelCalcFastCheckbox, RCFastSrch);
				SetControlValue (RelCalcSlowCheckbox, !RCFastSrch);
				}
			
			RelCalcEntry = false; // reinit
         CurrentIndiRecN = ListViewSelectRecN = HldIndiRecN; // do this in case user exits app
			
      	FrmDrawForm (FrmGetActiveForm ());
	
			RCHistLast = 0; // must init here
			StrCopy (RCStr, "and");
			RelCalcInit ();
         handled= true;
         break;
		
		case frmUpdateEvent:
			FrmDrawForm (FrmGetActiveForm ());
			RelCalcInit ();
   		handled = true;
			break;

		case frmCloseEvent:
			RCFastSrch = (Boolean) GetControlValue (RelCalcFastCheckbox);
         break;

      default:
      	break;
   	}
      
   return (handled);
}
