


C       Here we define the common block for storing aliases.

	INTEGER         NUMALIAS
	COMMON  /VISTAALIASNUMBER/      NUMALIAS

	INTEGER         MAXNUMALIAS
	PARAMETER       (MAXNUMALIAS = 50)

	CHARACTER*80    VISTASTRINGS(MAXNUMALIAS)
	CHARACTER*80    ALIASES(MAXNUMALIAS)
	COMMON  /VISTAALIASSTRING/      VISTASTRINGS, ALIASES
