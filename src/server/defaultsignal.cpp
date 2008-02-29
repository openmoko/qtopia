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

#include "defaultsignal.h"
#include <QSettings>
#include <QSignalSource>
#include <QValueSpaceItem>
#include "qtopiaserverapplication.h"

/* 
   \class DefaultSignal
   \brief The DefaultSignal class provides a proxy for another system signal.

   To simplify the design of system themes and other mechanisms that source 
   information directly from the value space, the DefaultSignal class proxies
   the information of another, configurable system signal under the name
   "DefaultSignalSource".

   The Signal to proxy can be manually configured by setting the 
   \c {SignalSources/DefaultSignalSource} key in the \c {Trolltech/SignalStatus}
   configuration file.  If this setting is missing, the DefaultSignal class 
   will proxy the first available signal source of type modem. If Qtopia is configured
   without a modem the first wlan signal source that becomes available will be used.

   The DefaultSignal class provides the \c {DefaultSignal} task.
  */

/*!
  \internal
  */
DefaultSignal::DefaultSignal(QObject *parent)
: QSignalSourceProvider(QLatin1String("virtual"), QLatin1String("DefaultSignal"), parent),
  m_signalSource(0), m_signalSourceVS(0)
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("SignalStatus"));
    m_primary = cfg.value("SignalSources/DefaultSignalSource").toString();

    if(m_primary.isEmpty()) {
        m_signalSourceVS = new QValueSpaceItem( QLatin1String("/Hardware/Accessories/SignalSources") );
        connect( m_signalSourceVS, SIGNAL(contentsChanged()), this, SLOT(accessoryAdded()) );
    } else {
        m_signalSource = new QSignalSource(m_primary);
        initSignalSource();
    }
        
    syncSignalSource();
}

void DefaultSignal::syncSignalSource()
{
    if(m_signalSource) {
        setAvailability(m_signalSource->availability());
        setSignalStrength(m_signalSource->signalStrength());
    } else {
        setAvailability(QSignalSource::Invalid);
        setSignalStrength(-1);
    }
}

void DefaultSignal::initSignalSource()
{
    QObject::connect(m_signalSource, SIGNAL(availabilityChanged(QSignalSource::Availability)), this, SLOT(pAvailabilityChanged(QSignalSource::Availability)));
    QObject::connect(m_signalSource, SIGNAL(signalStrengthChanged(int)), this, SLOT(pSignalStrengthChanged(int)));
}

void DefaultSignal::accessoryAdded()
{
    QList<QString> signalSources = m_signalSourceVS->subPaths();
    
    for(int i = 0; m_signalSourceVS && i < signalSources.count(); i++) {
        QSignalSource *ps = new QSignalSource(signalSources.at(i));
        if ( ps->availability() == QSignalSource::Invalid ) {
            return;
        }
#if QTOPIA_CELL
        if ( ps->type() == QLatin1String("modem") ) {
#elif QTOPIA_VOIP
        if ( ps->type() == QLatin1String("wlan") ) {
#else
        if ( ps->type() != QLatin1String("virtual") ) {
#endif
            m_signalSource = ps;
            m_signalSourceVS->disconnect();
            m_signalSourceVS->deleteLater();
            m_signalSourceVS = 0;
            initSignalSource();
            syncSignalSource();
        }
    }
}

void DefaultSignal::pAvailabilityChanged(QSignalSource::Availability a)
{
    setAvailability(a);
}

void DefaultSignal::pSignalStrengthChanged(int c)
{
    setSignalStrength(c);
}

QTOPIA_TASK(DefaultSignal, DefaultSignal);
#include "defaultsignal.h"
