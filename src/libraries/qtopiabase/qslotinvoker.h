/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef _QTOPIAREMOTE_QSLOTINVOKER_H
#define _QTOPIAREMOTE_QSLOTINVOKER_H

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>
#include <qtopiaglobal.h>

class QSlotInvokerPrivate;

class QTOPIABASE_EXPORT QSlotInvoker : public QObject
{
    Q_OBJECT
public:
    QSlotInvoker( QObject *receiver, const QByteArray& member,
                  QObject *parent=0 );
    ~QSlotInvoker();

    bool memberPresent() const;
    bool canInvoke( int numArgs ) const;
    QObject *receiver() const;
    QByteArray member() const;
    int *parameterTypes() const;
    int numParameterTypes() const;

public slots:
    QVariant invoke( const QList<QVariant>& args );

private slots:
    void receiverDestroyed();

private:
    QSlotInvokerPrivate *d;
};

#endif /* _QTOPIAREMOTE_QSLOTINVOKER_H */
