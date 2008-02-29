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
** Implementation of QMultiInputContextPlugin class
**
** Copyright (C) 2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to TROLLTECH ASA under their own
** license. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef QT_NO_IM
#include "qmultiinputcontext.h"
#include "qmultiinputcontextplugin.h"
#include <qinputcontextplugin.h>
#include <qstringlist.h>


QMultiInputContextPlugin::QMultiInputContextPlugin()
{
}

QMultiInputContextPlugin::~QMultiInputContextPlugin()
{
}

QStringList QMultiInputContextPlugin::keys() const
{
    // input method switcher should named with "imsw-" prefix to
    // prevent to be listed in ordinary input method list.
    return QStringList( "imsw-multi" );
}

QInputContext *QMultiInputContextPlugin::create( const QString &key )
{
    if (key != QLatin1String("imsw-multi"))
        return 0;
    return new QMultiInputContext;
}

QStringList QMultiInputContextPlugin::languages( const QString & )
{
    return QStringList();
}

QString QMultiInputContextPlugin::displayName( const QString &key )
{
    if (key != QLatin1String("imsw-multi"))
        return QString();
    return tr( "Multiple input method switcher" );
}

QString QMultiInputContextPlugin::description( const QString &key )
{
    if (key != QLatin1String("imsw-multi"))
        return QString();
    return tr( "Multiple input method switcher that uses the context menu of the text widgets" );
}


Q_EXPORT_STATIC_PLUGIN(QMultiInputContextPlugin)
Q_EXPORT_STATIC_PLUGIN2(qimsw_multi, QMultiInputContextPlugin)

#endif
