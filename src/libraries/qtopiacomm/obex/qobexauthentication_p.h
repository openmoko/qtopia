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
#ifndef __QOBEXAUTHENTICATION_P_H__
#define __QOBEXAUTHENTICATION_P_H__

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

#include <qobexglobal.h>

#include <QByteArray>

class QTOPIA_AUTOTEST_EXPORT QObexAuth
{
public:
    enum DigestChallengeTag
    {
        ChallengeNonceTag = 0x00,
        OptionsTag = 0x01,
        RealmTag = 0x02
    };

    enum DigestResponseTag
    {
        RequestDigestTag = 0x00,
        UserIdTag = 0x01,
        ResponseNonceTag = 0x02
    };

    enum RealmCharSetCode
    {
        CharSetAscii = 0,
        CharSetISO8859_1 = 1,
        CharSetISO8859_2 = 2,
        CharSetISO8859_3 = 3,
        CharSetISO8859_4 = 4,
        CharSetISO8859_5 = 5,
        CharSetISO8859_6 = 6,
        CharSetISO8859_7 = 7,
        CharSetISO8859_8 = 8,
        CharSetISO8859_9 = 9,
        CharSetUnicode = 0xFF
    };

    enum TagValueSize
    {
        NonceSize = 16,
        OptionsSize = 1,
        RequestDigestSize = 16
    };

    static void generateNonce(QByteArray &buf);
};

#endif
