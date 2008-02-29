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

#include <qcombobox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qlayout.h>
#include <qscrollview.h>
#include <qpixmap.h>
#ifdef QTOPIA_PHONE
#include <qtopia/qpeapplication.h>
#include <qtopia/contextmenu.h>
#include <qtopia/fileselector.h>
#include <qtopia/resource.h>
#include <qtopia/vscrollview.h>
#endif

#include "voipmain.h"
#include "voipidentity.h"
#include "voipsettings.h"

VOIPMain :: VOIPMain( QWidget* parent,  const char* name, WFlags fl )
          : QWidget( parent, name, fl )
{
    if ( !name )
    setName( "VOIPMain" );
    setCaption( tr( "VoIP" ) );
    resize( 400, 500 );


    ContextMenu * voipmenu = new ContextMenu(this);
    voipmenu = ContextMenu :: menuFor(this);

    voip_register = new QAction( tr( "Register" ), Resource::loadIconSet( "voip/connect" ),
            QString::null, 0, this, 0 );
    voip_register->setWhatsThis( tr("Register") );
    connect( voip_register, SIGNAL( activated() ), this, SLOT( voipRegister() ) );

    voip_identity = new QAction( tr( "Identity" ), Resource::loadIconSet( "voip/online" ),
                QString::null, 0, this, 0 );
    voip_identity->setWhatsThis( tr("Registration for VoIP.") );
    connect( voip_identity, SIGNAL( activated() ), this, SLOT( voipIdentity() ) );

    voip_settings = new QAction( tr( "Settings" ), Resource::loadIconSet( "voip/settings" ),
            QString::null, 0, this, 0 );
    voip_settings->setWhatsThis( tr("Settings for VoIP calls.") );
    connect( voip_settings, SIGNAL( activated() ), this, SLOT( voipSettings() ) );

    voip_logout=new QAction(tr("Logout"), Resource::loadIconSet("voip/logout"),
         QString::null, 0, this, 0 );
    voip_logout->setWhatsThis( tr("Logout") );
    connect( voip_logout, SIGNAL( activated() ), this, SLOT( voipAppLogout() ) );

    voip_register->addTo(voipmenu);
    voip_identity->addTo(voipmenu);
    voip_settings->addTo(voipmenu);
    voip_logout->addTo(voipmenu);

    mainVScroll = new VScrollView( this );
    mainVerticalLayout = new QVBoxLayout( this );
    mainVerticalLayout->addWidget( mainVScroll );

    container = mainVScroll->widget();

    VOIPMainLayout = new QVBoxLayout( container );
    VOIPMainLayout->setSpacing( 3 );
    VOIPMainLayout->setMargin( 6 );

    identityIMGHLayout = new QHBoxLayout;
    identityIMGHLayout->setSpacing( 2 );
    identityIMGHLayout->setMargin( 0 );

    idIMG = new QLabel( container, "idIMG" );
    idIMG->setPixmap( QPixmap(Resource::loadIconSet( "voip/online" ).pixmap( QIconSet::Small, TRUE )) );
    idIMG->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0,
                          idIMG->sizePolicy().hasHeightForWidth() ) );
    idIMG->setScaledContents( FALSE );
    idIMG->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    identityIMGHLayout->addWidget( idIMG );

    label = new QLabel( container, "label" );
    label->setText( tr( "<b>Identity</b>" ) );
    label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5,
                          label->sizePolicy().hasHeightForWidth() ) );
    label->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    identityIMGHLayout->addWidget( label );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    identityIMGHLayout->addItem( spacer );
    VOIPMainLayout->addLayout( identityIMGHLayout );

    idInfoHLayout = new QHBoxLayout;
    idInfoHLayout->setSpacing( 2 );
    idInfoHLayout->setMargin( 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 17, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    idInfoHLayout->addItem( spacer_2 );

    idInfoVLayout = new QVBoxLayout;
    idInfoVLayout->setSpacing( 2 );
    idInfoVLayout->setMargin( 0 );

    userName_LBL = new QLabel( container, "userName_LBL" );
    userName_LBL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1,
                                    userName_LBL->sizePolicy().hasHeightForWidth() ) );
    userName_LBL->setScaledContents( FALSE );
    userName_LBL->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    idInfoVLayout->addWidget( userName_LBL );

    sipIdentity_LBL = new QLabel( container, "sipIdentity_LBL" );
    sipIdentity_LBL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1,
                                        sipIdentity_LBL->sizePolicy().hasHeightForWidth() ) );
    sipIdentity_LBL->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    idInfoVLayout->addWidget( sipIdentity_LBL );
    idInfoHLayout->addLayout( idInfoVLayout );
//    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
//    idInfoHLayout->addItem( spacer_3 );
    VOIPMainLayout->addLayout( idInfoHLayout );

    serverIMGHLayout = new QHBoxLayout;
    serverIMGHLayout->setSpacing( 2 );
    serverIMGHLayout->setMargin( 0 );

    serverIMG = new QLabel( container, "serverIMG" );
    serverIMG->setPixmap( QPixmap(Resource::loadIconSet( "voip/server" ).pixmap( QIconSet::Small, TRUE )) );
    serverIMG->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0,
                              serverIMG->sizePolicy().hasHeightForWidth() ) );
    serverIMG->setScaledContents( FALSE );
    serverIMG->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    serverIMGHLayout->addWidget( serverIMG );

    label = new QLabel( container, "label" );
    label->setText( tr( "<b>Server Name</b>" ) );
    label->setMinimumSize( QSize( 100, 0 ) );
    serverIMG->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0,
                              serverIMG->sizePolicy().hasHeightForWidth() ) );
    label->setScaledContents( FALSE );
    label->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    serverIMGHLayout->addWidget( label );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    serverIMGHLayout->addItem( spacer_4 );
    VOIPMainLayout->addLayout( serverIMGHLayout );

    serverInfoHLayout = new QHBoxLayout;
    serverInfoHLayout->setSpacing( 2 );
    serverInfoHLayout->setMargin( 0 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    serverInfoHLayout->addItem( spacer_5 );

    serverName_LBL = new QLabel( container, "serverName_LBL" );
    serverName_LBL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1,
                                   serverName_LBL->sizePolicy().hasHeightForWidth() ) );
    serverName_LBL->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    serverInfoHLayout->addWidget( serverName_LBL );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    serverInfoHLayout->addItem( spacer_6 );
    VOIPMainLayout->addLayout( serverInfoHLayout );

    regStatusIMGHLayout = new QHBoxLayout;
    regStatusIMGHLayout->setSpacing( 2 );
    regStatusIMGHLayout->setMargin( 0 );

    regStatusIMG = new QLabel( container, "regStatusIMG" );
    regStatusIMG->setPixmap(QPixmap(Resource::loadIconSet( "voip/connect" ).pixmap( QIconSet::Small, TRUE )) );
    regStatusIMG->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0,
                                 regStatusIMG->sizePolicy().hasHeightForWidth() ) );
    regStatusIMG->setScaledContents( FALSE );
    regStatusIMGHLayout->addWidget( regStatusIMG );

    label = new QLabel( container, "label" );
    label->setText( tr( "<b>Registration Status</b>" ) );
    label->setMinimumSize( QSize( 148, 0 ) );
    label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5,
                          label->sizePolicy().hasHeightForWidth() ) );
    regStatusIMGHLayout->addWidget( label );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    regStatusIMGHLayout->addItem( spacer_7 );
    VOIPMainLayout->addLayout( regStatusIMGHLayout );

    regStatusInfoHLayout = new QHBoxLayout;
    regStatusInfoHLayout->setSpacing( 2 );
    regStatusInfoHLayout->setMargin( 0 );
    QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    regStatusInfoHLayout->addItem( spacer_8 );

    regStatus_LBL = new QLabel( container, "regStatus_LBL" );
    regStatus_LBL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1,
                                  regStatus_LBL->sizePolicy().hasHeightForWidth() ) );
    regStatus_LBL->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    regStatusInfoHLayout->addWidget( regStatus_LBL );
    QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    regStatusInfoHLayout->addItem( spacer_9 );
    VOIPMainLayout->addLayout( regStatusInfoHLayout );

    voipStatusHLayout = new QVBoxLayout;
    voipStatusHLayout->setSpacing( 2 );
    voipStatusHLayout->setMargin( 0 );


    voipStatusLBL = new QLabel( container, "voipStatusLBL" );
    voipStatusLBL->setText( tr( "<b>VoIP Status</b>" ) );
    voipStatusLBL->setTextFormat( QLabel::AutoText );
    voipStatusLBL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)5,
                                  voipStatusLBL->sizePolicy().hasHeightForWidth() ) );
    voipStatusLBL->setMinimumSize( QSize( 90, 0 ) );
    voipStatusLBL->setAlignment( int( QLabel::AlignHCenter | QLabel::AlignLeft ) );
    voipStatusLBL->setIndent( 1 );
    voipStatusHLayout->addWidget( voipStatusLBL );

    voipStatusCombo = new QComboBox( FALSE, container, "voipStatusCombo" );
    voipStatusCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1,
                                    voipStatusCombo->sizePolicy().hasHeightForWidth() ) );
    voipStatusCombo->setMinimumSize( QSize( 30, 0 ) );
    voipStatusCombo->insertItem( tr( "Unavailable" ), 0 );
    voipStatusCombo->insertItem( tr( "Available" ),1 );
    voipStatusHLayout->addWidget( voipStatusCombo );
    
    QSpacerItem* spacer_10 = new QSpacerItem( 1, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    voipStatusHLayout->addItem( spacer_10 );
    
    VOIPMainLayout->addLayout( voipStatusHLayout );
    
    QSpacerItem* spacer_11 = new QSpacerItem( 20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding );
    VOIPMainLayout->addItem( spacer_11 );

    testCombo = new QComboBox( FALSE, container, "testCombo" );
    testCombo->setMaximumSize( QSize( 0, 0 ) );
//    VOIPMainLayout->addWidget( testCombo );

    voipRegInfo = VoIPConfig::getInstance();
    connect(voipRegInfo, SIGNAL(updateRegistrationMessage(int)),
             this, SLOT(setRegistrationMessage(int)));
    connect(voipRegInfo, SIGNAL(updateSipIdentity(SipIdentity_t&)),
             this, SLOT(setUserIdentityInfo(SipIdentity_t&)));
    connect(voipRegInfo, SIGNAL(setCurrentRegistration(int)),
             this, SLOT(setRegistrationMessage(int)));
    connect(voipStatusCombo, SIGNAL(activated(const QString& )),
             this, SLOT(sendAvailabilityStatus(const QString& )));
    connect(voipRegInfo, SIGNAL(setCurrentAvailabilityStatus(int)),
             this, SLOT(setCurrentAvailabilityStatus(int)));

    voipRegInfo->getSipIdentity();
    voipRegInfo->getCurrentRegistrationInfo();
    voipRegInfo->getCurrentAvailabilityStatus();

   //tabOrder
   setTabOrder( idIMG,  userName_LBL);
   setTabOrder( userName_LBL, voipStatusCombo);
}

VOIPMain :: ~VOIPMain()
{
	QString msgString = QString("%1::%2::%3").arg(__FILE__).arg("VOIPMain :: ~VOIPMain()").arg(__LINE__+2);
	qDebug(msgString);
}

void VOIPMain :: setRegistrationMessage(int regMessage)
{
  voipRegInfo->getSipIdentity();
  eRegStatus =  (RegistrationStatus_t)regMessage;
  switch(regMessage)
  {
    case VOIP_PROXY_UNREGISTERED :{
      updatedStatus = tr("Unregistered");
      break;
    }
    case VOIP_PROXY_CONTACTING_PROXY :{
      updatedStatus = tr("Contacting server...");
      break;
    }
    case VOIP_PROXY_CONNECTION_FAILED :{
      updatedStatus = tr("Connnection failed");
      break;
    }
    case VOIP_PROXY_AUTHENITCATION_FAILED :{
      updatedStatus = tr("User authentication failed");
      break;
    }
    case VOIP_PROXY_AUTHENITCATION_FAILED_NEED_NEW_PASSWORD :{
      updatedStatus = tr("Authentication failed Enter correct password");
      break;
    }
    case VOIP_PROXY_REGISTRATION_TIMEDOUT :{
      updatedStatus = tr("Registration timed out");
      break;
    }
    case VOIP_PROXY_REGISTERED :{
      updatedStatus = tr("Registered");
      break;
    }
    case VOIP_PROXY_CONNECTION_LOST :{
      updatedStatus = tr("Connection lost");
      break;
    }
    default: {
      updatedStatus = " ";
      break;
    }
  }
  regStatus_LBL->clear();
  regStatus_LBL->setText(updatedStatus);

  switch(regMessage)
  {
    case VOIP_PROXY_REGISTERED :
    {
        regStatusIMG->setPixmap(QPixmap(Resource::loadIconSet( "voip/connect" ).pixmap( QIconSet::Small, TRUE )) );
        voip_register->setEnabled(FALSE);
        voip_identity->setEnabled(FALSE);
        voip_settings->setEnabled(FALSE);
        if(voipStatusCombo->isEnabled() == FALSE)
        {
           voipStatusLBL->setEnabled(TRUE);
           voipStatusCombo->setEnabled(TRUE);
           voipRegInfo->getCurrentAvailabilityStatus();
        }
        voip_logout->setEnabled(TRUE);
        break;
     }
    case VOIP_PROXY_CONTACTING_PROXY :
        regStatusIMG->setPixmap(QPixmap(Resource::loadIconSet( "voip/disconnect" ).pixmap( QIconSet::Small, TRUE )) );
        voip_register->setEnabled(FALSE);
        voip_identity->setEnabled(FALSE);
        voip_settings->setEnabled(FALSE);
        if(voipStatusCombo->isEnabled()){
//          voipStatusLBL->setEnabled(FALSE);
          voipStatusCombo->setEnabled(FALSE);
        }
        voip_logout->setEnabled(TRUE);
        break;

    case VOIP_PROXY_CONNECTION_FAILED :
    case VOIP_PROXY_CONNECTION_LOST :
    case VOIP_PROXY_AUTHENITCATION_FAILED :
    case VOIP_PROXY_AUTHENITCATION_FAILED_NEED_NEW_PASSWORD :
    case VOIP_PROXY_REGISTRATION_TIMEDOUT :
    case VOIP_PROXY_UNREGISTERED :
         regStatusIMG->setPixmap(QPixmap(Resource::loadIconSet( "voip/disconnect" ).pixmap( QIconSet::Small, TRUE )) );
	 if ( sipIdentity_LBL->text().isEmpty() )
	    voip_register->setEnabled(FALSE);
	else
	    voip_register->setEnabled(TRUE);
         voip_identity->setEnabled(TRUE);
         voip_settings->setEnabled(TRUE);
         if(voipStatusCombo->isEnabled()){
//           voipStatusLBL->setEnabled(FALSE);
           voipStatusCombo->setEnabled(FALSE);
         }
         voip_logout->setEnabled(FALSE);
         break;
    default:
         regStatusIMG->setPixmap(QPixmap(Resource::loadIconSet( "voip/disconnect" ).pixmap( QIconSet::Small, TRUE )) );
         voip_register->setEnabled(TRUE);
         voip_identity->setEnabled(TRUE);
         voip_settings->setEnabled(TRUE);
         voip_logout->setEnabled(FALSE);
         break;
  }
}

void VOIPMain :: setUserIdentityInfo(SipIdentity_t& SIPIdentity)
{
   userName_LBL->setText(SIPIdentity.fullName);

   if(!((SIPIdentity.userPartOfSipUri).isNull() ||(SIPIdentity.hostPartOfSipUri).isNull() ||
       (SIPIdentity.userPartOfSipUri).isEmpty() || (SIPIdentity.hostPartOfSipUri).isEmpty()))
     sipIdentity_LBL->setText("sip:" + SIPIdentity.userPartOfSipUri + "@" + SIPIdentity.hostPartOfSipUri);

   serverName_LBL->setText(SIPIdentity.hostPartOfSipUri);


  if(!((SIPIdentity.userPartOfSipUri).isNull() ||
    (SIPIdentity.hostPartOfSipUri).isNull() ||
    (SIPIdentity.userPartOfSipUri).isEmpty() ||
    (SIPIdentity.hostPartOfSipUri).isEmpty()))
   {
    if(voip_logout->isEnabled())
       voip_register->setEnabled(FALSE);
    else
       voip_register->setEnabled(TRUE);
   }else
       voip_register->setEnabled(FALSE);

}

void VOIPMain :: voipRegister()
{
    voipRegInfo->loginAndRegister();

    // reclaim focus
    setFocus();
}

void VOIPMain :: keyPressEvent( QKeyEvent *e )
{
    QKeyEvent *ke = (QKeyEvent *)e;
    int key =  ke->key();
    switch( key )
    {
      case Key_Back:
      case Key_No:
	    {
	        close();
		 break;
		}
      default:
			break;
   }
}


void VOIPMain::voipSettings()
{
    settingsDialog = new VOIPSettings((QWidget*)parent(), WStyle_NormalBorder | WDestructiveClose );
    QPEApplication::execDialog( settingsDialog );
//    delete settingsDialog;
//    settingsDialog = 0;
}

void VOIPMain::voipIdentity()
{
    identityDialog = new VOIPIdentity((QWidget*)parent(), WStyle_NormalBorder | WDestructiveClose );
    QPEApplication::execDialog( identityDialog );
//    delete identityDialog;
//    identityDialog = 0;

}

void VOIPMain::voipAppLogout()
{
  voipRegInfo->logout();
}

void VOIPMain::voipAppClose()
{
    close();
}

void VOIPMain::sendAvailabilityStatus(const QString& aStatus)
{
  QString availStr = (QString)aStatus;

  if(availStr == tr("Unavailable")){
    qDebug("VOIPMain::sendAvailabilityStatus  status = UNAVAILABLE ");
    voipStatusCombo->setCurrentItem(0);
    availStatus = UNAVAILABLE;
  }
  else
    if(availStr == tr("Available")){
      qDebug("VOIPMain::sendAvailabilityStatus  status = AVAILABLE ");
      voipStatusCombo->setCurrentItem(1);
      availStatus = AVAILABLE;
    }
  voipRegInfo->setStatus(availStatus);
}

void VOIPMain :: setCurrentAvailabilityStatus(int aStatus)
{
    availStatus = (AvailabilityStatus_t)aStatus;

    if(voipStatusCombo->isEnabled())
    {
       switch(availStatus)
      {
         case UNAVAILABLE:
             voipStatusCombo->setCurrentItem(0);
             break;
         case AVAILABLE:
             voipStatusCombo->setCurrentItem(1);
             break;
      }
    }
}

