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

#ifndef __QTOPIA_MEDIASERVER_QTOPIAMEDIAPROVIDER_H
#define __QTOPIA_MEDIASERVER_QTOPIAMEDIAPROVIDER_H

#include <QString>
#include <QUuid>
#include <QtopiaAbstractService>
#include <QMediaSessionRequest>

namespace mediaserver
{

class SessionManager;

class QtopiaMediaProviderPrivate;

class QtopiaMediaProvider : public QtopiaAbstractService
{
    Q_OBJECT

public:
    QtopiaMediaProvider(SessionManager* sessionManager);
    ~QtopiaMediaProvider();

public slots:
    void createSession(QString const& responseChannel,
                       QMediaSessionRequest const& request);

    void destroySession(QUuid const& id);

private:
    QtopiaMediaProviderPrivate* d;
};

}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_QTOPIAMEDIAPROVIDER_H

