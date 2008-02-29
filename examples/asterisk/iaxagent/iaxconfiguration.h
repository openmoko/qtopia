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

#ifndef IAXCONFIGURATION_H
#define IAXCONFIGURATION_H

#include <qtelephonyconfiguration.h>

class IaxTelephonyService;

class IaxConfiguration : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    IaxConfiguration( IaxTelephonyService *service );
    ~IaxConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private:
    IaxTelephonyService *service;
};

#endif /* IAXCONFIGURATION_H */
