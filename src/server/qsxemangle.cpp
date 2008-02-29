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

#include "qsxemangle.h"
#include <qsiminfo.h>
#include <qtelephonyconfiguration.h>

/*!
  \class IdMangler
  \ingroup QtopiaServer
  \brief The IdMangler class servers to randomize key generation for SXE.

  When running on a device with a GSM modem, attempt to retrieve the
  devices IMSI and IMEI numbers, and use these to permute the key value

  This makes it extremely difficult to attack the randomness of the key
  without the attacker (usually the author of downloaded software)
  knowing these numbers.
*/

/*!
  Construct an IdMangler instance with the specified \a parent.
  */
IdMangler::IdMangler( QObject *parent )
    : QObject( parent )
{
    init();
}

/*!
  Destroy the IdMangler instance.
  */
IdMangler::~IdMangler()
{
}

/*!
  \fn void IdMangler::gotIds(const QString &value)

  Emitted whenever the id is retrieved or changes.  \a value is the new value.
 */
void IdMangler::init()
{
    simInfo = new QSimInfo( QString(), this );
    connect( simInfo, SIGNAL(inserted()), this, SLOT(simIdentityChanged()) );
    simIdentityChanged();   // In case it is already available.

    QTelephonyConfiguration *config;
    config = new QTelephonyConfiguration( "modem", this );  // No tr
    connect( config, SIGNAL(notification(QString,QString)),
             this, SLOT(notification(QString,QString)) );
    config->request( "serial" );
}

void IdMangler::simIdentityChanged()
{
    QString id = simInfo->identity();
    if ( !id.isEmpty() )
        emit gotIds( id );
}

void IdMangler::notification( const QString& name, const QString& value )
{
    if ( name == "serial " )
        emit gotIds( value );
}
