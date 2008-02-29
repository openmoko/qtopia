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

#include "accountconfig.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <qtopiaapplication.h>
#include <qtranslatablesettings.h>


/*!
  \class AccountPage
  \brief The AccountPage class provides the user interface for the network account configuration.
  \mainclass
  \internal

  The AccountPage widget is exclusively used by the Qtopia network plug-ins. It 
  allows the user to edit the general account details such as:

  \list
    \o user name
    \o password
    \o APN
    \o dialup number
    \o Autostart behavoir
  \endlist

  Note that not all of these details are always enabled. The type of the network plug-in 
  determines which subwidgets are relevant for a particular network interface type.

  This is not a public class.
  */
AccountPage::AccountPage(
        QtopiaNetwork::Type type, const QtopiaNetworkProperties& cfg,
        QWidget* parent, Qt::WFlags flags )
    : QWidget( parent, flags | Qt::Window ), accountType( type )
{
    init();
    readConfig( cfg );

#ifdef QTOPIA_PHONE
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this , true );
#endif
    //this is required for help lookup
    if ( accountType & QtopiaNetwork::Dialup )
        setObjectName("dialup-account");
    else if ( accountType & QtopiaNetwork::GPRS )
        setObjectName("gprs-account");
    else if ( accountType & QtopiaNetwork::BluetoothDUN )
        setObjectName(QLatin1String("bluetooth-account"));
    else
        setObjectName("lan-account");
}

AccountPage::~AccountPage()
{
}

QtopiaNetworkProperties AccountPage::properties()
{
    QtopiaNetworkProperties props;

    //prevent duplication of interface names
    QStringList allInterfaces = QtopiaNetwork::availableNetworkConfigs( accountType );
    QStringList allNames;
    foreach( QString iface, allInterfaces ) {
        QTranslatableSettings cfg( iface, QSettings::IniFormat );
        allNames << cfg.value(QLatin1String("Info/Name")).toString();
    } 
    QString n = name->text();
    bool initialName = true;
    int idx = 1;
    int count = allNames.count( n );
    //find next available name
    //note: allNames always contains this iface too. we must ensure that we don't trigger a false positive.
    while( (count > 1 && initialName) || ( count>=1 && !initialName ) ) { 
        n = name->text()+QString::number(idx);
        idx++; 
        initialName = false;
        count = allNames.count( n );
    } 
   
    props.insert( "Info/Name", n );
    if ( accountType & ~QtopiaNetwork::BluetoothDUN )
        props.insert( "Properties/Autostart", startup->currentIndex() ? "y" : "n" );

    if ( accountType & (QtopiaNetwork::Dialup | QtopiaNetwork::GPRS | QtopiaNetwork::BluetoothDUN) ) {
        props.insert("Properties/UserName", user->text());
        props.insert("Properties/Password", password->text());

        if ( accountType & QtopiaNetwork::GPRS )
            props.insert( "Serial/APN", dialup->text() );
        else if ( accountType & QtopiaNetwork::Dialup )
            props.insert( "Serial/Phone", dialup->text() );
    }

    return props;
}

void AccountPage::init()
{
    QVBoxLayout* vb = new QVBoxLayout( this );
    vb->setMargin( 5 );
    vb->setSpacing( 4 );

    QLabel *name_label = new QLabel( tr("Account name:"), this );
    vb->addWidget( name_label );
    name = new QLineEdit( this );
    vb->addWidget( name );

    startup_label = new QLabel( tr("Startup mode:"), this );
    vb->addWidget( startup_label );
    startup = new QComboBox( this );
    startup->addItems( QStringList() << tr("When needed") << tr("Always online") );
    vb->addWidget( startup );

    dialup_label = new QLabel( this );
    vb->addWidget( dialup_label );
    dialup = new QLineEdit( this );
    vb->addWidget( dialup );

    user_label = new QLabel( tr("Username:"), this );
    vb->addWidget( user_label );
    user = new QLineEdit( this );
#ifdef QTOPIA_PHONE
    QtopiaApplication::setInputMethodHint( user, QtopiaApplication::Text );
#endif
    vb->addWidget( user );

    password_label = new QLabel( tr("Password:"), this );
    vb->addWidget( password_label );
    password = new QLineEdit( this );
    password->setEchoMode( QLineEdit::PasswordEchoOnEdit );
#ifdef QTOPIA_PHONE
    QtopiaApplication::setInputMethodHint( password, QtopiaApplication::Text );
#endif
    vb->addWidget( password );

    QSpacerItem* spacer = new QSpacerItem( 20, 20,
            QSizePolicy::Minimum, QSizePolicy::Expanding );
    vb->addItem( spacer );
}

void AccountPage::readConfig( const QtopiaNetworkProperties& prop)
{
    if ( accountType & ( QtopiaNetwork::Dialup | QtopiaNetwork::GPRS | QtopiaNetwork::BluetoothDUN ) ) {
        user->setText( prop.value("Properties/UserName").toString() );
        password->setText( prop.value("Properties/Password").toString() );

        if ( accountType & QtopiaNetwork::GPRS ) {
            dialup_label->setText( tr("APN:", "GPRS access point") );
            dialup->setText( prop.value("Serial/APN").toString());
#ifdef QTOPIA_PHONE
            QtopiaApplication::setInputMethodHint( dialup, QtopiaApplication::Text );
#endif
        } else if ( accountType & QtopiaNetwork::BluetoothDUN ) {
            dialup_label->hide();
            dialup->hide();
        } else {
            dialup_label->setText( tr("Dialup number:") );
#ifdef QTOPIA_PHONE
            QtopiaApplication::setInputMethodHint( dialup, QtopiaApplication::Number );
#endif
            dialup->setText( prop.value("Serial/Phone").toString() );
        }
    } else {
        dialup_label->hide();
        dialup->hide();
        user_label->hide();
        user->hide();
        password_label->hide();
        password->hide();
    }
    name->setText( prop.value("Info/Name").toString() );
    if ( accountType & ~QtopiaNetwork::BluetoothDUN ) {
        startup->setCurrentIndex( prop.value("Properties/Autostart").toString() == "y" );
    } else {
        startup_label->hide();
        startup->hide();
    }
}

