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

#include "headset.h"

#include <qtopiaapplication.h>
#include <qtopia/comm/qbluetoothaudiogateway.h>
#include <qtopia/comm/qbluetoothdeviceselector.h>
#include <qtopia/comm/qbluetoothlocaldevice.h>
#include <qtopia/comm/qbluetoothremotedevice.h>
#include <qtopia/comm/qsdap.h>
#include <qwaitwidget.h>
#include <qtopialog.h>

HeadsetDialog::HeadsetDialog(QBluetooth::SDPProfile profile, QWidget *parent, Qt::WFlags)
    : QDialog(parent), m_ui(0), m_ag(0), m_waitWidget(0), m_sdap(0), m_profile(profile)
{
    QString serviceString;
    QString prettyProfileString;

    if (profile == QBluetooth::HeadsetProfile) {
        serviceString = "BluetoothHeadset";
        prettyProfileString = tr("Headset");
    }
    else if (profile == QBluetooth::HandsFreeProfile) {
        serviceString = "BluetoothHandsfree";
        prettyProfileString = tr("Handsfree");
    }

    m_ag = new QBluetoothAudioGateway(serviceString);
    setWindowTitle(prettyProfileString);

    if (!m_ag->available()) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QString errorMsg;
        if ( profile == QBluetooth::HandsFreeProfile )
            errorMsg = tr("<P>No handsfree support found");
        else if ( profile == QBluetooth::HeadsetProfile )
            errorMsg = tr("<P>No headset support found");
        QLabel * label = new QLabel( errorMsg );
        label->setWordWrap(true);
        layout->addWidget(label);
        setLayout(layout);
        return;
    }

    m_ui = new Ui::MyHeadset();
    m_ui->setupUi(this);

    m_ui->profileLabel->setText(prettyProfileString);
    m_ui->audioIO->setItemText(1, prettyProfileString);
    m_ui->headsetTabWidget->setTabText(0, prettyProfileString);

    connect(m_ag, SIGNAL(speakerVolumeChanged()),
            this, SLOT(speakerVolumeChanged()));
    connect(m_ag, SIGNAL(microphoneVolumeChanged()),
            this, SLOT(microphoneVolumeChanged()));
    connect(m_ui->micVol, SIGNAL(valueChanged(int)),
            m_ag, SLOT(setMicrophoneVolume(int)));
    connect(m_ui->spkrVol, SIGNAL(valueChanged(int)),
            m_ag, SLOT(setSpeakerVolume(int)));

    connect(m_ui->audioIO, SIGNAL(activated(int)),
            this, SLOT(changeAudio(int)));

    connect(m_ag, SIGNAL(audioStateChanged()),
            this, SLOT(audioStateChanged()));

    connect(m_ag, SIGNAL(connectResult(bool, const QString &)),
            this, SLOT(connected(bool, const QString &)));
    connect(m_ag, SIGNAL(headsetDisconnected()),
            this, SLOT(disconnected()));
    connect(m_ag, SIGNAL(newConnection(const QBluetoothAddress &)),
            this, SLOT(newConnection(const QBluetoothAddress &)));

    m_waitWidget = new QWaitWidget( 0 );
    m_sdap = new QSDAP();
    connect( m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult&)),
             this, SLOT(searchComplete(const QSDAPSearchResult&)) );

    audioStateChanged();
    microphoneVolumeChanged();
    speakerVolumeChanged();

    if (m_ag->isConnected()) {
        newConnection(m_ag->remotePeer());
    }
    else {
        disconnected();
    }
}

HeadsetDialog::~HeadsetDialog()
{
    delete m_ui;
    delete m_ag;
    delete m_waitWidget;
    delete m_sdap;
}

void HeadsetDialog::start()
{
    this->setModal( true );
    QtopiaApplication::execDialog( this );
}

void HeadsetDialog::speakerVolumeChanged()
{
    m_ui->spkrVol->blockSignals(true);
    m_ui->spkrVol->setValue(m_ag->speakerVolume());
    m_ui->spkrVol->blockSignals(false);
}

void HeadsetDialog::microphoneVolumeChanged()
{
    m_ui->micVol->blockSignals(true);
    m_ui->micVol->setValue(m_ag->microphoneVolume());
    m_ui->micVol->blockSignals(false);
}

void HeadsetDialog::audioStateChanged()
{
    if (m_ag->audioEnabled()) {
        m_ui->audioIO->setCurrentIndex(1);
    }
    else {
        m_ui->audioIO->setCurrentIndex(0);
    }
}

void HeadsetDialog::changeAudio(int item)
{
    if (item == 0) {
        m_ag->releaseAudio();
    }
    else {
        m_ag->connectAudio();
    }
}

void HeadsetDialog::searchComplete(const QSDAPSearchResult &result)
{
    qLog(Bluetooth) << "Service searching complete";

    foreach ( QSDPService service, result.services() ) {
        if ( QSDPService::isInstance( service, m_profile ) ) {
            int channel = QSDPService::rfcommChannel(service);
            if (channel != -1) {
                m_waitWidget->setText( tr("Connecting...") );
                m_ag->connect(m_addr, channel);
                return;
            }
        }
    }

    QMessageBox::warning( 0, tr( "Error" ),
                          tr( "<qt>The selected device does not support this service</qt>" ),
                          QMessageBox::Ok );
    m_waitWidget->hide();
}

void HeadsetDialog::connected(bool success, const QString &msg)
{
    Q_UNUSED(msg);

    if (success) {
        disconnect(m_ui->connectButton, SIGNAL(pressed()),
                   this, SLOT(doConnect()));
        connect(m_ui->connectButton, SIGNAL(pressed()),
                   this, SLOT(doDisconnect()));
        m_ui->connectButton->setText( tr("Disconnect") );
        m_waitWidget->hide();
        m_ui->headsetInfo->setText(m_addr.toString());
        m_ui->headsetTabWidget->setTabEnabled(1, true);
    }
    else {
        QMessageBox::warning( 0, tr( "Error" ),
                                  tr( "<qt>Connection to headset failed.</qt>" ),
                                  QMessageBox::Ok );
        m_waitWidget->hide();
    }
}

void HeadsetDialog::doConnect()
{
    QSet<QBluetooth::SDPProfile> profiles;
    profiles.insert(m_profile);
    QBluetoothAddress addr = QBluetoothDeviceSelector::getRemoteDevice(profiles);

    if ( addr.valid() ) {
        m_addr = addr;
        m_waitWidget->setText( tr( "Searching..." ) );
        m_waitWidget->setCancelEnabled(true);
        connect(m_waitWidget, SIGNAL(cancelled()),
                m_waitWidget, SLOT(reject()));
        connect(m_waitWidget, SIGNAL(cancelled()),
                m_sdap, SLOT(cancelSearch()));
        QBluetoothLocalDevice localDev;
        m_sdap->searchServices( m_addr, localDev, m_profile );
        m_waitWidget->show();
    }
}

void HeadsetDialog::doDisconnect()
{
    m_ag->disconnect();
    m_waitWidget->setText( tr("Disconnecting") );
    m_waitWidget->show();
}

void HeadsetDialog::disconnected()
{
    disconnect(m_ui->connectButton, SIGNAL(pressed()),
               this, SLOT(doDisconnect()));
    connect(m_ui->connectButton, SIGNAL(pressed()),
               this, SLOT(doConnect()));
    m_ui->connectButton->setText( tr("Connect") );
    m_ui->headsetInfo->setText( tr("None") );
    m_waitWidget->hide();
    m_ui->headsetTabWidget->setTabEnabled(1, false);
    m_ui->headsetTabWidget->setCurrentIndex(0);
}

void HeadsetDialog::newConnection(const QBluetoothAddress &addr)
{
    disconnect(m_ui->connectButton, SIGNAL(pressed()),
               this, SLOT(doConnect()));
    connect(m_ui->connectButton, SIGNAL(pressed()),
            this, SLOT(doDisconnect()));
    m_ui->connectButton->setText( tr("Disconnect") );
    m_ui->headsetInfo->setText(addr.toString());
    m_ui->headsetTabWidget->setTabEnabled(1, true);
}
