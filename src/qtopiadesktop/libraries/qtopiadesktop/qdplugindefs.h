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
#ifndef QDPLUGINDEFS_H
#define QDPLUGINDEFS_H

#include <QList>
#include <qdglobal.h>
#include <trace.h>

class QObject;

class QDPlugin;
class QDPluginData;
class QDAppPlugin;
class QDAppPluginData;
class QDLinkPlugin;
class QDConPlugin;
class QDDevPlugin;
class QDSyncPlugin;
class QDClientSyncPlugin;
class QDClientSyncPluginFactory;
class QDServerSyncPlugin;

class CenterInterface;

typedef QList<QDPlugin*> QDPluginList;
typedef QList<QDAppPlugin*> QDAppPluginList;
typedef QList<QDConPlugin*> QDConPluginList;
typedef QList<QDDevPlugin*> QDDevPluginList;
typedef QList<QDLinkPlugin*> QDLinkPluginList;
typedef QList<QDSyncPlugin*> QDSyncPluginList;

typedef QDPlugin *(*qdPluginCreateFunc_t)(QObject*);
QD_EXPORT void qd_registerPlugin(qdPluginCreateFunc_t createFunc);

#ifdef PLUGIN
#include "_qdautoplugin.h"
#else
#define QD_REGISTER_PLUGIN(IMPLEMENTATION) \
    static QDPlugin *create_ ## IMPLEMENTATION( QObject *parent ) \
        { return new IMPLEMENTATION(parent); } \
    static qdPluginCreateFunc_t append_ ## IMPLEMENTATION() \
        { qd_registerPlugin(create_ ## IMPLEMENTATION); \
            return create_ ## IMPLEMENTATION; } \
    static qdPluginCreateFunc_t dummy_ ## IMPLEMENTATION = \
        append_ ## IMPLEMENTATION();
#endif

#define QD_CONSTRUCT_PLUGIN(CLASS,PARENT) \
    public: \
        CLASS( QObject *parent = 0 ) \
            : PARENT( parent ) {} \
        ~CLASS() {\
            TRACE(TRACE) << #CLASS "::~" #CLASS;\
        } \
    private:

#define QD_EXPORT_PLUGIN(IMPLEMENTATION) QD_REGISTER_PLUGIN(IMPLEMENTATION)

#endif
