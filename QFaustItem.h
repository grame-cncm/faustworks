/*
 * QFaustItem.h
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
 
#ifndef FAUST_ITEM_H
#define FAUST_ITEM_H

#include "QLanguageItem.h"

#include <QProcess>
#include <QTime>
#include <QSvgRenderer>
#include <QPen>
#include <QBrush>
#include <QTextEdit>

#include "QSwitcher.h"

#define FAUST_ITEM_PALETTE	1

#define FAUST_ITEM_BASE_RGB 238,219,0

extern QTextEdit* gErrorWindow;

class QGraphicsSvgItem;

class QBuildItem;

class QFaustItem : public QLanguageItem
{
	Q_OBJECT

	public:

		enum {
			SCRIPT_NO_FILE = 0 ,
			SCRIPT_ERROR,
			SCRIPT_CRASHED,
			SCRIPT_FILE_NOT_FOUND
		};
		
		/**
		*	\brief Constructor.
		*/
		QFaustItem(QGraphicsItem * parent = 0);
		QFaustItem(const QMimeData * mimeData		, QGraphicsItem * parent = 0);
		QFaustItem(const QDomElement * domElement	, QGraphicsItem * parent = 0);
		QFaustItem(const QFaustItem * other			, QGraphicsItem * parent = 0);
		
		~QFaustItem();
		
		void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
		
        void save(const QString& fileName);

		bool	setCode( const QString& code );
		QString	code() const;
		QString cppCode() const;
		
		void checkBinary();
		void setModified( bool isModified );

		QString lastErrorMessage() const;
		bool isValid() const;

		float currentScale() const;
	
	Q_SIGNALS:
	
		void launchScriptError( const QString& command );
		void buildError( int errorType , const QString& msg );

    public Q_SLOTS:
        void runBinary();           // run the binary code of the item
        void exploreSVG ();         // run an external brwoser to explore the SVG block-diagram
        void generateMath ();       // generate mathematical documentation
        void generateLoopGraph ();  // generate internal loop DAG
        void generateSigGraph ();   // generate internal signal graph

		void resized( const QRectF& newRect );

	protected Q_SLOTS:
	
		void buildFinished ( int exitCode, QProcess::ExitStatus exitStatus );
		void updateNameLabel();
	
 	protected:
	
        virtual QMenu * buildContextMenu();
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *  );
	
		void connectNotify (const QMetaMethod &signal);
	
		bool generateSVG();
		bool generateCPP();
		bool generateBinary();
		QString interpretCommand(const QString& command) const;
		
		void setupAnimation();
		
		void faustUpdateGeometry( const QRectF& newGeometry );

		QString svgFolder() const;
		QString svgRootFile() const;
		QString svgRootFileQuoted() const;
		QString cppFile() const;
		QString cppFileQuoted() const;
		QString dspFile() const;
		QString dspFileQuoted() const;
        QString dspFileQuotedSpecial() const;
		QString tempName() const;
		QString itemName() const;
		//QString namedDSPFile() const;
		
		void setFile(const QString& file);
		
		void cleanFiles();
		void cleanWorkingDirectoryFiles();
		void cleanSVGFiles();
		void cleanNamedFiles();
		void cleanBinaries();

		void init();
		void init( const QMimeData * mimeData );
		void init( const QDomElement * e );
		void init( const QFaustItem * other );
		
		/*!
		*	\brief Specialization of QLanguageItem::saveToDomElement
		*/
		QDomElement saveToDomElement( QDomDocument * doc);
		
		/*!
		*	\brief Specialization of QLanguageItem::buildMimeData
		*/
		QMimeData *	buildMimeData();
		
		void updateName();
		void updatePenAndBrush();
		
		/*!
		*	\brief QGraphicsItem standard event handler implementation.
		*/
		QVariant itemChange( GraphicsItemChange change, const QVariant& value );

		/*!
		*	\brief QGraphicsItem standard event handler implementation.
		*/		
		void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
		
		/*!
		*	\brief QGraphicsItem standard event handler implementation.
		*/
		void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
				
		QImage*	buildDragImage();

		/*
		*	\brief Scales and centers the SVG Item, as it's opaque : we have
		*	to diminish its size to see the selection border's pen.
		*/
//		void centerSVGItem();
		
		QGraphicsSvgItem * mSVGItem;
		QSvgRenderer		mSVGRenderer;
		
		QGraphicsSimpleTextItem * mFileNameItem;

		QBuildItem * mBuildAnimationItem;
		
		struct PenBrush {
			QPen mPen;
			QBrush mBrush;
			PenBrush(const QPen& p = QPen() , const QBrush& b = QBrush()) { mPen = p; mBrush = b; }
		};
		QSwitcher<PenBrush> mPenBrushSwitcher;

		//QString mDSPFile;
		QString mFile;
		QString mCode;
		QString mCppCode;
		QString mTempName;
		QString mWorkingDirectory;
		
		QString mItemBuildCommand;
		QString mItemBuildOptions;

		QProcess * mBuildProcess;
		
		int		mItemId;
		QString mIdString;
		
		QString mExName;
		
		QStringList mFiles;
		
		bool mIsValid;
		bool mIsBinaryReady;
		bool mBuildError;
		QStringList mBinaryFiles;
        QTime   mSaveTime;           // updated when saved

// -------------------- Static members --------------------

	public:
		
		static void setFaustPath( const QString& faustPath );

		static void setBuildCommand( const QString& buildCommand );
		static void setBuildOptions( const QString& buildOptions );
		
		enum DroppedFileType 
		{
			DROP_SVG = 0 ,
			DROP_CPP ,
			DROP_DSP ,
			DROP_BINARY
		};
		static void setDroppedFileType( int droppedFileType );

	protected:
	
		static QString mFaustPath;
		static QString mBuildCommand;
		static QString mBuildOptions;
		static int mDroppedFileType;
		static int mCount;
};

#include <QUrl>
/*
#include <QWebView>
#include <QtDebug>
#include <QUrl>
#include <QWebFrame>

class MyWebView : public QWebView
{
	Q_OBJECT

	public : 
	
		MyWebView();

	protected Q_SLOTS :

		void loadIsFinished(bool );

};
*/
#endif //LANGUAGE_ITEM_H
