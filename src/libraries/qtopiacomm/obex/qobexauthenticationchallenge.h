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
#ifndef __QOBEXAUTHENTICATIONCHALLENGE_H__
#define __QOBEXAUTHENTICATIONCHALLENGE_H__

#include <qobexnamespace.h>
#include <qobexglobal.h>

class QObexAuthenticationChallengePrivate;

class QTOPIAOBEX_EXPORT QObexAuthenticationChallenge
{
public:
    QObexAuthenticationChallenge();
    QObexAuthenticationChallenge(const QObexAuthenticationChallenge &other);
    ~QObexAuthenticationChallenge();

    QObexAuthenticationChallenge &operator=(const QObexAuthenticationChallenge &other);

    void setUser(const QString &userName);
    QString user() const;

    void setPassword(const QString &password);
    QString password() const;

    QObex::AuthChallengeOptions options() const;
    QString realm() const;

private:
    friend class QObexAuthenticationChallengePrivate;
    QObexAuthenticationChallengePrivate *m_data;
};

#endif
