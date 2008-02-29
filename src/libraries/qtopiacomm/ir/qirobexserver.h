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

#ifndef __QIROBEXSERVER_H__
#define __QIROBEXSERVER_H__

#include <QObject>
#include <QString>
#include <qglobal.h>

#include <qtopiaglobal.h>
#include <qobexserver.h>
#include <qirnamespace.h>

class QIrObexServer_Private;

class QTOPIACOMM_EXPORT QIrObexServer : public QObexServer
{
    Q_OBJECT
public:

    explicit QIrObexServer(const QString &service,
                           QIr::DeviceClasses classes = QIr::OBEX,
                           QObject *parent = 0);
    ~QIrObexServer();

    const QString &service() const;
    QIr::DeviceClasses deviceClasses() const;

    virtual QObexSocket *nextPendingConnection();

protected:
    virtual void *registerServer();

private:
    QIrObexServer_Private *m_data;
    Q_DISABLE_COPY(QIrObexServer)
};

#endif
