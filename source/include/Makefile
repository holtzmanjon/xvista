#    ********* DO NOT EDIT THIS FILE *********
#  It is automatically generated and your changes will not be used! 
MACHINE=macosx
CPP=/usr/bin/cpp
INCCPP=/usr/bin/cpp
FPP=for
CPPFLAGS=-DX11_exists -I/usr/X11/include -traditional -P
CC=cc
CFLAGS=-O -DX11_exists -I/usr/X11/include -DFORT -DUNIX -I/usr/include/bsd -I/sw/include
FC=gfortran
FFLAGS=-O -ffixed-line-length-132 
LD=gfortran
LDFLAGS=-L/sw/lib
RM=rm
AR=ar
DX11=-DX11_exists -I/usr/X11/include
X11DEV=x11_.o x11.o xgets.o trackbox.o
LX11DEV=$(MLIB)(x11_.o) $(MLIB)(x11.o) $(MLIB)(xgets.o) $(MLIB)(trackbox.o)
X11LIB=-L/usr/X11/lib64 -L/usr/X11/lib -lX11
SUNIEEE=
OPTFLAG=
BSTATIC=-lgcc
DOOBJS=$(LOBJS)
MAR=
MRM=$(RM) *.o
LREADLINE=
WCSLIB=
VMSTREE=/usr/tmp/vista
INCS=display.pgm image.pgm misc.pgm simulate.pgm spectra.pgm \
starphot.pgm surfphot.pgm tape.pgm unsupported.pgm \
aed.inc alias.inc aperinc.inc array.inc customize.inc error.inc \
express.inc help.inc imagelink.inc mask.inc openfiles.inc output.inc \
photfiles.inc procedure.inc profile.inc psflib.inc shortcom.inc \
string.inc symbol.inc tapestat.inc timedata.inc tree.inc tv.inc \
vistalink.inc wavecal.inc work.inc mult.inc polyvert.inc
VMSCODE=display.Pgm image.Pgm misc.Pgm simulate.Pgm spectra.Pgm \
starphot.Pgm surfphot.Pgm tape.Pgm unsupported.Pgm \
aed.Inc alias.Inc aperinc.Inc array.Inc customize.Inc error.Inc \
express.Inc help.Inc imagelink.Inc mask.Inc openfiles.Inc output.Inc \
photfiles.Inc procedure.Inc profile.Inc psflib.Inc shortcom.Inc \
string.Inc symbol.Inc tapestat.Inc timedata.Inc tree.Inc tv.Inc \
vistalink.Inc wavecal.Inc work.Inc mult.Inc polyvert.Inc
#
.SUFFIXES:
.SUFFIXES: .out .o .c .e .r .F .f .y .l .s .p .F .PGM .inc .pgm
#       Teach make how to handle Fortran programs with C-preprocessor lines
.F.inc: ; $(INCCPP) $(CPPFLAGS) $*.F  $*.inc
.F.pgm: ; $(INCCPP) $(CPPFLAGS) $*.F  $*.pgm

.SUFFIXES: .vmsf .for .inc .vmsc .com .F .Inc .PGM .Pgm
.F.Inc: ; $(INCCPP) $(VMSFLAGS) $*.INC $*.Inc
.PGM.Pgm: ; $(INCCPP) $(VMSFLAGS) $*.PGM $*.Pgm
#
datefile::Vista.h $(INCS)
	date > datefile
#       Don't let make delete the object library!
clean: tidy
	-rm -f *.inc *.pgm *.bak
tidy:
	-rm -f Vista.h datefile *.inc *.pgm
Vista.h:
	ln -s ../Vista.$(MACHINE) Vista.h
#       THE FOLLOWING STUFF MAKES CODE FOR THE VAXen
avmscode: Vista.h.avms $(VMSCODE)
	-mkdir $(VMSTREE)/source/include
	mv *.Inc $(VMSTREE)/source/include
	mv *.Pgm $(VMSTREE)/source/include
	cp *.h $(VMSTREE)/source/include
	-rm Vista.h avmscode
	date >  avmscode
Vista.h.avms:
	ln -s ../Vista.avms Vista.h
uvmscode: Vista.h.uvms $(VMSCODE)
	-mkdir $(VMSTREE)/source/include
	mv *.Inc $(VMSTREE)/source/include
	mv *.Pgm $(VMSTREE)/source/include
	cp *.h $(VMSTREE)/source/include
	-rm Vista.h uvmscode
	date >  uvmscode
Vista.h.uvms:
	ln -s ../Vista.uvms Vista.h
pvmscode: Vista.h.pvms $(VMSCODE)
	-mkdir $(VMSTREE)/source/include
	mv *.Inc $(VMSTREE)/source/include
	mv *.Pgm $(VMSTREE)/source/include
	cp *.h $(VMSTREE)/source/include
	-rm Vista.h pvmscode
	date >  pvmscode
Vista.h.pvms:
	ln -s ../Vista.pvms Vista.h
