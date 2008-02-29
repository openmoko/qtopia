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

#include <qdplugin.h>
#include <qmap.h>

static QMap<QString,qdPluginCreateFunc_t> *qdInternalPlugins()
{
    static QMap<QString,qdPluginCreateFunc_t> *internalPlugins = new QMap<QString,qdPluginCreateFunc_t>;
    return internalPlugins;
}

void qd_registerPlugin_AUTOPLUGIN_TARGET(const QString &id, qdPluginCreateFunc_t createFunc)
{
    (*qdInternalPlugins())[id] = createFunc;
}

class PluginFactory_AUTOPLUGIN_TARGET : public QDPluginFactory
{
    QD_CONSTRUCT_PLUGIN(PluginFactory_AUTOPLUGIN_TARGET,QDPluginFactory)
public:
    QString executableName() const
    {
        return QTOPIA_TARGET;
    }
    QStringList keys() const
    {
        return qdInternalPlugins()->keys();
    }
    QDPlugin *create( const QString &key )
    {
        if ( qdInternalPlugins()->contains( key ) ) {
            qdPluginCreateFunc_t func = (*qdInternalPlugins())[key];
            QDPlugin *plugin = func( this );
            return plugin;
        }
        return 0;
    }
};

Q_EXPORT_PLUGIN(PluginFactory_AUTOPLUGIN_TARGET)

