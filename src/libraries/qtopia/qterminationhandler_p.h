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

#ifndef _QTERMINATIONHANDLERDATA_H_
#define _QTERMINATIONHANDLERDATA_H_

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

#include "qterminationhandler.h"

#include <QString>
#include <QtopiaServiceRequest>
#include <QDataStream>

struct QTerminationHandlerData
{
    QTerminationHandlerData()
    {
    }

    QTerminationHandlerData( const QString& n, const QString& t, const QString& bt, const QString& i, const QtopiaServiceRequest& a )
        : name(n), text(t), buttonText(bt), buttonIcon(i), action(a)
    {
    }

    QTerminationHandlerData( const QTerminationHandlerData& other ) {
        (*this) = other;
    }

    QTerminationHandlerData& operator=( const QTerminationHandlerData& other ) {
        name = other.name;
        text = other.text;
        buttonText = other.buttonText;
        buttonIcon = other.buttonIcon;
        action = other.action;
        return *this;
    }

    QString name;
    QString text;
    QString buttonText;
    QString buttonIcon;
    QtopiaServiceRequest action;
};

inline QDataStream& operator<<( QDataStream& ostream, const QTerminationHandlerData& data ) {
    ostream << data.name << data.text << data.buttonText << data.buttonIcon << data.action;
    return ostream;
}

inline QDataStream& operator>>( QDataStream& istream, QTerminationHandlerData& data ) {
    QString n, t, bt, bi;
    QtopiaServiceRequest a;
    /*
    istream >> n;
    istream >> t;
    istream >> bt;
    istream >> bi;
    istream >> a;
    */
    istream >> n >> t >> bt >> bi >> a;
    data.name = n;
    data.text = t;
    data.buttonText = bt;
    data.buttonIcon = bi;
    data.action = a;
    return istream;
}

#endif
