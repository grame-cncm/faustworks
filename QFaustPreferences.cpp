/*
 * QFaustPreferences.cpp
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
 
#include "QFaustPreferences.h"

#include "ui_FaustPreferences.h"
#include "FaustMainWindow.h"

#include <assert.h>

#include <QSettings>
#include <QtDebug>
#include <QFileDialog>

//-------------------------------------------------------------------------
QFaustPreferences::QFaustPreferences(FaustMainWindow * parent) 
: QDialog(parent)
{
	mMainWindow = parent;

	saveSettings();

	mUI = new Ui::FaustPreferences();
	mUI->setupUi(this);
	
	mUI->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setAutoDefault( false );
	mUI->buttonBox->button(QDialogButtonBox::Apply)->setAutoDefault( true );

	QObject::connect( mUI->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
	QObject::connect( mUI->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(cancel()));
	QObject::connect( mUI->buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), this, SLOT(reset()));

	QObject::connect( mUI->findFaustPathButton , SIGNAL(clicked()) , this , SLOT(findFaustPath()) );
	
	QObject::connect( mUI->addTargetButton , SIGNAL(clicked()) , this , SLOT(addTarget()) );
	QObject::connect( mUI->addOptionButton , SIGNAL(clicked()) , this , SLOT(addOption()) );
	
	updateWidgets();
}

//-------------------------------------------------------------------------
void QFaustPreferences::reset()
{
	mMainWindow->reinitPreferencesSettings();
	updateWidgets();
}

//-------------------------------------------------------------------------
void QFaustPreferences::findFaustPath()
{
	QString faustPath = QFileDialog::getOpenFileName ( this, "Find faust" , mUI->faustPathEdit->text() );
	mUI->faustPathEdit->setText( faustPath );
}


#define TARGET_WIDGET	"TargetWidget"
#define TARGET_NAME		"TargetName"
#define TARGET_COMMAND	"TargetCommand"
#define OPTION_WIDGET	"OptionWidget"
#define OPTION_NAME		"OptionName"
#define OPTION_COMMAND	"OptionCommand"

//-------------------------------------------------------------------------
void QFaustPreferences::apply()	
{
    QSettings settings("grame.fr", "FaustWorks");
	settings.setValue( FAUST_PATH_SETTING , mUI->faustPathEdit->text() );
	
	applyConfiguration( TARGETS_SETTING , TARGET_WIDGET , TARGET_NAME , TARGET_COMMAND , mUI->targetsBox );
	applyConfiguration( OPTIONS_SETTING , OPTION_WIDGET , OPTION_NAME , OPTION_COMMAND , mUI->optionsBox );

	accept();
}

//-------------------------------------------------------------------------
void QFaustPreferences::cancel()
{
	restoreSettings();

	reject();
}

//-------------------------------------------------------------------------
void QFaustPreferences::addTarget()
{
	addTarget("untitled_target","/path/to/my/Script/myScript $DSP $OPTIONS");
}

//-------------------------------------------------------------------------
void QFaustPreferences::removeTarget()
{
	QToolButton * removeButton = (QToolButton *)sender();
	QWidget * targetWidget = (QWidget *)removeButton->parent();
	targetWidget->deleteLater();
	
	updateTabOrder();
}

//-------------------------------------------------------------------------
void QFaustPreferences::addOption()
{
	addOption("untitled_option","-myOption");
}

//-------------------------------------------------------------------------
void QFaustPreferences::applyConfiguration( const QString& settingKey , const QString& widgetName , const QString& nameEditName , const QString& commandEditName , QWidget * box )
{
    QSettings settings("grame.fr", "FaustWorks");
	
	settings.remove( settingKey );
//  QList<QWidget*> targetWidgets = qFindChildren<QWidget*>( box , widgetName );
    QList<QWidget*> targetWidgets = box->findChildren<QWidget*>( widgetName );
    for ( int i = 0 ; i < targetWidgets.size() ; i++)
	{
        QList<QLineEdit*> nameEdit = targetWidgets[i]->findChildren<QLineEdit*>( nameEditName );
        QList<QLineEdit*> commandEdit =  targetWidgets[i]->findChildren<QLineEdit*>(commandEditName );
		assert( nameEdit.size() == 1 );
		assert( commandEdit.size() == 1 );
		settings.setValue(settingKey + "/" + nameEdit[0]->text() , commandEdit[0]->text() );
	}
}
		
//-------------------------------------------------------------------------
void QFaustPreferences::updateWidgets()
{
    QSettings settings("grame.fr", "FaustWorks");
	mUI->faustPathEdit->setText( settings.value( FAUST_PATH_SETTING ).toString() );
	
	updateConfigurationWidgets( TARGETS_SETTING , TARGET_WIDGET , TARGET_NAME , TARGET_COMMAND , mUI->targetsBox );
	updateConfigurationWidgets( OPTIONS_SETTING , OPTION_WIDGET , OPTION_NAME , OPTION_COMMAND , mUI->optionsBox );
}

//-------------------------------------------------------------------------
void QFaustPreferences::updateTabOrder()
{
	setTabOrder( mUI->faustPathEdit , mUI->findFaustPathButton );
	
}

//-------------------------------------------------------------------------
void QFaustPreferences::updateConfigurationWidgets( 
	const QString& settingKey , const QString& widgetName , const QString& nameEditName , const QString& commandEditName , QWidget * parent )
{
    QSettings settings("grame.fr", "FaustWorks");

	//Clear targets
//  QList<QWidget*> targetWidgets = qFindChildren<QWidget*>( parent , widgetName );
    QList<QWidget*> targetWidgets =  parent->findChildren<QWidget*>(widgetName );
    for ( int i = 0 ; i < targetWidgets.size() ; i++)
	{
		delete targetWidgets[i];
	}
	
	//Add targets that are in the QSettings.
	settings.beginGroup(settingKey);
	QStringList keys = settings.allKeys();
	for ( int i = 0 ; i < keys.size() ; i++ )
	{
		QString command = settings.value( keys[i] ).toString();
		QString target = keys[i];
		addConfigurationLine( target , command , widgetName , nameEditName , commandEditName , parent );
	}
	settings.endGroup();
	
	updateTabOrder();
}

//-------------------------------------------------------------------------
void QFaustPreferences::addConfigurationLine( 
	const QString& targetName , const QString& targetCommand , 
	const QString& widgetName , const QString& nameEditName , const QString& commandEditName , QWidget * parent )
{
	QWidget * target = new QWidget(this);
	target->setObjectName( widgetName );
	
	QSizePolicy p;
	
    QLabel * targetNameLabel	= new QLabel(tr("Name :") , target);
    QLineEdit * targetNameEdit	= new QLineEdit(targetName , target);
	targetNameEdit->setObjectName( nameEditName );
	p = targetNameEdit->sizePolicy();
	p.setHorizontalStretch( 1 );
	targetNameEdit->setSizePolicy( p );
    QLabel * targetCommandLabel = new QLabel(tr("Command :") , target);
    QLineEdit *targetCommandEdit= new QLineEdit(targetCommand , target);
	targetCommandEdit->setObjectName( commandEditName );
	p = targetCommandEdit->sizePolicy();
	p.setHorizontalStretch( 3 );
	targetCommandEdit->setSizePolicy( p );
	
	QToolButton * removeTargetButton = new QToolButton( target );
    removeTargetButton->setText( tr("Remove") );

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(targetNameLabel);
	layout->addWidget(targetNameEdit);
	layout->addWidget(targetCommandLabel);
	layout->addWidget(targetCommandEdit);
	layout->addWidget(removeTargetButton);
	
	target->setLayout(layout);
	p = target->sizePolicy();
	p.setVerticalPolicy( QSizePolicy::Fixed );
	target->setSizePolicy( p );
	
	((QVBoxLayout*)parent->layout())->insertWidget( parent->layout()->count() - 1 ,  target );
		
	QObject::connect( removeTargetButton , SIGNAL(clicked()) , this , SLOT(removeTarget()) );
}

/*
//-------------------------------------------------------------------------
void QFaustPreferences::addTarget( const QString& targetName , const QString& targetCommand )
{
	QWidget * target = new QWidget(this);
	target->setObjectName( TARGET_WIDGET );
	
	QSizePolicy p;
	
	QLabel * targetNameLabel	= new QLabel("Name :" , target);
    QLineEdit * targetNameEdit	= new QLineEdit(targetName , target);
	targetNameEdit->setObjectName( TARGET_NAME );
	p = targetNameEdit->sizePolicy();
	p.setHorizontalStretch( 1 );
	targetNameEdit->setSizePolicy( p );
	QLabel * targetCommandLabel = new QLabel("Command :" , target);
    QLineEdit *targetCommandEdit= new QLineEdit(targetCommand , target);
	targetCommandEdit->setObjectName( TARGET_COMMAND );
	p = targetCommandEdit->sizePolicy();
	p.setHorizontalStretch( 3 );
	targetCommandEdit->setSizePolicy( p );
	
	QToolButton * removeTargetButton = new QToolButton( target );
	removeTargetButton->setText( "Remove" );

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(targetNameLabel);
	layout->addWidget(targetNameEdit);
	layout->addWidget(targetCommandLabel);
	layout->addWidget(targetCommandEdit);
	layout->addWidget(removeTargetButton);
	
	target->setLayout(layout);
	p = target->sizePolicy();
	p.setVerticalPolicy( QSizePolicy::Fixed );
	target->setSizePolicy( p );

	((QVBoxLayout*)mUI->targetsBox->layout())->insertWidget( mUI->targetsBox->layout()->count() - 1 ,  target );
	
	QObject::connect( removeTargetButton , SIGNAL(clicked()) , this , SLOT(removeTarget()) );
}
*/

//-------------------------------------------------------------------------
void QFaustPreferences::addTarget( const QString& targetName , const QString& targetCommand )
{
	addConfigurationLine(targetName,targetCommand,TARGET_WIDGET,TARGET_NAME,TARGET_COMMAND,mUI->targetsBox);
	updateTabOrder();
}

//-------------------------------------------------------------------------
void QFaustPreferences::addOption( const QString& optionName , const QString& optionCommand )
{
	addConfigurationLine(optionName,optionCommand,OPTION_WIDGET,OPTION_NAME,OPTION_COMMAND,mUI->optionsBox);
	updateTabOrder();
}

QHash<QString,QVariant> QFaustPreferences::mSavedSettings;
//-------------------------------------------------------------------------
void QFaustPreferences::saveSettings()
{
	mSavedSettings.clear();
	
    QSettings settings("grame.fr", "FaustWorks");
	
	QStringList keys = settings.allKeys();
	for ( int i = 0 ; i < keys.size() ; i++ )
	{
		mSavedSettings[keys[i]] = settings.value( keys[i] );
	}
}

//-------------------------------------------------------------------------
void QFaustPreferences::restoreSettings()
{
    QSettings settings("grame.fr", "FaustWorks");
	settings.clear();
	
	QStringList keys = mSavedSettings.keys();
	for ( int i = 0 ; i < keys.size() ; i++ )
	{
		settings.setValue( keys[i] , mSavedSettings[keys[i]] );
	}
}
