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

#ifndef QTELEPHONYCONFIGURATION_H
#define QTELEPHONYCONFIGURATION_H

#include <qcomminterface.h>

class QTOPIAPHONE_EXPORT QTelephonyConfiguration : public QCommInterface
{
    Q_OBJECT
public:
    explicit QTelephonyConfiguration( const QString& service,
                                      QObject *parent = 0,
                                      QCommInterface::Mode mode = Client );
    ~QTelephonyConfiguration();

public slots:
    virtual void update( const QString& name, const QString& value );
    virtual void request( const QString& name );

signals:
    void notification( const QString& name, const QString& value );
};

#endif /* QTELEPHONYCONFIGURATION_H */
