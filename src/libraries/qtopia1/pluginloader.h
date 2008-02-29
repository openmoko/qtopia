/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <qtopia/qlibrary.h>
#include <qtopia/qcom.h>

class PluginLoaderPrivate;

#ifdef PLUGINLOADER_INTERN
#define PLUGINLOADER PluginLoaderIntern
class PLUGINLOADER
#else
#define PLUGINLOADER PluginLoader
class QTOPIA_EXPORT PLUGINLOADER
#endif
{
public:
    PLUGINLOADER( const QString &type );
    ~PLUGINLOADER();

    void clear();

    const QStringList &list() const;
    const QStringList &disabledList() const;
    QRESULT queryInterface( const QString &name, const QUuid&, QUnknownInterface** );
    void releaseInterface( QUnknownInterface *iface );

    void setEnabled( const QString &name, bool enabled=TRUE );
    bool isEnabled( const QString &name ) const;

    static bool inSafeMode();
    static void init();

private:
    void initType();
    QStringList languageList() const;
    QString stripSystem( const QString &libFile ) const;

private:
    PluginLoaderPrivate *d;
};

#endif // PLUGINLOADER_H
