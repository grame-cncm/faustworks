system	:= $(shell uname -s)
qm4 := $(shell which qmake-qt4)
qm := $(if $(qm4),$(qm4),qmake)


# for osx
DISTRIB=FaustWorks
version="0.4"

ifeq ($(system), Darwin)
	SPEC := -spec macx-llvm
	DST  := "FaustWorks.app/Contents/"

else
	SPEC := ""
	PREFIX := /usr
endif


all : Makefile.qt4
	make -f Makefile.qt4

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


dmg : $(DISTRIB).dmg

$(DISTRIB).dmg : all
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


# make a FaustWorks source distribution using git archive
distribution :
	git archive -o $(DISTRIB)-$(version).zip HEAD

clean : Makefile.qt4
	make -f Makefile.qt4 clean
	rm -f FaustWorks.pro.user

Makefile.qt4: 
	$(qm) $(SPEC) -o Makefile.qt4

	
	
	
