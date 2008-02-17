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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SQLPROPERTYMAP_H
#define Q3SQLPROPERTYMAP_H

#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_FORM

class QWidget;
class Q3SqlPropertyMapPrivate;

class Q_COMPAT_EXPORT Q3SqlPropertyMap
{
public:
    Q3SqlPropertyMap();
    virtual ~Q3SqlPropertyMap();

    QVariant      property(QWidget * widget);
    virtual void  setProperty(QWidget * widget, const QVariant & value);

    void insert(const QString & classname, const QString & property);
    void remove(const QString & classname);

    static Q3SqlPropertyMap * defaultMap();
    static void installDefaultMap(Q3SqlPropertyMap * map);

private:
    Q_DISABLE_COPY(Q3SqlPropertyMap)

    Q3SqlPropertyMapPrivate* d;
};

#endif // QT_NO_SQL_FORM

QT_END_HEADER

#endif // Q3SQLPROPERTYMAP_H
