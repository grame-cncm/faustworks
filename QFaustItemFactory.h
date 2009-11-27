/*
 * QFaustItemFactory.h
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

#ifndef FAUST_ITEM_FACTORY_H
#define FAUST_ITEM_FACTORY_H

#include "QLanguageItemFactory.h"

class QFaustItemFactory : public QLanguageItemFactory
{

	public:

		QLanguageItem * buildLanguageItem( QGraphicsItem * parent = 0 );
		QLanguageItem * buildLanguageItem( const QString& fileName , QGraphicsItem * parent = 0 );
		QLanguageItem * buildLanguageItem( const QLanguageItem * other		, QGraphicsItem * parent = 0);
		QLanguageItem * buildLanguageItem( const QMimeData * mimeData		, QGraphicsItem * parent = 0);
		QLanguageItem * buildLanguageItem( const QDomElement * domElement	, QGraphicsItem * parent = 0);

};

#endif //FAUST_ITEM_FACTORY_H
