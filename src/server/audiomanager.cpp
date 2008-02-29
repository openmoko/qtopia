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

#ifdef HAVE_ALSA

#include "audiomanager.h"

#include <QTimer>
#include <qtopia/comm/qcommservicemanager.h>
#include <qtopia/comm/qbluetoothaudiogateway.h>
#include <qtopialog.h>
#include <qvaluespace.h>
#include "qtopiaserverapplication.h"

class AudioManagerPrivate : public QObject
{
    Q_OBJECT

public:
    AudioManagerPrivate(QObject *parent);
    ~AudioManagerPrivate();

public slots:
    void headsetAudioStateChanged();
    void handsfreeAudioStateChanged();
    void serviceAdded();
    void serviceRemoved(const QString &service);

private:
    QValueSpaceObject *m_valueSpace;
    QBluetoothAudioGateway *m_headset;
    QBluetoothAudioGateway *m_handsfree;
    QCommServiceManager *m_servicemanager;
};

AudioManagerPrivate::AudioManagerPrivate(QObject *parent)
    : QObject(parent)
{
    QByteArray path = "/AudioManager";
    m_valueSpace = new QValueSpaceObject( path, this );
    m_valueSpace->setAttribute("DefaultDevice", "default");
    m_valueSpace->setAttribute("Frequency", 44100);
    QValueSpaceObject::sync();

    m_headset = 0;
    m_handsfree = 0;

    m_servicemanager = new QCommServiceManager(this);
    QObject::connect(m_servicemanager, SIGNAL(serviceAdded(const QString &)),
                     this, SLOT(serviceAdded()));
    QObject::connect(m_servicemanager, SIGNAL(serviceRemoved(const QString &)),
                     this, SLOT(serviceRemoved(const QString &)));

    QTimer::singleShot(0, this, SLOT(serviceAdded()));
}

AudioManagerPrivate::~AudioManagerPrivate()
{
    delete m_valueSpace;
    if (m_headset)
        delete m_headset;
    delete m_servicemanager;
}

void AudioManagerPrivate::headsetAudioStateChanged()
{
    if (m_headset->audioEnabled()) {
        qLog(QtopiaServer) << "Setting to Headset device...";
        m_valueSpace->setAttribute("DefaultDevice", m_headset->audioDevice());
        m_valueSpace->setAttribute("Frequency", m_headset->audioFrequency());
    }
    else {
        qLog(QtopiaServer) << "Setting to default device...";
        m_valueSpace->setAttribute("DefaultDevice", "default");
        m_valueSpace->setAttribute("Frequency", 44100);
    }

    QValueSpaceObject::sync();
}

void AudioManagerPrivate::handsfreeAudioStateChanged()
{
    if (m_handsfree->audioEnabled()) {
        qLog(QtopiaServer) << "Setting to Handsfree device...";
        m_valueSpace->setAttribute("DefaultDevice", m_handsfree->audioDevice());
        m_valueSpace->setAttribute("Frequency", m_handsfree->audioFrequency());
    }
    else {
        qLog(QtopiaServer) << "Setting to default device...";
        m_valueSpace->setAttribute("DefaultDevice", "default");
        m_valueSpace->setAttribute("Frequency", 44100);
    }

    QValueSpaceObject::sync();
}

void AudioManagerPrivate::serviceAdded()
{
    if (!m_headset) {
        m_headset = new QBluetoothAudioGateway("BluetoothHeadset", this);
        if (m_headset->available()) {
            qLog(Bluetooth) << "Headset service was added...";
            QObject::connect(m_headset, SIGNAL(audioStateChanged()),
                            this, SLOT(headsetAudioStateChanged()));
            return;
        }

        delete m_headset;
        m_headset = 0;
    }

    if (!m_handsfree) {
        m_handsfree = new QBluetoothAudioGateway("BluetoothHandsfree", this);
        if (m_handsfree->available()) {
            qLog(Bluetooth) << "Handsfree service was added...";
            QObject::connect(m_handsfree, SIGNAL(audioStateChanged()),
                             this, SLOT(handsfreeAudioStateChanged()));
            return;
        }

        delete m_handsfree;
        m_handsfree = 0;
    }
}

void AudioManagerPrivate::serviceRemoved(const QString &service)
{
    if (m_headset && m_headset->service() == service) {
        m_valueSpace->setAttribute("DefaultDevice", "default");
        m_valueSpace->setAttribute("Frequency", 44100);

        delete m_headset;
        m_headset = 0;
        QValueSpaceObject::sync();

        qLog(Bluetooth) << "Headset service was removed..";
    }

    if (m_handsfree && m_handsfree->service() == service) {
        m_valueSpace->setAttribute("DefaultDevice", "default");
        m_valueSpace->setAttribute("Frequency", 44100);

        delete m_handsfree;
        m_handsfree = 0;
        QValueSpaceObject::sync();

        qLog(Bluetooth) << "Handsfree service was removed...";
    }
}

AudioManager::AudioManager( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "AudioManager started";

    m_data = new AudioManagerPrivate(this);
}

AudioManager::~AudioManager()
{
    if (m_data)
        delete m_data;
}

#include "audiomanager.moc"
QTOPIA_TASK( AudioManager, AudioManager );

#endif
