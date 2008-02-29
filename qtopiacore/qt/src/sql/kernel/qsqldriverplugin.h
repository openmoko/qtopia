/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class QSqlDriver;

struct Q_SQL_EXPORT QSqlDriverFactoryInterface : public QFactoryInterface
{
    virtual QSqlDriver *create(const QString &name) = 0;
};

#define QSqlDriverFactoryInterface_iid "com.trolltech.Qt.QSqlDriverFactoryInterface"
Q_DECLARE_INTERFACE(QSqlDriverFactoryInterface, QSqlDriverFactoryInterface_iid)

class Q_SQL_EXPORT QSqlDriverPlugin : public QObject, public QSqlDriverFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QSqlDriverFactoryInterface:QFactoryInterface)
public:
    explicit QSqlDriverPlugin(QObject *parent = 0);
    ~QSqlDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QSqlDriver *create(const QString &key) = 0;

};

QT_END_HEADER

#endif // QSQLDRIVERPLUGIN_H
