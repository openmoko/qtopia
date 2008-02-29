/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#include <qvbox.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qsettings.h>
#include <qlayout.h>
#include <qdialog.h>

#include "../config.h"
#include "callaudio.h"
#include "dspoutoss.h"
#include "audiocontrol.h"

AudioControl::AudioControl( QWidget *parent, const char *name )
	: QDialog( parent, QString( name ) + tr("Audio Properties"), true )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

#ifdef ALSA_SUPPORT
	bg4 = new QHButtonGroup( tr("Audio Mode"), vbox );
	ossButton = new QRadioButton( "OSS", bg4 );
	bg4->insert( ossButton, oss );
	connect( ossButton, SIGNAL( clicked() ), this, SLOT( ossButtonClicked() ) );
	alsaButton = new QRadioButton( "ALSA", bg4 );
	bg4->insert( alsaButton, alsa );
	connect( alsaButton, SIGNAL( clicked() ), this, SLOT( alsaButtonClicked() ) );
#endif

#ifdef JACK_SUPPORT
	jackButton = new QRadioButton( "Jack", bg4 );
	bg4->insert( jackButton, jack );
	connect( jackButton, SIGNAL( clicked() ), this, SLOT( jackButtonClicked() ) );
#endif

	bg2 = new QHButtonGroup( tr("OSS Device Mode"), vbox );
	rwButton = new QRadioButton( tr("ReadWrite"), bg2 );
	bg2->insert( rwButton, rw );
	rowoButton = new QRadioButton( tr("ReadOnly / WriteOnly"), bg2 );
	bg2->insert( rowoButton, rowo );
	connect( rwButton, SIGNAL( clicked() ), this, SLOT( rwButtonClicked() ) );
	connect( rowoButton, SIGNAL( clicked() ), this, SLOT( rowoButtonClicked() ) );
	QSettings settings;
	QString dev = settings.readEntry( "/kphone/audio/oss-filename" );
	QString dev2 = settings.readEntry( "/kphone/audio/oss-filename2" );
	(void) new QLabel( tr("Device for WriteOnly or ReadWrite:"), vbox );
	dspdev = new QLineEdit( dev, vbox );
	dspdev->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	(void) new QLabel( tr("Device for ReadOnly:"), vbox );
	dspdev2 = new QLineEdit( dev2, vbox );
	dspdev2->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	bg1 = new QHButtonGroup( tr("Preferred Codec"), vbox );
	pcmuButton = new QRadioButton( tr("G711u"), bg1 );
	bg1->insert( pcmuButton, pcmu );
	pcmaButton = new QRadioButton( tr("G711a"), bg1 );
	bg1->insert( pcmaButton, pcma );
	gsmButton = new QRadioButton( tr("GSM"), bg1 );
	bg1->insert( gsmButton, gsm );
	ilbcButton = new QRadioButton( tr("iLBC"), bg1 );
	bg1->insert( ilbcButton, ilbc );
	connect( pcmuButton, SIGNAL( clicked() ), this, SLOT( pcmuButtonClicked() ) );
	connect( pcmaButton, SIGNAL( clicked() ), this, SLOT( pcmaButtonClicked() ) );
	connect( gsmButton, SIGNAL( clicked() ), this, SLOT( gsmButtonClicked() ) );
	connect( ilbcButton, SIGNAL( clicked() ), this, SLOT( ilbcButtonClicked() ) );

#ifdef ALSA_SUPPORT
	QString audiomode = settings.readEntry( "/kphone/audio/audio", "alsa" );
	if( audiomode == "alsa" ) {
		alsaButton->setChecked(TRUE);
		rwButton->setEnabled(FALSE);
		rowoButton->setEnabled(FALSE);
		dspdev->setEnabled(FALSE);
		dspdev2->setEnabled(FALSE);
#ifdef JACK_SUPPORT
	} else if( audiomode == "jack" ) {
		jackButton->setChecked(TRUE);
		rwButton->setEnabled(FALSE);
		rowoButton->setEnabled(FALSE);
		dspdev->setEnabled(FALSE);
		dspdev2->setEnabled(FALSE);
#endif
	} else {
		ossButton->setChecked(TRUE);
		rwButton->setEnabled(TRUE);
		QString mode = settings.readEntry( "/kphone/audio/mode", "readwrite" );
		if( mode == "readwrite" ) {
			rwButton->setChecked(TRUE);
			dspdev2->setEnabled(FALSE);
		} else {
			rowoButton->setChecked(TRUE);
			dspdev2->setEnabled(TRUE);
		}
	}
#endif

	QString codec = settings.readEntry( "/kphone/audio/Codec", "PCMU" );
	if( codec == "PCMU" ) {
		pcmuButton->setChecked(TRUE);
	} else if( codec == "PCMA" ) {
		pcmaButton->setChecked(TRUE);
	} else if( codec == "GSM" ) {
		gsmButton->setChecked(TRUE);
	} else if( codec == "ILBC" ) {
		ilbcButton->setChecked(TRUE);
	} else if( codec == "ILBC_20" ) {
		ilbcButton->setChecked(TRUE);
	}

	// Size of Payload
	bg3 = new QHButtonGroup( tr("Size of Payload"), vbox );
	s80 = new QRadioButton( tr("80 (10 ms)"), bg3 );
	bg3->insert( s80, size80 );
	s160 = new QRadioButton( tr("160 (20 ms)"), bg3 );
	bg3->insert( s160, size160 );
	s240 = new QRadioButton( tr("240 (30 ms)"), bg3 );
	bg3->insert( s240, size240 );
	switch( settings.readNumEntry( "/kphone/dsp/SizeOfPayload", 80 ) ) {
		case 80: s80->setChecked(TRUE); break;
		case 160: s160->setChecked(TRUE); break;
		case 240: s240->setChecked(TRUE); break;
	}

	if( codec == "ILBC_20" ) {
		s160->setChecked(TRUE);
	} else if( codec == "ILBC" ) {
		s240->setChecked(TRUE);
	}

	if( codec == "PCMU" ) {
		s80->setEnabled(TRUE);
		s160->setEnabled(TRUE);
		s240->setEnabled(FALSE);
	}
	if( codec == "PCMA" ) {
		s80->setEnabled(TRUE);
		s160->setEnabled(TRUE);
		s240->setEnabled(FALSE);
	}
	if( codec == "GSM" ) {
		s80->setEnabled(FALSE);
		s160->setEnabled(TRUE);
		s240->setEnabled(FALSE);
	}
	if( codec == "ILBC" || codec == "ILBC_20" ) {
		s80->setEnabled(FALSE);
		s160->setEnabled(TRUE);
		s240->setEnabled(TRUE);
	}

	// Ringing tone
	ringing = new QHButtonGroup( tr("Ringing tone  (system bell)"), vbox );
	QRadioButton *noRingingtone = new QRadioButton( tr("Don't use ringing tone"), ringing );
	ringing->insert( noRingingtone, ringingtoneNo );
	QRadioButton *yesRingingtone = new QRadioButton( tr("Use ringing tone"), ringing );
	ringing->insert( yesRingingtone, ringingtoneYes );
	if( settings.readEntry( "/kphone/General/ringingtone", "No" ) == "Yes" ) {
		ringing->setButton( ringingtoneYes );
	} else {
		ringing->setButton( ringingtoneNo );
	}

	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel")  );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
}

AudioControl::~AudioControl( void )
{
}

void AudioControl::ossButtonClicked( void )
{
	bg2->setEnabled(TRUE);
	dspdev->setEnabled(TRUE);
	if (rwButton->isChecked() ) {
		dspdev2->setEnabled(FALSE);
	} else {
		dspdev2->setEnabled(TRUE);
	}
}

void AudioControl::alsaButtonClicked( void )
{
	bg2->setEnabled(FALSE);
	dspdev->setEnabled(FALSE);
	dspdev2->setEnabled(FALSE);
}

void AudioControl::jackButtonClicked( void )
{
	bg2->setEnabled(FALSE);
	dspdev->setEnabled(FALSE);
	dspdev2->setEnabled(FALSE);
}

void AudioControl::rwButtonClicked( void )
{
	dspdev2->setEnabled(FALSE);
}

void AudioControl::rowoButtonClicked( void )
{
	dspdev2->setEnabled(TRUE);
}

void AudioControl::pcmuButtonClicked( void )
{
	s80->setEnabled(TRUE);
	s160->setEnabled(TRUE);
	s240->setEnabled(FALSE);
	if( s240->isChecked() ) {
		s160->setChecked(TRUE);
	}
}
void AudioControl::pcmaButtonClicked( void )
{
	s80->setEnabled(TRUE);
	s160->setEnabled(TRUE);
	s240->setEnabled(FALSE);
	if( s240->isChecked() ) {
		s160->setChecked(TRUE);
	}
}

void AudioControl::gsmButtonClicked( void )
{
	s80->setEnabled(FALSE);
	s160->setEnabled(TRUE);
	s240->setEnabled(FALSE);
	s160->setChecked(TRUE);
}

void AudioControl::ilbcButtonClicked( void )
{
	s80->setEnabled(FALSE);
	s160->setEnabled(TRUE);
	s240->setEnabled(TRUE);
	if( s80->isChecked() ) {
		s160->setChecked(TRUE);
	}
}

void AudioControl::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void AudioControl::slotApply( void )
{
	QSettings settings;

	switch( bg1->id( bg1->selected() ) ) {
		case pcmu:
			settings.writeEntry( "/kphone/audio/Codec", "PCMU");
			break;
		case gsm:
			settings.writeEntry( "/kphone/audio/Codec", "GSM");
			break;
		case pcma:
			settings.writeEntry( "/kphone/audio/Codec", "PCMA");
			break;
		case ilbc:
			if( s160->isChecked() ) {
				settings.writeEntry( "/kphone/audio/Codec", "ILBC_20");
			} else {
				settings.writeEntry( "/kphone/audio/Codec", "ILBC");
			}
			break;
	}

	switch( bg2->id( bg2->selected() ) ) {
		case rw:
			settings.writeEntry( "/kphone/audio/mode", "readwrite");
			break;
		case rowo:
			settings.writeEntry( "/kphone/audio/mode", "readonly_writeonly");
			break;
	}

#ifdef ALSA_SUPPORT
	switch( bg4->id( bg4->selected() ) ) {
#ifdef JACK_SUPPORT
		case jack:
			settings.writeEntry( "/kphone/audio/audio", "jack");
			break;
#endif
		case alsa:
			settings.writeEntry( "/kphone/audio/audio", "alsa");
			break;
		case oss:
			settings.writeEntry( "/kphone/audio/audio", "oss");
			break;
	}
#endif

	int payload;
	switch( bg3->id( bg3->selected() ) ) {
		case size80:
			payload = 80;
			break;
		case size240:
			payload = 240;
			break;
		case size160:
		default:
			payload = 160;
			break;
	}
	settings.writeEntry( "/kphone/dsp/SizeOfPayload", payload );

	switch( ringing->id( ringing->selected() ) ) {
		case ringingtoneYes:
			settings.writeEntry( "/kphone/General/ringingtone", "Yes" );
			break;
		default:
			settings.writeEntry( "/kphone/General/ringingtone", "No" );
			break;
	}
}

void AudioControl::slotCancel( void )
{
	QDialog::reject();
}

VideoControl::VideoControl( QWidget *parent, const char *name )
	: QDialog( parent, QString( name ) + tr("Video Properties"), true )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );
	QSettings settings;
	(void) new QLabel( "/kphone/video/Video SW:", vbox );
	QString videosw = settings.readEntry(
		"/kphone/video/videoSW", "/usr/local/bin/vic" );
	QString codec = settings.readEntry( "/kphone/video/codec", "h261" );
	videoSW = new QLineEdit( videosw, vbox );
	videoSW->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	bg1 = new QHButtonGroup( tr("Preferred Video Codec"), vbox );
	QRadioButton *h261Button = new QRadioButton( tr("H261"), bg1 );
	bg1->insert( h261Button, h261 );
	QRadioButton *h263Button = new QRadioButton( tr("H263"), bg1 );
	bg1->insert( h263Button, h263 );
	if( codec == "h261" ) {
		h261Button->setChecked(TRUE);
	} else if( codec == "h263" ) {
		h263Button->setChecked(TRUE);
	}

	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ),
		this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ),
		this, SLOT( slotCancel() ) );

}

VideoControl::~VideoControl( void )
{
}

void VideoControl::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void VideoControl::slotApply( void )
{
	QSettings settings;
	settings.writeEntry( "/kphone/video/videoSW", videoSW->text() );
	switch( bg1->id( bg1->selected() ) ) {
		case h261:
			settings.writeEntry( "/kphone/video/codec", "h261");
			break;
		case h263:
			settings.writeEntry( "/kphone/video/codec", "h263");
			break;
	}
}

void VideoControl::slotCancel( void )
{
	QDialog::reject();
}
