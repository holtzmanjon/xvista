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
X11LIB=-L/usr/X11/lib -L/usr/X11/lib -lX11
SUNIEEE=
OPTFLAG=
BSTATIC=-lgcc
DOOBJS=$(LOBJS)
MAR=
MRM=$(RM) *.o
LREADLINE=
WCSLIB=
#   This is the main Vista makefile. You should not need to change
#     anything here, with the possible exception of changing the INSTALLDIR
#     which is where the executables get installed with make install.
#     We recommend that instead, you set up symbolic links to the
#     executables in the xvista/bin subdirectory
#
#     These following variables may need changing for your site, but
#     really are relevant only for distribution.
#
COMPTAR=xvista7-12.tar
VMSTREE=/home/avalon10/tmp/vista/vms
KATADIR=/home/avalon10/tmp/vista
MONGODIR=/home/avalon2/mongo/lickmongo
TAPE=/dev/rmt0
INSTALLDIR=/usr/local/bin
BINDIR=../bin
#
#       Teach make how to make things it may not know on some systems.
.SUFFIXES:
.SUFFIXES: .out .o .c .e .r .F .f .y .l .s .p .for .COM .com
.F.o:
	$(CPP) $(CPPFLAGS) $*.F $*.$(FPP)
	$(FC) -c $(FFLAGS) $*.$(FPP)
	$(RM) -f $*.$(FPP)
.F.a:
	$(CPP) $(CPPFLAGS) $< $*.$(FPP)
	$(FC) -c $(FFLAGS) $*.$(FPP)
	$(AR) rv $@ $*.o
	$(RM) -f $*.$(FPP) 
	rm -f $*.o
#       Teach make how to make files for export to a VMS system
.F.for: ; $(CPP) $(VMSFLAGS) $*.F $*.for
.COM.com: ; $(CPP) $(VMSFLAGS) $*.COM $*.com
#
#
#       The Vista Library Directory (for ease in making private versions)
VLD=vistadisk/lib
#
#       The bulk of the libraries needed by Vista (order is irrelevant)
#
VLIB=$(VLD)/base.$(MACHINE).a $(VLD)/image.$(MACHINE).a \
$(VLD)/misc.$(MACHINE).a $(VLD)/parser.$(MACHINE).a \
$(VLD)/simulate.$(MACHINE).a $(VLD)/spectra.$(MACHINE).a \
$(VLD)/surfphot.$(MACHINE).a $(VLD)/tape.$(MACHINE).a \
$(VLD)/starphot.$(MACHINE).a
MLIB=$(VLD)/main.$(MACHINE).a
OBJS=vista.o execute.o
LOBJS=$(MLIB)(vista.o) $(MLIB)(execute.o)
# Don't allow user to run make from source directory anymore, to insure
#  proper machine dependencies are set with the configure script
default:
	echo "You must run make from the xvista subdirectory (not source):"
#
# The next set of lines covers making of xvista, for an X11 version
#
# For system version
xvista:restart Config.h Vista.h mongoinc vistalib nosupport xlib \
	dosystem.$(MACHINE) xvista.$(MACHINE) restart help
	date > datefile
xvista.$(MACHINE): $(MLIB) \
	$(VLD)/user.$(MACHINE).a $(VLIB) $(VLD)/display.$(MACHINE).a \
        $(VLD)/xtv.$(MACHINE).a \
	$(VLD)/utility.$(MACHINE).a $(VLD)/libmgo.$(MACHINE).a
	$(AR) xv $(VLD)/main.$(MACHINE).a vista.o execute.o
	$(AR) xv $(VLD)/base.$(MACHINE).a cmemory.o
	$(LD) -o $(BINDIR)/xvista.$(MACHINE) ${LDFLAGS} vista.o cmemory.o \
	execute.o $(VLD)/user.$(MACHINE).a \
	$(VLIB)  $(VLD)/display.$(MACHINE).a $(VLD)/xtv.$(MACHINE).a \
	$(VLD)/utility.$(MACHINE).a \
	$(VLD)/libmgo.$(MACHINE).a \
	$(WCSLIB) $(IRAFLIB) $(SUNWINDOW) $(X11LIB) $(LREADLINE) $(BSTATIC)
	-rm vista.o cmemory.o execute.o
#
# For personal version
myxvista:restart Config.h Vista.h mongoinc mylib myxvista.$(MACHINE) restart
	date > datefile
myxvista.$(MACHINE):vista.o $(VLD)/user.$(MACHINE).a
	$(AR) xv $(VLD)/main.$(MACHINE).a execute.o
	$(AR) xv $(VLD)/base.$(MACHINE).a cmemory.o
	$(LD) -o $(BINDIR)/myxvista.$(MACHINE) ${LDFLAGS} \
	vista.o cmemory.o execute.o \
	$(VLD)/user.$(MACHINE).a \
	$(VLIB) $(VLD)/display.$(MACHINE).a $(VLD)/xtv.$(MACHINE).a \
	$(VLD)/utility.$(MACHINE).a \
	$(VLD)/libmgo.$(MACHINE).a \
	$(WCSLIB) $(IRAFLIB) $(SUNWINDOW) $(X11LIB) $(LREADLINE) $(BSTATIC)
	-rm cmemory.o execute.o xvista
	-ln myxvista.$(MACHINE) xvista
#
# Here follows a bunch of targets used by the above lines
#
dosystem.$(MACHINE): dosystem.c
	cc dosystem.c -o $(BINDIR)/dosystem.$(MACHINE)
$(MLIB): $(DOOBJS)
	$(MAR)
	$(MRM)
	-ranlib $(MLIB)
mongoinc:
	ln -s mongosrc/MONGOLUN.F MONGOLUN.F
	ln -s mongosrc/MONGOPAR.F MONGOPAR.F
	ln -s mongosrc/MONGODEV.F MONGODEV.F
	$(INCCPP) $(CPPFLAGS) MONGOLUN.F MONGOLUN.inc
	$(INCCPP) $(CPPFLAGS) MONGOPAR.F MONGOPAR.inc
	$(INCCPP) $(CPPFLAGS) MONGODEV.F MONGODEV.inc
#       Vista.h need remaking if it is older than the master for this machine
Vista.h: 
	ln -s Vista.$(MACHINE) Vista.h
#       Config.h for Lick Mongo needs remaking if older than machine master
Config.h:
	-rm -f lickmongo/source/Config.$(MACHINE)
	-ln -s ../../Vista.$(MACHINE) lickmongo/source/Config.$(MACHINE)
	ln -s mongosrc/Config.$(MACHINE) Config.h
#       These are the generic libraries for any version of Lick Vista
vistalib:
	-mkdir ../bin
	-mkdir ../lib
	(cd include      ; make -k)
	(cd base         ; make -k)
	(cd image        ; make -k)
	(cd misc         ; make -k)
	(cd parser       ; make -k)
	(cd simulate     ; make -k)
	(cd spectra      ; make -k)
	(cd starphot     ; make -k)
	(cd surfphot     ; make -k)
	(cd tape         ; make -k)
#	(cd tape/unix    ; make -k)
	(cd sysdep/unix  ; make -k)
	(cd user         ; make -k)
	(cd utility      ; make -k)
#	(cd ../color     ; make -k)
	(cd mongosrc     ; make -k mongolib)
help:
	(cd ../help      ; make -k)
#       This is the library for the video display using X11
xlib:
	(cd display      ; make -k)
	(cd display/xtv  ; make -k)
#	For the user linking his own version
mylib:
	-ln -s vistadisk/source/Vista.$(MACHINE) Vista.$(MACHINE)
	(cd user         ; make -k)
#
install:
	-rm $(INSTALLDIR)/xvista
	-cp $(BINDIR)/xvista.$(MACHINE) $(INSTALLDIR)/xvista
restart:
	-rm -f datefile Vista.h Config.h MONGO*.F MONGO*.inc
tidy: restart
	-rm -f Makefile */Makefile */*/Makefile */*/*/Makefile
	-rm -f datefile* */datefile* */*/datefile* */*/*/datefile*
	-rm -f Makefile.$(MACHINE) Vista.$(MACHINE)
	cp ../Makefile.sav ../Makefile
	cp Makefile.dist Makefile
clean: 
#	-rm $(BINDIR)/xvista $(BINDIR)/xvista.$(MACHINE) $(BINDIR)/dosystem.$(MACHINE)
	-rm Vista.h
	-rm $(VLIB) $(VLD)/user.$(MACHINE).a $(VLD)/xtv.$(MACHINE).a 
	-rm $(VLD)/display.$(MACHINE).a $(VLD)/utility.$(MACHINE).a $(VLD)/main.$(MACHINE).a
	-rm $(VLD)/libmgo.$(MACHINE).a
	-rm vista.o execute.o
	(cd include      ; make clean)
	(cd base         ; make clean)
	(cd display      ; make clean)
#	(cd display/suntv; make clean)
	(cd display/xtv  ; make clean)
	(cd image        ; make clean)
	(cd misc         ; make clean)
	(cd parser       ; make clean)
	(cd simulate     ; make clean)
	(cd spectra      ; make clean)
	(cd starphot     ; make clean)
	(cd surfphot     ; make clean)
	(cd tape         ; make clean)
	(cd tape/unix    ; make clean)
	(cd sysdep/unix  ; make clean)
	(cd user         ; make clean)
	(cd utility      ; make clean)
#	(cd ../color     ; make clean)
	(cd ../help      ; make clean)
	(cd ../docs      ; make clean)
	(cd mongosrc     ; make clean)
	make tidy
#       This is used to pack up everything for distribution from a Sun.
comptar: 
	-rm -f $(COMPTAR) $(COMPTAR).gz
	(cd include      ; make tidy)
	(cd ../help      ; make clean)
	(cd ../docs      ; make clean)
	make tidy
	(cd ../..; tar cfvX $(COMPTAR) xvista/source/.exclude.tar xvista)
	gzip ../../$(COMPTAR)
#	(cd ../help      ; make -k)
#       Since only Sun tar has the -X option, this is for other systems.
#       Note that it does not exclude things, so you pack up lots of junk.
dcomptar:
	-rm -f $(COMPTAR) $(COMPTAR).gz
	(cd include      ; make tidy)
	(cd ../help      ; make clean)
	(cd ../docs      ; make clean)
	make tidy
	-mkdir /tmp/xvista
	mv ../bin /tmp/xvista
	mv ../lib /tmp/xvista
	mv ../old /tmp/xvista
	mv unsupported /tmp/xvista
	(cd ../..; tar cf $(COMPTAR) xvista)
	mv /tmp/xvista/bin ../bin
	mv /tmp/xvista/lib ../lib
	mv /tmp/xvista/old ../old
	mv /tmp/xvista/unsupported unsupported
	gzip $(COMPTAR)
# for distribution need to mutilate all the Lick Mongo routines and calls
# make a copy of the xvista tree in $(KATADIR), set above.
distribute:
	-rm -rf $(KATADIR)/xvista/*
	-mkdir $(KATADIR)
#       copy over the entire xvista tree
	(cd ../..; tar cfX - xvista/source/.exclude.tar xvista | (cd $(KATADIR); tar xfBp -))
	(cd $(MONGODIR); make KATADIR=$(KATADIR)/xvista/source mutilate);
	(cd $(KATADIR)/xvista/source; make katatome)
katatome:
	-rm -f mongosrc
	ln -s lickmongo/source mongosrc
	touch mongosrc/interact/touva.F
	rm mongosrc/make
	ln -s ../../make mongosrc/make
	make moyle
	(PATH=$$PATH:$(KATADIR)/xvista/source; export PATH; make -f .katatome KATA=KATA KATATOME)
	-rm -f moyle moyle.f
moyle: moyle.f
	(cd lickmongo/source/devices; make Config.h MONGOLUN.inc)
	$(FC) $(FFLAGS) moyle.f -o $(BINDIR)/moyle

#       THE FOLLOWING STUFF MAKES CODE FOR THE VAXen
#
#       This will copy the entire directory tree of VMS code to the Lick
#       MicroVAX called CRUNCH.  It relies on SunLink DNI.
crunch: uvmscode
	./.sunlink.dni ./.ccdvax.dni
vesto: pvmscode
	./.sunlink.dni ./.vesto.dni
vmstape:
	cd $(VMSTREE) ; /usr/local/vista/xvista/export/vmsprep - . | \
	/usr/local/vista/xvista/export/ansitape cvlnf  VISTA - $(TAPE)
avmscode: Vista.h.avms vista.for makevms.com compile.com library.com
	-mkdir $(VMSTREE)
	-mkdir $(VMSTREE)/color
	(cd ../color     ; make avmscode)
	-mkdir $(VMSTREE)/ccd
	-mkdir $(VMSTREE)/data
	-mkdir $(VMSTREE)/help
	cp ../help/helpfile.hlp $(VMSTREE)/help
	cp ../help/makehelp.for $(VMSTREE)/help
	cp ../help/message.txt $(VMSTREE)/help
	-mkdir $(VMSTREE)/spectra
	cp ../spectra/* $(VMSTREE)/spectra
	-mkdir $(VMSTREE)/procedure
	-mkdir $(VMSTREE)/source
	-mkdir $(VMSTREE)/source/lickmongo
	mv vista.for $(VMSTREE)/source
	mv makevms.com $(VMSTREE)
	mv compile.com $(VMSTREE)/source
	mv library.com $(VMSTREE)/source
	sed 's/defined//g' < ../hack.h >$(VMSTREE)/source/hack.h
	(cd base          ; make avmscode)
	(cd display       ; make avmscode)
	(cd display/aed512; make -k)
	(cd image         ; make avmscode)
	(cd include       ; make avmscode)
	(cd misc          ; make avmscode)
	(cd parser        ; make avmscode)
	(cd simulate      ; make avmscode)
	(cd spectra       ; make avmscode)
	(cd starphot      ; make avmscode)
	(cd surfphot      ; make avmscode)
	(cd sysdep/vms    ; make -k)
	(cd tape          ; make avmscode)
	(cd tape/vms      ; make -k)
	(cd user          ; make avmscode)
	(cd utility       ; make avmscode)
	(cd lickmongo     ; make uvmscode)
	-rm Vista.h Config.h avmscode
	date >  avmscode
Vista.h.avms:
	-rm Vista.h
	ln -s $(VLD)/Vista.avms Vista.h
	-rm Config.h
	ln -s mongosrc/Config.uvms Config.h
uvmscode: Vista.h.uvms vista.for makevms.com compile.com library.com
	-mkdir $(VMSTREE)
	-mkdir $(VMSTREE)/color
	(cd ../color     ; make uvmscode)
	-mkdir $(VMSTREE)/ccd
	-mkdir $(VMSTREE)/data
	-mkdir $(VMSTREE)/help
	cp ../help/helpfile.hlp $(VMSTREE)/help
	cp ../help/makehelp.for $(VMSTREE)/help
	cp ../help/message.txt $(VMSTREE)/help
	-mkdir $(VMSTREE)/spectra
	cp ../spectra/* $(VMSTREE)/spectra
	-mkdir $(VMSTREE)/procedure
	-mkdir $(VMSTREE)/source
	-mkdir $(VMSTREE)/source/lickmongo
	mv vista.for $(VMSTREE)/source
	mv makevms.com $(VMSTREE)
	mv compile.com $(VMSTREE)/source
	mv library.com $(VMSTREE)/source
	sed 's/defined//g' < ../hack.h >$(VMSTREE)/source/hack.h
	(cd base          ; make uvmscode)
	(cd display       ; make uvmscode)
	(cd display/aed1024; make -k)
	(cd image         ; make uvmscode)
	(cd include       ; make uvmscode)
	(cd misc          ; make uvmscode)
	(cd parser        ; make uvmscode)
	(cd simulate      ; make uvmscode)
	(cd spectra       ; make uvmscode)
	(cd starphot      ; make uvmscode)
	(cd surfphot      ; make uvmscode)
	(cd sysdep/vms    ; make -k)
	(cd tape          ; make uvmscode)
	(cd tape/vms      ; make -k)
	(cd user          ; make uvmscode)
	(cd utility       ; make uvmscode)
	(cd lickmongo     ; make uvmscode)
	-rm Vista.h Config.h uvmscode
	date >  uvmscode
Vista.h.uvms:
	-rm Vista.h
	ln -s $(VLD)/Vista.uvms Vista.h
	-rm Config.h
	ln -s mongosrc/Config.uvms Config.h
pvmscode: Vista.h.pvms vista.for makevms.com compile.com library.com
	-mkdir $(VMSTREE)
	-mkdir $(VMSTREE)/color
	(cd ../color     ; make pvmscode)
	-mkdir $(VMSTREE)/ccd
	-mkdir $(VMSTREE)/data
	-mkdir $(VMSTREE)/help
	cp ../help/helpfile.hlp $(VMSTREE)/help
	cp ../help/makehelp.for $(VMSTREE)/help
	cp ../help/message.txt $(VMSTREE)/help
	-mkdir $(VMSTREE)/spectra
	cp ../spectra/* $(VMSTREE)/spectra
	-mkdir $(VMSTREE)/procedure
	-mkdir $(VMSTREE)/source
	-mkdir $(VMSTREE)/source/lickmongo
	mv vista.for $(VMSTREE)/source
	mv makevms.com $(VMSTREE)
	mv compile.com $(VMSTREE)/source
	mv library.com $(VMSTREE)/source
	sed 's/defined//g' < ../hack.h >$(VMSTREE)/source/hack.h
	(cd base          ; make pvmscode)
	(cd display       ; make pvmscode)
	(cd display/peritek; make -k)
	(cd image         ; make pvmscode)
	(cd include       ; make pvmscode)
	(cd misc          ; make pvmscode)
	(cd parser        ; make pvmscode)
	(cd simulate      ; make pvmscode)
	(cd spectra       ; make pvmscode)
	(cd starphot      ; make pvmscode)
	(cd surfphot      ; make pvmscode)
	(cd sysdep/vms    ; make -k)
	(cd tape          ; make pvmscode)
	(cd tape/vms      ; make -k)
	(cd user          ; make pvmscode)
	(cd utility       ; make pvmscode)
	(cd lickmongo     ; make uvmscode)
	-rm Vista.h Config.h pvmscode
	date >  pvmscode
Vista.h.pvms:
	-rm Vista.h
	ln -s $(VLD)/Vista.pvms Vista.h
	-rm Config.h
	ln -s mongosrc/Config.uvms Config.h
xvmscode: Vista.h.xvms vista.for makevms.com compile.com library.com
	-mkdir $(VMSTREE)
	-mkdir $(VMSTREE)/color
	(cd ../color     ; make pvmscode)
	-mkdir $(VMSTREE)/ccd
	-mkdir $(VMSTREE)/data
	-mkdir $(VMSTREE)/help
	cp ../help/helpfile.hlp $(VMSTREE)/help
	cp ../help/makehelp.for $(VMSTREE)/help
	cp ../help/message.txt $(VMSTREE)/help
	-mkdir $(VMSTREE)/spectra
	cp ../spectra/* $(VMSTREE)/spectra
	-mkdir $(VMSTREE)/procedure
	-mkdir $(VMSTREE)/source
	-mkdir $(VMSTREE)/source/lickmongo
	mv vista.for $(VMSTREE)/source
	mv makevms.com $(VMSTREE)
	mv compile.com $(VMSTREE)/source
	mv library.com $(VMSTREE)/source
	sed 's/defined//g' < ../hack.h >$(VMSTREE)/source/hack.h
	(cd base          ; make pvmscode)
	(cd display       ; make pvmscode)
	(cd display/xtv   ; make xvmscode)
	(cd image         ; make pvmscode)
	(cd include       ; make pvmscode)
	(cd misc          ; make pvmscode)
	(cd parser        ; make pvmscode)
	(cd simulate      ; make pvmscode)
	(cd spectra       ; make pvmscode)
	(cd starphot      ; make pvmscode)
	(cd surfphot      ; make pvmscode)
	(cd sysdep/vms    ; make -k)
	(cd tape          ; make pvmscode)
	(cd tape/vms      ; make -k)
	(cd user          ; make pvmscode)
	(cd utility       ; make pvmscode)
	(cd lickmongo     ; make uvmscode)
	-rm Vista.h Config.h xvmscode
	date >  xvmscode
Vista.h.xvms:
	-rm Vista.h
	ln -s $(VLD)/Vista.xvms Vista.h
	-rm Config.h
	ln -s mongosrc/Config.uvms Config.h
nosupport: 
