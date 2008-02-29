/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "encryptionconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QButtonGroup>
#include <QDebug>
#include <QMessageBox>
#include <QSet>

#include <hexkeyvalidator.h>

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

WirelessEncryptionPage::WirelessEncryptionPage( const QtopiaNetworkProperties& cfg,
        QWidget* parent, Qt::WFlags flags )
    : QWidget( parent, flags ), lastIndex( 0 )
{
    ui.setupUi( this );

    QButtonGroup* grp = new QButtonGroup( this );
    grp->addButton( ui.key1_check );
    grp->addButton( ui.key2_check );
    grp->addButton( ui.key3_check );
    grp->addButton( ui.key4_check );

    HexKeyValidator* wepValidator = new HexKeyValidator( this );
    ui.key1->setValidator( wepValidator );
    ui.key2->setValidator( wepValidator );
    ui.key3->setValidator( wepValidator );
    ui.key4->setValidator( wepValidator );

    init( cfg );

#ifdef QTOPIA_PHONE
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
#endif
    selectEncryptAlgorithm( ui.encrypt->currentIndex() );
    connect( ui.netSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(newNetSelected(int)) );
    connect( ui.encrypt, SIGNAL(currentIndexChanged(int)), this, SLOT(selectEncryptAlgorithm(int)) );
    connect( ui.encrypt_type, SIGNAL(currentIndexChanged(int)), this, SLOT(selectEncryptType(int)) );
    connect( ui.passphrase, SIGNAL(editingFinished()), this, SLOT(checkPassword()) );
    connect( ui.WPAtype, SIGNAL(currentIndexChanged(int)), this, SLOT(wpaEnterpriseChanged(int)) );
}

WirelessEncryptionPage::~WirelessEncryptionPage()
{
}

QtopiaNetworkProperties WirelessEncryptionPage::properties()
{
    if ( !isEnabled() )
        return QtopiaNetworkProperties();

    saveConfig();
    return props;
}

void WirelessEncryptionPage::init( const QtopiaNetworkProperties& cfg )
{
    props.clear();
    lastIndex = 0;
    ui.netSelector->clear();
    const QList<QString> keys = cfg.keys();
    const int numWLANs = cfg.value( QLatin1String("WirelessNetworks/size"), 0 ).toInt();
    const bool hasWLANs = numWLANs > 0;
    if ( hasWLANs ) {
        setEnabled( true );
    } else {
        setEnabled( false );
        ui.netSelector->addItem("<No WLAN defined>");
        return;
    }
    int idx = 0;
    QString normalizedKey;
    foreach( QString key, keys ) {
        if ( !key.startsWith("WirelessNetworks")  )
            continue;
        idx = key.mid(17, key.indexOf(QChar('/'), 17)-17).toInt();
        if ( idx <= numWLANs ) {
            //copy all values into props where we keep track of changes
            props.insert( key, cfg.value( key ) );
            if ( key.endsWith( "ESSID" ) ) {
                QString text = cfg.value( key ).toString();
                if ( text.isEmpty() )
                    text = tr("Unnamed network");
                ui.netSelector->addItem( text );
            }
        }
    }

    readConfig( );
}

void WirelessEncryptionPage::readConfig( )
{
    const QString s = QString("WirelessNetworks/%1/").arg( ui.netSelector->currentIndex() +1 );
    QString enc = props.value(s+"Encryption").toString();
    if ( enc == "none" )
        ui.encrypt->setCurrentIndex( 0 );
    else if ( enc == "shared" )
        ui.encrypt->setCurrentIndex( 2 );
    else if ( enc == "WPA-PSK" )
        ui.encrypt->setCurrentIndex( 3 );
    else if ( enc == "WPA-EAP" )
        ui.encrypt->setCurrentIndex( 4 );
    else
        ui.encrypt->setCurrentIndex( 1 ); //Open authentication

    QString key = props.value(s+"SelectedKey").toString();
    if ( !key.isEmpty() && key.at(key.length()-1).isDigit() ) {
        switch( key.at(key.length()-1).digitValue() ) {
            case 0:
                ui.key1_check->setChecked( true );
                break;
            case 1:
                ui.key2_check->setChecked( true );
                break;
            case 2:
                ui.key3_check->setChecked( true );
                break;
            case 3:
                ui.key4_check->setChecked( true );
                break;
            default:
                break;
        }
        ui.encrypt_type->setCurrentIndex( 1 );
    } else {
        ui.encrypt_type->setCurrentIndex( 0 ); // use passphrase
    }

    if ( props.value(s+"KeyLength").toInt() != 64 )
        ui.bit128->setChecked( true );
    else
        ui.bit64->setChecked( true );

    ui.key1->setText( props.value(s+"WirelessKey_1").toString() );
    ui.key2->setText( props.value(s+"WirelessKey_2").toString() );
    ui.key3->setText( props.value(s+"WirelessKey_3").toString() );
    ui.key4->setText( props.value(s+"WirelessKey_4").toString() );
    ui.passphrase->setText( props.value(s+"PRIV_GENSTR").toString());

    const QString wpa_eap = props.value(s+"WPAEnterprise", QLatin1String("TLS")).toString();
    int wpa_eap_idx = 0; //defaults to TLS
    if ( wpa_eap == "TTLS" )
        wpa_eap_idx = 1;
    else if ( wpa_eap == "PEAP" )
        wpa_eap_idx = 2;
    ui.WPAtype->setCurrentIndex( wpa_eap_idx );
    wpaEnterpriseChanged( wpa_eap_idx );

    ui.identity->setText( props.value(s+"Identity").toString() );
    ui.ident_password->setText( props.value(s+"IdentityPassword").toString());
    ui.client_cert->setText( props.value(s+"ClientCert").toString() );
    ui.server_cert->setText( props.value(s+"ServerCert").toString() );
}

/*!
  \internal

  Save options of current WLAN
*/
void WirelessEncryptionPage::saveConfig()
{
    if ( lastIndex < 0 || lastIndex >= ui.netSelector->count() )
        return;

    const QString s = QString("WirelessNetworks/%1/").arg(lastIndex+1);
    switch( ui.encrypt->currentIndex() ) {
        case 0:
            props.insert( s+"Encryption", "none" );
            break;
        case 2:
            props.insert( s+"Encryption", "shared" );
            break;
        case 3:
            props.insert( s+"Encryption", "WPA-PSK" );
            break;
        case 4:
            props.insert( s+"Encryption", "WPA-EAP" );
            break;
        case 1:
        default:
            props.insert( s+"Encryption", "open" );
            break;
    }

    if ( ui.encrypt_type->currentIndex() == 0)
        props.insert(s+"SelectedKey", "PP");
    else {
        if (ui.key2_check->isChecked())
            props.insert(s+"SelectedKey", "K1");
        else if (ui.key3_check->isChecked())
            props.insert(s+"SelectedKey", "K2");
        else if (ui.key4_check->isChecked())
            props.insert(s+"SelectedKey", "K3");
        else
            props.insert(s+"SelectedKey", "K0");
    }

    props.insert(s+"KeyLength", ui.bit64->isChecked() ? 64 : 128 );

    props.insert(s+"WirelessKey_1", ui.key1->text());
    props.insert(s+"WirelessKey_2", ui.key2->text());
    props.insert(s+"WirelessKey_3", ui.key3->text());
    props.insert(s+"WirelessKey_4", ui.key4->text());
    props.insert(s+"PRIV_GENSTR", ui.passphrase->text());

    //WPA-EAP parameter
    QString wpa_eap;
    switch( ui.WPAtype->currentIndex() ) {
        case 0:
            wpa_eap = "TLS";
            break;
        case 1:
            wpa_eap = "TTLS";
            break;
        case 2:
            wpa_eap = "PEAP";
            break;
    }
    props.insert(s+"WPAEnterprise", wpa_eap);
    props.insert(s+"Identity", ui.identity->text());
    props.insert(s+"IdentityPassword", ui.ident_password->text());
    props.insert(s+"ClientCert", ui.client_cert->text());
    props.insert(s+"ServerCert", ui.server_cert->text());
}

void WirelessEncryptionPage::newNetSelected( int newIdx )
{
    if ( newIdx < 0 || newIdx >= ui.netSelector->count() )
        return;

    saveConfig();
    lastIndex = newIdx;
    readConfig();
}

void WirelessEncryptionPage::selectEncryptAlgorithm( int index )
{
    switch( index ) {
        case 0: //None
            ui.encrypt_type->setVisible( false );
            ui.passphrase->setVisible( false );
            ui.multkeys->setVisible( false );
            ui.EAPBox->setVisible( false );
            break;
        case 1: //Open
        case 2: //Shared key
        default:
            {
                ui.encrypt_type->setVisible( true );
                ui.encrypt_type->setEnabled( true );
                const bool multikey = ui.encrypt_type->currentIndex();
                ui.multkeys->setVisible( multikey );
                ui.passphrase->setVisible( !multikey );
                ui.EAPBox->setVisible( false );
            }
            break;
        case 3: // WPA-PSK
            ui.encrypt_type->setVisible( true );
            ui.encrypt_type->setEnabled( false );
            ui.passphrase->setVisible( true );
            ui.multkeys->setVisible( false );
            ui.EAPBox->setVisible( false );
            ui.encrypt_type->setCurrentIndex( 0 ); //passphrase only
            break;
        case 4: // WPA-EAP
            ui.encrypt_type->setVisible( false );
            ui.passphrase->setVisible( false );
            ui.multkeys->setVisible( false );
            ui.EAPBox->setVisible( true );
            break;
    }
}

void WirelessEncryptionPage::selectEncryptType( int index )
{
    const bool multikey = index == 1;
    ui.multkeys->setVisible( multikey );
    ui.passphrase->setVisible( !multikey );
}

void WirelessEncryptionPage::setProperties( const QtopiaNetworkProperties& cfg )
{
    init( cfg );
}

void WirelessEncryptionPage::checkPassword()
{
    if ( ui.encrypt->currentIndex() == 3 ) { //WPA-PSK
        const QString pw = ui.passphrase->text();
        if ( pw.length() < 8 ) {
            QMessageBox::critical( this, tr("WPA-PSK error"), tr("<qt>Password must be 8 characters or longer.</qt>"),
                     QMessageBox::Ok, QMessageBox::NoButton );
            ui.passphrase->setEditFocus( true );
        }
    }
}

void WirelessEncryptionPage::wpaEnterpriseChanged(int index)
{
    const bool useTLS = index == 0;
    ui.identity->setVisible( useTLS );
    ui.identityLabel->setVisible( useTLS );
    ui.ident_password->setVisible( useTLS );
    ui.ident_passwordLabel->setVisible( useTLS );
    ui.client_cert->setVisible( !useTLS );
    ui.clientCertLabel->setVisible( !useTLS );
    ui.server_cert->setVisible( !useTLS );
    ui.serverCertLabel->setVisible( !useTLS );
}
#endif // NO_WIRELESS_LAN
