system	:= $(shell uname -s)


# for osx
DISTRIB=FaustWorks
version="12.04"

ifeq ($(system), Darwin)
	SPEC := -spec macx-llvm
	DST  := "FaustWorks.app/Contents/"

else
	SPEC := ""
	PREFIX := /usr/local
endif


all : Makefile.qt4
	make -f Makefile.qt4

install : install-$(system)
uninstall : uninstall-$(system)

install-Linux :
	cp FaustWorks $(PREFIX)/bin
	cp Resources/FaustWorks.svg /usr/share/icons/hicolor/scalable/apps/
	cp FaustWorks.desktop /usr/share/applications/
	cp FaustWorks.desktop /usr/share/app-install/desktop/FaustWorks\:FaustWorks.desktop

uninstall-Linux :
	rm -f $(PREFIX)/bin/FaustWorks
	rm -f /usr/share/icons/hicolor/scalable/apps/FaustWorks.svg
	rm -f /usr/share/applications/FaustWorks.desktop
	rm -f /usr/share/app-install/desktop/FaustWorks\:FaustWorks.desktop

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


clean : Makefile.qt4
	make -f Makefile.qt4 clean
	rm FaustWorks.pro.user

Makefile.qt4: 
	qmake $(SPEC) -o Makefile.qt4

	
	
	
