/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCREENDRIVERPLUGIN_QWS_H
#define QSCREENDRIVERPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_LIBRARY

class QScreen;

struct Q_GUI_EXPORT QScreenDriverFactoryInterface : public QFactoryInterface
{
    virtual QScreen* create(const QString& driver, int displayId) = 0;
};

#define QScreenDriverFactoryInterface_iid "com.trolltech.Qt.QScreenDriverFactoryInterface"
Q_DECLARE_INTERFACE(QScreenDriverFactoryInterface, QScreenDriverFactoryInterface_iid)

class Q_GUI_EXPORT QScreenDriverPlugin : public QObject, public QScreenDriverFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QScreenDriverFactoryInterface:QFactoryInterface)
public:
    explicit QScreenDriverPlugin(QObject *parent = 0);
    ~QScreenDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QScreen *create(const QString& driver, int displayId) = 0;
};

#endif // QT_NO_LIBRARY

QT_END_HEADER

#endif // QSCREENDRIVERPLUGIN_QWS_H
