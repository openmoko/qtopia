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

#ifndef __QTOPIA_MEDIA_ABSTRACTCONTROLSERVER_H
#define __QTOPIA_MEDIA_ABSTRACTCONTROLSERVER_H

#include <QObject>
#include <QString>
#include <QVariant>

#include "qmediahandle_p.h"

#include <qtopiaglobal.h>

class QMediaAbstractControlServerPrivate;

class QTOPIAMEDIA_EXPORT QMediaAbstractControlServer : public QObject
{
    Q_OBJECT

public:
    QMediaAbstractControlServer(QMediaHandle const& handle,
                                QString const& name,
                                QObject* parent = 0);
    ~QMediaAbstractControlServer();

signals:
    void controlAvailable(QString const& name);
    void controlUnavailable(QString const& name);

protected:
    void setValue(QString const& name, QVariant const& value);

    void proxyAll();

private slots:
    void itemRemove(const QByteArray &attribute);
    void itemSetValue(const QByteArray &attribute, const QVariant &value);

private:
    QMediaAbstractControlServerPrivate* d;
};

#endif  // __QTOPIA_MEDIA_ABSTRACTCONTROLSERVER_H
