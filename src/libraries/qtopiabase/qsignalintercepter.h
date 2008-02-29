/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _QTOPIAREMOTE_QSIGNALINTERCEPTER_H
#define _QTOPIAREMOTE_QSIGNALINTERCEPTER_H

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>
#include <qtopiaglobal.h>

class QSignalIntercepterPrivate;

class QTOPIABASE_EXPORT QSignalIntercepter : public QObject
{
    // Note: Do not put Q_OBJECT here.
    friend class QSlotInvoker;
    friend class QCopProxy;
public:
    QSignalIntercepter( QObject *sender, const QByteArray& signal,
                        QObject *parent=0 );
    ~QSignalIntercepter();

    QObject *sender() const;
    QByteArray signal() const;

    bool isValid() const;

    static const int QVariantId = -243;

    static int *connectionTypes( const QByteArray& member, int& nargs );

protected:
    int qt_metacall( QMetaObject::Call c, int id, void **a );
    virtual void activated( const QList<QVariant>& args ) = 0;

private:
    QSignalIntercepterPrivate *d;

    static int typeFromName( const QByteArray& name );
};

#endif /* _QTOPIAREMOTE_QSIGNALINTERCEPTER_H */
