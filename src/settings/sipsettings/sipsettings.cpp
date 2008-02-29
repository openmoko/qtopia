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

#include "sipsettings.h"
#include <qsettings.h>
#include <qvalidator.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <dissipate2/sipuri.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qpresence.h>

SipSettings::SipSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setWindowTitle( tr("VoIP") );
    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );

    registered = false;
    available = false;

    netReg = new QNetworkRegistration( "voip", this );
    connect( netReg, SIGNAL(registrationStateChanged()),
             this, SLOT(registrationStateChanged()) );

    config = new QTelephonyConfiguration( "voip", this );

    presence = new QPresence( QString::null, this );
    connect( presence, SIGNAL(localPresenceChanged()),
             this, SLOT(presenceChanged()) );

    available = ( presence->localPresence() == QPresence::Available );

    settings = new Ui::SipSettingsBase();
    settings->setupUi( this );
    settings->tabWidget->setCurrentWidget( settings->identity );

    // Populate the codec combobox.
    settings->preferredCodec->addItem( tr("G711u"), QString( "PCMU/8000" ) );
    settings->preferredCodec->addItem( tr("G711a"), QString( "PCMA/8000" ) );
    settings->preferredCodec->addItem( tr("GSM"), QString( "GSM/8000" ) );
    settings->preferredCodec->setCurrentIndex( 0 );

    // Make the QLineEdit fields act right according to the input methods.
    QtopiaApplication::setInputMethodHint
        ( settings->userUri, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->hostPart, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->fullName, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->password, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->proxyHost, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->proxyPort, QtopiaApplication::Number );
    QtopiaApplication::setInputMethodHint
        ( settings->proxyUserId, QtopiaApplication::Text );

    registerAction = new QAction( tr("Register"), this );
    connect( registerAction, SIGNAL(triggered()), this, SLOT(actionRegister()) );

    availableAction = new QAction( tr("Make Available"), this );
    connect( availableAction, SIGNAL(triggered()), this, SLOT(actionAvailable()) );

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(registerAction);
    contextMenu->addAction(availableAction);

    copyToWidgets();

    // Get the current registration state and update the UI.
    registrationStateChanged();
}

SipSettings::~SipSettings()
{
    delete settings;
}

void SipSettings::accept()
{
    bool changed = isChanged();
    copyFromWidgets();

    // Force sipagent to change to the new registration details.
    if ( changed ) {
        if ( registered )
            deregisterFromProxy();
        updateRegistrationConfig();
        if ( registered )
            registerToProxy();
    }

    // If we have useful values, ask if user wants to register now.
    // If the user wants auto-registration, the sipagent daemon
    // will do the registration automatically for us so no need to ask.
    if ( changed && !registered && !savedAutoRegister ) {
        int result = QMessageBox::warning
                ( this, tr("VoIP"),
                  tr("<qt>Would you like to register to the network now?</qt>"),
                  QMessageBox::Yes, QMessageBox::No);
        if ( result == QMessageBox::Yes ) {
            registerToProxy();
        }
    }

    QDialog::accept();
    close();
}

void SipSettings::reject()
{
    QDialog::reject();
    close();
}

void SipSettings::copyToWidgets()
{
    QSettings config( "Trolltech", "SIPAgent" );
    config.beginGroup( "Registration" );

    // Get the expiry time.
    int expires = config.value( "Expires", 900 ).toInt();
    if ( expires <= 0 )
        expires = 900;
    settings->registrationExpiry->setMinimum( 1 );
    settings->registrationExpiry->setMaximum( 100 );
    settings->registrationExpiry->setValue( expires / 60 );
    savedExpires = expires;

    // Get the user and server identities.
    QString useruri = config.value( "UserUri", QString("<sip:@>") ).toString();
    savedUserUri = useruri;
    QString server = config.value( "Proxy", QString(":0") ).toString();
    savedProxy = server;
    SipUri uri( useruri );
    SipUri serveruri( server );
    settings->userUri->setText( uri.getUsername() );
    settings->hostPart->setText( uri.getHostname() );
    settings->fullName->setText( uri.getFullname() );
    settings->proxyHost->setText( serveruri.getHostname() );
    settings->proxyPort->setText
        ( QString::number( serveruri.getPortNumber() ) );
    settings->proxyPort->setValidator( new QIntValidator( 0, 65535, this ) );
    bool autoRegister = config.value( "AutoRegister", false ).toBool();
    if ( autoRegister )
        settings->autoRegister->setCheckState( Qt::Checked );
    else
        settings->autoRegister->setCheckState( Qt::Unchecked );
    savedAutoRegister = autoRegister;

    // Get the proxy auth details.
    QString username =
        config.value( "ProxyUserName", QString( "" ) ).toString();
    QString password =
        config.value( "ProxyPassword", QString( "" ) ).toString();
    if ( password.startsWith( ":" ) ) {
        // The password has been base64-encoded.
        password = QString::fromUtf8
            ( QByteArray::fromBase64( password.mid(1).toLatin1() ) );
    }
    settings->proxyUserId->setText( username );
    settings->password->setText( password );
    savedProxyUserName = username;
    savedProxyPassword = password;

    config.endGroup();      // Registration

    // Configure presence options.
    config.beginGroup( "Presence" );

    // Set the presence subscribe expiry time.
    expires = config.value( "SubscribeExpires", 600 ).toInt();
    if ( expires <= 0 )
        expires = 600;
    settings->presenceExpiry->setMinimum( 1 );
    settings->presenceExpiry->setMaximum( 100 );
    settings->presenceExpiry->setValue( expires / 60 );
    savedSubscribeExpires = expires;

    config.endGroup();      // Presence

    // Configure the media options.
    config.beginGroup( "Media" );

    // Set the combo box to reflect the current codec selection.
    QString codec = config.value( "Codec", QString( "PCMU/8000" ) ).toString();
    for ( int index = 0; index < settings->preferredCodec->count(); ++index ) {
        if ( settings->preferredCodec->itemData( index ).toString() == codec ) {
            settings->preferredCodec->setCurrentIndex( index );
            break;
        }
    }
    savedCodec = codec;

    config.endGroup();      // Media
}

void SipSettings::copyFromWidgets()
{
    QSettings config( "Trolltech", "SIPAgent" );
    config.beginGroup( "Registration" );

    config.setValue( "Expires", settings->registrationExpiry->value() * 60 );
    SipUri useruri;
    useruri.setProtocolName( "sip" );
    useruri.setFullname( settings->fullName->text() );
    useruri.setUsername( settings->userUri->text() );
    useruri.setHostname( settings->hostPart->text() );
    config.setValue( "UserUri", useruri.nameAddr() );
    config.setValue( "Proxy", settings->proxyHost->text() + ":" +
                              settings->proxyPort->text() );
    config.setValue( "AutoRegister",
                     ( settings->autoRegister->checkState() == Qt::Checked ) );
    config.setValue( "ProxyUserName", settings->proxyUserId->text() );
    QByteArray password = settings->password->text().toUtf8();
    config.setValue( "ProxyPassword",
                     ":" + QString::fromLatin1( password.toBase64() ) );

    config.endGroup();      // Registration
    config.beginGroup( "Presence" );

    config.setValue( "SubscribeExpires",
                     settings->presenceExpiry->value() * 60 );

    config.endGroup();      // Presence

    config.beginGroup( "Media" );

    QString codec = settings->preferredCodec->itemData
            ( settings->preferredCodec->currentIndex() ).toString();
    config.setValue( "Codec", codec );

    config.endGroup();      // Media

    savedUserUri = useruri.nameAddr();
    savedProxy = settings->proxyHost->text() + ":" +
                 settings->proxyPort->text();
    savedProxyUserName = settings->proxyUserId->text();
    savedProxyPassword = settings->password->text();
    savedExpires = settings->registrationExpiry->value() * 60;
    savedSubscribeExpires = settings->presenceExpiry->value() * 60;
    savedAutoRegister = ( settings->autoRegister->checkState() == Qt::Checked );
    savedCodec = codec;
}

bool SipSettings::isChanged() const
{
    if ( savedExpires != ( settings->registrationExpiry->value() * 60 ) )
        return true;
    if ( savedSubscribeExpires != ( settings->presenceExpiry->value() * 60 ) )
        return true;
    if ( settings->autoRegister->checkState() == Qt::Checked ) {
        if ( !savedAutoRegister )
            return true;
    } else if ( savedAutoRegister ) {
        return true;
    }
    if ( savedCodec != settings->preferredCodec->itemData( settings->preferredCodec->currentIndex() ).toString() )
        return true;

    SipUri useruri;
    useruri.setProtocolName( "sip" );
    useruri.setFullname( settings->fullName->text() );
    useruri.setUsername( settings->userUri->text() );
    useruri.setHostname( settings->hostPart->text() );
    if ( savedUserUri != useruri.nameAddr() )
        return true;

    if ( ( settings->proxyHost->text() + ":" + settings->proxyPort->text() )
            != savedProxy )
        return true;
    if ( savedProxyUserName != settings->proxyUserId->text() )
        return true;
    if ( savedProxyPassword != settings->password->text() )
        return true;

    return false;
}

void SipSettings::actionRegister()
{
    if ( isChanged() ) {
        copyFromWidgets();
        updateRegistrationConfig();
    }
    if ( registered )
        deregisterFromProxy();
    else
        registerToProxy();
}

void SipSettings::actionAvailable()
{
    available = !available;
    presence->setLocalPresence
        ( available ? QPresence::Available : QPresence::Unavailable );
    updateAvailable();
}

void SipSettings::updateRegister()
{
    registerAction->setEnabled( true );
    registerAction->setVisible( true );
    if ( registered )
        registerAction->setText( tr("Unregister") );
    else
        registerAction->setText( tr("Register") );
}

void SipSettings::updateAvailable()
{
    availableAction->setEnabled( registered );
    availableAction->setVisible( registered );
    if ( available )
        availableAction->setText( tr("Make Unavailable") );
    else
        availableAction->setText( tr("Make Available") );
}

void SipSettings::updateRegistrationConfig()
{
    config->update( "general", QString() );      // No tr
    config->update( "registration", QString() ); // No tr
}

void SipSettings::registerToProxy()
{
    netReg->setCurrentOperator( QTelephony::OperatorModeAutomatic );
}

void SipSettings::deregisterFromProxy()
{
    netReg->setCurrentOperator( QTelephony::OperatorModeDeregister );
}

void SipSettings::registrationStateChanged()
{
    registered =
        ( netReg->registrationState() == QTelephony::RegistrationHome );
    updateRegister();
    updateAvailable();
}

void SipSettings::presenceChanged()
{
    available = ( presence->localPresence() == QPresence::Available );
    updateAvailable();
}

void SipSettings::appMessage( const QString &msg, const QByteArray & )
{
    if ( msg == "VoIP::configure()" ) {
        QtopiaApplication::instance()->showMainWidget();
    }
}
