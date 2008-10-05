/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QOBEXAUTHENTICATIONRESPONSE_H
#define QOBEXAUTHENTICATIONRESPONSE_H

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
