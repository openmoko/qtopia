/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessibleplugin.h"

#ifndef QT_NO_ACCESSIBILITY

#include "qaccessible.h"

/*!
    \class QAccessiblePlugin
    \brief The QAccessiblePlugin class provides an abstract base for
    accessibility plugins.

    \ingroup plugins
    \ingroup accessibility

    Writing an accessibility plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    create(), and exporting the class with the Q_EXPORT_PLUGIN2()
    macro.

    \sa QAccessibleBridgePlugin, {How to Create Qt Plugins}
*/

/*!
    Constructs an accessibility plugin with the given \a parent. This
    is invoked automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QAccessiblePlugin::QAccessiblePlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the accessibility plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QAccessiblePlugin::~QAccessiblePlugin()
{
}

/*!
    \fn QStringList QAccessiblePlugin::keys() const

    Returns the list of keys this plugin supports.

    These keys must be the class names that this plugin provides
    an accessibility implementation for.

    \sa create()
*/

/*!
    \fn QAccessibleInterface *QAccessiblePlugin::create(const QString &key, QObject *object)

    Creates and returns a QAccessibleInterface implementation for the
    class \a key and the object \a object. Keys are case sensitive.

    \sa keys()
*/

#endif // QT_NO_ACCESSIBILITY
