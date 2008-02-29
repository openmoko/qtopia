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

#ifndef QDAUTOPLUGIN_H
#define QDAUTOPLUGIN_H

#define QD_REGISTER_PLUGIN(IMPLEMENTATION) \
    void qd_registerPlugin_AUTOPLUGIN_TARGET(const QString &id, qdPluginCreateFunc_t createFunc);\
    static QDPlugin *create_ ## IMPLEMENTATION( QObject *parent ) \
        { return new IMPLEMENTATION(parent); } \
    static qdPluginCreateFunc_t append_ ## IMPLEMENTATION() \
        { qd_registerPlugin_AUTOPLUGIN_TARGET(#IMPLEMENTATION, create_ ## IMPLEMENTATION); \
            return create_ ## IMPLEMENTATION; } \
    static qdPluginCreateFunc_t dummy_ ## IMPLEMENTATION = \
        append_ ## IMPLEMENTATION();

#endif
