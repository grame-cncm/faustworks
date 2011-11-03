
system	:= $(shell uname -s)
scriptssrc	:= ../faust/tools/faust2appls
scriptslin	:= scripts.lin
scriptsosx	:= scripts.osx


# for osx
DISTRIB=FaustWorks
version="11.05"
DST="FaustWorks.app/Contents/"

ifeq ($(system), Darwin)
	SPEC := -spec macx-g++
else
	SPEC := ""
endif


all : Makefile.QT
	make -f Makefile.QT
	
dmg : fillscripts $(DISTRIB).dmg

$(DISTRIB).dmg : all
	cp -rf scripts.osx $(DST)
	macdeployqt FaustWorks.app/
	rm -rf $(DISTRIB)
	rm -rf $(DISTRIB).dmg
	mkdir $(DISTRIB)
	cp -r FaustWorks.app $(DISTRIB)
	hdiutil create $(DISTRIB).dmg -srcfolder $(DISTRIB)


# make a FaustWorks distribution by cloning the git repository
clonedist :
	git clone git://faudiostream.git.sourceforge.net/gitroot/faudiostream/FaustWorks FaustWorks-$(version)
	rm -rf FaustWorks-$(version)/.git
	rm -f FaustWorks-$(version).tar.gz
	tar czfv FaustWorks-$(version).tar.gz FaustWorks-$(version)
	rm -rf FaustWorks-$(version)


clean : Makefile.QT
	make -f Makefile.QT clean

Makefile.QT: 
	qmake $(SPEC) -o Makefile.QT
	
##############################################
fillscripts : fillscripts-lin fillscripts-osx

####
fillscripts-lin : $(scriptslin)/alsagtk $(scriptslin)/jackgtk $(scriptslin)/jackqt $(scriptslin)/csound $(scriptslin)/puredata $(scriptslin)/portaudioqt $(scriptslin)/supercollider 

$(scriptslin)/alsagtk : $(scriptssrc)/faust2alsa
	cp $(scriptssrc)/faust2alsa $(scriptslin)/alsagtk
	
$(scriptslin)/jackgtk : $(scriptssrc)/faust2jack
	cp $(scriptssrc)/faust2jack $(scriptslin)/jackgtk
	
$(scriptslin)/jackqt: $(scriptssrc)/faust2jaqt
	cp $(scriptssrc)/faust2jaqt $(scriptslin)/jackqt
	
$(scriptslin)/csound : $(scriptssrc)/faust2csound
	cp $(scriptssrc)/faust2csound $(scriptslin)/csound
	
$(scriptslin)/puredata : $(scriptssrc)/faust2puredata
	cp $(scriptssrc)/faust2puredata $(scriptslin)/puredata
	
$(scriptslin)/portaudioqt : $(scriptssrc)/faust2paqt
	cp $(scriptssrc)/faust2paqt $(scriptslin)/portaudioqt
	
$(scriptslin)/supercollider : $(scriptssrc)/faust2supercollider
	cp $(scriptssrc)/faust2supercollider $(scriptslin)/supercollider

####
fillscripts-osx : $(scriptsosx)/coreaudio-qt $(scriptsosx)/max-msp $(scriptsosx)/puredata $(scriptsosx)/csound $(scriptsosx)/puredata $(scriptsosx)/supercollider 

$(scriptsosx)/coreaudio-qt : $(scriptssrc)/faust2caqt
	cp $(scriptssrc)/faust2caqt $(scriptsosx)/coreaudio-qt
	
$(scriptsosx)/max-msp : $(scriptssrc)/faust2msp
	cp $(scriptssrc)/faust2msp $(scriptsosx)/max-msp
	
$(scriptsosx)/csound : $(scriptssrc)/faust2csound
	cp $(scriptssrc)/faust2csound $(scriptsosx)/csound
	
$(scriptsosx)/puredata : $(scriptssrc)/faust2puredata
	cp $(scriptssrc)/faust2puredata $(scriptsosx)/puredata
	
$(scriptsosx)/supercollider : $(scriptssrc)/faust2supercollider
	cp $(scriptssrc)/faust2supercollider $(scriptsosx)/supercollider
	
	
	
