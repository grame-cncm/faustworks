/*
 * FaustMainWindow.h
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
 
#ifndef FAUST_MAINWINDOW_H
#define FAUST_MAINWINDOW_H

#define FAUST_PATH_SETTING	"faustPath"
#define TARGETS_SETTING		QString("targetsSetting")
#define OPTIONS_SETTING		QString("optionsSetting")

#include <QAction>
#include <QDockWidget>
#include <QComboBox>
#include <QToolBar>

#include "GraphicsSceneMainWindow.h"

class QFaustItem;

class FaustMainWindow : public GraphicsSceneMainWindow
{
	Q_OBJECT

	public:
	
		FaustMainWindow();
		
		void reinitPreferencesSettings();
		
	protected slots:
	
		void preferences();
		void updateCode();
		void targetArchitectureChanged(int index);
		void buildOptionsChanged(int index);
		void droppedFileTypeChanged(int droppedFileTypeIndex);
		void itemLaunchScriptError(const QString& command);
		void itemBuildError(int errorType , const QString& msg);
		void combineItems(QGraphicsItem* dropped,QGraphicsItem* target,int,int,int interactionId);

	protected:
	
		void init();
		void reinitSettings();
		void readSettings();
		void readPreferencesSettings();
		void updateBuildComboBox( QComboBox * comboBox , const char * indexChangedSlot , const QString& settingsKey , const QString& settingsIndexKey );

		void	createActions();
		void	createMenus();
		void	createToolBars();
		
		void	setupNAddItem(QLanguageItem* decorator);
		bool	addLanguageItem(const QString &gmnSource, bool sourceIsAFile , QLanguageItem** createdItem);
		QItemResizer * plugResizer( QLanguageItem * itemContainer );
		
		void updateWindowState();
		void changeFontSize( float newFontPointSize );
		
		void setBuildCommand( const QString& buildCommand );
		void setBuildOptions( const QString& buildCommand );

		virtual void	reloadTextEdits();
		
		void closeEvent(QCloseEvent *event);
		
		QString availableDefaultName(const QString& baseName);
						
		static QString buildCombinationCode( const QString& file1 , const QString& file2 , const QString& faustOperator );
		
		QList<QFaustItem*> selectedFaustItems(); 
		
		QAction * mPreferencesAct;
		
		QTextEdit*		mCPPTextEdit;
		QDockWidget*	mCPPTextEditDock;
        QDockWidget*	mErrorDock;
		QComboBox *		mTargetsComboBox;
		QComboBox *		mOptionsComboBox;
		QComboBox *		mDropTypeComboBox;
		QToolBar *		mBuildToolBar;
};

/*
#include <QHash>
class QHashStringString : public QHash<QString,QString>
{
	public : QHashStringString() : QHash<QString,QString>() {}
};
Q_DECLARE_METATYPE(QHashStringString)
*/

#endif
