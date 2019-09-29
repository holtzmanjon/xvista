C       This file defines the common block used to save the wavelength
C       calibration line identifications.  This file is "INCLUDEd" in
C       files LINEFIND.FOR, LINEFIT.FOR, PRINTOUT.FOR, and EWAVE.FOR.

	INTEGER         NSAVES
	PARAMETER       (NSAVES = 1000)
	DOUBLE PRECISION SAVECENTER(NSAVES),SAVEWAVE(NSAVES)
	DOUBLE PRECISION SAVEWEIGHT(NSAVES)
	INTEGER*4       SAVEN(NSAVES), SAVEM(NSAVES)
        REAL*4 		SAVEDSP
        INTEGER		NUMSAVED,NCOLUMNS
	COMMON/SAVELAMBDA/ NUMSAVED,NCOLUMNS,SAVECENTER,SAVEWAVE,
     .  SAVEWEIGHT,SAVEM,SAVEN,SAVEDSP

