////////////////////////////////////////////////////////////////////////////////////
//
// PROJECT:       GedWise Version 6.0 & 6.1
//
// FILE:          Soundex.c
//
// AUTHOR:        Daniel T. Rencricca: May 10, 2004
//
// DESCRIPTION:   Application's Soundex Code converter routines.
//
////////////////////////////////////////////////////////////////////////////////////
// Copyright © 2001 - 2005 Battery Park Software Corporation
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////

#include <PalmOS.h>
#include "AppMain_res.h"
#include "Soundex.h"
#include "AppMain.h"
#include "AppMisc.h"
#include "Defines.h"

////////////////////////////////////////////////////////////////////////////////////
// Externally defined variables
////////////////////////////////////////////////////////////////////////////////////
// None

////////////////////////////////////////////////////////////////////////////////////
// Local Variables and Defines
////////////////////////////////////////////////////////////////////////////////////
Char    SoundexCode[5];
Char    ErrStr[] = "Err "; // must be 4 characters

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:        GetObjectPtr
//
// DESCRIPTION:     This routine returns a pointer to an object in the current form.
//
// PARAMETERS:      -> objectID - id of the form object to get pointer to
//
// RETURNED:        Nothing
//
// REVISIONS:       None.
////////////////////////////////////////////////////////////////////////////////////
static void* GetObjectPtr (const UInt16 objectID)
{
   FormPtr frm = FrmGetActiveForm ();
   return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SoundexGetCode
//
// DESCRIPTION:   Called when "Code" button tapped on SoundexForm.  It checks
//                to make sure SoundexNameField is not empty and calls the 
//                Soundex function to get the Soundex Code.
//
// PARAMETERS:    None.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void SoundexGetCode (void)
{
   FieldPtr        fldP1;
   FieldPtr        fldP2;
   Char*           name;
   Int16           length;

   fldP1 = (FieldPtr) GetObjectPtr (SoundexNameField);

   name = FldGetTextPtr (fldP1); 

   length = FldGetTextLength (fldP1);
     if (length > 0) {
      fldP2 = (FieldPtr) GetObjectPtr (SoundexCodeField);
      FldSetTextPtr (fldP2, Soundex (name));  
      FldDrawField (fldP2);
      FldRecalculateField (fldP2, true);
      }
   }

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SoundexCheckChar
//
// DESCRIPTION:   Checks to make sure last character entered was a 
//                valid character.
//
// PARAMETERS:    -> event -    pointer to event from which to get charactered
//                                        entered.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void SoundexCheckChar (EventPtr event)
{
   FieldPtr     fldP;
   Char*        fldTextP;
   Char         aChar;
   Int16        length;

   fldP = (FieldPtr) GetObjectPtr (SoundexNameField);
   
   if (FldHandleEvent (fldP, event) || event->eType == fldChangedEvent) {
      
      fldTextP = FldGetTextPtr (fldP);
                               
      length = FldGetTextLength (fldP);
       
       if (length > 0)
          
          aChar = fldTextP[length-1];
          
          if (! (aChar >= 'a' && aChar <= 'z' || aChar >= 'A' && aChar <= 'Z')) {
            FldDelete (fldP, length - 1, length);
            }
          }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SoundexClearSoundexCode
//
// DESCRIPTION:   Clears the soundex code field.
//
// PARAMETERS:    None.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void SoundexClearSoundexCode (void)
{
   FieldPtr fldP;
   Int16    length;

   fldP = (FieldPtr) GetObjectPtr (SoundexCodeField);
                              
   length = FldGetTextLength (fldP);
    
    if (length > 0) {
        FldSetTextPtr (fldP, "\0");
      FldFreeMemory (fldP);
       }
       
    FldRecalculateField (fldP, true); // don't call FldDrawField, it crashes cobalt
    
   FldDrawField (fldP);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SoundexClearForm
//
// DESCRIPTION:   Clears the fields in the Soundex Form.
//
// PARAMETERS:    None.
//                      
// RETURNED:      Nothing.
//
// REVISIONS:     None.
////////////////////////////////////////////////////////////////////////////////////
static void SoundexClearForm (void)
{
   FieldPtr fldP;
   Int16    length;
   
   fldP = (FieldPtr) GetObjectPtr (SoundexNameField);
   FldSetSelection (fldP,0,0);  // set so this function does not crash Palm III's

   length = FldGetTextLength (fldP);
    if (length > 0)
      FldFreeMemory (fldP);
   FldDrawField (fldP);

   fldP = (FieldPtr) GetObjectPtr (SoundexCodeField);
                              
   length = FldGetTextLength (fldP);
   
    if (length > 0) {
        FldSetTextPtr (fldP, "\0");
      FldFreeMemory (fldP);
      }
   
   FldRecalculateField(fldP, true); // don't call FldDrawField, it crashes cobalt
      FldDrawField (fldP);
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:        ConvertAChar
//
// DESCRIPTION:     This routine converts a character to its soundex code equivalent.
//
// PARAMETERS:      -> aChar  - a character to convert.
//
// RETURNED:        The soundex code for the given character.
//
// REVISIONS:       None.
////////////////////////////////////////////////////////////////////////////////////
static Char ConvertAChar (const Char aChar)
{
    switch (aChar)
        {
         case 'b':
         case 'f':
         case 'p':
         case 'v':
             return '1';

         case 'c':
         case 's':
         case 'g':
         case 'j':
         case 'k':
         case 'q':
         case 'x':
         case 'z':
         case (char) chrSmall_S_Caron:
         case (char) chrSmall_C_Cedilla:
             return '2';

         case 'd':
         case 't':
             return '3';

         case 'l':
             return '4';

         case 'm':
         case 'n':
         case (char) chrSm_N_WithTilda:
             return '5';

         case 'r':
             return '6';

         case 'h': // not coded
         case 'w': // not coded
             return '7';

         default: //  else letters a,e,i,o,u and y are not coded
             return '0';
        }
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:        Soundex
//
// DESCRIPTION:     This routine converts a string representing a surname into
//                  a soundex code.
//
//                  Checks:   Burroughs B620        Ashcraft  A261        Pfister      P236
//                  Tymczak     T522     Jackson      J250        Cowjeset  C223
//                  Cwjeset     C230     Thompson  T512        Tuttle     T340
//                  Thuttle     T340     Euler         E460        Ellery     E460
//                  Heilbronn   H416     Chebyshev C121     Lisajous  L222
//                  Hyhyhy      H000     Ghosh         G200     Phillips  P412
//                  Chase       C200     Cwhseles     C420     Cwhoseles C424
//
// PARAMETERS:      -> astring  - a string containing a person's surname.
//
// RETURNED:        The soundex code as a pointer to a global string (Soundexcode).
//
// REVISIONS:         None.
////////////////////////////////////////////////////////////////////////////////////
Char *Soundex (const Char* aString)
{
    UInt16    sdxPos = 0;
    UInt16    namPos = 0;
    Char        aChar;        // current character from namStr
    Char     cCode;        // current code for current character
    Char     lCode;        // last code
    Char        startChar;  // starting character from namStr
    Char     nameStr[16] = "\0"; // should match field lenght on Soudex Form + 1

    if (aString == NULL || *aString == '\0')
       goto BadData;
    
    StrNCat (nameStr, aString, sizeof (nameStr));
    StrToLower (nameStr, nameStr);
    StrCopy (SoundexCode, "0000"); // init with zeros
    
    // Process the characters in the nameStr
    while ((sdxPos < 4) && (namPos < StrLen (nameStr))) {
    
       aChar = nameStr[namPos];
        
        // ignore blanks
        if (aChar == ' ') {
            namPos++;
            continue;
            }
            
        // End the name on comma's and hyphens.
        if (aChar == chrComma || aChar == chrHyphenMinus)
           break;
    
        cCode = ConvertAChar (aChar);
        
      if (sdxPos > 0) { // skip duplicate & invalid letters
         if (cCode != '0' && cCode != '7' && cCode != lCode) {
            SoundexCode[sdxPos] = cCode;
            sdxPos++;
                  }
         }
      else { // only reach here on first character

           // Convert first character to uppercase.
           if (aChar >= 'a' && aChar <= 'z')
                startChar = aChar - ('a' - 'A');
            else if (aChar >= 'A' && aChar <= 'Z')
                startChar = aChar;
            else // character not in alphabet, so error
                goto BadData;
                
         SoundexCode[sdxPos] = cCode;
         sdxPos++;
         }

        // Update last code, except if current code is 'w' or 'h'
      if (cCode != '7')
          lCode = cCode;
           
        namPos++;   
        }  // while (sdxPos < 4 && namPos < length)

    // Put back letter at position zero.
    if (sdxPos > 0)
        SoundexCode[0] = startChar;

    return SoundexCode;
        
    ///////
    BadData:
    ///////
    
    return ErrStr; // must return exactly 4 characters
}

////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:      SoundexHandleEvent
//
// DESCRIPTION:   This routine is the event handler for the Soundex Form.
//
// PARAMETERS:    -> event    -    a pointer to an EventType structure
//
// RETURNED:      true if the event has handle and should not be passed
//                to a higher level handler.
//
// REVISIONS:        None.
////////////////////////////////////////////////////////////////////////////////////
Boolean SoundexHandleEvent (EventPtr event)
{
   Boolean     handled     = false;
   FormPtr    frm         = FrmGetActiveForm ();
   UInt16    objIndx     = FrmGetObjectIndex (frm, SoundexNameField);
       
   switch (event->eType)
       {
          case ctlSelectEvent:
            switch (event->data.ctlEnter.controlID)
                {
                case SoundexDoneButton:
                  FrmReturnToForm (0);
               handled = true;
               break;
               
            case SoundexGetCodeButton:
               SoundexGetCode ();
               handled = true;
               break;
               
            case SoundexClearButton:
               SoundexClearForm ();
               //SetNavFocusRing (SoundexNameField);
               FrmSetFocus (frm, objIndx);
               handled = true;
               break;
                          
                 case SoundexInfoButton:
               FrmHelp (SoundexHelpString);
               handled = true;
               break;
               
            default:
               break;
                }
         break;
      
      case keyDownEvent:
         if (event->data.keyDown.chr == linefeedChr) {
                SoundexGetCode ();
                handled = true;
                }
           else if (event->data.keyDown.chr == backspaceChr) {
              SoundexClearSoundexCode ();
              // do not set handled to true.
              }
           else  { // adding characters always clears the code.
              SoundexCheckChar (event);
              SoundexClearSoundexCode ();
              handled = true;
              }
         break;

      case fldEnterEvent:
          //SetNavFocusRing (SoundexNameField);
         FrmSetFocus (frm, objIndx); // if user puts pen in code field
         // do not set handled = true!
         break;

      case frmOpenEvent:
         case frmUpdateEvent:
         FrmDrawForm (FrmGetActiveForm ());
    
            SetNavFocusRing (SoundexNameField); // DTR: 12-20-2005
            FrmSetFocus (frm, objIndx);
         
         handled = true;
         break;

      default:
          break;
       }
      
   return (handled);
}
