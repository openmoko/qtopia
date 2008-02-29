/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONTENTPLUGINMANAGER_H
#define CONTENTPLUGINMANAGER_H

#include <qcontentplugin.h>
#include <QMultiHash>
#include <QList>
#include <QSettings>
#include <qpluginmanager.h>



class DotDirectoryContentPlugin : public QContentPlugin
{
public:
    DotDirectoryContentPlugin();
    virtual ~DotDirectoryContentPlugin();

    virtual QStringList keys() const;

    virtual bool installContent( const QString &filePath, QContent *content );
};

class DotDesktopContentPlugin : public QContentPlugin
{
public:
    DotDesktopContentPlugin();
    virtual ~DotDesktopContentPlugin();

    virtual QStringList keys() const;

    virtual bool installContent( const QString &filePath, QContent *content );
};


class ContentPluginManager
{
public:
    ContentPluginManager();
    ~ContentPluginManager();

    QList< QContentPlugin * > findPlugins( const QString &type );

private:
    QPluginManager manager;

    QMultiHash< QString, QContentPlugin * > typePluginMap;

    QList< QContentPlugin * > plugins;
};

#endif
