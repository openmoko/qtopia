/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3PTRCOLLECTION_H
#define Q3PTRCOLLECTION_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3GVector;
class Q3GList;
class Q3GDict;

class Q_COMPAT_EXPORT Q3PtrCollection			// inherited by all collections
{
public:
    bool autoDelete()	const	       { return del_item; }
    void setAutoDelete(bool enable)  { del_item = enable; }

    virtual uint  count() const = 0;
    virtual void  clear() = 0;			// delete all objects

    typedef void *Item;				// generic collection item

protected:
    Q3PtrCollection() { del_item = false; }		// no deletion of objects
    Q3PtrCollection(const Q3PtrCollection &) { del_item = false; }
    virtual ~Q3PtrCollection() {}

    bool del_item;				// default false

    virtual Item     newItem(Item);		// create object
    virtual void     deleteItem(Item) = 0;	// delete object
};

QT_END_HEADER

#endif // Q3PTRCOLLECTION_H
