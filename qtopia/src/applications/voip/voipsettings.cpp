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
#include <qgroupbox.h>
#include <qcheckbox.h>
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
#include <qaction.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qtopia/fileselector.h>
#endif
#include <qtopia/vscrollview.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>

#include "voipsettings.h"


VOIPSettings :: VOIPSettings( QWidget* parent,  WFlags fl )
              : QDialog( parent, "settings", TRUE, fl )
{
    setCaption( tr( "VoIP Settings" ) );
    resize( 400, 500 );

    ContextMenu * voipSettingsCMenu = new ContextMenu(this);
    voipSettingsCMenu = ContextMenu :: menuFor(this);

    VOIPSettingsLayout = new QVBoxLayout( this );
    VOIPSettingsLayout->setSpacing( 0 );
    VOIPSettingsLayout->setMargin( 0 );

    settingsTab = new QTabWidget( this, "settingsTab" );
    VOIPSettingsLayout->addWidget( settingsTab );

    //sip tab
    sipTab = new VScrollView( settingsTab, "sipTab"  );
    QWidget *wSipTab = sipTab->widget();

    sipTabLayout = new QVBoxLayout( wSipTab );
    sipTabLayout->setResizeMode(QLayout :: FreeResize);
    sipTabLayout->setSpacing( 2 );
    sipTabLayout->setMargin( 4 );

    hideViaBTNGRP = new QButtonGroup( wSipTab, "hideViaBTNGRP" );
    hideViaBTNGRP->setTitle( tr( "Hide Via" ) );
    hideViaBTNGRP->setColumnLayout(0, Qt::Vertical );
    hideViaBTNGRP->layout()->setSpacing( 0 );
    hideViaBTNGRP->layout()->setMargin( 0 );
    hideViaBTNGRPLayout = new QGridLayout( hideViaBTNGRP->layout() );
    hideViaBTNGRPLayout->setAlignment( Qt::AlignTop );
    hideViaBTNGRPLayout->setSpacing( 2 );
    hideViaBTNGRPLayout->setMargin( 2 );

    dontHideRB = new QRadioButton( hideViaBTNGRP, "dontHideRB" );
    QFont dontHideRB_font(  dontHideRB->font() );
    dontHideRB->setText( tr( "Don't Hide" ) );
    dontHideRB->setAutoResize( TRUE );

    hideViaBTNGRPLayout->addWidget( dontHideRB, 0, 0 );

    reqNextHopRB = new QRadioButton( hideViaBTNGRP, "reqNextHopRB" );
    reqNextHopRB->setText( tr( "Request Next Hop", "Hop:a routing term as in hopping to the next routing server" ) );
    reqNextHopRB->setAutoResize( TRUE );

    hideViaBTNGRPLayout->addWidget( reqNextHopRB, 1, 0 );

    reqFullRouteCB = new QRadioButton( hideViaBTNGRP, "reqFullRouteCB" );
    QFont reqFullRouteCB_font(  reqFullRouteCB->font() );
    reqFullRouteCB->setText( tr( "Request Full Route" ) );
    reqFullRouteCB->setAutoResize( TRUE );

    hideViaBTNGRPLayout->addWidget( reqFullRouteCB, 2, 0 );
    QSpacerItem* spacer = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    hideViaBTNGRPLayout->addItem( spacer, 2, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    hideViaBTNGRPLayout->addItem( spacer_2, 1, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    hideViaBTNGRPLayout->addItem( spacer_3, 0, 1 );

    sipTabLayout->addWidget( hideViaBTNGRP );


    label = new QLabel( wSipTab, "label" );
    label->setText( tr( "Max Forwards", "number of maximum hopping" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    sipTabLayout->addWidget( label );

    maxForwardsLE = new QLineEdit( wSipTab, "maxForwardsLE" );
    maxForwardsLE->setAlignment( int( QLineEdit::AlignLeft ) );
    intValidator = new QIntValidator(0, 1000, this );
    maxForwardsLE->setValidator( intValidator );
    sipTabLayout->addWidget( maxForwardsLE );

    label = new QLabel( wSipTab, "label" );
    label->setText( tr( "Expire Time of Registration(Sec)", "timeout duration for an registration request to the server" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    sipTabLayout->addWidget( label );

    expireTimeRegLE = new QLineEdit( wSipTab, "expireTimeRegLE" );
    expireTimeRegLE->setAlignment( int( QLineEdit::AlignLeft ) );
    expireTimeRegLE->setValidator( intValidator );
    sipTabLayout->addWidget( expireTimeRegLE );

    label = new QLabel( wSipTab, "label" );
    label->setText( tr( "Expire Time of Presence Subscription(Sec)", "Presence=availability for conversation" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    sipTabLayout->addWidget( label );

    expireTimeSubsLE = new QLineEdit( wSipTab, "expireTimeSubsLE" );
    sipTabLayout->addWidget( expireTimeSubsLE );
    expireTimeSubsLE->setValidator( intValidator );

    QSpacerItem* spacer21 = new QSpacerItem( 20, 15, QSizePolicy::Minimum, QSizePolicy::Expanding );
    sipTabLayout->addItem( spacer21 );
    
    settingsTab->addTab( sipTab, tr( "SIP", "short for Session Initiation Protocol" ) );

    //audio tab
    audioTab = new VScrollView( settingsTab, "audioTab"  );
    QWidget *wAudioTab = audioTab->widget();

    audioTabLayout = new QVBoxLayout( wAudioTab );
    audioTabLayout->setSpacing( 2 );
    audioTabLayout->setMargin( 4 );

    ossDevModeBTNGRP = new QButtonGroup( wAudioTab, "ossDevModeBTNGRP" );
    ossDevModeBTNGRP->setTitle( tr( "OSS Device Mode" ) );
    ossDevModeBTNGRP->setColumnLayout(0, Qt::Vertical );
    ossDevModeBTNGRP->layout()->setSpacing( 0 );
    ossDevModeBTNGRP->layout()->setMargin( 0 );
    ossDevModeBTNGRPLayout = new QVBoxLayout( ossDevModeBTNGRP->layout() );
    ossDevModeBTNGRPLayout->setAlignment( Qt::AlignTop );
    ossDevModeBTNGRPLayout->setSpacing( 2 );
    ossDevModeBTNGRPLayout->setMargin( 2 );

    ossDevGLayout = new QVBoxLayout;
    ossDevGLayout->setSpacing( 2 );
    ossDevGLayout->setMargin( 0 );

    rdonlyWronlyRB = new QRadioButton( ossDevModeBTNGRP, "rdonlyWronlyRB" );
    rdonlyWronlyRB->setText( tr( "RO/WO" ) );
    rdonlyWronlyRB->setAutoResize( TRUE );
    ossDevGLayout->addWidget( rdonlyWronlyRB );

    readWriteRB = new QRadioButton( ossDevModeBTNGRP, "readWriteRB" );
    readWriteRB->setText( tr( "R/W" ) );
    readWriteRB->setAutoResize( TRUE );
    ossDevGLayout->addWidget( readWriteRB );

    ossDevModeBTNGRPLayout->addLayout( ossDevGLayout );
    audioTabLayout->addWidget( ossDevModeBTNGRP );


    label = new QLabel( wAudioTab, "label" );
    label->setText( tr( "Device for WO or R/W" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    audioTabLayout->addWidget( label );

    deviceRdWrLE = new QLineEdit( wAudioTab, "deviceRdWrLE" );
    audioTabLayout->addWidget( deviceRdWrLE );

    devRdOnlylabel = new QLabel( wAudioTab, "devRdOnlylabel" );
    devRdOnlylabel->setText( tr( "Device for RO" ) );
    devRdOnlylabel->setTextFormat( QLabel::AutoText );
    devRdOnlylabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    audioTabLayout->addWidget( devRdOnlylabel );

    deviceRdLE = new QLineEdit( wAudioTab, "deviceRdLE" );
    deviceRdLE->setEnabled( FALSE );
    audioTabLayout->addWidget( deviceRdLE );

    preferredCodecHLayout = new QHBoxLayout;
    preferredCodecHLayout->setSpacing( 2 );
    preferredCodecHLayout->setMargin( 2 );

    label = new QLabel( wAudioTab, "label" );
    label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, label->sizePolicy().hasHeightForWidth() ) );
    label->setText( tr( "Preferred Codec" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    preferredCodecHLayout->addWidget( label );

    preferredCodecCombo = new QComboBox( FALSE, wAudioTab, "preferredCodecCombo" );
    preferredCodecCombo->insertItem( "G711u" ); //no tr
    preferredCodecCombo->insertItem( "G711a" ); //no tr
    preferredCodecCombo->insertItem( "GSM" );   //no tr
    preferredCodecCombo->insertItem( "iLBC" );  //no tr
    preferredCodecHLayout->addWidget( preferredCodecCombo );
    audioTabLayout->addLayout( preferredCodecHLayout );

    payloadSizeBTNGRP = new QButtonGroup( wAudioTab, "payloadSizeBTNGRP" );
    payloadSizeBTNGRP->setTitle( tr( "Size of Payload" ) );
    payloadSizeBTNGRP->setColumnLayout(0, Qt::Vertical );
    payloadSizeBTNGRP->layout()->setSpacing( 0 );
    payloadSizeBTNGRP->layout()->setMargin( 0 );
    payloadSizeBTNGRPLayout = new QGridLayout( payloadSizeBTNGRP->layout() );
    payloadSizeBTNGRPLayout->setAlignment( Qt::AlignTop );
    payloadSizeBTNGRPLayout->setSpacing( 2 );
    payloadSizeBTNGRPLayout->setMargin( 2 );
    QSpacerItem* spacer_7 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    payloadSizeBTNGRPLayout->addItem( spacer_7, 1, 1 );
    QSpacerItem* spacer_8 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    payloadSizeBTNGRPLayout->addItem( spacer_8, 2, 1 );

    payloadSize160RB = new QRadioButton( payloadSizeBTNGRP, "payloadSize160RB" );
    QString milliseconds(tr("ms", "short for milliseconds"));
    payloadSize160RB->setText( QString("160 (20 %1)").arg(milliseconds) );

    payloadSizeBTNGRPLayout->addWidget( payloadSize160RB, 1, 0 );

    payloadSize240RB = new QRadioButton( payloadSizeBTNGRP, "payloadSize240RB" );
    QFont payloadSize240RB_font(  payloadSize240RB->font() );
    payloadSize240RB->setText( QString("240 (30 %1)").arg(milliseconds) );

    payloadSizeBTNGRPLayout->addWidget( payloadSize240RB, 2, 0 );

    payloadSize80RB = new QRadioButton( payloadSizeBTNGRP, "payloadSize80RB" );
    QFont payloadSize80RB_font(  payloadSize80RB->font() );
    payloadSize80RB->setText( QString("80 (10 %1)").arg(milliseconds) );
    payloadSize80RB->setChecked( FALSE );

    payloadSizeBTNGRPLayout->addWidget( payloadSize80RB, 0, 0 );
    QSpacerItem* spacer_9 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    payloadSizeBTNGRPLayout->addItem( spacer_9, 0, 1 );
    audioTabLayout->addWidget( payloadSizeBTNGRP );
    
    QSpacerItem* spacer_22 = new QSpacerItem( 20, 15, QSizePolicy::Minimum, QSizePolicy::Expanding );
    audioTabLayout->addItem( spacer_22 );

    settingsTab->addTab( audioTab, tr( "Audio" ) );

    //socket tab
    socketTab = new VScrollView( settingsTab, "socketTab"  );
    QWidget *wSocketTab = socketTab->widget();

    socketTabLayout = new QVBoxLayout( wSocketTab );
    socketTabLayout->setSpacing( 2 );
    socketTabLayout->setMargin( 4 );

    socketProtocolHLayout = new QHBoxLayout;
    socketProtocolHLayout->setSpacing( 6 );
    socketProtocolHLayout->setMargin( 2 );

    label = new QLabel( wSocketTab, "label" );
    label->setText( tr( "Socket Protocol" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    socketProtocolHLayout->addWidget( label );

    socketProtocolCombo = new QComboBox( FALSE, wSocketTab, "socketProtocolCombo" );
    socketProtocolCombo->insertItem( "UDP" ); //no tr
    socketProtocolCombo->insertItem( "TCP" ); //no tr
    socketProtocolCombo->setMaximumSize( QSize( 32767, 22 ) );
    socketProtocolHLayout->addWidget( socketProtocolCombo );
    socketTabLayout->addLayout( socketProtocolHLayout );

    symSigHLayout = new QHBoxLayout;
    symSigHLayout->setSpacing( 2 );
    symSigHLayout->setMargin( 0 );

    symmetricSignallingChkBox = new QCheckBox( wSocketTab, "symmetricSignallingChkBox" );
    symmetricSignallingChkBox->setText( tr( "Symmetric Signalling" ) );
    symmetricSignallingChkBox->setAutoResize( TRUE );
    symmetricSignallingChkBox->setSizePolicy(QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed) );
    symSigHLayout->addWidget( symmetricSignallingChkBox );

    QSpacerItem* spacerH1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    symSigHLayout->addItem( spacerH1 );

    socketTabLayout->addLayout( symSigHLayout );

    symMedHLayout = new QHBoxLayout;
    symMedHLayout->setSpacing( 2 );
    symMedHLayout->setMargin( 0 );

    symmetricMediaChkBox = new QCheckBox( wSocketTab, "symmetricMediaChkBox" );
    symmetricMediaChkBox->setText( tr( "Symmetric Media" ) );
    symmetricMediaChkBox->setAutoResize( TRUE );
    symmetricMediaChkBox->setSizePolicy(QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed) );
    symMedHLayout->addWidget(symmetricMediaChkBox);

    QSpacerItem* spacerH2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    symMedHLayout->addItem( spacerH2 );

    socketTabLayout->addLayout( symMedHLayout );

    stunChkHLayout = new QHBoxLayout;
    stunChkHLayout->setSpacing( 2 );
    stunChkHLayout->setMargin( 0 );

    useSTUNServerChkBox = new QCheckBox( wSocketTab, "useSTUNServerChkBox" );
    useSTUNServerChkBox->setText( tr( "STUN Server" ) );
    useSTUNServerChkBox->setAutoResize( TRUE );
    useSTUNServerChkBox->setSizePolicy(QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed) );
    stunChkHLayout->addWidget( useSTUNServerChkBox );

    QSpacerItem* spacerH3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    stunChkHLayout->addItem( spacerH3 );

    socketTabLayout->addLayout( stunChkHLayout );

    stunServerOptGRPBOX = new QGroupBox( wSocketTab, "stunServerOptGRPBOX" );
    stunServerOptGRPBOX->setTitle( "" ); //no tr
    stunServerOptGRPBOXLayout = new QVBoxLayout( stunServerOptGRPBOX );
//    stunServerOptGRPBOX->setColumnLayout(0, Qt::Vertical );
  //  stunServerOptGRPBOX->layout()->setSpacing( 4 );
  //  stunServerOptGRPBOX->layout()->setMargin( 2 );
    stunServerOptGRPBOXLayout->setAlignment( Qt::AlignTop );
    stunServerOptGRPBOXLayout->setSpacing( 2 );
    stunServerOptGRPBOXLayout->setMargin( 4 );

    label = new QLabel( stunServerOptGRPBOX, "label" );
    label->setMinimumSize( QSize( 110, 20 ) );
    label->setText( tr( "name (address:port)" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    stunServerOptGRPBOXLayout->addWidget( label );

    stunServerLE = new QLineEdit( stunServerOptGRPBOX, "stunServerLE" );
    stunServerLE->setMaximumSize( QSize( 32767, 22 ) );
    stunServerOptGRPBOXLayout->addWidget( stunServerLE );

    label = new QLabel( stunServerOptGRPBOX, "label" );
    label->setMinimumSize( QSize( 110, 20 ) );
    label->setText( tr( "Req.Period [Sec]" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    stunServerOptGRPBOXLayout->addWidget( label, 1, 0 );

    reqPeriodSTUNServerLE = new QLineEdit( stunServerOptGRPBOX, "reqPeriodSTUNServerLE" );
    reqPeriodSTUNServerLE->setEnabled( TRUE );
    reqPeriodSTUNServerLE->setValidator( intValidator );
    reqPeriodSTUNServerLE->setMaximumSize( QSize( 32767, 22 ) );
    stunServerOptGRPBOXLayout->addWidget( reqPeriodSTUNServerLE );

    socketTabLayout->addWidget( stunServerOptGRPBOX );

    settingTabOptLayout = new QGridLayout;
    settingTabOptLayout->setSpacing( 4 );
    settingTabOptLayout->setMargin( 2 );

    label = new QLabel( wSocketTab, "label" );
    label->setText( tr( "Media Min Port" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    settingTabOptLayout->addWidget( label, 0, 0 );

    QSpacerItem* spacer_23 = new QSpacerItem( 24, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    settingTabOptLayout->addItem( spacer_23, 0, 1 );

    mediaMinPortLE = new QLineEdit( wSocketTab, "mediaMinPortLE" );
    mediaMinPortLE->setMaximumSize( QSize( 80, 15 ) );
    mediaMinPortLE->setValidator( intValidator );
    settingTabOptLayout->addWidget( mediaMinPortLE, 0, 2 );

    label = new QLabel( wSocketTab, "label" );
    label->setText( tr( "Media Max port" ) );
    label->setTextFormat( QLabel::AutoText );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    settingTabOptLayout->addWidget( label, 1, 0 );
    QSpacerItem* spacer_4 = new QSpacerItem( 24, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    settingTabOptLayout->addItem( spacer_4, 1, 1 );

    mediaMaxPortLE = new QLineEdit( wSocketTab, "mediaMaxPortLE" );
    mediaMaxPortLE->setEnabled( TRUE );
    mediaMaxPortLE->setMaximumSize( QSize( 80, 15 ) );
    mediaMaxPortLE->setValidator( intValidator );
    settingTabOptLayout->addWidget( mediaMaxPortLE, 1, 2 );

    socketTabLayout->addLayout( settingTabOptLayout );
    
    QSpacerItem* spacer_24 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    socketTabLayout->addItem( spacer_24 );
    settingsTab->addTab( socketTab, tr( "Socket" ) );

    //call tab
    callTab = new VScrollView( settingsTab, "callTab"  );
    QWidget *wCallTab = callTab->widget();

    callTabLayout = new QVBoxLayout( wCallTab );
    callTabLayout->setSpacing( 2 );
    callTabLayout->setMargin( 4 );

    callTabGRPBOX = new QGroupBox( wCallTab, "callTabGRPBOX" );
    callTabGRPBOX->setTitle( tr( "Call Preferences" ) );
    callTabGRPBOX->setColumnLayout(0, Qt::Vertical );
    callTabGRPBOX->layout()->setSpacing( 0 );
    callTabGRPBOX->layout()->setMargin( 0 );
    callTabGRPBOXLayout = new QVBoxLayout( callTabGRPBOX->layout() );
    callTabGRPBOXLayout->setAlignment( Qt::AlignTop );
    callTabGRPBOXLayout->setSpacing( 2 );
    callTabGRPBOXLayout->setMargin( 2 );

    label = new QLabel( callTabGRPBOX, "label" );
    label->setText( tr( "Busy Message" ) );
    label->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );
    callTabGRPBOXLayout->addWidget( label );

    bussyMessageTV = new QTextView( callTabGRPBOX, "bussyMessageTV" );
    callTabGRPBOXLayout->addWidget( bussyMessageTV );
    callTabLayout->addWidget( callTabGRPBOX );
    settingsTab->addTab( callTab, tr( "Call" ) );

    /*Busy box is not implemented yet. Reenable it here*/
    settingsTab->setTabEnabled( callTab, FALSE );
    /*-----------------*/
    
    voipSettingsInfo = VoIPConfig::getInstance();
    qDebug("after VoIPConfig");

    connect( rdonlyWronlyRB, SIGNAL( clicked() ), this, SLOT( rdonlyWronlySelected() ) );
    connect( readWriteRB, SIGNAL( clicked() ), this, SLOT( rdonlyWronlySelected() ) );
    connect( useSTUNServerChkBox, SIGNAL(clicked() ) , this, SLOT(stunServerSelected()));

    connect(voipSettingsInfo, SIGNAL(updateSipPreferences(SipPreferences_t&)),
             this, SLOT(setSipPreferences(SipPreferences_t&)));
    connect(voipSettingsInfo, SIGNAL(updateAudioPreferences(AudioPreferences_t&)),
             this, SLOT(setAudioPreferences(AudioPreferences_t&)));
    connect(preferredCodecCombo, SIGNAL(activated(const QString& )),
             this, SLOT(setProperPayload(const QString& )));

    voipSettingsInfo->getSipPreferences();
    voipSettingsInfo->getAudioPreferences();

    // tab order
    setTabOrder( settingsTab, dontHideRB );
    setTabOrder( dontHideRB, reqNextHopRB );
    setTabOrder( reqNextHopRB, reqFullRouteCB );
    setTabOrder( reqFullRouteCB, maxForwardsLE );
    setTabOrder( maxForwardsLE, expireTimeRegLE );
    setTabOrder( expireTimeRegLE, expireTimeSubsLE );
    setTabOrder( expireTimeSubsLE, readWriteRB );
    setTabOrder( readWriteRB, rdonlyWronlyRB );
    setTabOrder( rdonlyWronlyRB, deviceRdWrLE );
    setTabOrder( deviceRdWrLE, deviceRdLE );
    setTabOrder( deviceRdLE, preferredCodecCombo );
    setTabOrder( preferredCodecCombo, payloadSize80RB );
    setTabOrder( payloadSize80RB, payloadSize160RB );
    setTabOrder( payloadSize160RB, payloadSize240RB );
    setTabOrder( payloadSize240RB, socketProtocolCombo );
    setTabOrder( socketProtocolCombo, symmetricSignallingChkBox );
    setTabOrder( symmetricSignallingChkBox, symmetricMediaChkBox );
    setTabOrder( symmetricMediaChkBox, useSTUNServerChkBox );
    setTabOrder( useSTUNServerChkBox, stunServerLE );
    setTabOrder( stunServerLE, reqPeriodSTUNServerLE );
    setTabOrder( reqPeriodSTUNServerLE, mediaMinPortLE );
    setTabOrder( mediaMinPortLE, mediaMaxPortLE );
    setTabOrder( mediaMaxPortLE, bussyMessageTV );
}

VOIPSettings :: ~VOIPSettings()
{
}

void VOIPSettings :: rdonlyWronlySelected()
{
  if(rdonlyWronlyRB->isChecked()){
      devRdOnlylabel->setEnabled( TRUE );
      deviceRdLE->setEnabled( TRUE );
  }
  else{
      devRdOnlylabel->setEnabled( FALSE );
      deviceRdLE->setEnabled( FALSE );
  }
}


void VOIPSettings :: stunServerSelected()
{
  if(useSTUNServerChkBox->isChecked())
      stunServerOptGRPBOX->setEnabled( TRUE );
  else
      stunServerOptGRPBOX->setEnabled( FALSE );
}

void VOIPSettings :: setProperPayload(const QString& selectedCodec)
{
    if(selectedCodec == "G711u" || selectedCodec == "G711a"){
        payloadSize80RB->setEnabled(TRUE);
        payloadSize160RB->setEnabled(TRUE);
        payloadSize240RB->setChecked(FALSE);
        payloadSize240RB->setEnabled(FALSE);
    }else
    if(selectedCodec == "GSM"){
        payloadSize80RB->setChecked(FALSE);
        payloadSize80RB->setEnabled(FALSE);
        payloadSize160RB->setEnabled(TRUE);
        payloadSize160RB->setChecked(TRUE);
        payloadSize240RB->setChecked(FALSE);
        payloadSize240RB->setEnabled(FALSE);
    }else
    if(selectedCodec == "iLBC"){
        payloadSize80RB->setChecked(FALSE);
        payloadSize80RB->setEnabled(FALSE);
        payloadSize160RB->setEnabled(TRUE);
        payloadSize240RB->setEnabled(TRUE);
    }
}

void VOIPSettings :: getPreferences(SipPreferences_t& SIPPreferences, AudioPreferences_t& AudioPreferences)
{
    if(dontHideRB->isChecked())
      SIPPreferences.hideVia = 1;
    if(reqNextHopRB->isChecked())
      SIPPreferences.hideVia = 2;
    if(reqFullRouteCB->isChecked())
      SIPPreferences.hideVia = 3;
    SIPPreferences.maxForwards = (maxForwardsLE->text()).toInt();
    SIPPreferences.expTimeForRegistration = (expireTimeRegLE->text()).toInt();
    SIPPreferences.expTimeForPresenceSubscription = (expireTimeSubsLE->text()).toInt();

    if(socketProtocolCombo->currentText()=="TCP")
        SIPPreferences.socketProtocol = 1;
    else
    if(socketProtocolCombo->currentText()=="UDP")
        SIPPreferences.socketProtocol = 2;

    SIPPreferences.useSymmetricSignaling = (bool)(symmetricSignallingChkBox->isChecked())? 1 : 0;
    SIPPreferences.useMediaSignaling = (bool)(symmetricMediaChkBox->isChecked())? 1 : 0;
    SIPPreferences.useStunServer = (bool)(useSTUNServerChkBox->isChecked())? 1 : 0;
//    if(SIPPreferences.useStunServer){
    SIPPreferences.STUNServerAddress = stunServerLE->text();
    SIPPreferences.requestPeriodForSTUNServer = (reqPeriodSTUNServerLE->text()).toInt();
//    }
    SIPPreferences.mediaMinPort = (mediaMinPortLE->text()).toInt();
    SIPPreferences.mediaMaxPort = (mediaMaxPortLE->text()).toInt();
    SIPPreferences.busyMessage = bussyMessageTV->text();
    if(readWriteRB->isChecked())
      AudioPreferences.OSSDeviceMode = 1;
    if(rdonlyWronlyRB->isChecked())
      AudioPreferences.OSSDeviceMode = 2;
    AudioPreferences.deviceForReadOnlyWriteOnly = deviceRdWrLE->text();
    AudioPreferences.deviceForReadOnly = deviceRdLE->text();
    AudioPreferences.preferredCodec = preferredCodecCombo->currentText();
    if(payloadSize80RB->isChecked())
      AudioPreferences.preferredPayloadSize = 1;
    if(payloadSize160RB->isChecked())
      AudioPreferences.preferredPayloadSize = 2;
    if(payloadSize240RB->isChecked())
      AudioPreferences.preferredPayloadSize = 3;

}


void VOIPSettings :: setSipPreferences(SipPreferences_t& SIPPreferences )
{
    if(SIPPreferences.hideVia == 1)
      dontHideRB->setChecked(TRUE);
    else
    if(SIPPreferences.hideVia == 2)
      reqNextHopRB->setChecked(TRUE);
    else
    if(SIPPreferences.hideVia == 3)
      reqFullRouteCB->setChecked(TRUE);
    QString strNumber;
    strNumber = strNumber.setNum((int)SIPPreferences.maxForwards);
    maxForwardsLE->setText(strNumber);
    strNumber = strNumber.setNum((int)SIPPreferences.expTimeForRegistration);
    expireTimeRegLE->setText(strNumber);
    strNumber = strNumber.setNum((int)SIPPreferences.expTimeForPresenceSubscription);
    expireTimeSubsLE->setText(strNumber);

    if(SIPPreferences.socketProtocol == 1)   //TCP
        socketProtocolCombo->setCurrentItem(1);
    else
    if(SIPPreferences.socketProtocol == 2)
        socketProtocolCombo->setCurrentItem(0); //UDP

    symmetricSignallingChkBox->setChecked((((int)SIPPreferences.useSymmetricSignaling) == 1 ? TRUE :FALSE));
    symmetricMediaChkBox->setChecked((((int)SIPPreferences.useMediaSignaling) == 1 ? TRUE :FALSE));
    useSTUNServerChkBox->setChecked((((int)SIPPreferences.useStunServer) == 1 ? TRUE :FALSE));
    stunServerSelected();
    if(SIPPreferences.useStunServer){
        stunServerLE->setText(SIPPreferences.STUNServerAddress);
        strNumber = strNumber.setNum((int)SIPPreferences.requestPeriodForSTUNServer);
        reqPeriodSTUNServerLE->setText(strNumber);
    }
    strNumber = strNumber.setNum((int)SIPPreferences.mediaMinPort);
    mediaMinPortLE->setText(strNumber);
    strNumber = strNumber.setNum((int)SIPPreferences.mediaMaxPort);
    mediaMaxPortLE->setText(strNumber);
    bussyMessageTV->setText(SIPPreferences.busyMessage);
}

void VOIPSettings :: setAudioPreferences(AudioPreferences_t& AudioPreferences )
{
    if(AudioPreferences.OSSDeviceMode == 1)
      readWriteRB->setChecked(TRUE);
    if(AudioPreferences.OSSDeviceMode == 2)
      rdonlyWronlyRB->setChecked(TRUE);
    rdonlyWronlySelected();
    deviceRdWrLE->setText(AudioPreferences.deviceForReadOnlyWriteOnly);
    deviceRdLE->setText(AudioPreferences.deviceForReadOnly);
//    preferredCodecCombo->setCurrentText(AudioPreferences.preferredCodec);
    if(AudioPreferences.preferredCodec == "G711u")
        preferredCodecCombo->setCurrentItem(0);
    if(AudioPreferences.preferredCodec == "G711a")
        preferredCodecCombo->setCurrentItem(1);
    if(AudioPreferences.preferredCodec == "GSM")
        preferredCodecCombo->setCurrentItem(2);
    if(AudioPreferences.preferredCodec == "iLBC")
        preferredCodecCombo->setCurrentItem(3);
    setProperPayload(AudioPreferences.preferredCodec);
    if(AudioPreferences.preferredPayloadSize == 1)
      payloadSize80RB->setChecked(TRUE);
    if(AudioPreferences.preferredPayloadSize == 2)
      payloadSize160RB->setChecked(TRUE);
    if(AudioPreferences.preferredPayloadSize == 3)
      payloadSize240RB->setChecked(TRUE);

}

void VOIPSettings :: accept()
{
	getPreferences(SIPPreferences, AudioPreferences);
	voipSettingsInfo->setSipPreferences(SIPPreferences);
	voipSettingsInfo->setAudioPreferences(AudioPreferences);
    close();
}

