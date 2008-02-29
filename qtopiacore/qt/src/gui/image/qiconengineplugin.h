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

#ifndef QICONENGINEPLUGIN_H
#define QICONENGINEPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QIconEngine;

struct Q_GUI_EXPORT QIconEngineFactoryInterface : public QFactoryInterface
{
    virtual QIconEngine *create(const QString &filename) = 0;
};

#define QIconEngineFactoryInterface_iid "com.trolltech.Qt.QIconEngineFactoryInterface"
Q_DECLARE_INTERFACE(QIconEngineFactoryInterface, QIconEngineFactoryInterface_iid)

class Q_GUI_EXPORT QIconEnginePlugin : public QObject, public QIconEngineFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QIconEngineFactoryInterface:QFactoryInterface)
public:
    QIconEnginePlugin(QObject *parent = 0);
    ~QIconEnginePlugin();

    virtual QStringList keys() const = 0;
    virtual QIconEngine *create(const QString &filename) = 0;
};

QT_END_HEADER

#endif // QICONENGINEPLUGIN_H
