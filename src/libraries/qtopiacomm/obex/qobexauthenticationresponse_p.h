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
#ifndef __QOBEXAUTHENTICATIONRESPONSE_P_H__
#define __QOBEXAUTHENTICATIONRESPONSE_P_H__

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

#include <qobexauthenticationresponse.h>
#include <QObject>

class QTOPIA_AUTOTEST_EXPORT QObexAuthenticationResponsePrivate
{
public:
    QByteArray m_requestDigest;
    QString m_user;
    QByteArray m_nonce;     // used to validate the password in match()

    inline static const QObexAuthenticationResponsePrivate *getPrivate(const QObexAuthenticationResponse &response) { return response.m_data; }

    static QObexAuthenticationResponse createResponse(const QByteArray &challengeNonce);

    static bool parseRawResponse(const QByteArray &bytes,
                                 QObexAuthenticationResponse &response);
};

#endif
