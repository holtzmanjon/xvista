
			 -----------------------------
				Lick Mongo 1989
			 Hershey Font Utility Programs
			 -----------------------------


R. Pogge (McDonald Observatory)
1989 August 20


Overview:
--------

   Lick Mongo (unlike the original Tonry MONGO) makes uses of the extensive
collection of Hershey fonts.  These fonts are generated by stroking out the
characters using a set of points stored in binary files readable to Lick
Mongo.  It is possible to build a font file using the programs in this
directory.

   The main program is MKMBFONT (MaKe Mongo Binary FONTs), which constructs
the binary font files from a set of "fontmap" files created by the user. There
are two other programs, ALPHA and PSFONTS, which are used to inspect the full
set of fonts in a given binary font file directly. ALPHA is used to display
them on the screen, and PSFONTS is used in conjunction with a PostScript laser
printer to show all of the on-board (firmware) fonts available to Lick Mongo
for plotting.  It is also useful for making font tables for general use by the
users.


WARNING:
-------

   This utility in MONGO is pretty poorly developed, and so you are entering
this directory at your own risk.  However, should you go and blow away the
font files and need to reconstruct them, you've come to the right place.  If
you truly want to "roll your own", then poke about and have fun.


How to use MKMBFONT:
-------------------


  Simply compile and link MKMBFONT.

      $ fortran mkmbfont
      $ link mkmbfont

  Then, run MKMBFONT.  You will first be asked for the name of the FONTMAP
file (see below), then it will spew onto your screen as it builds each font.

  When it is done (it can take a while), you will find a file called FONTS.NEW
in this directory.  Rename this to whatever is appropriate (for example:  if
rebuilding FONTS.SIM, you'd use FONTMAP.SIM and then rename FONTS.NEW to
FONTS.SIM).  Reassign FONTNEW to point at the new binary font file, and you're
all set.  If you want EVERYONE to use the new font file, the you'll have to
change the logical assignment to FONTNEW in both MONGO.LOG and MONGO.COM
(provided you followed the installation instructions).


Inspecting Font Files:
---------------------

   If you are unsure of the contents of a binary font file, use the program
ALPHA to inspect it on your terminal screen.  If you need to look at the
internal PostScript machine fonts, you would use the PSFONTS program discussed
below.

   Compile and link ALPHA.FOR as follows:

      $ fortran/extend alpha.for
      $ link alpha,[-]mongo/lib

   Delete, the file ALPHA.OBJ, and run the ALPHA.EXE file.  Answer the
questions.  To point at a particular font file, you must first reset the
FONTNEW logical name to point at it before executing ALPHA.


Inspecting PostScript Fonts:
---------------------------

  All PostScript devices come with a set of firmware fonts (sometimes called
"machine" or "internal" fonts) which are contained internally and produce the
highest quality characters at the maximum possible speed.  To make a set of
font tables showing the available fonts, use the PSFONTS program in this
directory.  You'll need to compile and link the program as for ALPHA.FOR
above.


Files:
-----

   HERSHEY.DAT    Contains the first (and most useful) of the 4 files of
		  Hershey fonts.  It's in a big and ugly format.  For a
		  complete listing of the contents, ask Steve Allen
		  (sla@helios.ucsc.edu) real nice.  We work from an old
		  printout provided by the folks that gave us the font file.


Pre-Prepared Font Maps:
----------------------
	Several of these exist purely for the fun of exploring the
	Hershey fonts.

   FONTMAP.SIM    Basic Lick Mongo Simplex Fonts
   FONTMAP.VIS    variant on FONTMAP.SIM
   FONTMAP.BIN    Old Tonry MONGO fonts.
   FONTMAP.CYR    Cyrillic font (for gettin' into Glasnost...)
   FONTMAP.ITA    Italian Gothic Font
   FONTMAP.MAP    Map Making Symbols
   FONTMAP.NEW    Random experimental font map.


Pre-Prepared Binary Font Files:
------------------------------
	These will only be supplied if you have obtained Lick Mongo
	directly from another VMS machine which has them.

   FONTS.SIM      Basic Lick Mongo Simplex fonts
   FONTS.BIN      Old Tonry MONGO fonts
   FONTS.CYR      Cyrillic Font (you're really doing a talk in Bulgaria...?)
   FONTS.ITA      Italian Gothic Font



