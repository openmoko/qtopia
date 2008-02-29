/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qtopiaglobal.h>
#include <qplugin.h>
#undef Q_EXPORT_STATIC_PLUGIN
#undef Q_EXPORT_PLUGIN2
#define Q_EXPORT_STATIC_PLUGIN(x)\
    Q_DECL_EXPORT QObject *qt_plugin_instance_##x()\
    Q_PLUGIN_INSTANCE(x)\
    Q_IMPORT_PLUGIN(x)
#define Q_EXPORT_PLUGIN2(x,y)
#include "main.cpp"
