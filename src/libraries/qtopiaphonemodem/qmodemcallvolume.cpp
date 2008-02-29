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

#include <qmodemcallvolume.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qtopiaipcadaptor.h>
#include <QtopiaIpcEnvelope>

#include <QSettings>
#include <QTimer>

#include <QDebug>

/*!
    \class QModemCallVolume
    \mainclass
    \preliminary
    \brief The QModemCallVolume class implements call volume functionality for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+VGR} and \c{AT+VGT} commands.

    QModemCallVolume implements the QCallVolume telephony interface.  Client applications
    should use QCallVolume instead of this class to access the modem's call volume
    functionality.

    \sa QCallVolume
*/

class QModemVolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    explicit QModemVolumeService( QModemCallVolume *callVolume );
    ~QModemVolumeService();

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

private slots:
    void registerService();

private:
    QModemCallVolume *callVolume;
};

QModemVolumeService::QModemVolumeService( QModemCallVolume *callVolume )
    : QtopiaIpcAdaptor("QPE/AudioVolumeManager/QModemVolumeService", callVolume)
{
    this->callVolume = callVolume;

    publishAll(Slots);

    QTimer::singleShot(0, this, SLOT(registerService()));
}

QModemVolumeService::~QModemVolumeService()
{
}

void QModemVolumeService::setVolume(int volume)
{
    callVolume->setSpeakerVolume(volume);
}

void QModemVolumeService::increaseVolume(int increment)
{
    callVolume->setSpeakerVolume(callVolume->speakerVolume() + increment);
}

void QModemVolumeService::decreaseVolume(int decrement)
{
    callVolume->setSpeakerVolume(callVolume->speakerVolume() - decrement);
}

void QModemVolumeService::setMute(bool mute)
{
    Q_UNUSED(mute);
}

void QModemVolumeService::registerService()
{
    QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Phone") << QString("QPE/AudioVolumeManager/QModemVolumeService");
}


class QModemCallVolumePrivate
{
public:
    QModemService *service;
    QModemVolumeService *mvs;
};


/*!
    Create an AT-based call volume handler for \a service.
*/
QModemCallVolume::QModemCallVolume( QModemService *service )
    : QCallVolume( service->service(), service, QCommInterface::Server )
{
    d = new QModemCallVolumePrivate;
    d->service = service;
    d->mvs = new QModemVolumeService(this);

    QTimer::singleShot(0, this, SLOT(initialize()));
}

/*!
    Destroy this AT-based call volume handler.
*/
QModemCallVolume::~QModemCallVolume()
{
    delete d;
}

/*!
    Returns true if the modem requires delayed initialization of call volume
    settings; false otherwise.  The default implementation returns false.

    Subclasses that reimplement this function to return true, must call
    callVolumesReady() when the modem is ready to accept call volume related
    commands.

    \sa callVolumesReady()
*/
bool QModemCallVolume::hasDelayedInit() const
{
    return false;
}

/*!
    \reimp
*/
void QModemCallVolume::setSpeakerVolume( int volume )
{
    int boundedVolume = qBound(value("MinimumSpeakerVolume").toInt(), volume,
                               value("MaximumSpeakerVolume").toInt());

    d->service->chat( QString("AT+VGR=%1").arg(boundedVolume) );
    setValue( "SpeakerVolume", boundedVolume );
    emit speakerVolumeChanged(boundedVolume);
}

/*!
    \reimp
*/
void QModemCallVolume::setMicrophoneVolume( int volume )
{
    int boundedVolume = qBound(value("MinimumMicrophoneVolume").toInt(), volume,
                              value("MaximumMicrophoneVolume").toInt());

    d->service->chat( QString("AT+VGT=%1").arg(boundedVolume) );
    setValue( "MicrophoneVolume", boundedVolume );
    emit microphoneVolumeChanged(boundedVolume);
}

/*!
    Indicate that the modem is now ready to accept call volume related
    commands.  See hasDelayedInit() for more information.

    \sa hasDelayedInit()
*/
void QModemCallVolume::callVolumesReady()
{
    d->service->chat( "AT+VGR=?", this, SLOT(vgrRangeQuery(bool,QAtResult)) );
    d->service->chat( "AT+VGR?", this, SLOT(vgrQuery(bool,QAtResult)) );
    d->service->chat( "AT+VGT=?", this, SLOT(vgtRangeQuery(bool,QAtResult)) );
    d->service->chat( "AT+VGT?", this, SLOT(vgtQuery(bool,QAtResult)) );
}

void QModemCallVolume::initialize()
{
    if (!hasDelayedInit())
        callVolumesReady();
}

void QModemCallVolume::vgrRangeQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    int minimum = 0;
    int maximum = 0;

    QAtResultParser parser( result );
    while ( parser.next( "+VGR:" ) ) {
        QList<QAtResultParser::Node> nodes = parser.readList();
        if (!nodes.isEmpty()) {
            if (nodes.at(0).isRange()) {
                minimum = nodes.at(0).asFirst();
                maximum = nodes.at(0).asLast();
            }
        }
    }

    setValue( "MinimumSpeakerVolume", minimum );
    setValue( "MaximumSpeakerVolume", maximum );
}

void QModemCallVolume::vgrQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    QAtResultParser parser( result );
    if ( parser.next( "+VGR:" ) ) {
        int modemDefault = parser.readNumeric();

        QSettings cfg( "Trolltech", "Phone" );
        cfg.beginGroup( "CallVolume" );

        int defaultVolume = cfg.value( "SpeakerVolume", modemDefault ).toInt();

        if (defaultVolume != modemDefault)
            d->service->chat( QString("AT+VGR=%1").arg(defaultVolume) );

        setValue( "SpeakerVolume", defaultVolume );
    }
}

void QModemCallVolume::vgtRangeQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    int minimum = 0;
    int maximum = 0;

    QAtResultParser parser( result );
    if ( parser.next( "+VGT:" ) ) {
        QList<QAtResultParser::Node> nodes = parser.readList();
        if (!nodes.isEmpty()) {
            if (nodes.at(0).isRange()) {
                minimum = nodes.at(0).asFirst();
                maximum = nodes.at(0).asLast();
            }
        }
    }

    setValue( "MinimumMicrophoneVolume", minimum );
    setValue( "MaximumMicrophoneVolume", maximum );
}

void QModemCallVolume::vgtQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    QAtResultParser parser( result );
    if ( parser.next( "+VGT:" ) ) {
        int modemDefault = parser.readNumeric();

        QSettings cfg( "Trolltech", "Phone" );
        cfg.beginGroup( "CallVolume" );

        int defaultVolume = cfg.value( "MicrophoneVolume", modemDefault ).toInt();

        if (defaultVolume != modemDefault)
            d->service->chat( QString("AT+VGT=%1").arg(defaultVolume) );

        setValue( "MicrophoneVolume", defaultVolume );
    }
}

#include <qmodemcallvolume.moc>
