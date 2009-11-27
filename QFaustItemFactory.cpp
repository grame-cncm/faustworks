/*
 * QFaustItemFactory.cpp
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
 
#include "QFaustItemFactory.h"

#include <assert.h>

#include "QFaustItem.h"

//------------------------------------------------------
QLanguageItem * QFaustItemFactory::buildLanguageItem( QGraphicsItem * parent )
{
	return new QFaustItem( parent );
}

//------------------------------------------------------
QLanguageItem * QFaustItemFactory::buildLanguageItem( const QString& fileName , QGraphicsItem * parent )
{
	QFaustItem * result = new QFaustItem( parent );
	result->loadFile( fileName );
	return result;
}

//------------------------------------------------------
QLanguageItem * QFaustItemFactory::buildLanguageItem( const QLanguageItem * other , QGraphicsItem * parent )
{
	assert(dynamic_cast<const QFaustItem*>(other));
	QFaustItem * result = new QFaustItem( (QFaustItem*)other , parent );
	return result;
}

//------------------------------------------------------
QLanguageItem * QFaustItemFactory::buildLanguageItem( const QMimeData * mimeData , QGraphicsItem * parent )
{
	return new QFaustItem( mimeData , parent );
}

//------------------------------------------------------
QLanguageItem * QFaustItemFactory::buildLanguageItem( const QDomElement * domElement , QGraphicsItem * parent )
{
	return new QFaustItem( domElement , parent );
}
