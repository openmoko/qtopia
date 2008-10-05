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

#ifndef QMODEMPINMANAGER_P_H
#define QMODEMPINMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
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

#endif
