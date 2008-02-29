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

#ifndef _GSMKEYFILTER_H_
#define _GSMKEYFILTER_H_

#include <qtopiaipcmarshal.h>
#include <QObject>
#include <QFlags>
#include <QRegExp>

class GsmKeyFilterPrivate;

class GsmKeyFilter : public QObject
{
    Q_OBJECT
public:
    explicit GsmKeyFilter( QObject *parent = 0 );
    ~GsmKeyFilter();

    enum Flag
    {
        Send        = (1<<0),
        Immediate   = (1<<1),
        OnCall      = (1<<2),
        Incoming    = (1<<3),
        BeforeDial  = (1<<4),
        TestOnly    = (1<<5)
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum ServiceAction
    {
        Activate,
        Deactivate,
        Interrogate,
        Registration,
        Erasure
    };

    bool filter( const QString& digits, Flags flags );

    void addAction
        ( const QString& digits, QObject *target, const char *slot,
          GsmKeyFilter::Flags flags = GsmKeyFilter::Immediate );
    void addAction
        ( const QRegExp& regex, QObject *target, const char *slot,
          GsmKeyFilter::Flags flags = GsmKeyFilter::Immediate );

    void addService
        ( const QString& code, QObject *target, const char *slot,
          GsmKeyFilter::Flags flags = GsmKeyFilter::Send );

signals:
    void setBusy();
    void releaseHeld();
    void releaseActive();
    void releaseAllAcceptIncoming();
    void release( int call );
    void activate( int call );
    void swap();
    void join();
    void transfer();
    void deflect( const QString& number );

private slots:
    void releaseId( const QString& id );
    void activateId( const QString& id );

private:
    GsmKeyFilterPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GsmKeyFilter::Flags);
Q_DECLARE_USER_METATYPE_ENUM(GsmKeyFilter::ServiceAction);

#endif // _GSMKEYFILTER_H_
