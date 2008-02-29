/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QIROBEXSOCKET_H__
#define __QIROBEXSOCKET_H__

#include <qobexsocket.h>
#include <QtCore/qglobal.h>
#include <QString>
#include <qtopiaglobal.h>

class QTOPIACOMM_EXPORT QIrObexSocket : public QObexSocket
{
public:
    QIrObexSocket();
    explicit QIrObexSocket(const QString &service, uint addr = 0);
    virtual ~QIrObexSocket();

    const QString &service() const;
    void setService(const QString &service);

    uint address() const;
    void setAddress(uint addr);

    virtual bool connect();

private:
    Q_DISABLE_COPY(QIrObexSocket)
    uint m_addr;
    QString m_service;
};

#endif
