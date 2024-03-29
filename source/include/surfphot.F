        ELSE IF (COM .EQ. 'PRADIAL' .OR. COM .EQ. 'CUT') THEN
C          Radial profiles.
             NEEDIM = 1
             IMSP   = 1
             IF (GO) THEN
                 IMSP = 0
                 CALL CCPRADIAL(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)
             END IF

        ELSE IF (COM .EQ. 'TEMPLATE') THEN
C          Reconstruct profile
             NEEDIM = 2
             IF (GO) CALL CCTEMPLATE(LOCIM,IMSR,IMER,
     .                     IMSC,IMEC,LOCB, JMSR,JMER,JMSC,JMEC)

        ELSE IF (COM .EQ. 'RECON') THEN
C          Reconstruct profile.
            NEEDIM = 1
            IF (GO) CALL CCRECON(LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'APER') THEN
C          Aperture photometry
            NEEDIM = 1
            IF (GO) CALL CCAPERTURE(LOCIM,NROW,NCOL)

        ELSE IF (COM .EQ. 'PROFILE') THEN
C          Radial profile
            NEEDIM = 1
            IMSP   = 1
            IF (GO) THEN
                    IMSP = 0
                    CALL CCPROFILE(LOCIM,NROW,NCOL)
            END IF

        ELSE IF (COM .EQ. 'ANNULUS') THEN
C          Azimuthal Average
            NEEDIM = 1
            IMSP   = 1
            IF (GO) THEN
                IMSP = 0
                CALL CCANNULUS(LOCIM,IMSR,IMER,IMSC,IMEC)
            END IF

        ELSE IF (COM .EQ. 'AEDIT') THEN
             NEEDIM = 1
             IF (GO) CALL CCAEDIT(LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'DOEDIT') THEN
             GO = .TRUE.
             CALL DOEDIT

        ELSE IF (COM .EQ. 'GPROF') THEN
             PRINT *, ' Use the command PROFILE instead '
             PRINT *, ' GPROF results are reproduced with the GPROF ',
     &                'keyword '
C             NEEDIM = 1
C             IF (GO) THEN
C                  CALL CCPROFILE(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)
C             END IF

        ELSE IF (COM .EQ. 'EMAG') THEN
             NEEDIM = 1
             IF (GO) CALL CCEMAG(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)

        ELSE IF (COM .EQ. 'GPSAVE') THEN
              PRINT *, 'Use the SAVE command to store PROFILE results'
              PRINT *, 'Use the PRINT PROF command to generate output'
              PRINT *, '  to look at. PRINT PROF redirected to a file'
              PRINT *, '  will reproduce the old GPSAVE command'
C             GO = .TRUE.
C             CALL GPSAVE

        ELSE IF (COM .EQ. 'GPGET') THEN
              PRINT *, 'Use GET and SAVE PROF= to get and save PROFILE',
     &                ' calculations'
C             GO = .TRUE.
C             CALL GPGET

        ELSE IF (COM .EQ. 'OPREP') THEN
             GO = .TRUE.
             CALL OPREP
         
        ELSE IF (COM .EQ. 'CLPREP') THEN
             GO = .TRUE.
             CALL OPREP

        ELSE IF (COM .EQ. 'RPREP') THEN
             GO = .TRUE.
             CALL RPREP

        ELSE IF (COM .EQ. 'WPREP') THEN
             GO = .TRUE.
             CALL WPREP

        ELSE IF (COM .EQ. 'RMARK') THEN
             GO = .TRUE.
             CALL RMARK

        ELSE IF (COM .EQ. 'EMARK') THEN
             GO = .TRUE.
             CALL EMARK

        ELSE IF (COM .EQ. 'TVPROF') THEN
             GO = .TRUE.
             CALL TVPROF

        ELSE IF (COM .EQ. 'CPROF') THEN
             GO = .TRUE.
             CALL CPROF

        ELSE IF (COM .EQ. 'CLPROF') THEN
             GO = .TRUE.
             CALL CLPROF

	ELSE IF (COM .EQ. 'SNUC') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCSIMULNUC(LOCIM,IMSR,IMER,IMSC,IMEC)

	ELSE IF (COM .EQ. 'HISTNUC') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCHISTNUC(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)

	ELSE IF (COM .EQ. 'RENUC') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCRENUC(LOCIM,IMSR,IMER,IMSC,IMEC)

	ELSE IF (COM .EQ. 'EGAL') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCEGAL(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)
	
	ELSE IF (COM .EQ. 'ELLMAG') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCELLMAG(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)
	
	ELSE IF (COM .EQ. 'SURFBRIGHT') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCSURFBRIGHT(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)

        ELSE IF (COM .EQ. 'AUTOCEN') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCAUTOCEN(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)

	ELSE IF (COM .EQ. 'SECTOR') THEN
	     NEEDIM = 1
	     IF (GO) CALL CCSECTOR(LOCIM,IMER-IMSR+1,IMEC-IMSC+1)

        ELSE IF (COM .EQ. 'BDFIT2D' .OR. COM .EQ. 'BDFIT') THEN
             NEEDIM = 1
             IF (GO) CALL CCBDFIT(LOCIM,IMSR,IMER,IMSC,IMEC)

        ELSE IF (COM .EQ. 'XPROF') THEN
             GO = .TRUE.
             CALL XPROF

        ELSE IF (COM .EQ. 'PETROMAG') THEN
             GO = .TRUE.
             CALL PETROMAG
