	SUBROUTINE VMSALLOC(NBYTES,LOCATION)

C	Allocates NBYTES bytes of data, returning its address in LOCATION.

	INCLUDE 'VINCLUDE:VISTALINK.INC'	! Error communication.
	
	EXTERNAL SS$_NORMAL
	INTEGER  STATUS

	IF (NBYTES .LE. 0.0) GOTO 999

	STATUS = LIB$GET_VM(NBYTES,LOCATION)
	IF (STATUS .NE. %LOC(SS$_NORMAL)) GOTO 999
D	TYPE *,'NEW IMAGE ADDRESS=',LOCATION
D	CALL LIB$SHOW_VM(,,)
	RETURN

 999	TYPE *,'Cannot get virtual memory.'
	CALL SYSERRPRINT(STATUS,'ALLOCATION ERROR:')
	XERR = .TRUE.
	RETURN

	END

C	-----------------------------------------------------------------------

	SUBROUTINE VMSFREE(NBYTES,LOCATION)

C	Releases memory obtained by ALLOC.

	INCLUDE 'VINCLUDE:VISTALINK.INC'

	EXTERNAL SS$_NORMAL
	INTEGER  STATUS

	STATUS = LIB$FREE_VM(NBYTES,LOCATION)
	IF (STATUS .NE. %LOC(SS$_NORMAL)) GOTO 999
D	TYPE *,'AFTER FRREING MEMORY:'
D	CALL LIB$SHOW_VM(,,)
	RETURN

 999	TYPE *,'Error in releasing virtual memory.'
	CALL SYSERRPRINT(STATUS,'ALLOCATION ERROR:')
	XERR = .TRUE.
	RETURN

	END

C	-----------------------------------------------------------------------

	SUBROUTINE STRING_ALLOC(NBYTES,DESCRIPTOR)

C	Allocates NBYTES bytes of storage for a string, whose descriptor
C	is passed in DESCRIPTOR.

	BYTE DESCRIPTOR(8)
D	BYTE ADDRBYTES(4)
D	INTEGER*4 ADDRESS
D	EQUIVALENCE (ADDRESS,ADDRBYTES(1))

	INCLUDE 'VINCLUDE:VISTALINK.INC'	! Communication with VISTA

	INTEGER  STATUS
	EXTERNAL SS$_NORMAL

	IF (NBYTES .LE. 0) GOTO 999

	DESCRIPTOR(3) = 14			! Code for 'TEXT' datatype
	DESCRIPTOR(4) = 2			! Dynamic allocation class

	STATUS = LIB$SGET1_DD(NBYTES,DESCRIPTOR)
	IF (STATUS .NE. %LOC(SS$_NORMAL)) GOTO 999
D	DO I=1,4
D		ADDRBYTES(I) = DESCRIPTOR(I+4)
D	END DO
D	TYPE *,'NEW STRING ADDRESS =',ADDRESS
	RETURN

 999	TYPE *,'Cannot get <string> virtual memory.'
	CALL SYSERRPRINT(STATUS,'ALLOCATION ERROR:')
	RETURN
	END

C	-----------------------------------------------------------------------

	SUBROUTINE STRING_FREE(DESCRIPTOR)

C	Releases a dynamic virtual string.

	BYTE DESCRIPTOR(8)

	INCLUDE 'VINCLUDE:VISTALINK.INC'	! Communication

	INTEGER  STATUS
	EXTERNAL SS$_NORMAL

	IF (DESCRIPTOR(4) .NE. 2) THEN
		TYPE *,'String was not dynamically allocated.'
		XERR = .TRUE.
		RETURN
	END IF

	STATUS = LIB$SFREE1_DD(DESCRIPTOR)
	IF (STATUS .NE. %LOC(SS$_NORMAL)) THEN
		TYPE *,'Cannot release <string> virtual memory.'
		CALL SYSERRPRINT(STATUS,'ALLOCATION ERROR:')
		XERR = .TRUE.
	END IF

	RETURN
	END
