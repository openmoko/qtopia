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

#ifndef QPRESENCE_H
#define QPRESENCE_H

#include <qtopiaglobal.h>
#include <qcomminterface.h>
#include <qtopiaipcmarshal.h>

class QPresencePrivate;

class QTOPIAPHONE_EXPORT QPresence : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(QPresence::Status localPresence READ localPresence WRITE setLocalPresence)
    Q_PROPERTY(QStringList monitoredUris READ monitoredUris)
    Q_PROPERTY(QStringList allMonitoredUris READ allMonitoredUris)
public:
    explicit QPresence( const QString& service = QString(),
                        QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QPresence();

    enum Status
    {
        Unavailable,
        Available
    };

    QPresence::Status localPresence() const;
    QStringList monitoredUris() const;
    QStringList allMonitoredUris() const;
    Q_INVOKABLE QPresence::Status monitoredUriStatus( const QString& uri ) const;

public slots:
    virtual bool startMonitoring( const QString& uri );
    virtual bool stopMonitoring( const QString& uri );
    virtual void setLocalPresence( QPresence::Status status );

signals:
    void monitoredPresence( const QString& uri, QPresence::Status status );
    void localPresenceChanged();

protected:
    void updateMonitoredPresence
            ( const QString& uri, QPresence::Status status );

private:
    QPresencePrivate *d;
};

Q_DECLARE_USER_METATYPE_ENUM(QPresence::Status)

#endif /* QPRESENCE_H */
