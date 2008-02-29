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

#include "qopenvpngui_p.h"
#include "qkey.h"


#ifndef QTOPIA_NO_OPENVPN

#include <QFontMetrics>
#include <QSettings>
#include <QDebug>

#include <qdocumentselector.h>
#include <qtopiaapplication.h>
#include <ipvalidator.h>

/*!
  \internal
  \class GeneralOpenVPNPage
  */

GeneralOpenVPNPage::GeneralOpenVPNPage( QWidget* parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "GeneralOpenVPNPage" );
    ui.setupUi( this );

    connect( ui.remote, SIGNAL(editingFinished()), this, SLOT(forceRemoteName()) );
}

GeneralOpenVPNPage::~GeneralOpenVPNPage()
{
}

void GeneralOpenVPNPage::init()
{
    QSettings cfg( config, QSettings::IniFormat );
    ui.name->setText( cfg.value("Info/Name").toString() );
    ui.description->insertPlainText( cfg.value("Info/Description").toString() );

    cfg.beginGroup( QLatin1String("Properties") );
    ui.remote->setText( cfg.value( QLatin1String("Remote") ).toString() );

    ui.ping->setValue( cfg.value( "Ping", 10 ).toInt() );
    ui.pingRestart->setValue( cfg.value( "PingRestart", 60 ).toInt() );
}

void GeneralOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.setValue( "Info/Name", ui.name->text() );
    cfg.setValue( "Info/Description", ui.description->toPlainText() );

    cfg.beginGroup( "Properties" );
    cfg.setValue( "Remote", ui.remote->text() );
    cfg.setValue( "Ping", ui.ping->value() );
    cfg.setValue( "PingRestart", ui.pingRestart->value() );
}

void GeneralOpenVPNPage::forceRemoteName()
{
    if ( ui.remote->text().isEmpty() ) {
        QMessageBox::warning( this, tr("Remote peer"),
               tr("<qt>You must enter the address of the VPN peer.</qt>"), QMessageBox::Ok, QMessageBox::NoButton );

        ui.remote->setEditFocus( true );
    }
}

/*!
  \internal
  \class CertificateOpenVPNPage
  */

CertificateOpenVPNPage::CertificateOpenVPNPage( QWidget* parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "CertificateOpenVPNPage" );
    ui.setupUi( this );
    connect( ui.authentication, SIGNAL(currentIndexChanged(int)),
            this, SLOT(authenticationChanged(int)) );
    connect( ui.secretKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.certKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.privKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.caKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.authKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
}

CertificateOpenVPNPage::~CertificateOpenVPNPage()
{
}

static QString certDescription( const QCertificate& cert )
{
    if ( !cert.isValid() )
        return CertificateOpenVPNPage::tr("Unknown certificate");
    QKey pubKey = cert.publicKey();
    QString result = CertificateOpenVPNPage::tr("%1 (%2, %3 bit)", "X509 (RSA, 1024 bit)");
    result = result.arg(cert.certificateType()).arg(pubKey.algorithm()).arg(pubKey.keySize());
    return result;
}

static QString keyDescription( const QKey& key )
{
    if ( key.type() == QKey::Unknown )
        return CertificateOpenVPNPage::tr( "Unknown key" );

    QString result( CertificateOpenVPNPage::tr("%1, %2 bit", "RSA, 1024 bit") );
    result = result.arg( key.algorithm() ).arg( key.keySize() );
    return result;
}


void CertificateOpenVPNPage::init()
{
    toDocument.clear();
    QSettings cfg(config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties") );
    QString temp = cfg.value( QLatin1String("Authentication"), QLatin1String("TLS") ).toString();

    if ( temp == QLatin1String("None") ) {
        ui.authentication->setCurrentIndex( 0 );
    } else if ( temp == QLatin1String("Static") ) {
        ui.authentication->setCurrentIndex( 1 );
    } else { //SSL/TLS
        ui.authentication->setCurrentIndex( 2 );
    }
    authenticationChanged( ui.authentication->currentIndex() );

    static const short numButtons = 5;
    static const QString keys[numButtons] = { "Secret", "Certificate", "PrivKey", "CA", "TLSauth" };
    QPushButton *const buttons[numButtons] = { ui.secretKey, ui.certKey, ui.privKey, ui.caKey, ui.authKey };
    QLabel *const labels[numButtons] = {ui.secretLabel, ui.certLabel, ui.privLabel, ui.caLabel, ui.authLabel };

    for (int i = 0; i< numButtons; ++i ) {
        temp = cfg.value( keys[i] ).toString();
        QContent c( temp );
        toDocument.insert( buttons[i], c );
        if ( !c.isValid( true ) ) {
            buttons[i]->setText( tr("Select...") );
            labels[i]->setText( "" );
        } else {
            if ( buttons[i] == ui.certKey || buttons[i] == ui.caKey )
                labels[i]->setText( certDescription( QCertificate( c ) ) );
            else
                labels[i]->setText( keyDescription( QKey( c ) ) );
            labels[i]->resize( labels[i]->width(), labels[i]->heightForWidth( labels[i]->width() ) );
            buttons[i]->setText( c.name() );
        }
    }
}

void CertificateOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    switch( ui.authentication->currentIndex() ) {
        case 0: //none
            cfg.setValue( QLatin1String("Authentication"), QLatin1String("None") );
            break;
        case 1: //static key
            {
                cfg.setValue( QLatin1String("Authentication"), QLatin1String("Static") );
                QContent doc = toDocument[ui.secretKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Security key") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("Secret"), doc.file() );
            }
            break;
        default:
        case 2: //SSL/TLS
            {
                cfg.setValue( QLatin1String("Authentication"), QLatin1String("TLS") );
                QContent doc = toDocument[ui.certKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories()<< QLatin1String("Certificate") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("Certificate"), doc.file() );

                doc = toDocument[ui.privKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Security key") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("PrivKey"), doc.file() );

                doc = toDocument[ui.caKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Certificate") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("CA"), doc.file() );

                doc = toDocument[ui.authKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Security key") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("TLSauth"), doc.file() );
            }
                break;
    }
}

void CertificateOpenVPNPage::authenticationChanged( int idx )
{
    ui.certFrame->setVisible( idx == 2 );
    ui.secretFrame->setVisible( idx == 1 );
}


void CertificateOpenVPNPage::selectFile()
{
    QPushButton* b = qobject_cast<QPushButton*>(sender());
    if ( !b )
        return;

    QDocumentSelectorDialog dlg;
    dlg.setModal( true );

    QContentFilter filter;

    bool certificate = false;
    bool secKey = false;
    QMap<QPushButton*,QLabel*> btnToLabel;
    btnToLabel.insert( ui.secretKey, ui.secretLabel );
    btnToLabel.insert( ui.certKey, ui.certLabel );
    btnToLabel.insert( ui.privKey, ui.privLabel );
    btnToLabel.insert( ui.caKey, ui.caLabel );
    btnToLabel.insert( ui.authKey, ui.authLabel );

    if ( b == ui.certKey || b == ui.caKey )
        certificate = true;
    else
        secKey = true;

    dlg.setFilter( QContentFilter( QContent::Document ) );
    dlg.setDefaultCategories( QStringList() << "Certificate" << "Security Key" );

    QLabel* label = btnToLabel[b];
    if ( QtopiaApplication::execDialog( &dlg ) == QDialog::Accepted ) {
        QContent doc = dlg.selectedDocument();

        if ( !doc.isValid() ) {
            btnToLabel[b]->setText("");
            return;
        }

        if ( b == ui.caKey || b == ui.certKey )
            label->setText( certDescription( QCertificate( doc ) ) );
        else
            label->setText( keyDescription( QKey( doc ) ) );
        label->resize( label->width(), label->heightForWidth( label->width() ) );


        toDocument.insert( b, doc );
#if QT_VERSION >= 0x040200
        QFont f = b->font();
        QFontMetrics fm( b->font() );
        b->setText( fm.elidedText(doc.name(), Qt::ElideRight, b->size().width() ) );
#else
        b->setText( doc.name() );
#endif
    } else {
        //TODO currently there is no way of unselecting a document
        //for now the user has to open the document selector and must press Cancel
        //in order to unselect the document
        b->setText(tr("Select..."));
        label->setText("");
    }
}

/*!
  \internal
  \class OptionsOpenVPNPage
  */

OptionsOpenVPNPage::OptionsOpenVPNPage( QWidget* parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "OptionsOpenVPNPage" );
    ui.setupUi( this );

    connect( ui.configFile, SIGNAL(clicked()), this, SLOT(selectConfigScript()) );
}

OptionsOpenVPNPage::~OptionsOpenVPNPage()
{
}

void OptionsOpenVPNPage::init()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    ui.LZO->setCheckState( cfg.value(QLatin1String("LZO"), false).toBool() ? Qt::Checked : Qt::Unchecked );
    ui.pull->setCheckState( cfg.value(QLatin1String("Pull"), false).toBool() ? Qt::Checked : Qt::Unchecked );

    ui.exitNotify->setEnabled(
        cfg.value( QLatin1String("Protocol"), "udp" ).toString() == QLatin1String("udp") );

    ui.exitNotify->setValue( cfg.value(QLatin1String("ExitNotification"), 2).toInt() );
    //ui.verbosity->setValue( cfg.value(QLatin1String("Verbosity"), 4).toInt() );
    //ui.mute->setValue( cfg.value(QLatin1String("Mute"), 10).toInt() );

    QString file = cfg.value(QLatin1String("ConfigScript")).toString();
    if ( !file.isEmpty() )
        configScript = QContent( file );
}

void OptionsOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    cfg.setValue(QLatin1String("LZO"), ui.LZO->checkState() == Qt::Checked );
    cfg.setValue(QLatin1String("Pull"), ui.pull->checkState() == Qt::Checked );

    cfg.setValue( QLatin1String("ExitNotification"), ui.exitNotify->value() );
    //cfg.setValue( QLatin1String("Verbosity"), ui.verbosity->value() );
    //cfg.setValue( QLatin1String("Mute"), ui.mute->value() );

    cfg.setValue( QLatin1String("ConfigScript"), configScript.isValid() ? configScript.file(): QString() );
}

void OptionsOpenVPNPage::selectConfigScript()
{
    QDocumentSelectorDialog dlg;
    dlg.setModal( true );

    QContentFilter filter;

    if ( QtopiaApplication::execDialog( &dlg ) == QDialog::Accepted ) {
        QContent doc = dlg.selectedDocument();

        if ( !doc.isValid() ) {
            ui.configFile->setText( tr("Select...") );
            configScript = QContent();
            return;
        }

        configScript = doc;
#if QT_VERSION >= 0x040200
        QFont f = ui.configFile->font();
        QFontMetrics fm( ui.configFile->font() );
        ui.configFile->setText( fm.elidedText(doc.name(), Qt::ElideRight, ui.configFile->size().width() ) );
#else
        ui.configFile->setText( doc.name() );
#endif
    } else {
        //TODO currently there is no way of unselecting a document
        //for now the user has to open the document selector and must press Cancel
        //in order to unselect the document
        ui.configFile->setText(tr("Select..."));
        configScript = QContent();
    }
}

/*!
  \internal
  \class DeviceOpenVPNPage
  */

DeviceOpenVPNPage::DeviceOpenVPNPage( QWidget * parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "OptionsOpenVPNPage" );
    ui.setupUi( this );

    IPValidator* val = new IPValidator( this );
    ui.localIP->setValidator( val );
    ui.remoteIP->setValidator( val );
#ifdef QTOPIA_PHONE
    QtopiaApplication::setInputMethodHint( ui.localIP, QLatin1String("netmask") );
    QtopiaApplication::setInputMethodHint( ui.remoteIP, QLatin1String("netmask") );
#endif

    ui.device->addItem( QLatin1String("tap") ); //no tr
    ui.device->addItem( QLatin1String("tun") ); //no tr
    connect( ui.device, SIGNAL(currentIndexChanged(int)), this, SLOT(resetRemoteLabel(int)) );
}

DeviceOpenVPNPage::~DeviceOpenVPNPage()
{
}

void DeviceOpenVPNPage::init()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties") );

    QString temp = cfg.value("Device", "tap" ).toString();
    ui.device->setCurrentIndex( temp == "tap" ? 0: 1 );
    resetRemoteLabel( ui.device->currentIndex() );
    temp = cfg.value("Protocol", "udp" ).toString();
    if ( temp == QLatin1String("tcp-client") )
        ui.protocol->setCurrentIndex( 1 );
    else if ( temp == QLatin1String("tcp-server") )
        ui.protocol->setCurrentIndex( 2 );
    else
        ui.protocol->setCurrentIndex( 0 );

    ui.port->setValue( cfg.value( "Port", 1194 ).toInt() );
    ui.localIP->setText( cfg.value( QLatin1String("LocalIP") ).toString() );
    ui.remoteIP->setText( cfg.value( QLatin1String("RemoteIP") ).toString() );
}

void DeviceOpenVPNPage::resetRemoteLabel( int newDevType )
{
    //see man openvpn for different types of --ifconfig parameter when using tun/tap device
    switch( newDevType ) {
        case 0: //tap device
            ui.remoteIPLabel->setText( tr("Subnet mask:") );
            break;
        case 1: //tun device
            ui.remoteIPLabel->setText( tr("Remote IP:") );
            break;
        default:
            qWarning() << "DeviceOpenVPNPage::resetRemoteLabel: Unknown device type";
   }
}

void DeviceOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    int idx = ui.device->currentIndex();
    cfg.setValue( QLatin1String("Device"), idx == 0 ? QLatin1String("tap") : QLatin1String("tun") );
    idx = ui.protocol->currentIndex();
    QString prot = "udp";
    if ( idx == 1 )
        prot = "tcp-client";
    else if ( idx == 2 )
        prot = "tcp-server";
    cfg.setValue( "Protocol", prot );

    cfg.setValue( "Port", ui.port->value() );
    cfg.setValue( QLatin1String("LocalIP"), ui.localIP->text() );
    cfg.setValue( QLatin1String("RemoteIP"), ui.remoteIP->text() );

}
#endif //QTOPIA_NO_OPENVPN
