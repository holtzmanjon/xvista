#include "Vista.h"
C       PRINT ERROR MESSAGES

C       Modified by:    April Atwood    6/30/86

        SUBROUTINE ERRPRINT(ERCODE)
        INTEGER ERCODE
#ifdef VMS
        INCLUDE 'VINCLUDE:ERROR.INC'
#else
        INCLUDE 'vistadisk/source/include/error.inc'
#endif

      PARAMETER (MAXER = 28)
      CHARACTER*40 ERRTEXT(MAXER)
      DATA ERRTEXT/
     .'EXPRESSION TOO COMPLEX','ILLEGAL USE OF PARENTHESIS',
     .'BAD USE OF UNARY OPERATOR','BAD USE OF BINARY OPERATOR',
     .'BADLY FORMED NUMBER:','SYMBOL HAS NO DEFINED VALUE',
     .'UNDEFINED SYMBOL USED:','NO MORE ROOM IN SYMBOL TABLE',
     .'BAD FORMAT','BAD FLOATING POINT VALUE',
     .'ILLEGAL USE OF A SYMBOL','UNBALANCED FUNCTION SQUARE BRACKETS',
     .' ','INCORRECT USE OF EQUATE OPERATOR',
     .'NON-ALPHANUMERIC CHARACTER USED:','UNDEFINED FUNCTION CALLED:',
     .'WRONG NUMBER OF PARAMETERS','WRONG TYPE OF PARAMETER',
     .'BAD FILE INPUT:','UNDEFINED/MISSING LOGICAL FILE NAME:',
     .'NO SUCH COLUMN OR NON-NUMERIC DATA:','ERROR READING FILE:',
     .'END OF FILE WHILE READING:','CHARACTER SYMBOL NOT ALLOWED:',
     .'TOO MANY EXPRESSIONS ','WRONG NUMBER OF ARRAY DIMENSIONS',
     .'INDEX OUTSIDE ARRAY LIMITS','ATTEMPT TO REDECLARE SYMBOL TYPE:'/

        IF(ERCODE .EQ. SUCCESS) RETURN

        IF(ERCODE .GT. MAXER) THEN
                WRITE(*,*) 'UNKNOWN ERROR CODE: ',ERCODE
                RETURN
        END IF

        NC = NUMCHAR(ERRTEXT(ERCODE))
        IF (NC .EQ. 0) THEN
                KK = 0
        ELSE
                KK = 1
        END IF

        IF(ERRTEXT(ERCODE)(NC:NC) .EQ. ':') THEN
                MC = NUMCHAR(MERROR)
                IF (MC .EQ. 0) THEN
                        WRITE(*,*) ERRTEXT(ERCODE)(KK:NC)
                ELSE
                        WRITE(*,*) ERRTEXT(ERCODE)(KK:NC),' ',
     &                        MERROR(1:MC)
                END IF
        ELSE
                WRITE(*,*) ERRTEXT(ERCODE)(KK:NC)
        END IF
        RETURN
        END
