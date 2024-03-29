#include "Vista.h"
C       Stellar photometry commands:

        ELSE IF (COM .EQ. 'FIND') THEN
C        Find stars
             IF (.NOT. GO) CALL COPY
             NEEDIM = 2
             IF (GO) THEN
               CALL DAOLOAD(LOCB,JMSR,JMER,JMSC,JMEC,
     &              LOCIM,IMSR,IMER,IMSC,IMEC)
               CALL FIND
             END IF

        ELSE IF (COM .EQ. 'MULTISTAR') THEN
C        Profile fitting
             GO = .TRUE.
             CALL MULTISTAR

        ELSE IF (COM .EQ. 'VSUB*' .OR. COM .EQ. 'SUB*' .OR.
     &           COM .EQ. 'VSUBSTAR' .OR. COM .EQ. 'SUBSTAR') THEN
C        Subtract profile fits
             NEEDIM = 1
             IF (GO) CALL CCSUBSTARV(LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'PHOTOMETRY') THEN
C        Aperture photometry
             NEEDIM = 1
             IF (GO) THEN
               CALL DAOLOAD(LOCIM,IMSR,IMER,IMSC,IMEC,
     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
               CALL FOTOMETRY
             END IF

        ELSE IF (COM .EQ. 'SUBPHOT') THEN
C        Aperture photometry on PSF-subtracted frame
             NEEDIM = 2
             IF (GO) CALL CCSUBPHOT(LOCB,JMSR,JMER,JMSC,JMEC,
     &              LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'DAOSKY') THEN
C        Sky determination
             NEEDIM = 1
             IF (GO) THEN
               CALL DAOLOAD(LOCIM,IMSR,IMER,IMSC,IMEC,
     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
               CALL DAOSKY
             END IF

        ELSE IF (COM .EQ. 'PSF') THEN
C        PSF determination
             NEEDIM = 1
             IF (GO) THEN
               CALL DAOLOAD(LOCIM,IMSR,IMER,IMSC,IMEC,
     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
               CALL GETPSF
             END IF

        ELSE IF (COM .EQ. 'ADD*' .OR. 
     &           COM .EQ. 'ADDSTAR') THEN
C        Add fake stars
             NEEDIM = 1
             IF (GO) THEN
               CALL DAOLOAD(LOCIM,IMSR,IMER,IMSC,IMEC,
     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
               CALL ADDSTAR
             END IF

        ELSE IF (COM .EQ. 'DAOLIB') THEN
C        Create PSF library
             GO = .TRUE.
             CALL ADDSTAR

        ELSE IF (COM .EQ. 'OPTIONS' .OR. COM .EQ. 'MONITOR' .OR.
     &           COM .EQ. 'NOMONITOR') THEN
C        Change DAOPHOT ptions
             GO = .TRUE.
             CALL GETOPT(WORK)

        ELSE IF (COM .EQ. 'SORT') THEN
C        Sort DAOPHOT files
             GO = .TRUE.
             CALL SORT

        ELSE IF (COM .EQ. 'NMAG') THEN
             GO = .TRUE.
             CALL NMAG

        ELSE IF (COM .EQ. 'SELECT') THEN
             GO = .TRUE.
             CALL SELECT

        ELSE IF (COM .EQ. 'APPEND') THEN
             GO = .TRUE.
             CALL APPEND

        ELSE IF (COM .EQ. 'OFFSET') THEN
             GO = .TRUE.
             CALL OFFSET

        ELSE IF (COM .EQ. 'OLDGROUP' .OR. COM .EQ. 'GROUP') THEN
             GO = .TRUE.
             CALL GROUP

        ELSE IF (COM .EQ. 'AUTOMARK') THEN
C           Automatic marking
                NEEDIM = 1
                IF (GO) CALL CCAUTOMARK(LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'APERSTAR' .OR.
     &           COM .EQ. 'APER*') THEN
C           Aperture photometry
                NEEDIM = 1
                IF (GO) CALL CCAPERSTAR(LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'MODPHOT') THEN
C           Modify entries
                IF (GO) CALL MODPHOT

        ELSE IF (COM .EQ. 'COORDS') THEN
C           Compute coordinates
                IF (GO) CALL COORDS

C       Here are the DAOPHOT commands.  If this file is included in the
C       VISTA.FOR, these commands can be run.

C        ELSE IF (COM .EQ. 'DAOPHOT') THEN
C                GO = .TRUE.
C                CALL DAOPHOT('  ',LOCDUM,IDUM,IDUM,IDUM,IDUM,
C     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
C
C        ELSE IF (COM .EQ. 'PEAK') THEN
C             NEEDIM = 1
C             IF (GO) CALL DAOPHOT('PE',LOCIM,IMSR,IMER,IMSC,IMEC,
C     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
C
C        ELSE IF (COM .EQ. 'NSTAR') THEN
C             NEEDIM = 1
C             IF (GO) CALL DAOPHOT('NS',LOCIM,IMSR,IMER,IMSC,IMEC,
C     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
C
C        ELSE IF (COM .EQ. 'SUB*' .OR.
C     &           COM .EQ. 'SUBSTAR') THEN
C             IF (.NOT. GO) CALL COPY
C             NEEDIM = 2
C             IF (GO) CALL DAOPHOT('SU',LOCB,JMSR,JMER,JMSC,JMEC,
C     &              LOCIM,IMSR,IMER,IMSC,IMEC)

C        ELSE IF (COM .EQ. 'DUMP') THEN
C             NEEDIM = 1
C             IF (GO) CALL DAOPHOT('DU',LOCIM,IMSR,IMER,IMSC,IMEC,
C     &              LOCDUM,IDUM,IDUM,IDUM,IDUM)
C
        ELSE IF (COM .EQ. 'DAOFILES') THEN
             GO = .TRUE.
             CALL DAOFILES

        ELSE IF (COM .EQ. 'REGISTER') THEN
             GO = .TRUE.
             CALL REGISTER

        ELSE IF (COM .EQ. 'SHORTAP') THEN
             GO = .TRUE.
             CALL SHORTAP

        ELSE IF (COM .EQ. 'PLOTAP' .OR. COM .EQ. 'APCOR') THEN
             GO = .TRUE.
             CALL PLOTAP

        ELSE IF (COM .EQ. 'CORRECT') THEN
             GO = .TRUE.
             CALL CORRECT

	ELSE IF (COM .EQ. 'FITSTAR' .OR.
     &           COM .EQ. 'FIT*') THEN
	     GO = .TRUE.
	     CALL FITSTARS

C	ELSE IF (COM .EQ. 'DOUB') THEN
C	     GO = .TRUE.
C	     CALL DOUB

	ELSE IF (COM .EQ. 'PSFFIT') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCPSFFIT(LOCIM,IMSR,IMER,IMSC,IMEC)

	ELSE IF (COM .EQ. 'PLOTFOCUS') THEN
	     GO = .TRUE.
	     CALL PLOTFOCUS

        ELSE IF (COM .EQ. 'PSFINTERP') THEN
             GO = .TRUE.
             CALL PSFINTERP

        ELSE IF (COM .EQ. 'COMBINE') THEN
             GO = .TRUE.
             CALL COMBINE

        ELSE IF (COM .EQ. 'MAGAVER') THEN
             GO = .TRUE.
             CALL MAGAVER

	ELSE IF (COM .EQ. 'STARPLOT') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCSTARPLOT(LOCIM,IMSR,IMER,IMSC,IMEC)

	ELSE IF (COM .EQ. 'TVSTAR') THEN
	     NEEDIM = -1
	     IF (GO) CALL CCSTARPLOT(LOCTV,IMTVSR,IMTVER,IMTVSC,IMTVEC)

	ELSE IF (COM .EQ. 'MERGEMAG') THEN
	     GO = .TRUE.
	     CALL MERGEMAG

	ELSE IF (COM .EQ. 'APTRN') THEN
	     GO = .TRUE.
	     CALL APTRN

	ELSE IF (COM .EQ. 'WEED') THEN
	     GO = .TRUE.
	     CALL WEED

	ELSE IF (COM .EQ. 'CMD') THEN
	     GO = .TRUE.
	     CALL CMD

	ELSE IF (COM .EQ. 'MAGAVERAGE') THEN
	     GO = .TRUE.
	     CALL MAGAVERAGE

        ELSE IF (COM .EQ. 'CMDERR') THEN
             NEEDIM = 2
             IF (GO) CALL CCCMDERR(LOCIM,IMSR,IMER,IMSC,IMEC,
     &                              LOCB,JMSR,JMER,JMSC,JMEC)



