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

#ifndef QMODEMPINMANAGER_P_H
#define QMODEMPINMANAGER_P_H

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

#include <qobject.h>

class QModemPendingPin : public QObject
{
    Q_OBJECT
public:
    explicit QModemPendingPin( QObject *parent = 0 ) : QObject( parent ) {}
    ~QModemPendingPin() {}

    QString type() const { return _type; }
    void setType( const QString& value ) { _type = value; }

    void emitHavePin( const QString& pin ) { emit havePin( pin ); }
    void emitCancelPin() { emit cancelPin(); }

signals:
    void havePin( const QString& pin );
    void cancelPin();

private:
    QString _type;
};

#endif /* QMODEMPINMANAGER_P_H */
