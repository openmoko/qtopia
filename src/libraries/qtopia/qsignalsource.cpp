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

#include "qsignalsource.h"

static const char* const QSIGNALPROVIDER_NAME = "QSignalSource";
static const char* const QSIGNALPROVIDER_TYPE = "Type";
static const char* const QSIGNALPROVIDER_AVAILABILITY = "Availability";
static const char* const QSIGNALPROVIDER_SIGNAL_STRENGTH = "SignalStrength";



/*!
  \class QSignalSource 
  \mainclass
  \brief The QSignalSource class provides access to information about signal sources on devices.

  Signal sources are accessories that provide radio signal details to Qtopia. A 
  signal source could be an internal modem monitoring the GSM/3G network signal 
  or a wireless LAN interface on VoIP devices as indicated by type().
  QSignalSource allows applications to query the availability through availability(),
  the signal level through signalStrength() and emits signals availabilityChanged() and
  signalStrengthChanged() when those values change.

  In addition to the above hardware related signal sources Qtopia provides a virtual 
  default signal source. This default source is selected from the list of available 
  QSignalSource providers. The selection may be configured
  in the \c{Trolltech/SignalStatus} configuration file. The following keys apply:

  \table
  \header   \o key    \o Decription
  \row      \o SignalSources/DefaultSignalSource       
            \o Name (ID) of the QSignalSource to use as the default signal source.
  \endtable

  If the default signal source is not explicitly configured via the configuration file above 
  the modem signal source is preferred over a WLAN signal source. If there
  are several signal sources of the same type the first signal source that 
  is created will be used. If the default signal source is configured, but the 
  specified provider does not exist, then QSignalSource becomes invalid. The default signal
  is provided by the DefaultSignal server task.

  A specific QSignalSource can be selected as shown in the following example:

  \code
        QString requestedType = "wlan";
        QHardwareManager man( "QSignalSource" );
        QStringList providers = man.providers();

        QSignalSource* src = 0;
        //find a WLAN signal source
        foreach( QString signalSourceId, providers )
        {
            src = new QSignalSource( signalSourceId, this );
            if ( src->type() == requestedType ) {
                break;
            } else {
                delete src;
                src = 0;
            }
        }

        if ( !src ) {
            //could not find signal source for WLAN 
            //fall back to virtual default signal source
            src = new QSignalSource( "DefaultSignal", this );
            if ( src->availability() == QSignalSource::Invalid ) {
                //no signal source available at all
            }
        }
  \endcode

  New signal sources can be added to Qtopia via the QSignalSourceProvider class.
  
  \sa QSignalSourceProvider, QHardwareManager, DefaultSignal

  \ingroup hardware
  */

/*!
  \enum QSignalSource::Availability

  Represents whether the signal source is available.

  \value Available The signal source is available.
  \value NotAvailable The Signal source is not available. If the signal source is of 
        type \c "wlan" the signal strength is temporarily not available while the WLAN 
        interface is not connected. 
  \value Invalid The signal source is not valid. This is distinct from the not available case. 
        A reason for this state could be that QSignalSource was initialised with an invalid/not existing 
        ID or in the case of the default signal source there simply is no signal source available
        that the default signal source could map to.
  */

/*!
  \fn void QSignalSource::availabilityChanged( QSignalSource::Availability availability )

  This signal is emitted whenever the availability of the signal source changes;
  \a availability is the new value.
  */

/*!
  \fn void QSignalSource::signalStrengthChanged( int signalStrength )

  This signal is emitted whenever the strength of the signal changes; \a signalStrength 
  is the new value.
  */

/*!
  Constructs a new signal source for provider \a id with the specified \a parent.

  If \a id is not passed, this class will use the default signal source.
*/  
QSignalSource::QSignalSource( const QString& id, QObject* parent )
    : QHardwareInterface( QSIGNALPROVIDER_NAME, id.isEmpty() ? "DefaultSignal": id, parent, Client )
{
    proxy( SIGNAL(availabilityChanged(QSignalSource::Availability)) );
    proxy( SIGNAL(signalStrengthChanged(int)) );
}

QSignalSource::QSignalSource( const QString& id, QObject* parent,
                            QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( QSIGNALPROVIDER_NAME, id, parent, mode )
{
    proxy( SIGNAL(availabilityChanged(QSignalSource::Availability)) );
    proxy( SIGNAL(signalStrengthChanged(int)) );
}

/*!
  Destroys the QSignalSource.
   */
QSignalSource::~QSignalSource()
{
}

/*!
  Returns the type of the signal source, e.g. \c "wlan", \c "modem", or an empty
  string if this signal source is invalid.
  */
QString QSignalSource::type() const
{
    return value( QSIGNALPROVIDER_TYPE, QString() ).toString();
}

/*!
  Returns the signal source availability.
  */
QSignalSource::Availability QSignalSource::availability() const
{
    QString t = value(QSIGNALPROVIDER_AVAILABILITY).toString();
    if ( t == QLatin1String("Available") )
        return Available;
    else if ( t == QLatin1String("NotAvailable") )
        return NotAvailable;
    else
        return Invalid;
}

/*!
  Returns the signal source strength as a percentage of the maximum strength, 
  or -1 if the strength is unavailable or invalid.
  */
int QSignalSource::signalStrength() const
{
    return value( QSIGNALPROVIDER_SIGNAL_STRENGTH, -1 ).toInt();
}

struct QSignalSourceProviderPrivate
{
    int strength;
    QSignalSource::Availability avail;
};

/*!
  \class QSignalSourceProvider
  \mainclass
  \brief The QSignalSourceProvider class provides an interface for signal sources to 
  integrate into Qtopia.

  Signal sources are components that provide radio signal details to Qtopia. 
  A signal source could be an internal modem monitoring the GSM/3G network 
  signal or a wireless LAN interface on VoIP devices. QSignalSource
  allows applications to query the status of such signals.

  Every device wanting to publish a new signal source should create a 
  QSignalSourceProvider instance. This will create a new signal source in Qtopia 
  which can be accessed via the QSignalSource class. The setAvailability() function 
  should be used to determine the general availability and setSignalStrength() should
  be called when indicating the exact strength of the signal as a percentage of the 
  maximum strength.

  The following code creates a signal source for a PCMCIA based WLAN card which has a 
  signal strength of 80% of its maximum value.

  \code
    QSignalSourceProvider* provider = new QSignalSourceProvider( "wlan", "pcmciaCard" );
    provider->setSignalStrength( 80 );
    provider->setAvailability( QSignalSource::Available );
  \endcode

  \sa QSignalSource, DefaultSignal
  \ingroup hardware
  */

/*!
  Creates a new QSignalSourceProvider with the given \a type, \a id and \a parent.
  The constructed provider immediately becomes visible to Qtopia as \c NotAvailable signal
  source.

  \sa QSignalSource
  */
QSignalSourceProvider::QSignalSourceProvider( const QString& type, const QString& id, QObject* parent )
    : QSignalSource( id, parent, Server )
{
    Q_ASSERT( !type.isEmpty() );
    d = new QSignalSourceProviderPrivate();
    d->avail = QSignalSource::Invalid;
    d->strength = -1;
    
    setValue( QSIGNALPROVIDER_TYPE, type );    
    setAvailability( QSignalSource::NotAvailable );
}

/*!
  Destroys the QSignalSourceProvider instance and removes information 
  about the signal source from Qtopia.
  */
QSignalSourceProvider::~QSignalSourceProvider()
{
    delete d;
}

/*!
  Sets the \a availability of the signal source. 

  \sa QSignalSource
  */
void QSignalSourceProvider::setAvailability( QSignalSource::Availability availability )
{
    if ( availability == d->avail )
        return;

    QString t;
    switch( availability )
    {
        case Available:
            t = QLatin1String("Available");
            break;
        case NotAvailable:
            t = QLatin1String("NotAvailable");
            break;
        case Invalid:
            t = QLatin1String("Invalid");
            break;
    } 

    d->avail = availability;
    setValue( QSIGNALPROVIDER_AVAILABILITY, t );
    emit availabilityChanged( availability );
}

/*!
  Sets the signal source \a strength as a percentage of the maximum strength. If the signal
  strength is not available \a strength should be set to -1.

  \sa QSignalSource
  */
void QSignalSourceProvider::setSignalStrength(int strength)
{
    Q_ASSERT( strength <= 100 && strength >= -1 );
    if ( strength == d->strength )
        return;

    d->strength = strength;
    setValue( QSIGNALPROVIDER_SIGNAL_STRENGTH, strength );
    emit signalStrengthChanged( strength );
    
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QSignalSource::Availability);
