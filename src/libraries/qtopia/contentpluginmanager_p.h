/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qcontentplugin.h>
#include <QMultiHash>
#include <QList>
#include <QSettings>
#include <qpluginmanager.h>



class DotDesktopContentPlugin : public QContentPlugin
{
public:
    DotDesktopContentPlugin();
    virtual ~DotDesktopContentPlugin();

    virtual QStringList keys() const;

    virtual bool installContent( const QString &filePath, QContent *content );
    virtual bool updateContent( QContent *content );
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

class QContentFactory
{
public:
    static bool installContent( const QString &fileName, QContent *content );
    static bool updateContent( QContent *content );
};


#endif
