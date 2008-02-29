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

#ifndef QSIGNALSOURCE_H
#define QSIGNALSOURCE_H

#include <QHardwareInterface>
#include <qtopiaipcmarshal.h>

class QTOPIA_EXPORT QSignalSource : public QHardwareInterface
{
    Q_OBJECT
public:
    enum Availability { 
        Available, 
        NotAvailable,
        Invalid
    };
        
    explicit QSignalSource( const QString& id = QString(), QObject* parent = 0 );
    virtual ~QSignalSource();

    QString type() const;
    Availability availability() const;
    int signalStrength() const;

signals:
    void availabilityChanged( QSignalSource::Availability availability );
    void signalStrengthChanged( int strength );
    
private:
    friend class QSignalSourceProvider;
    QSignalSource( const QString& id, QObject* parent,
                            QAbstractIpcInterface::Mode mode );
};

Q_DECLARE_USER_METATYPE_ENUM(QSignalSource::Availability);

class QSignalSourceProviderPrivate;
class QTOPIA_EXPORT QSignalSourceProvider : public QSignalSource
{
    Q_OBJECT
public:
    explicit QSignalSourceProvider( const QString& type, const QString& id, QObject* parent = 0 );
    virtual ~QSignalSourceProvider();

public slots:
    void setAvailability( QSignalSource::Availability availability );
    void setSignalStrength( int currentStrength );

private slots:
    void update();

private:
    QSignalSourceProviderPrivate *d;
};

#endif //QSIGNALSOURCE_H
