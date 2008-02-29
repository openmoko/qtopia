/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** Definition of QInputContextFactory class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to TROLLTECH ASA under their own
** license. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef QINPUTCONTEXTFACTORY_H
#define QINPUTCONTEXTFACTORY_H

#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_IM

class QInputContext;
class QWidget;

class Q_GUI_EXPORT QInputContextFactory
{
public:
    static QStringList keys();
    static QInputContext *create( const QString &key, QObject *parent ); // should be a toplevel widget
    static QStringList languages( const QString &key );
    static QString displayName( const QString &key );
    static QString description( const QString &key );
};

#endif // QT_NO_IM

QT_END_HEADER

#endif // QINPUTCONTEXTFACTORY_H
