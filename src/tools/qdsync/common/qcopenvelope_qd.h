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
#ifndef QCOPENVELOPE_QD_H
#define QCOPENVELOPE_QD_H

#include <qdglobal.h>

#include <QDataStream>
#include <QString>

namespace qdsync {

class QD_EXPORT QCopEnvelope : public QDataStream
{
public:
    QCopEnvelope( const QString &channel, const QString &message );
    ~QCopEnvelope();

private:
    QString ch, msg;
};

};

using namespace qdsync;

#endif
