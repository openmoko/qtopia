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


#ifndef __MEDIASERVER_BUILDERNEGOTIATOR_H
#define __MEDIASERVER_BUILDERNEGOTIATOR_H


#include <QString>

#include <QMediaSessionBuilder>


class QMediaSessionBuilder;
class QMediaServerSession;


namespace mediaserver
{

class BuilderNegotiator : public QMediaSessionBuilder
{
public:
    virtual ~BuilderNegotiator();

    virtual void addBuilder(QString const& tag, int priority, QMediaSessionBuilder* sessionBuilder) = 0;
    virtual void removeBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder) = 0;
};

}   // ns mediaserver

#endif  // __MEDIASERVER_BUILDERNEGOTIATOR_H

