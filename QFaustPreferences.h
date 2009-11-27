/*
 * QFaustPreferences.h
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
 
#ifndef FaustPreferences_H
#define FaustPreferences_H

#include <QDialog>
#include <QHash>
#include <QVariant>

namespace Ui
{
	class FaustPreferences;
}

class FaustMainWindow;

//------------------------------------------------------------------------------------------------------------------------
class QFaustPreferences : public QDialog
{
	Q_OBJECT

	public:
		QFaustPreferences(FaustMainWindow * parent = 0);

	protected slots:
		void apply();
		void cancel();
		void reset();
		void findFaustPath();
		void addTarget();
		void removeTarget();
		void addOption();
								
	protected:
	
		void updateWidgets();
		void updateConfigurationWidgets( 
			const QString& settingKey , const QString& widgetName , 
			const QString& nameEditName , const QString& commandEditName , QWidget * parent );
		void addConfigurationLine( 
			const QString& targetName , const QString& targetCommand , 
			const QString& widgetName , const QString& nameEditName , const QString& commandEditName , QWidget * parent );
	
		void updateTabOrder();
	
		void addTarget( const QString& targetName , const QString& targetCommand );
		void addOption( const QString& optionName , const QString& optionCommand );
		
		void applyConfiguration( const QString& settingKey , const QString& widgetName , const QString& nameEditName , const QString& commandEditName , QWidget * box );
		
		FaustMainWindow* mMainWindow;
		
		static void saveSettings();
		static void restoreSettings();
		static QHash<QString,QVariant> mSavedSettings;

	private:
		Ui::FaustPreferences * mUI;
};

#endif
