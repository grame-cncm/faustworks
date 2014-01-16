system	:= $(shell uname -s)
qm4 := $(shell which qmake-qt4)
qm := $(if $(qm4),$(qm4),qmake)



VERSION="0.4"
DISTRIB=FaustWorks-$(VERSION)
TMPDIR=$(DISTRIB)-Distribution

ifeq ($(system), Darwin)
	SPEC := -spec macx-g++
	DST  := "FaustWorks.app/Contents/"

else
	SPEC := ""
	PREFIX := /usr
endif


all : Makefile_QT
	make -f Makefile_QT

install : install-$(system)
uninstall : uninstall-$(system)

install-Linux :
	install FaustWorks $(PREFIX)/bin
	install faustworks.desktop $(PREFIX)/share/applications/
	install Resources/faustworks.xpm $(PREFIX)/share/pixmaps/
	install Resources/faustworks.png $(PREFIX)/share/icons/hicolor/32x32/apps/
	install Resources/faustworks.svg $(PREFIX)/share/icons/hicolor/scalable/apps/

uninstall-Linux :
	rm -f $(PREFIX)/bin/FaustWorks
	rm -f $(PREFIX)/share/applications/faustworks.desktop 
	rm -f $(PREFIX)/share/pixmaps/faustworks.xpm 
	rm -f $(PREFIX)/share/icons/hicolor/32x32/apps/faustworks.png
	rm -f $(PREFIX)/share/icons/hicolor/scalable/apps/faustworks.svg

install-Darwin :
	cp -r FaustWorks.app /Applications

dmg : $(DISTRIB).dmg

$(DISTRIB).dmg : all
	rm -rf $(TMPDIR)
	mkdir $(TMPDIR)
	cp -r FaustWorks.app $(TMPDIR)
	macdeployqt $(TMPDIR)/FaustWorks.app/
	rm -rf $(DISTRIB).dmg
	hdiutil create $(DISTRIB).dmg -srcfolder $(TMPDIR)
	rm -rf $(TMPDIR)


# make a FaustWorks distribution by cloning the git repository
clonedist :
	git clone git://faudiostream.git.sourceforge.net/gitroot/faudiostream/FaustWorks FaustWorks-$(version)
	rm -rf FaustWorks-$(version)/.git
	rm -f FaustWorks-$(version).tar.gz
	tar czfv FaustWorks-$(version).tar.gz FaustWorks-$(version)
	rm -rf FaustWorks-$(version)


# make a FaustWorks source distribution using git archive
distribution :
	git archive -o $(DISTRIB).zip HEAD

clean : Makefile_QT
	make -f Makefile_QT clean
	rm -f FaustWorks.pro.user
	rm -rf FaustWorks.app
	rm -f $(DISTRIB).dmg
	rm -f $(DISTRIB).zip

Makefile_QT: 
	$(qm) $(SPEC) -o Makefile_QT

	
	
	
