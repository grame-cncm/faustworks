/*
 * main.cpp
 *
 * Created by Christophe Daudin on 12/05/09.
 * Copyright 2009 Grame. All rights reserved.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
 
#include <QApplication>
#include <QFileOpenEvent>
#include <QString>
#include <QFontDatabase>
#include <QFile>
#include <QMainWindow>

#include "FaustMainWindow.h"
//FaustMainWindow* gMainWin;
QMainWindow* gMainWin;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Q_INIT_RESOURCE( application );
    app.setWindowIcon(QIcon(":/FaustWorks.png"));

	QCoreApplication::setOrganizationName( "GRAME" );
    QCoreApplication::setApplicationName( "FaustWorks" );

	GraphicsSceneMainWindowSettings s;
	s.mLanguageNameShort = "Faust";
	s.mLanguageNameLong = s.mLanguageNameShort;
	s.mDefaultLanguageCode = "process = +;";
	s.mLanguageFileExtension = "dsp";			
	s.mSceneFileExtension = "fsc";
	s.mLanguageCommandsFile = ":/FaustCommands.xml";

	s.mMinScale = 0;
	s.mMaxScale = 0;
	s.mMinItemSize = QSize( 40 , 40 );
	s.mMaxItemSize = QSize( 400 , 300 );

	s.mDesactiveUIElements << SIZE_TOOL_BAR << RESCALE_ACT << H_ALIGN_ACT << V_ALIGN_ACT;

    s.mHasHistory = false;
    s.mHasStorage = false;
	GraphicsSceneMainWindow::initApplicationSettings(s);

    //FaustMainWindow mainWin;
    gMainWin = new FaustMainWindow();
	
    //QApplication::instance()->installEventFilter( &mainWin );
    QApplication::instance()->installEventFilter( gMainWin );

	if ( argc >= 2 )
	{
		QString fileName( argv[1] );
		QFileOpenEvent fileOpenEvent( fileName );
        //QApplication::sendEvent( &mainWin , &fileOpenEvent );
        QApplication::sendEvent( gMainWin , &fileOpenEvent );
    }

    //mainWin.show();
    gMainWin->show();
    return app.exec();
}
