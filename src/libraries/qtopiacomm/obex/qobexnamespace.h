/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef __QOBEXNAMESPACE_H__
#define __QOBEXNAMESPACE_H__

#include <qobexglobal.h>

#ifndef Q_QDOC
namespace QObex
{
#else
class QObex
{
public:
#endif

    enum Request {
        Connect,
        Disconnect,
        Put,
        PutDelete,
        Get,
        SetPath,
        NoRequest = 100
    };

    enum ResponseCode {
        Success = 0x20,
        Created = 0x21,
        Accepted = 0x22,
        NonAuthoritative = 0x23,
        NoContent = 0x24,
        ResetContent = 0x25,
        PartialContent = 0x26,

        MultipleChoices = 0x30,
        MovedPermanently = 0x31,
        MovedTemporarily = 0x32,
        SeeOther = 0x33,
        NotModified = 0x34,
        UseProxy = 0x35,

        BadRequest = 0x40,
        Unauthorized = 0x41,
        PaymentRequired = 0x42,
        Forbidden = 0x43,
        NotFound = 0x44,
        MethodNotAllowed = 0x45,
        NotAcceptable = 0x46,
        ProxyAuthenticationRequired = 0x47,
        RequestTimeOut = 0x48,
        Conflict = 0x49,
        Gone = 0x4a,
        LengthRequired = 0x4b,
        PreconditionFailed = 0x4c,
        RequestedEntityTooLarge = 0x4d,
        RequestedUrlTooLarge = 0x4e,
        UnsupportedMediaType = 0x4f,

        InternalServerError = 0x50,
        NotImplemented = 0x51,
        BadGateway = 0x52,
        ServiceUnavailable = 0x53,
        GatewayTimeout = 0x54,
        HttpVersionNotSupported = 0x55,

        DatabaseFull = 0x60,
        DatabaseLocked = 061
    };

    enum SetPathFlag {
        BackUpOneLevel = 0x1,
        NoPathCreation = 0x2
    };
    Q_DECLARE_FLAGS(SetPathFlags, SetPathFlag)

    enum AuthChallengeOption {
        UserIdRequired = 0x01,
        ReadOnlyAccess = 0x2
    };
    Q_DECLARE_FLAGS(AuthChallengeOptions, AuthChallengeOption)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QObex::SetPathFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(QObex::AuthChallengeOptions);

#endif
