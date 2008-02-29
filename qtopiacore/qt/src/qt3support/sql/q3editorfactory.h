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

#ifndef Q3EDITORFACTORY_H
#define Q3EDITORFACTORY_H

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_EDIT_WIDGETS

class Q_COMPAT_EXPORT Q3EditorFactory : public QObject
{
public:
    Q3EditorFactory (QObject * parent = 0);
    ~Q3EditorFactory();

    virtual QWidget * createEditor(QWidget * parent, const QVariant & v);

    static Q3EditorFactory * defaultFactory();
    static void installDefaultFactory(Q3EditorFactory * factory);

private:
    Q_DISABLE_COPY(Q3EditorFactory)
};

#endif // QT_NO_SQL_EDIT_WIDGETS

QT_END_HEADER

#endif // Q3EDITORFACTORY_H
