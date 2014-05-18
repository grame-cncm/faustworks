/*
 * FaustMainWindow.cpp
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
 
#include "FaustMainWindow.h"

#include <QSettings>
#include <QCoreApplication>
#include <QProcess>
#include <QMessageBox>
#include <QtDebug>
#include <QMenu>
#include <QScrollBar>
#include <QFileInfo>
#include <QDir>
#include <QStringList>


#include "QFaustItemFactory.h"
#include "QFaustItem.h"
#include "MainWindowObjects.h"
#include "QFaustPreferences.h"
#include "QItemResizer.h"
#include "QPaletteItem.h"

#include "FaustHighlighter.h"
#include "CPPHighlighter.h"

#define RSC_DIR	QString(":/")

#if defined WIN32
	#define DEFAULT_FAUST_PATH		"faust.exe"
    #define DIR_SEP                 "\\"
	#define FONT_FAMILY				"Monospace"
#elif defined __APPLE__
    #define DEFAULT_FAUST_PATH		"faust"
    #define DIR_SEP                 "/"
	#define FONT_FAMILY				"Courier"
#elif defined linux
    #define DEFAULT_FAUST_PATH		"faust"
    #define DIR_SEP                 "/"
	#define FONT_FAMILY				"Monospace"
#endif

#define CURRENT_TARGET_ARCHITECTURE_SETTING		"CurrentTargetArchitecture"
#define CURRENT_BUILD_OPTION_SETTING			"CurrentBuildOption"
#define COMPANY_NAME		"GRAME"
#define APP_NAME			"FaustWorks"

#define PALETTE_ID          1
#define TAB_SPACE 4

/**
 * The different ways to combine two faust expressions. Basically the block diagram algebra
 * in two flavors : normal arguments and reversed arguments
 */
enum {  COMBINATION_SEQ1=1, COMBINATION_SEQ2,
        COMBINATION_PAR1, COMBINATION_PAR2,
        COMBINATION_SPLIT1, COMBINATION_SPLIT2,
        COMBINATION_MERGE1, COMBINATION_MERGE2,
        COMBINATION_REC1, COMBINATION_REC2
    };

//-----------------------------------------------------------------------
FaustMainWindow::FaustMainWindow()
	:	GraphicsSceneMainWindow( new LanguageGraphicsView() , new QFaustItemFactory() )
{
    QSettings settings("grame.fr", "FaustWorks");

	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	mCPPTextEdit = new QTextEdit("" , this);
	mCPPTextEdit->setAcceptRichText( false );
	mCPPTextEdit->setReadOnly( true );
	mCPPTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    mCPPTextEdit->setFontFamily( FONT_FAMILY );

    mCPPTextEditDock = new QDockWidget(tr("C++ code") , this);
	mCPPTextEditDock->setWidget( mCPPTextEdit );
	mCPPTextEditDock->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable );
	mCPPTextEditDock->setObjectName("CPPTextEditDock");
    addDockWidget( Qt::RightDockWidgetArea , mCPPTextEditDock );

    // Error Window as a dock
    if (!gErrorWindow) {
        gErrorWindow = new QTextEdit("", this);
        //gErrorWindow->setWindowFlags(Qt::WindowStaysOnTopHint|Qt::Tool );
        gErrorWindow->setFontPointSize(9);
        gErrorWindow->setTextInteractionFlags(Qt::NoTextInteraction);
        gErrorWindow->ensureCursorVisible();
        gErrorWindow->hide();
        //gErrorWindow->setWindowTitle("Invalid Faust Code");

        mErrorDock = new QDockWidget(tr("Error Messages") , this);
        mErrorDock->setWidget( gErrorWindow );
        mErrorDock->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable );
        mErrorDock->setObjectName("ErrorDockObject");
        addDockWidget( Qt::RightDockWidgetArea , mErrorDock );
    }
    //-------------

	init();
	
    new FaustHighlighter( mLanguageTextEdit->document() ); // note : we need it, but we don't have to store the highlighter
	mLanguageTextEdit->setAcceptRichText( false );
    mLanguageTextEdit->selectAll();
    mLanguageTextEdit->setFontFamily( FONT_FAMILY );

	new CPPHighlighter( mCPPTextEdit->document() );

	changeFontSize(settings.value(FONT_SIZE_SETTING, DEFAULT_FONT_SIZE).toDouble());

	// Create a QPaletteItem for FaustItems (id=FAUST_ITEM_PALETTE), radius 100, no parent (=independant).
	QPaletteItem * paletteItem = new QPaletteItem( FAUST_ITEM_PALETTE , 150 , 0 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "seq2.png" ) ,		COMBINATION_SEQ2 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "rec2.png" ) ,		COMBINATION_REC2 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "par1.png" ) ,		COMBINATION_PAR1 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "merge1.png" ) ,		COMBINATION_MERGE1 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "split1.png" ) ,		COMBINATION_SPLIT1 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "seq1.png" ) ,		COMBINATION_SEQ1 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "rec1.png" ) ,		COMBINATION_REC1 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "par2.png" ) ,		COMBINATION_PAR2 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "merge2.png" ) ,		COMBINATION_MERGE2 );
	paletteItem->addInteraction( buildPixmapItem(RSC_DIR + "split2.png" ) ,		COMBINATION_SPLIT2 );
	mGraphicsScene->addItem( paletteItem );
	QPaletteManager::instance()->setActivationDelay(200);
	connect( QPaletteManager::instance() , SIGNAL(interactionActivated(QGraphicsItem*,QGraphicsItem*,int,int,int)) , this , SLOT( combineItems(QGraphicsItem*,QGraphicsItem*,int,int,int) ) );
	paletteItem->setBrush( QBrush( QColor(FAUST_ITEM_BASE_RGB , 50) ) );
	paletteItem->setPen( QPen( QColor(FAUST_ITEM_BASE_RGB , 250) ) );
	//Palette's positioning policy
	paletteItem->setPositioningPolicy(QPaletteItem::CENTER_POLICY);
}

//#define NO_OPTIONS "No options"
#define NO_OPTIONS "Scalar"
//-----------------------------------------------------------------------
void FaustMainWindow::reinitSettings()
{
	GraphicsSceneMainWindow::reinitSettings();

	reinitPreferencesSettings();
	
    QSettings settings("grame.fr", "FaustWorks");
	settings.setValue( CURRENT_TARGET_ARCHITECTURE_SETTING , "" );
	settings.setValue( CURRENT_BUILD_OPTION_SETTING , NO_OPTIONS );
}

//-------------------------------------------------------------------------
void FaustMainWindow::reinitPreferencesSettings()
{
    QSettings   settings("grame.fr", "FaustWorks");


	//	----- Faust Path
	settings.setValue( FAUST_PATH_SETTING , DEFAULT_FAUST_PATH );
	
	// ----- Target configurations
	settings.remove( TARGETS_SETTING );
	settings.beginGroup(TARGETS_SETTING);

    //	----- Fill the configuration with existing script files in the scripts folder

#if defined linux
    settings.setValue( "jack-gtk",  QString("faust2jack") + " $DSP $OPTIONS" );
    settings.setValue( "alsa-gtk",  QString("faust2alsa") + " $DSP $OPTIONS" );
    settings.setValue( "jack-qt",  QString("faust2jaqt") + " $DSP $OPTIONS" );
    settings.setValue( "alsa-qt",  QString("faust2alqt") + " $DSP $OPTIONS" );
    settings.setValue( "csound",  QString("faust2csound") + " $DSP $OPTIONS" );
    settings.setValue( "puredata",  QString("faust2puredata") + " $DSP $OPTIONS" );
    settings.setValue( "supercollider",  QString("faust2supercollider") + " $DSP $OPTIONS" );
    settings.setValue( "ladspa",  QString("faust2ladspa") + " $DSP $OPTIONS" );
    settings.setValue( "dssi",  QString("faust2dssi") + " $DSP $OPTIONS" );

#elif defined __APPLE__
    settings.setValue( "coreaudio-qt",  QString("faust2caqt") + " $DSP $OPTIONS" );
    settings.setValue( "maxmsp",  QString("faust2msp") + " $DSP $OPTIONS" );
    settings.setValue( "vst",  QString("faust2vst") + " $DSP $OPTIONS" );
    settings.setValue( "csound",  QString("faust2csound") + " $DSP $OPTIONS" );
    settings.setValue( "puredata",  QString("faust2puredata") + " $DSP $OPTIONS" );
    settings.setValue( "supercollider",  QString("faust2supercollider") + " $DSP $OPTIONS" );

#elif defined WIN32
#endif

	settings.endGroup();

    // ----- Build options
	settings.remove( OPTIONS_SETTING );
	settings.beginGroup( OPTIONS_SETTING );
    settings.setValue( "Scalar" ,	"" );
    settings.setValue( "Vectorial" , "-vec -vs 64" );
    settings.setValue( "Parallel (OpenMP)", "-omp -vs 512" );
    settings.setValue( "Parallel (WS)", "-sch -vs 512" );
    settings.endGroup();
}

//-------------------------------------------------------------------------
//								Protected slots							///
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
void FaustMainWindow::preferences()
{
    QFaustPreferences * dialog = new QFaustPreferences(this);

	if ( dialog->exec() == QDialog::Accepted )
	{
		GraphicsSceneMainWindow::readPreferencesSettings();
		readPreferencesSettings();
	}
}

//-------------------------------------------------------------------------
void FaustMainWindow::updateCode()
{
	int vFaustEditScrollValue = mLanguageTextEdit->verticalScrollBar()->value();
	int hFaustEditScrollValue = mLanguageTextEdit->horizontalScrollBar()->value();
	GraphicsSceneMainWindow::updateCode();
	mLanguageTextEdit->verticalScrollBar()->setValue( vFaustEditScrollValue );
	mLanguageTextEdit->horizontalScrollBar()->setValue( hFaustEditScrollValue );

//	if ( ( mFirstSelectedItem ) && ( mCPPTextEdit->isEnabled() ) )
	if ( mCPPTextEdit->isEnabled() )
	{
		QString newCode = ((QFaustItem*)mFirstSelectedItem)->cppCode();
		if ( mCPPTextEdit->toPlainText() != newCode )
		{
			int vSliderValue = mCPPTextEdit->verticalScrollBar()->value();
			int hSliderValue = mCPPTextEdit->horizontalScrollBar()->value();
			mCPPTextEdit->setText( newCode );
			mCPPTextEdit->horizontalScrollBar()->setValue( hSliderValue );
			mCPPTextEdit->verticalScrollBar()->setValue( vSliderValue );
		}
	}
}

//-------------------------------------------------------------------------
void FaustMainWindow::targetArchitectureChanged(int index)
{
    QSettings settings("grame.fr", "FaustWorks");
	settings.setValue( CURRENT_TARGET_ARCHITECTURE_SETTING , mTargetsComboBox->currentText() );
	setBuildCommand( mTargetsComboBox->itemData( index ).toString() );
}

//-------------------------------------------------------------------------
void FaustMainWindow::buildOptionsChanged(int index)
{
    QSettings settings("grame.fr", "FaustWorks");
	settings.setValue( CURRENT_BUILD_OPTION_SETTING , mOptionsComboBox->currentText() );
	setBuildOptions( mOptionsComboBox->itemData( index ).toString() );
	updateCode();
}

//-------------------------------------------------------------------------
void FaustMainWindow::droppedFileTypeChanged(int droppedFileTypeIndex)
{	
	QFaustItem::setDroppedFileType( mDropTypeComboBox->itemData(droppedFileTypeIndex).toInt() );
}

//-------------------------------------------------------------------------
void FaustMainWindow::itemLaunchScriptError(const QString& command)
{
    QMessageBox::warning( 0 , tr("Binary build error") ,
                          tr("Couldn't launch command %1!").arg(command));
}

//-------------------------------------------------------------------------
void FaustMainWindow::itemBuildError(int errorType , const QString& msg)
{
	QString statusMsg = "";
	switch ( errorType )
	{
		case QFaustItem::SCRIPT_ERROR	:
		{
            statusMsg = tr("Script error : ") + msg;
			break;
		}
		case QFaustItem::SCRIPT_CRASHED	:
		{
            statusMsg = tr("Script crashed");
			if ( msg.length() )
				statusMsg += (" : " + msg);
			break;
		}
		case QFaustItem::SCRIPT_NO_FILE	:
		{
            statusMsg = tr("Script : no output file specified");
			break;
		}
		case QFaustItem::SCRIPT_FILE_NOT_FOUND	:
		{
        statusMsg = tr("Script : output file(s) %1 not found.").arg(msg);
			break;
		}
	}
	//statusBar()->showMessage( statusMsg );
//	qDebug() << "FaustMainWindow::itemBuildError : " << statusMsg;

    QMessageBox::warning( 0 , tr("Binary build error") ,
		statusMsg );

}

//-------------------------------------------------------------------------

/**
 * Combine an item A with a dropped item B according to a combination type
 */
void FaustMainWindow::combineItems(QGraphicsItem* dropped,QGraphicsItem* target,int,int,int combinationType)
{
    QFaustItem *    fixedItem = (QFaustItem *)target;
    QFaustItem *    droppedItem = (QFaustItem *)dropped;

	//Auto-save the item, or save it to untitled_1/2/3... if
	//there wheren't saved.
	if ( fixedItem->file().length() )
		fixedItem->save( fixedItem->file() );
	else 
		fixedItem->save( availableDefaultName( fixedItem->name() ) );

	if ( droppedItem->file().length() )
		droppedItem->save( droppedItem->file() );
	else 
		droppedItem->save( availableDefaultName( droppedItem->name() ) );

    // the various ways to combine Faust programs
    QString code;
    switch ( combinationType )
    {
        case COMBINATION_SEQ1 :
            code = buildCombinationCode( droppedItem->file() , fixedItem->file(), ":");
            break;
        case COMBINATION_SEQ2 :
            code = buildCombinationCode( fixedItem->file() , droppedItem->file(), ":");
            break;

        case COMBINATION_PAR1 :
            code = buildCombinationCode( droppedItem->file() , fixedItem->file(), ",");
            break;
        case COMBINATION_PAR2 :
            code = buildCombinationCode( fixedItem->file() , droppedItem->file(), ",");
            break;

        case COMBINATION_SPLIT1 :
            code = buildCombinationCode( droppedItem->file() , fixedItem->file(), "<:");
            break;
        case COMBINATION_SPLIT2 :
            code = buildCombinationCode( fixedItem->file() , droppedItem->file(), "<:");
            break;

        case COMBINATION_MERGE1 :
            code = buildCombinationCode( droppedItem->file() , fixedItem->file(), ":>");
            break;
        case COMBINATION_MERGE2 :
            code = buildCombinationCode( fixedItem->file() , droppedItem->file(), ":>");
            break;

        case COMBINATION_REC1 :
            code = buildCombinationCode( droppedItem->file() , fixedItem->file(), "~");
            break;
        case COMBINATION_REC2 :
            code = buildCombinationCode( fixedItem->file() , droppedItem->file(), "~");
            break;
        default:
            qFatal("ERROR unrecognized combination code %d\n", combinationType);
    }

	//Set the code of the target item with the code computed by GuidoAR
	fixedItem->setCode( code );
	fixedItem->unlinkFile();

	//It's a fusion of 2 items, so we remove the dropped item (only if it was in the main scene: history&storage items are not affected).
	if ( droppedItem->scene() == mGraphicsScene )
	{
		droppedItem->deleteLater();
		itemRemoved( droppedItem );
	}
	unselectAll();
	fixedItem->setSelected( true );
	
	//Don't forget to call QGraphicsSceneMainWindow::addToHistory, because we changed the item's code
	GraphicsSceneMainWindow::addToHistory(fixedItem);
}

//------------------------------------------------------------------------
//							Protected members							//
//------------------------------------------------------------------------

//-----------------------------------------------------------------------
void FaustMainWindow::init()
{
	mIsWorkspaceModeOn = true;
	GraphicsSceneMainWindow::init();
}

// Update values / widgets / anything that depends on the Settings.
//-------------------------------------------------------------------------
void FaustMainWindow::readSettings()
{
	//Reads the type of settings that are set in the preference
	//Note: faust executable path & build commands are set here
	readPreferencesSettings();
	
	//Call GraphicsSceneMainWindow function
	//Note : the workspace-scene is loaded here, so the
	//faust executable path & build commands must have been set before.
	GraphicsSceneMainWindow::readSettings();

    QSettings settings("grame.fr", "FaustWorks");
	//Update changed values
	
	//Update text edits.
	changeFontSize(settings.value(FONT_SIZE_SETTING, DEFAULT_FONT_SIZE).toDouble() );
	reloadTextEdits();
}

QString makeAbsolutePath(const QString& path)
{
    if (path[0]!='.') return path;
    return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + path);
}

//-------------------------------------------------------------------------
void FaustMainWindow::readPreferencesSettings()
{
    QSettings settings("grame.fr", "FaustWorks");

    QString faustAbsolutePath = makeAbsolutePath(settings.value(FAUST_PATH_SETTING).toString());
    qDebug() << "faustAbsolutePath : " << faustAbsolutePath;

	//Update Faust Path
    QFaustItem::setFaustPath( faustAbsolutePath );
	
	QProcess p;
    p.start( faustAbsolutePath + " -v " );
	p.close();
	if ( p.error() == QProcess::FailedToStart )
	{
        QMessageBox::warning( 0 , tr("Faust not found") ,
        tr("Couldn't launch %1.\n"
           "The file may not exist, or you may not have the rights to execute it.\n"
           "You must define a valid faust executable in the Application Preferences.").arg(faustAbsolutePath) );
	}
	
	//Update combobox
	updateBuildComboBox( mTargetsComboBox , SLOT( targetArchitectureChanged(int) ) ,	TARGETS_SETTING , CURRENT_TARGET_ARCHITECTURE_SETTING );
	updateBuildComboBox( mOptionsComboBox , SLOT( buildOptionsChanged(int) ) ,			OPTIONS_SETTING , CURRENT_BUILD_OPTION_SETTING );
	setBuildCommand( mTargetsComboBox->itemData( mTargetsComboBox->currentIndex() ).toString() );
	setBuildOptions( mOptionsComboBox->itemData( mOptionsComboBox->currentIndex() ).toString() );

}

//-----------------------------------------------------------------------
void FaustMainWindow::updateBuildComboBox( QComboBox * comboBox , const char * indexChangedSlot , const QString& settingsKey , const QString& settingsIndexKey )
{
	disconnect( comboBox , SIGNAL(currentIndexChanged(int)) , this , indexChangedSlot );
	comboBox->clear();
	
    QSettings settings("grame.fr", "FaustWorks");
	settings.beginGroup(settingsKey);
	QStringList keys = settings.allKeys();
	for ( int i = 0 ; i < keys.size() ; i++ )
	{
		comboBox->addItem( keys[i] , settings.value( keys[i] ) );
	}
	settings.endGroup();
	connect( comboBox , SIGNAL(currentIndexChanged(int)) , this , indexChangedSlot );
	QString t = settings.value( settingsIndexKey ).toString();
	int selectedIndex = comboBox->findText( t );
	if ( selectedIndex == -1 )
		selectedIndex = 0;

	if ( comboBox->currentIndex() != selectedIndex )
		comboBox->setCurrentIndex( selectedIndex );
}

//-----------------------------------------------------------------------
void FaustMainWindow::createActions()
{
	 GraphicsSceneMainWindow::createActions();
	
	mPreferencesAct = new QAction(tr("Preferences"), this);
    mPreferencesAct->setStatusTip(tr("Setup the applications parameters"));
    connect(mPreferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));
	
	mAddAct->setIcon(QIcon(RSC_DIR + QString("addFaust.png")));
	mAddFromFileAct->setIcon(QIcon(RSC_DIR + QString("addFromFileFaust.png")));
	mRemoveAct->setIcon(QIcon(RSC_DIR + QString("removeFaust.png")));
}	

//-----------------------------------------------------------------------
void	FaustMainWindow::createMenus()
{
	GraphicsSceneMainWindow::createMenus();
	
	QAction * sep;
	sep = new QAction(this);
	sep->setSeparator(true);
	mFileMenu->insertAction(mExitAct , sep );
	mFileMenu->insertAction( mExitAct , mPreferencesAct);
	sep = new QAction(this);
	sep->setSeparator(true);
	mFileMenu->insertAction(mExitAct , sep );
}

//-----------------------------------------------------------------------
void FaustMainWindow::createToolBars()
{
	GraphicsSceneMainWindow::createToolBars();
	
	mBuildToolBar = addToolBar(tr("&Build"));
	mBuildToolBar->setObjectName("BuildToolBar");
/*
  // supprim par yo le 16 septembre 2010 car inutile....
	mDropTypeComboBox = new QComboBox( this );
	mDropTypeComboBox->setToolTip("Dropped-out file");
	mDropTypeComboBox->setStatusTip("Choose the type of file exported when dropping an item out of the application.");
	mDropTypeComboBox->addItem("Binary" , QVariant( QFaustItem::DROP_BINARY ) );
	mDropTypeComboBox->addItem("DSP" , QVariant( QFaustItem::DROP_DSP ) );
	mDropTypeComboBox->addItem("CPP" , QVariant( QFaustItem::DROP_CPP ) );
	mDropTypeComboBox->addItem("SVG" , QVariant( QFaustItem::DROP_SVG ) );
	connect( mDropTypeComboBox , SIGNAL(currentIndexChanged(int)) , this , SLOT(droppedFileTypeChanged(int)) );
	mBuildToolBar->addWidget( mDropTypeComboBox );
*/
	mTargetsComboBox = new QComboBox( this );
    mTargetsComboBox->setToolTip(tr("Target architecture"));
    mTargetsComboBox->setStatusTip(tr("Choose the exported plug-in architecture"));
	connect( mTargetsComboBox , SIGNAL(currentIndexChanged(int)) , this , SLOT(targetArchitectureChanged(int)) );
	mBuildToolBar->addWidget( mTargetsComboBox );
	
	mOptionsComboBox = new QComboBox( this );
    mOptionsComboBox->setToolTip(tr("Build options"));
    mOptionsComboBox->setStatusTip(tr("Build options used for $OPTIONS by binary build scripts"));
	connect( mOptionsComboBox , SIGNAL(currentIndexChanged(int)) , this , SLOT(buildOptionsChanged(int)) );
	mBuildToolBar->addWidget( mOptionsComboBox );
}

//-------------------------------------------------------------------------
void FaustMainWindow::setupNAddItem(QLanguageItem* decorator)
{
	GraphicsSceneMainWindow::setupNAddItem(decorator);
	
	QFaustItem * faustItem = (QFaustItem *)decorator;
	
	connect( faustItem , SIGNAL( buildError(int , const QString&) ) , this , SLOT( itemBuildError(int , const QString&)) );
	connect( faustItem , SIGNAL( launchScriptError(const QString&) ) , this , SLOT( itemLaunchScriptError(const QString&)) );
}

//-------------------------------------------------------------------------
bool FaustMainWindow::addLanguageItem(const QString &gmnSource, bool sourceIsAFile , QLanguageItem** createdItem)
{
	bool result = GraphicsSceneMainWindow::addLanguageItem( gmnSource , sourceIsAFile , createdItem );
	
	return result;
}

//-------------------------------------------------------------------------
QItemResizer * FaustMainWindow::plugResizer( QLanguageItem * itemContainer )
{
	QItemResizer * itemResizer = GraphicsSceneMainWindow::plugResizer( itemContainer );
	itemResizer->setKeepAspectRatio( true );
	itemResizer->setBrush( QBrush( QColor(FAUST_ITEM_BASE_RGB , 50) ) );
	itemResizer->setPen( QPen( QColor(FAUST_ITEM_BASE_RGB , 250) ) );
	return itemResizer;
}

//-------------------------------------------------------------------------
void FaustMainWindow::updateWindowState()
{
	int vFaustEditScrollValue = mLanguageTextEdit->verticalScrollBar()->value();
	int hFaustEditScrollValue = mLanguageTextEdit->horizontalScrollBar()->value();
	GraphicsSceneMainWindow::updateWindowState();
	mLanguageTextEdit->verticalScrollBar()->setValue( vFaustEditScrollValue );
	mLanguageTextEdit->horizontalScrollBar()->setValue( hFaustEditScrollValue );

	mCPPTextEdit->setEnabled( mLanguageTextEdit->isEnabled() );
	if ( !mCPPTextEdit->isEnabled() )
		mCPPTextEdit->setText( "" );
}

//-----------------------------------------------------------------------
void FaustMainWindow::changeFontSize( float newFontPointSize )
{
	// Make sure fontsize is appropriate before changing it
	if (newFontPointSize < MIN_FONT_SIZE) newFontPointSize = MIN_FONT_SIZE;
	if (newFontPointSize > MAX_FONT_SIZE) newFontPointSize = MAX_FONT_SIZE;

	mCPPTextEdit->setFontPointSize( newFontPointSize );
	mCPPTextEdit->setTabStopWidth( QFontMetrics(mCPPTextEdit->currentFont()).width ( "a" ) * TAB_SPACE );

	// Set mLanguageTextEdit->setFontPointSize here so that we can also call mLanguageTextEdit->setTabStopWidth,
	// that hasn't to be called by GraphicsSceneMainWindow.
	mLanguageTextEdit->setFontPointSize( newFontPointSize );
	mLanguageTextEdit->setTabStopWidth( QFontMetrics(mLanguageTextEdit->currentFont()).width ( "a" ) * TAB_SPACE );	
	//GraphicsSceneMainWindow::changeFontSize(newFontPointSize);
	//mLanguageTextEdit->setFontPointSize( newFontPointSize );
    QSettings("grame.fr", "FaustWorks").setValue( FONT_SIZE_SETTING , mLanguageTextEdit->fontPointSize() );
	reloadTextEdits();

}

//-------------------------------------------------------------------------
void FaustMainWindow::setBuildCommand( const QString& buildCommand )
{
	QFaustItem::setBuildCommand( buildCommand );
	
	QList<QFaustItem*> items = selectedFaustItems();
	for ( int i = 0 ; i < items.size() ; i++ )
	{
		items[i]->checkBinary();
	}
}

//-------------------------------------------------------------------------
void FaustMainWindow::setBuildOptions( const QString& buildOptions )
{
	QFaustItem::setBuildOptions( buildOptions );
	
	QList<QFaustItem*> items = selectedFaustItems();
	for ( int i = 0 ; i < items.size() ; i++ )
	{
		items[i]->checkBinary();
	}
}

//-------------------------------------------------------------------------
void FaustMainWindow::reloadTextEdits()
{
	GraphicsSceneMainWindow::reloadTextEdits();
	int vValue = mCPPTextEdit->verticalScrollBar()->value();
	int hValue = mCPPTextEdit->horizontalScrollBar()->value();
	mCPPTextEdit->setText( mCPPTextEdit->toPlainText() );
	mCPPTextEdit->verticalScrollBar()->setValue( vValue );
	mCPPTextEdit->horizontalScrollBar()->setValue( hValue );
}

//-------------------------------------------------------------------------
void FaustMainWindow::closeEvent(QCloseEvent *event)
{
    if (gErrorWindow) gErrorWindow->close();

	GraphicsSceneMainWindow::closeEvent(event);
}


//-------------------------------------------------------------------------
QString FaustMainWindow::availableDefaultName(const QString& baseName)
{
	int num = 0;
	QString generatedName;
	while ( (num==0) || QFile::exists( generatedName ) )
	{
		num++;
		generatedName = getFileDialogPath() + "/" + baseName + "_" + QVariant(num).toString() + "." + mSettings.mLanguageFileExtension ;
	}
	return generatedName;
}

//-------------------------------------------------------------------------
QString FaustMainWindow::buildCombinationCode( const QString& file1 , const QString& file2 , const QString& faustOperator )
{
	return "process = component( \"" + QFileInfo(file1).absoluteFilePath() + "\" ) " + faustOperator + " component( \"" + QFileInfo(file2).absoluteFilePath() + "\" );";
}

//-------------------------------------------------------------------------
QList<QFaustItem*> FaustMainWindow::selectedFaustItems()
{
	QList<QGraphicsItem*> selectedItems = mGraphicsScene->selectedItems();
	QList<QFaustItem*> result;
	for ( int i = 0 ; i < selectedItems.size() ; i++ )
	{
		QFaustItem* languageItem = dynamic_cast<QFaustItem*>( selectedItems[i] );
		if ( languageItem )
			result.append( languageItem );
	}
	return result;
}
