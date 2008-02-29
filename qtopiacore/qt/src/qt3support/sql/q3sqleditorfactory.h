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

#ifndef Q3SQLEDITORFACTORY_H
#define Q3SQLEDITORFACTORY_H

#include <Qt3Support/q3editorfactory.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_EDIT_WIDGETS

class QSqlField;

class Q_COMPAT_EXPORT Q3SqlEditorFactory : public Q3EditorFactory
{
public:
    Q3SqlEditorFactory (QObject * parent = 0);
    ~Q3SqlEditorFactory();
    virtual QWidget * createEditor(QWidget * parent, const QVariant & variant);
    virtual QWidget * createEditor(QWidget * parent, const QSqlField * field);

    static Q3SqlEditorFactory * defaultFactory();
    static void installDefaultFactory(Q3SqlEditorFactory * factory);

private:
    Q_DISABLE_COPY(Q3SqlEditorFactory)
};

#endif // QT_NO_SQL_EDIT_WIDGETS

QT_END_HEADER

#endif // Q3SQLEDITORFACTORY_H
