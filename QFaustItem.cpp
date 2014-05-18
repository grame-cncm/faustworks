/*
 * QFaustItem.cpp
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


#include "QFaustItem.h"

#include <QCoreApplication>
#include <QMimeData>
#include <QGraphicsSvgItem>

#include <QFile>
#include <QDir>
#include <QGraphicsSceneMouseEvent>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QMessageBox>
#include <qmath.h>
#include <QProcess>
#include <QTextStream>
#include <QtDebug>
#include <QPen>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QDesktopServices>
#include <QUrl>
#include <QTextEdit>
#include <QMainWindow>


#include "QPaletteItem.h"

#define PI 3.1415f

#define CPP_EXT QString(".cpp")
#define DSP_EXT	QString(".dsp")
#define SVG_FOLDER_EXT QString("-svg")

//#define FAUST_KEYWORD		"$FAUST"
//#define CPP_FILE_KEYWORD	"$CPP"
#define DSP_FILE_KEYWORD	"$DSP"
//#define FILE_KEYWORD		"$NAME"
#define OPTIONS_KEYWORD		"$OPTIONS"

#define PREFIX				QString("/tmp/")
#define FILE_MENU   "FileMenu"

#define IDLE_FLAG			1
#define HIGHLIGHTED_FLAG	2
#define SELECTED_FLAG		3

#define MIME_FAUST_ITEM_RECT_WIDTH	"MimeFaustItemRectWidth"
#define MIME_FAUST_ITEM_RECT_HEIGHT "MimeFaustItemRectHeight"
#define DOM_FAUST_ITEM_RECT_WIDTH	"DomFaustItemRectWidth"
#define DOM_FAUST_ITEM_RECT_HEIGHT	"DomFaustItemRectHeight"

QTextEdit* gErrorWindow = 0;


static void removeFolder( const QString& folderName );

// Removes recursively any folder or file.
//------------------------------------------------------------
static void remove(const QString& name)
{
    if ( QDir( name ).exists() ) {
		removeFolder(name);
    } else if ( QFile::exists(name) ) {
        QFile::remove(name);
	}
}

//------------------------------------------------------------
static void removeFolder( const QString& folderName )
{
	QDir dir( folderName );
	QStringList files = dir.entryList();
	for (int i = 0; i < files.size(); ++i)
	{
		if ( ( files[i] != "." ) && ( files[i] != ".." ) )
			remove( folderName + "/" + files[i] );
	}
	QDir d;
	d.rmdir ( folderName );
}

//------------------------------------------------------------
static void copyFolder( const QString& folderName , const QString& newName )
{
	removeFolder( newName );

	QDir dir( folderName );
	QStringList files = dir.entryList();
	
	QDir newDir;
	newDir.mkdir (newName);
	for (int i = 0; i < files.size(); ++i)
	{
		QFile::copy( folderName + "/" + files[i] , newName + "/" + files[i] );
	}
}

//------------------------------------------------------------------------
//								QBuildItem class						//
//------------------------------------------------------------------------

class QBuildItem : public QGraphicsRectItem
{
	
	public :
	
		QBuildItem( float ballSize , float radius , int ballNb , int rotationPeriod , QGraphicsItem * parent = 0 ) : QGraphicsRectItem( parent )
		{
			float rectSize = radius * 2 + ballSize;
			setRect( QRectF( -rectSize/2.0f , -rectSize/2.0f , rectSize , rectSize ) );

			setPen( Qt::NoPen );
			setBrush( Qt::NoBrush );

			for ( int i = 0 ; i < ballNb ; i++ )
			{
				QAbstractGraphicsShapeItem * ball = new QGraphicsEllipseItem( -ballSize/2.0f, -ballSize/2.0f, ballSize , ballSize );
				ball->setParentItem( this );		
				ball->setPos( radius * qCos( i * ( 2*PI / float(ballNb) ) ) , radius * qSin( i * ( 2*PI / float(ballNb) ) ) );
				ball->setPen( Qt::NoPen );
				ball->setBrush( QBrush( Qt::red ) );
				ball->setVisible( false );

				mBalls << ball;
			}

			int frameCount = int(rotationPeriod / 100.0f);
			mAnimationTimeLine = new QTimeLine(rotationPeriod);
			mAnimationTimeLine->setFrameRange(0,  frameCount );
			mAnimationTimeLine->setLoopCount( 0 );
			mAnimationTimeLine->setCurveShape( QTimeLine::LinearCurve );

			mAnimation = new QGraphicsItemAnimation();
			mAnimation->setItem(this);
			mAnimation->setTimeLine(mAnimationTimeLine);

			for (int i = 0; i < frameCount; ++i)
				mAnimation->setRotationAt(i / float(frameCount) , i * 360.0f / float(frameCount) );

			QGraphicsEllipseItem * ellipseItem = new QGraphicsEllipseItem( rect() );
			ellipseItem->setParentItem( this );
			ellipseItem->setPen( Qt::NoPen );
			ellipseItem->setBrush( QBrush( Qt::green ) );
			ellipseItem->setVisible( false );
			
			mIdleItem = ellipseItem;
		}
		
		~QBuildItem()
		{
			delete mAnimation;
			delete mAnimationTimeLine;
		}
		
		void start()
		{
//			mIdleItem->setVisible( false );
			showBalls( true );
			mAnimationTimeLine->start();
		}
		
		void done()
		{
//			mIdleItem->setVisible( true );
			showBalls( false );		
			mAnimationTimeLine->stop();
		}
		
		void error()
		{
//			mIdleItem->setVisible( true );
			showBalls( false );		
			mAnimationTimeLine->stop();
		}
		
	protected :
	
		void showBalls( bool areShown )
		{
			for ( int i = 0 ; i < mBalls.size() ; i++ )
				mBalls[i]->setVisible( areShown );
		}
	
		QTimeLine * mAnimationTimeLine;
		QGraphicsItemAnimation * mAnimation;
		QList<QGraphicsItem*> mBalls;
		
		QGraphicsItem * mIdleItem;
};

//------------------------------------------------------------------------
//								Public functions						//
//------------------------------------------------------------------------

//-------------------------------------------------------------------------
QFaustItem::QFaustItem(QGraphicsItem * parent) : QLanguageItem( parent )
{
	init();
}

//-------------------------------------------------------------------------
QFaustItem::QFaustItem(const QMimeData * mimeData		, QGraphicsItem * parent )
 : QLanguageItem( parent )
{
	init(mimeData);
}

//-------------------------------------------------------------------------
QFaustItem::QFaustItem(const QDomElement * domElement	, QGraphicsItem * parent )
 : QLanguageItem( parent )
{
	init(domElement);
}

//-------------------------------------------------------------------------
QFaustItem::QFaustItem(const QFaustItem * other			, QGraphicsItem * parent )
 : QLanguageItem( parent )
{
	init(other);
}

//-------------------------------------------------------------------------
QFaustItem::~QFaustItem()
{
	cleanFiles();
}

//------------------------------------------------------------
void QFaustItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * , QWidget *)
{
//	qDebug( "QFaustItem::paint : %02X" , this);
	painter->setPen( pen() );
	painter->setBrush( brush() );
	painter->drawRect( rect() );
}

//------------------------------------------------------------
bool QFaustItem::setCode( const QString& code )
{
//	qDebug() << "QFaustItem::setCode : code = " << code;

	if ( code == mCode )
		return true;

	mCode = code;

//	qDebug() << "QFaustItem::setCode : " << dspFile();
	QFile * file0 = new QFile( dspFile() );
    if (!file0->open(QIODevice::WriteOnly | QIODevice::Text))
		return false;
	
	QTextStream out(file0);
	out << mCode;
	delete file0;

#ifdef WIN32
	//This is a bug :
	//On Windows, we need to call the svg generation command twice,
	//the first one to create the folder, the second one to create
	//the SVG files.
	if ( !generateSVG() )
		return false;
#endif

	if ( !generateSVG() )
		return false;

	if ( mIsValid )
	{	
		mSVGRenderer.load( svgRootFile() );
		mSVGItem->setSharedRenderer(&mSVGRenderer);
		faustUpdateGeometry( rect() );
		//faustUpdateGeometry( QRect( 0 , 0 , mSVGItem->boundingRect().width() * mSVGItem->transform().m11() , mSVGItem->boundingRect().height() * mSVGItem->transform().m22() ) );
		
		generateCPP();
	}

	codeChanged();
	
	return isValid();
}

//------------------------------------------------------------
QString	QFaustItem::code() const
{
	return mCode;
}

//------------------------------------------------------------
QString QFaustItem::lastErrorMessage() const
{
    return "Invalid Faust Code";
}

//------------------------------------------------------------
bool QFaustItem::isValid() const
{
	return mIsValid;
}

//------------------------------------------------------------------------
//								Protected functions						//
//------------------------------------------------------------------------
//------------------------------------------------------------

void QFaustItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
    runBinary();
}

//------------------------------------------------------------
void QFaustItem::connectNotify (const QMetaMethod &signal)
{
	// It is possible that the launchScriptError signal is emitted
	// before that the connection is made, so we re-emit it if necessary.
// 	if (signal == SIGNAL(launchScriptError(const QString&)))
// 	{
// 		if ( mBuildError )
// 			Q_EMIT launchScriptError( mItemBuildCommand );
// 	}
}

/**
 * Run the binary code associated to a Faust item (if any)
 */
//------------------------------------------------------------
void QFaustItem::runBinary()
{
    
    if (mBinaryFiles.size() > 0) {
		QString bin = mBinaryFiles[0];

		//Little hack to use .app on MacosX.
        qDebug () << bin;
        if ( bin.right(4) == ".app" )   {
            bin = "open " + bin ;
		}

        QProcess::startDetached ( bin);
    } else {
        QMessageBox msgBox;
        msgBox.setText("Faust Item has no binary to execute");
        msgBox.exec();
    }
}

/**
 * Explore the SVG block-diagram using the appropriate desktop application
 */

void QFaustItem::exploreSVG ()
{
    QString filename = QFileInfo(svgRootFile()).absoluteFilePath();
    QUrl url = QUrl::fromLocalFile(filename);
    qDebug() << "QDesktopServices::openUrl (" << url << ")" ;
    bool b = QDesktopServices::openUrl(url);
    if (!b) {
       qDebug() << "ERROR : Can't open the SVG URL " << url ;
    }
}



/**
 * Call external script to generate and view mathematical documentation
 */
void QFaustItem::generateMath ()
{
    QProcess qproc;
    QString cmd =  "faust2mathviewer " + dspFileQuotedSpecial();
    bool b = qproc.startDetached(cmd);
    qDebug() << cmd;
    if (!b) {
            qDebug() << "ERROR : Can't generate math doc " ;
    }

}



/**
 * Call external script to generate and view internal Loop DAG
 */
void QFaustItem::generateLoopGraph ()
{
    QProcess qproc;
    QString cmd =  "faust2graphviewer " + dspFileQuotedSpecial();
    bool b = qproc.startDetached(cmd);
    qDebug() << cmd;
    if (!b) {
            qDebug() << "ERROR : Can't generate DAG view " ;
    }

}



/**
 * Call external script to generate and view internal signal directed graph
 */
void QFaustItem::generateSigGraph ()
{
    QProcess qproc;
    QString cmd =  "faust2sigviewer " + dspFileQuotedSpecial();
    bool b = qproc.startDetached(cmd);
    qDebug() << cmd;
    if (!b) {
            qDebug() << "ERROR : Can't generate signal graph view " ;
    }

}

//------------------------------------------------------------
void QFaustItem::resized( const QRectF& newRect )
{
//	qDebug() << "QFaustItem::resized : " << rect() << " -> " << newRect;

	if ( newRect.toRect() == rect().toRect() )
		return;

	moveBy( newRect.x() , newRect.y() );
	faustUpdateGeometry( QRect(0,0, newRect.width(), newRect.height()));
	Q_EMIT scaleChanged( currentScale() );
}

extern QMainWindow* gMainWin;

//------------------------------------------------------------
bool QFaustItem::generateSVG()
{
//	qDebug() << "QFaustItem::generateSVG() : " << mFaustPath + " -svg " + dspFileQuoted();

    QProcess faustProcess;
    faustProcess.start( mFaustPath + " -svg -blur " + " " + dspFileQuoted() );

    if (!faustProcess.waitForStarted())
        return false;   
        
    if (!faustProcess.waitForFinished())
        return false;
        
    mIsValid = ( faustProcess.exitCode() == 0 );

    // traitement des erreurs

    if (!gErrorWindow) {
        gErrorWindow = new QTextEdit(gMainWin);
        gErrorWindow->setWindowFlags(Qt::WindowStaysOnTopHint|Qt::Tool );
        gErrorWindow->setFontPointSize(10);
        gErrorWindow->setTextInteractionFlags(Qt::NoTextInteraction);
        gErrorWindow->ensureCursorVisible();
        //gErrorWindow->setWindowTitle("Invalid Faust Code");
    }
    if (faustProcess.exitCode()) {
        gErrorWindow->setWindowTitle(tr("Invalid Faust Code"));
        gErrorWindow->show();
        gErrorWindow->append(QString(faustProcess.readAllStandardError()));
        gErrorWindow->append("--------------------\n");
        qDebug() << "des erreurs";
        //qDebug() << faustProcess.readAllStandardError();
    } else {
        gErrorWindow->setWindowTitle(tr("Faust Code OK"));
    }

    return true;
}

//-------------------------------------------------------------------------
static QString getFileContent(const QString& fileName)
{
	if ( !QFile::exists(fileName) )
		return "";
		
	QString content = "";
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file.close();
		return "";
	}

    while (!file.atEnd()) 
	{
        QByteArray line = file.readLine();
        content += QString::fromUtf8(line);
    }
	file.close();
	return content;
}


//------------------------------------------------------------
bool QFaustItem::generateCPP()
{
	QProcess faustProcess;
	mItemBuildOptions = QFaustItem::mBuildOptions;
	faustProcess.start( mFaustPath + " -o " + cppFileQuoted() + " " + QFaustItem::mBuildOptions + " " + dspFileQuoted() );

	if (!faustProcess.waitForStarted())
	{
		mCppCode = "";
		return false;	
	}

	if (!faustProcess.waitForFinished())
	{
		mCppCode = "";
		return false;
	}
		
	mCppCode = getFileContent( cppFile() );
	
	return true;
}

//------------------------------------------------------------
void QFaustItem::checkBinary()
{
	if ( interpretCommand( QFaustItem::mBuildCommand ) != mItemBuildCommand )
	{
		if ( !isModified() )
		{
			generateBinary();
		}
	}
	if ( QFaustItem::mBuildOptions != mItemBuildOptions )
	{
		generateCPP();
	}
}

//------------------------------------------------------------
void QFaustItem::setModified( bool modified )
{	
	if ( isModified() != modified )
	{
		QLanguageItem::setModified( modified );
		
		if ( modified )
			cleanBinaries();
		else
			generateBinary();
	}
}

//------------------------------------------------------------
bool QFaustItem::generateBinary()
{
	if ( file().length() == 0 )
		return true;
		
	if ( code().length() == 0 )
		return true;
	
	if ( mBuildCommand.length() == 0 )
		return true;

	if ( mBuildProcess )
	{
		mBuildProcess->waitForFinished();
	}

	cleanBinaries();
	mIsBinaryReady = false;
	mBuildProcess = new QProcess();
	connect( mBuildProcess , SIGNAL( finished ( int , QProcess::ExitStatus ) ) , this , SLOT( buildFinished ( int , QProcess::ExitStatus ) ) );

    qDebug() << "mItemBuildCommand[1] = " << mItemBuildCommand;

    mItemBuildCommand = interpretCommand( mBuildCommand );

    qDebug() << "mItemBuildCommand[2] = " << mItemBuildCommand;

	mBuildProcess->setWorkingDirectory(mWorkingDirectory);
	
	mBuildProcess->start( mItemBuildCommand );

	if (!mBuildProcess->waitForStarted())
	{
		Q_EMIT launchScriptError( mItemBuildCommand );
		mBuildError = true;
		delete mBuildProcess;
		mBuildProcess = 0;
		return false;
	}
	mBuildError = false;
	
	mBuildAnimationItem->start();

	return true;
}

//------------------------------------------------------------
QVariant QFaustItem::itemChange( GraphicsItemChange change, const QVariant& value )
{
	QVariant result = QLanguageItem::itemChange( change , value );
	
	if ( change == QGraphicsItem::ItemSelectedHasChanged )
	{
		if ( value.toBool() )
		{
			checkBinary();
		}
		mPenBrushSwitcher.setFlag(SELECTED_FLAG, value.toBool() );
		updatePenAndBrush();
	}

	return result;
}

//------------------------------------------------------------
void QFaustItem::hoverEnterEvent ( QGraphicsSceneHoverEvent * )
{
	mPenBrushSwitcher.setFlag(HIGHLIGHTED_FLAG, true);
	updatePenAndBrush();
}

//------------------------------------------------------------
void QFaustItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent * )
{
	mPenBrushSwitcher.setFlag(HIGHLIGHTED_FLAG, false);
	updatePenAndBrush();
}

//------------------------------------------------------------
QImage*	QFaustItem::buildDragImage()
{
	return new QImage( itemToImage( this , 1.0f , 0 , true ) );
}

//------------------------------------------------------------
QMimeData *	QFaustItem::buildMimeData()
{
	QMimeData * result = QLanguageItem::buildMimeData();
	result->setData( MIME_FAUST_ITEM_RECT_WIDTH , QByteArray::number(rect().width()) );
	result->setData( MIME_FAUST_ITEM_RECT_HEIGHT , QByteArray::number(rect().height()) );

	if ( !mIsStorageModeOn )
	{
		QStringList copiedFile;
		QString extension;
		bool isDir = false;
		switch ( QFaustItem::mDroppedFileType )
		{
		case DROP_CPP:
			copiedFile << cppFile();
			extension = CPP_EXT;
			break;
		case DROP_SVG:
			copiedFile << svgFolder();
			extension = SVG_FOLDER_EXT;
			isDir = true;
			break;
		case DROP_BINARY:
		{
			if ( mIsBinaryReady )
			{
				QList<QUrl> binariesUrls;
				for ( int i = 0 ; i < mBinaryFiles.size() ; i++ )
				{
					binariesUrls << QUrl::fromLocalFile( QFileInfo( mBinaryFiles[i] ).absoluteFilePath() );
				}
				result->setUrls( binariesUrls );
			}
			return result;
		}
		case DROP_DSP:
			copiedFile << dspFile();
			extension = DSP_EXT;
			break;
		}

		QList<QUrl> urls;
		for ( int i = 0 ; i < copiedFile.size() ; i++ )
		{
			QString fileName = PREFIX + mIdString + "/" + itemName() + extension;
			if ( !isDir )
			{
				QFile::remove( fileName );
				QFile::copy( copiedFile[i] , fileName );
			}
			else
				copyFolder( copiedFile[i] , fileName );

			urls << QUrl::fromLocalFile( QFileInfo( fileName ).absoluteFilePath() );
		}

		result->setUrls( urls );
	}
	return result;
}

//------------------------------------------------------------
float QFaustItem::currentScale() const
{
	return mSVGItem->transform().m11();
}

/**
 * Fonction called at the end of the build process that checks if all the resulting files
 * have been properly created.
 */
void QFaustItem::buildFinished ( int exitCode, QProcess::ExitStatus exitStatus )
{
//	qDebug() << "QFaustItem::buildFinished";
	if ( exitStatus == QProcess::NormalExit )
	{
		if ( exitCode == 0 )
		{
//			qDebug() << "QFaustItem::buildFinished : return 0";
			
			mIsBinaryReady = true;
			mBuildAnimationItem->done();


            //----- list files names
            {
                mBinaryFiles = QStringList();
                QString s =  mBuildProcess->readAllStandardOutput();
                int     n = s.length();
                QString f;          //filename under construction
                int     i = 0;      // current position
                int     state = 0;  // state of the state-machine

                while(i<n) {
                    switch (state) {
                    case 0 :
                        if (s[i].isSpace() || (s[i] == ';')) {
                            // skip leading spaces and empty fields
                            i++; break;
                        } else {
                            // begin of a file name
                            f = s[i];
                            state = 1;
                            i++; break;
                        }
                    case 1 :
                        if (s[i] != ';') {
                            // the filename continues
                            f += s[i];
                            i++; break;
                        } else {
                            mBinaryFiles << f;
                            state = 0;
                            i++; break;
                        }

                    }
                }

                // handle case of a last filename without ';' at the end
                if (state==1) mBinaryFiles << f;

            }


			if ( mBinaryFiles.size() == 0 )
			{
                qDebug() << "QFaustItem::buildFinished : no file";
				Q_EMIT buildError( SCRIPT_NO_FILE , "" );
			}
			else
			{
                qDebug() << "QFaustItem::buildFinished : check files";
				QString missingFiles;
				QString sep = "";
				for ( int i = 0 ; i < mBinaryFiles.size() ; i++ )
				{
                    qDebug() << "QFaustItem::buildFinished : checking " << mBinaryFiles[i] <<"...";
                    if ( QDir(mBinaryFiles[i]).exists() ) {
                        qDebug() << "QFaustItem::buildFinished : it's a directory " << mBinaryFiles[i] <<"...";
                    }
					if ( !QFile::exists( mBinaryFiles[i] ) && !QDir(mBinaryFiles[i]).exists() )
					{
						missingFiles += ( sep + mBinaryFiles[i] );
						sep = ";";
					}
				}
				if ( missingFiles.size() )
				{
					Q_EMIT buildError(SCRIPT_FILE_NOT_FOUND, missingFiles );
				}
			}
			
//			mFiles << mBinaryFiles;
		}
		else
		{
			// Error with build.
			mBuildAnimationItem->error();
			
			Q_EMIT buildError( SCRIPT_ERROR , mBuildProcess->readAllStandardError() );
		}
	}
	else
	{
		//Error with executable.
		mBuildAnimationItem->error();
//		qDebug() << "QFaustItem::buildFinished : error";

		Q_EMIT buildError( SCRIPT_CRASHED , mBuildProcess->readAllStandardError() );
	}
	delete mBuildProcess;
	mBuildProcess = 0;
}

#define TITLE_BAR_TEXT_MARGIN 5
//------------------------------------------------------------
void QFaustItem::updateNameLabel()
{
	if ( !mIsStorageModeOn )
	{
		QString baseName = getDescriptiveFileName();
		mFileNameItem->setText( baseName );
		QString truncString = "...";
		int skippedChar = truncString.length() + 1;
		while ( ( mFileNameItem->boundingRect().width() > rect().width() - TITLE_BAR_TEXT_MARGIN ) && ( skippedChar < baseName.length() ) )
		{
			mFileNameItem->setText( baseName.left( baseName.length() - skippedChar ) + truncString );
			skippedChar++;
		}
		mFileNameItem->setPos( ( rect().width() - mFileNameItem->boundingRect().width() )/2.0f , 0 );
	}
}

/*
//------------------------------------------------------------
void QFaustItem::centerSVGItem()
{

	QRectF r = mapFromItem( mSVGItem , mSVGItem->boundingRect() ).boundingRect();
	float xScaleFactor = ( ( r.width() - mPenWhenSelected.width() ) / r.width() );
	float yScaleFactor = ( ( r.height() - mPenWhenSelected.width() ) / r.height() );
	mSVGItem->scale(xScaleFactor / mSVGItem->transform().m11() , yScaleFactor / mSVGItem->transform().m22() );
	float halfPenWidth = mPenWhenSelected.width() / 2.0f;
	mSVGItem->setPos( halfPenWidth , halfPenWidth );
}
*/

//------------------------------------------------------------
QString QFaustItem::svgFolder() const
{
        QString p = mWorkingDirectory + "/" + tempName() + SVG_FOLDER_EXT;
        qDebug() << "QFaustItem::svgFolder()" << p;
        return p;
}

//------------------------------------------------------------
QString QFaustItem::svgRootFile() const
{
	return svgFolder() + "/process.svg";
}

//------------------------------------------------------------
QString QFaustItem::svgRootFileQuoted() const
{
    QString p = "\"" + svgRootFile() + "\"";
    return p;
}

//------------------------------------------------------------
QString QFaustItem::cppFile() const
{
	return mWorkingDirectory + "/" + tempName() + CPP_EXT;
}

//------------------------------------------------------------
QString QFaustItem::cppFileQuoted() const
{
	return "\"" + cppFile() + "\"";
}

//------------------------------------------------------------
QString QFaustItem::dspFile() const
{
        return mWorkingDirectory + "/" + tempName() + DSP_EXT;
//	return mWorkingDirectory + "/" + tempName();
}

//------------------------------------------------------------
QString QFaustItem::dspFileQuoted() const
{
    return "\"" + dspFile() + "\"";
}

//------------------------------------------------------------
// If file modified but not saved, uses tmp filename otherwise
// uses real filename
QString QFaustItem::dspFileQuotedSpecial() const
{
    if (isModified()) {
        return "\"" + dspFile() + "\"";
    } else {
        return "\"" + file() + "\"";
    }
}

//------------------------------------------------------------
QString QFaustItem::tempName() const
{
	return mTempName;
}

//------------------------------------------------------------
QString QFaustItem::itemName() const
{
	if ( !file().length() )
		return "untitled";

	return QFileInfo( file() ).completeBaseName();
}

////------------------------------------------------------------
//QString QFaustItem::namedDSPFile() const
//{
//	return PREFIX + mIdString + "/" + itemName() + DSP_EXT;
//}
		
//------------------------------------------------------------
QString QFaustItem::cppCode() const
{
	return mCppCode;
}

//------------------------------------------------------------
void QFaustItem::setFile(const QString& f )
{
	mExName = itemName();

	QLanguageItem::setFile( f );

	mWorkingDirectory = QFileInfo( f ).dir().absolutePath();
	updateName();
	
	//Binaries only available for non-modified file-linked items.
	if ( ! (file().length()) )
		cleanBinaries();
}

/*
//------------------------------------------------------------
void QFaustItem::cleanFiles()
{
	cleanWorkingDirectoryFiles();
	cleanNamedFiles();
}
*/

//------------------------------------------------------------
void QFaustItem::cleanFiles()
{
	//Remove binaries
	cleanBinaries();
	
	//Remove all tmp files : tmp.dsp, svg, cpp, ...
	for ( int i = 0 ; i < mFiles.size() ; i++ )
	{
		remove( mFiles[i] );
	}
	removeFolder( PREFIX + mIdString );
}

//------------------------------------------------------------
void QFaustItem::cleanBinaries()
{
	for ( int i = 0 ; i < mBinaryFiles.size() ; i++ )
	{
		remove( mBinaryFiles[i] );
	}
}

//------------------------------------------------------------
void QFaustItem::init()
{
	mSVGItem = new QGraphicsSvgItem(this);
	mSVGItem->setSharedRenderer( &mSVGRenderer );
//#if !linux		// bug in Qt 4.4 with the linux cache mode
	mSVGItem->setCacheMode( QGraphicsItem::DeviceCoordinateCache );
//#endif

	mWorkingDirectory = PREFIX + ".";

	mItemId = QFaustItem::mCount++;
	updateName();
	
	mBuildError = false;

	mBuildProcess = 0;

	setupAnimation();

	connect( this, SIGNAL( descriptiveNameChanged() ), this , SLOT( updateNameLabel() ) );

    setAcceptHoverEvents(true);
//	mPenBrushSwitcher.addFlag(IDLE_FLAG,		0, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,50) , 3) ,		QBrush(QColor(FAUST_ITEM_BASE_RGB,50)) ) );
//	mPenBrushSwitcher.addFlag(HIGHLIGHTED_FLAG, 1, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,100) , 3) ,	QBrush(QColor(FAUST_ITEM_BASE_RGB,100)) ) );
//	mPenBrushSwitcher.addFlag(SELECTED_FLAG,	2, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,255) , 3) ,	QBrush(QColor(FAUST_ITEM_BASE_RGB,100)) ) );
    //mPenBrushSwitcher.addFlag(IDLE_FLAG,		0, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,50) , 3) ) );
    mPenBrushSwitcher.addFlag(IDLE_FLAG,		0, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,0) , 3) ) );       // pas de bord jaune quand l'item n'est pas selectionné
    mPenBrushSwitcher.addFlag(HIGHLIGHTED_FLAG, 1, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,100) , 3) ) );     // le bord devient visible quand on passe sur l'item
	mPenBrushSwitcher.addFlag(SELECTED_FLAG,	2, PenBrush( QPen(QColor(FAUST_ITEM_BASE_RGB,255) , 3) ) );

	mPenBrushSwitcher.setFlag(IDLE_FLAG, true);
	updatePenAndBrush();

	mFileNameItem = new QGraphicsSimpleTextItem(this);
//	mFileNameItem = new QTextPathItem( getDescriptiveFileName() , true , Qt::black , "Arial" , 1.0f , this );
	mFileNameItem->setZValue(2);

	QLanguageItem::plug( new QPaletteItemDropper( FAUST_ITEM_PALETTE , QList<int>() << FAUST_ITEM_PALETTE , this , this ) );

	faustUpdateGeometry( QRect(0,0,100,100) );
}

//------------------------------------------------------------
void QFaustItem::init(const QMimeData * mimeData)
{
	init();
	QLanguageItem::load( mimeData );
	
	// Load specific QFaustItem data
	resized( QRect(	0,0,
					mimeData->data( MIME_FAUST_ITEM_RECT_WIDTH ).toFloat(),
					mimeData->data( MIME_FAUST_ITEM_RECT_HEIGHT ).toFloat()
					) );
}

//------------------------------------------------------------
void QFaustItem::init(const QDomElement * domElement)
{
	init();
	QLanguageItem::load( domElement );

	// Load specific QFaustItem data	
	resized( QRect(	0,0,
					QVariant( domElement->attribute( DOM_FAUST_ITEM_RECT_WIDTH, "100" )).toDouble(),
					QVariant( domElement->attribute( DOM_FAUST_ITEM_RECT_HEIGHT, "100" )).toDouble()
					) );
}

//------------------------------------------------------------
void QFaustItem::init(const QFaustItem * other)
{
	init();
	QLanguageItem::load( other );
	
	// Load specific QFaustItem data
	resized( other->rect() );
}

//------------------------------------------------------------
QDomElement QFaustItem::saveToDomElement( QDomDocument * doc)
{
	QDomElement result = QLanguageItem::saveToDomElement(doc);

	result.setAttribute( DOM_FAUST_ITEM_RECT_WIDTH, rect().width() );
	result.setAttribute( DOM_FAUST_ITEM_RECT_HEIGHT, rect().height() );

	return result;
}

//------------------------------------------------------------
void QFaustItem::updateName()
{
	if ( !QDir( PREFIX ).exists() )
	{
		qDebug() << "Make dir" << PREFIX ;
		QDir().mkdir( PREFIX );
	}
	else
	{
		qDebug() << PREFIX << " already exists";
	}

//	qDebug() << "QFaustItem::updateName() : ex : " << mExName << " to " << itemName();
	QString exIdString = mIdString;

	mIdString = itemName() + "." + QVariant( mItemId ).toString();
        mTempName = mIdString + ".tmp" /* + DSP_EXT*/;
	
	if ( !QDir( PREFIX + mIdString ).exists() )
		QDir().mkdir( PREFIX + mIdString );
	
	QStringList exFiles = mFiles;
//	qDebug() << "exFiles : " << exFiles;
	
/*	
	for ( int i = 0 ; i < mBinaryFiles.size() ; i++ )
	{
		mBinaryFiles[i] = mBinaryFiles[i].replace( mExName , itemName() );
	}
*/
	mFiles.clear();
	mFiles
	//D&D files
	<< PREFIX + mIdString + "/" + itemName() + DSP_EXT
	<< PREFIX + mIdString + "/" + itemName() + CPP_EXT
	<< PREFIX + mIdString + "/" + itemName() + SVG_FOLDER_EXT
	//Tmp dsp
	<< dspFile()
	//Generated files from tmp Dsp
	<< cppFile()
	<< svgFolder()
//	//Binaries
//	<< mBinaryFiles
	;

//	qDebug() << "mFiles : " << mFiles;

	for ( int i = 0 ; i < exFiles.size() ; i++ )
	{
		if ( QDir(exFiles[i]).exists() )
		{
			//Is a directory
//			qDebug() << "QFaustItem::updateName() : dir renamed " << exFiles[i] << " -> " << mFiles[i];
			QDir().rename( exFiles[i] , mFiles[i] );
		}
		else
		{
//			qDebug() << "QFaustItem::updateName() : file renamed " << exFiles[i] << " -> " << mFiles[i];
			QFile::rename( exFiles[i] , mFiles[i] );
		}
	}

	if ( QDir(PREFIX + exIdString).exists() && exIdString.length() )
	{
//		qDebug() << "QFaustItem::updateName() : removed : " << PREFIX + exIdString;
		removeFolder( PREFIX + exIdString );
	}
}


//------------------------------------------------------------
void QFaustItem::updatePenAndBrush()
{
	setPen( mPenBrushSwitcher.activeObject().mPen );
	setBrush( mPenBrushSwitcher.activeObject().mBrush );
}

//------------------------------------------------------------
// Create the shell command that translates the dsp code
// into binary code
QString QFaustItem::interpretCommand(const QString& command) const
{
    QString result;
//    if (command.startsWith("/"))
//    {
//        result = command;
//    } else {
//        result = QCoreApplication::applicationDirPath() + "/" + command;
//    }

    result = command;
    result.replace( DSP_FILE_KEYWORD ,	"\""+file()+"\"" );
	result.replace( OPTIONS_KEYWORD , QFaustItem::mBuildOptions );
    //QFileInfo fileInfo( file() );
    //QString f = file().length() ? fileInfo.absolutePath() + fileInfo.completeBaseName() : "untitled";

	return result;
}

//------------------------------------------------------------
void QFaustItem::setupAnimation()
{
	int ballSize = 4;
	int radius = 5;
	int ballNb = 5;
	mBuildAnimationItem = new QBuildItem( ballSize , radius , ballNb , 2000 , this );
	mBuildAnimationItem->setZValue( 10 );
}

#define SVG_BORDER_MARGIN 5
//-------------------------------------------------------------------------
void QFaustItem::faustUpdateGeometry( const QRectF& newGeometry )
{
//	qDebug() << "QFaustItem::faustUpdateGeometry : " << newGeometry;
//	qDebug() << "QFaustItem::faustUpdateGeometry r = : " << mSVGItem->boundingRect();
	// QLanguageItem function: setRect, and update related children items' geometry.
	QLanguageItem::updateGeometry( newGeometry );

	// Scale the mSGVItem to fit inside the newGeometry.
	if ( mSVGItem->boundingRect().isValid() )
	{
		QRectF itemGeometry = mapFromItem( mSVGItem , mSVGItem->boundingRect() ).boundingRect();
		float heightRatio = float(rect().height() - 2*SVG_BORDER_MARGIN) / itemGeometry.height() ;
		float widthRatio = float(rect().width() - 2*SVG_BORDER_MARGIN) / itemGeometry.width();

		float minRatio = qMin( heightRatio , widthRatio );

//		qDebug() << "QFaustItem::faustUpdateGeometry : transform().m11() = "<< mSVGItem->transform().m11();
        //mSVGItem->scale( minRatio , minRatio );
        mSVGItem->setTransform(QTransform::fromScale(minRatio, minRatio), true);
        itemGeometry = mapFromItem( mSVGItem , mSVGItem->boundingRect() ).boundingRect();
		float emptyWidth = rect().width() - itemGeometry.width();
		float emptyHeight = rect().height() - itemGeometry.height();
		mSVGItem->setPos( emptyWidth/2.0f , emptyHeight/2.0f );
//		qDebug() << "QFaustItem::faustUpdateGeometry : transform().m11() = "<< mSVGItem->transform().m11();
	}
		
	// Move the mBuildAnimationItem to the top-right corner.
	mBuildAnimationItem->setPos( newGeometry.width() - mBuildAnimationItem->boundingRect().width()/2.0f , newGeometry.height() - mBuildAnimationItem->boundingRect().height()/2.0f );
	
	// Updates the item name label (center it).
	updateNameLabel();
}

//-------------------------------------------------------------------------
//								Static members							///
//-------------------------------------------------------------------------

QString QFaustItem::mFaustPath = "";
QString QFaustItem::mBuildCommand = "";
QString QFaustItem::mBuildOptions = "";
int QFaustItem::mDroppedFileType = QFaustItem::DROP_BINARY;
int QFaustItem::mCount = 0;


//------------------------------------------------------------
void QFaustItem::setFaustPath( const QString& faustPath )
{
	mFaustPath = faustPath;
}

//------------------------------------------------------------
void QFaustItem::setBuildCommand( const QString& buildCommand )
{
	mBuildCommand = buildCommand;
}

//------------------------------------------------------------
void QFaustItem::setBuildOptions( const QString& buildOptions )
{
	mBuildOptions = buildOptions;
}

//------------------------------------------------------------
void QFaustItem::setDroppedFileType( int droppedFileType )
{
	mDroppedFileType = droppedFileType;
}


//-------------------------------------------------------------------------

QMenu* QFaustItem::buildContextMenu()
{
    QMenu * m = new QMenu();

    m->addAction( tr("Save"), this , SLOT(saveItem()) );
    m->addAction( tr("Save as..."), this , SIGNAL(saveItemAs()) );
    //m->addAction( "Show Faust code", this , SIGNAL(showFaustCode()) );
    //m->addAction( "Show C++ code", this , SIGNAL(showCppCode()) );
    m->addAction( tr("Browse Diagram"), this , SLOT(exploreSVG()) );
    m->addAction( tr("Generate Math"), this , SLOT(generateMath()) );
    m->addAction( tr("Generate Loop Graph"), this , SLOT(generateLoopGraph()) );
    m->addAction( tr("Generate Signal Graph"), this , SLOT(generateSigGraph()) );
    m->addAction( tr("Run Binary"),     this , SLOT(runBinary()), QKeySequence(Qt::CTRL + Qt::Key_R) );

    return m;
}

//------------------------------------------------------------
void QFaustItem::save(const QString& fileName)
{
	QLanguageItem::save(fileName);
    mSaveTime = QTime::currentTime();
//    generateBinary();
}


//-------------------------------------------------------------------------
//								MyWebView class							///
//-------------------------------------------------------------------------
/*
//------------------------------------------------------------
MyWebView::MyWebView() 
{
//	setWindowModality( Qt::WindowModal );
//	setSizePolicy( QSizePolicy::Fixed , QSizePolicy::Fixed );
	connect( this , SIGNAL( loadFinished ( bool ) ) , this , SLOT( loadIsFinished(bool) ) );
}

#include <QSvgWidget>
#define SVG_MARGIN QSize(30,30)
//------------------------------------------------------------
void MyWebView::loadIsFinished(bool )
{
	QSvgWidget w( page()->mainFrame()->url().toLocalFile() );
	resize(w.sizeHint() * 1.1f );
}
*/
