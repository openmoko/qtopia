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


#ifndef __MEDIASERVER_URINEGOTIATOR_H
#define __MEDIASERVER_URINEGOTIATOR_H

#include "buildernegotiator.h"


namespace mediaserver
{

class UriNegotiatorPrivate;

class UriNegotiator : public BuilderNegotiator
{
public:
    UriNegotiator();
    ~UriNegotiator();

    QString type() const;
    Attributes const& attributes() const;

    void addBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder);
    void removeBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder);

    QMediaServerSession* createSession(QMediaSessionRequest sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

private:
    UriNegotiatorPrivate*   d;
};

}   // ns mediaserver

#endif  // __MEDIASERVER_URINEGOTIATOR_H
