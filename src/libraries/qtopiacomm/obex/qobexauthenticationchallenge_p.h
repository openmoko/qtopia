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
#ifndef __QOBEXAUTHENTICATIONCHALLENGE_P_H__
#define __QOBEXAUTHENTICATIONCHALLENGE_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qobexauthenticationchallenge.h>
#include <QObject>

class QTOPIA_AUTOTEST_EXPORT QObexAuthenticationChallengePrivate
{
public:
    QString m_user;
    QString m_password;
    QObex::AuthChallengeOptions m_options;
    QString m_realm;
    QByteArray m_nonce;
    bool m_modified; // whether user/password have been set at all

    inline static const QObexAuthenticationChallengePrivate *getPrivate(const QObexAuthenticationChallenge &challenge) { return challenge.m_data; }

    static bool parseRawChallenge(const QByteArray &bytes,
                                  QObexAuthenticationChallenge &challenge);

    static void writeRawChallenge(const QByteArray &nonce,
                                  QObex::AuthChallengeOptions options,
                                  const QString &realm,
                                  QByteArray &dest);

    bool toRawResponse(QByteArray &dest,
                       const QByteArray &nonce = QByteArray()) const;
};

#endif
