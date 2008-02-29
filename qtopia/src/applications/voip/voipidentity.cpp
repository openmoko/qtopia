/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qvalidator.h>
#include <qmessagebox.h>
#include <qscrollview.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qtopia/fileselector.h>
#endif
#include <qaction.h>
#include <qlayout.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/vscrollview.h>

#include "voipidentity.h"

VOIPIdentity :: VOIPIdentity( QWidget* parent, WFlags fl )
              : QDialog( parent, "identity", TRUE, fl )

{
    setCaption( tr( "VoIP Identity" ) );
    resize( 400, 500 );
    ContextMenu * voipIdentityCMenu = new ContextMenu(this);
    voipIdentityCMenu = ContextMenu :: menuFor(this);

    QAction * voip_register = new QAction( tr( "Register" ), Resource::loadIconSet( "voip/connect" ),
            QString::null, 0, this, 0 );
    voip_register->setWhatsThis( tr("Register") );
    connect( voip_register, SIGNAL( activated() ), this, SLOT( checkRegister() ) );
    voip_register->addTo(voipIdentityCMenu);

    mainVIScroll = new VScrollView( this );
    mainIVerticalLayout = new QVBoxLayout( this );
    mainIVerticalLayout->addWidget( mainVIScroll );
    container = mainVIScroll->widget();

    VOIPIdentityLayout = new QVBoxLayout( container );
    VOIPIdentityLayout->setSpacing( 2 );
    VOIPIdentityLayout->setMargin( 2 );

//    testTab = new QTabWidget( container, "testTab" );
//    testTab->setMaximumSize( QSize( 0, 0 ) );
//    testTab->setMinimumSize( QSize( 0, 0 ) );
//    VOIPIdentityLayout->addWidget( testTab );

    label = new QLabel( container, "label" );
    label->setText( tr( "Full Name:" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    VOIPIdentityLayout->addWidget( label );
    fullNameLE = new QLineEdit( container, "fullNameLE" );
    VOIPIdentityLayout->addWidget( fullNameLE );

    sipIdGRPBOX = new QGroupBox( container, "sipIdGRPBOX" );
    sipIdGRPBOX->setAutoMask( FALSE );
    sipIdGRPBOX->setFrameShape( QGroupBox::Box );
    sipIdGRPBOX->setTitle( tr( "SIP URI *" ) );
    QFont sipIdGRPBOX_font(  sipIdGRPBOX->font() );
    sipIdGRPBOX_font.setBold( TRUE );
    sipIdGRPBOX->setFont( sipIdGRPBOX_font );
    sipIdGRPBOX->setAlignment( int( QGroupBox::WordBreak | QGroupBox::AlignLeft ) );
    sipIdGRPBOX->setColumnLayout(0, Qt::Vertical );
    sipIdGRPBOX->layout()->setSpacing( 0 );
    sipIdGRPBOX->layout()->setMargin( 0 );
    sipIdGRPBOXLayout = new QGridLayout( sipIdGRPBOX->layout() );
    sipIdGRPBOXLayout->setAlignment( Qt::AlignTop );
    sipIdGRPBOXLayout->setSpacing( 4 );
    sipIdGRPBOXLayout->setMargin( 6 );

    label = new QLabel( sipIdGRPBOX, "userPart" );
    label->setText( tr( "User Part" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    label->setMinimumSize( QSize( 60, 20 ) );
    QFont label_font(  label->font() );
    label_font.setBold( FALSE );
    label->setFont( label_font );
    sipIdGRPBOXLayout->addWidget( label, 0, 0 );

    userSIPUriLE = new QLineEdit( sipIdGRPBOX, "userSIPUriLE" );
    QFont userSIPUriLE_font(  userSIPUriLE->font() );
    userSIPUriLE_font.setBold( FALSE );
    userSIPUriLE->setFont( userSIPUriLE_font );
    sipIdGRPBOXLayout->addWidget( userSIPUriLE, 0, 1 );

    label = new QLabel( sipIdGRPBOX, "hostPart" );
    label->setText( tr( "Host Part" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    label->setMinimumSize( QSize( 60, 20 ) );
    label_font.setBold( FALSE );
    label->setFont( label_font );
    sipIdGRPBOXLayout->addWidget( label, 1, 0 );

    hostSIPUriLE = new QLineEdit( sipIdGRPBOX, "hostSIPUriLE" );
    QFont hostSIPUriLE_font(  hostSIPUriLE->font() );
    hostSIPUriLE_font.setBold( FALSE );
    hostSIPUriLE->setFont( hostSIPUriLE_font );
    sipIdGRPBOXLayout->addWidget( hostSIPUriLE, 1, 1 );

    VOIPIdentityLayout->addWidget( sipIdGRPBOX );

    label = new QLabel( container, "label" );
    label->setText( tr( "Outbound Proxy(Optional)" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    VOIPIdentityLayout->addWidget( label );
    outboundProxyLE = new QLineEdit( container, "outboundProxyLE" );
    VOIPIdentityLayout->addWidget( outboundProxyLE );

    label = new QLabel( container, "label" );
    label->setText( tr( "Authentication Username(Optional)" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    VOIPIdentityLayout->addWidget( label );
    authUsernameLE = new QLineEdit( container, "authUsernameLE" );
    VOIPIdentityLayout->addWidget( authUsernameLE );

    label = new QLabel( container, "label" );
    label->setText( tr( "Password *" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    VOIPIdentityLayout->addWidget( label );
    authPasswordLE = new QLineEdit( container, "passwordLE" );
    authPasswordLE->setEchoMode( QLineEdit::Password );
    VOIPIdentityLayout->addWidget( authPasswordLE );

    label = new QLabel( container, "label" );
    label->setText( tr( "q-value between 0.0-1.0(Optional)" ) );
    label->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    label->setMaximumSize( QSize( 0, 0 ) );
    label->setMinimumSize( QSize( 0, 0 ) );
    VOIPIdentityLayout->addWidget(label);

    qValueLE = new QLineEdit( container, "qValueLE" );
    qValueLE->setValidator( new QDoubleValidator( 0.0, 1.0, 2, qValueLE ) );
    qValueLE->setMaximumSize( QSize( 0, 0 ) );
    qValueLE->setMinimumSize( QSize( 0, 0 ) );
    VOIPIdentityLayout->addWidget(qValueLE);

    autoRegHLayout = new QHBoxLayout;
    autoRegHLayout->setSpacing( 2 );
    autoRegHLayout->setMargin( 0 );

    autoRegisterCB = new QCheckBox( container, "autoRegisterCB" );
    autoRegisterCB->setText( tr( "Auto Register" ) );
    autoRegisterCB->setAutoResize( FALSE );
    autoRegisterCB->setSizePolicy(QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed) );
    autoRegHLayout->addWidget(autoRegisterCB);

    QSpacerItem* spacerH = new QSpacerItem( 20, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    autoRegHLayout->addItem( spacerH );

    VOIPIdentityLayout->addLayout( autoRegHLayout );

    QSpacerItem* spacerV = new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    VOIPIdentityLayout->addItem( spacerV );

    // tab order
    setTabOrder( fullNameLE, userSIPUriLE );
    setTabOrder( userSIPUriLE, hostSIPUriLE );
    setTabOrder( hostSIPUriLE, outboundProxyLE );
    setTabOrder( outboundProxyLE, authPasswordLE );
    setTabOrder( authPasswordLE, authUsernameLE );
    setTabOrder( authUsernameLE, authPasswordLE);
    setTabOrder( authPasswordLE, autoRegisterCB );

    voipIdentityInfo = VoIPConfig::getInstance();
    connect(voipIdentityInfo, SIGNAL(updateSipIdentity(SipIdentity_t&)),
             this, SLOT(setPreferences(SipIdentity_t&)));

    voipIdentityInfo->getSipIdentity();
    voipIdentityInfo->getCurrentRegistrationInfo();
}

VOIPIdentity :: ~VOIPIdentity()
{

}

void VOIPIdentity :: checkRegister()
{
    getPreferences(SIPIdentity);
    voipIdentityInfo->setSipIdentity(SIPIdentity);

    //fullNameLE->text() == "" ||
    if(userSIPUriLE->text().isEmpty() || hostSIPUriLE->text().isEmpty() || authPasswordLE->text().isEmpty() ||
       userSIPUriLE->text().isNull() || hostSIPUriLE->text().isNull() || authPasswordLE->text().isNull())
    {
      int s = QMessageBox::warning( this, tr("Invalid"), tr("<qt>Could not register!\nMandatory(*) information required.</qt>"),
                                 tr("Exit"),tr("Enter"), 0, 0, 1 );
        if(s == 0)
          voipIdentityCancelClicked();
    }
    else
    {
          voipIdentityInfo->getSipIdentity();
          voipIdentityInfo->loginAndRegister();
          voipIdentityCancelClicked();
    }
}

void VOIPIdentity :: getPreferences(SipIdentity_t& SIPIdentity)
{
   SIPIdentity.fullName = fullNameLE->text();
   SIPIdentity.userPartOfSipUri = userSIPUriLE->text();
   SIPIdentity.hostPartOfSipUri = hostSIPUriLE->text();
   SIPIdentity.outboundProxy = outboundProxyLE->text();
   SIPIdentity.authenticationUsername = authUsernameLE->text();
   SIPIdentity.authenticationPassword = authPasswordLE->text();
   SIPIdentity.qValue = qValueLE->text();
   SIPIdentity.autoRegister = (bool)(autoRegisterCB->isChecked())? 1 : 0;
//   qDebug("VOIPIdentity :: getPreferences   while sending :  %d", (int)SIPIdentity.autoRegister );
//   qDebug(SIPIdentity.fullName);
//   qDebug(SIPIdentity.userPartOfSipUri);
}

void VOIPIdentity :: setPreferences(SipIdentity_t& SIPIdentity)
{
//   qDebug(SIPIdentity.fullName);
//   qDebug(SIPIdentity.userPartOfSipUri);
   fullNameLE->setText(SIPIdentity.fullName);
   userSIPUriLE->setText(SIPIdentity.userPartOfSipUri);
   hostSIPUriLE->setText(SIPIdentity.hostPartOfSipUri);
   outboundProxyLE->setText(SIPIdentity.outboundProxy);
   authUsernameLE->setText(SIPIdentity.authenticationUsername);
   authPasswordLE->setText(SIPIdentity.authenticationPassword);
   qValueLE->setText(SIPIdentity.qValue);
   autoRegisterCB->setChecked((((int)SIPIdentity.autoRegister) == 1 ? TRUE :FALSE));
//   qDebug("VOIPIdentity :: setPreferences   Received :  %d", (int)SIPIdentity.autoRegister );

}

void VOIPIdentity :: voipIdentityCancelClicked()
{
  close();
}

void VOIPIdentity :: accept()
{
    if(userSIPUriLE->text().isEmpty() || hostSIPUriLE->text().isEmpty() || authPasswordLE->text().isEmpty() ||
      userSIPUriLE->text().isNull() || hostSIPUriLE->text().isNull() || authPasswordLE->text().isNull())
   {
     int s = QMessageBox::warning( this, tr("Invalid"), tr("<qt>Mandatory(*) information required.</qt>"),
                   tr("Exit"),tr("Enter"), 0, 0, 1);// QMessageBox::Yes, QMessageBox::No | QMessageBox::Default );
     if(s == 0)
       voipIdentityCancelClicked();
   }
   else
   {
      getPreferences(SIPIdentity);
      voipIdentityInfo->setSipIdentity(SIPIdentity);
      voipIdentityInfo->getSipIdentity();
      voipIdentityCancelClicked();
   }
}
