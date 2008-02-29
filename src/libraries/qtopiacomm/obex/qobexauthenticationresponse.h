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
#ifndef __QOBEXAUTHENTICATIONRESPONSE_H__
#define __QOBEXAUTHENTICATIONRESPONSE_H__

#include <qobexglobal.h>

class QObexAuthenticationResponsePrivate;

class QTOPIAOBEX_EXPORT QObexAuthenticationResponse
{
public:
    QObexAuthenticationResponse(const QObexAuthenticationResponse &other);
    ~QObexAuthenticationResponse();

    QObexAuthenticationResponse &operator=(const QObexAuthenticationResponse &other);

    QString user() const;
    bool match(const QString &password) const;

private:
    QObexAuthenticationResponse();
    QObexAuthenticationResponsePrivate *m_data;
    friend class QObexAuthenticationResponsePrivate;
};

#endif
