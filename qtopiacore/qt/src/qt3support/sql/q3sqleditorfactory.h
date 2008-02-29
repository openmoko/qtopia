/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
