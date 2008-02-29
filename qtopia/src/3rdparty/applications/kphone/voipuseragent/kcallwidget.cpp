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
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <qdatetime.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qdir.h>
// PD #include <qmessagebox.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <math.h>
#include <qgrid.h>
#include <qbuttongroup.h>
#include <qdatetime.h>
#include <qmultilineedit.h>

#include "../dissipate2/sdp.h"
#include "../dissipate2/sipmessage.h"
#include "../dissipate2/mimecontenttype.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/siptransaction.h"
#include "ksipauthentication.h"
#include "callaudio.h"
#include "kphone.h"
#include "kphoneview.h"
#include "kcallwidget.h"
#include <voipuseragent.h>
extern VoIPUserAgent *pVoipUserAgent;

KCallTransfer::KCallTransfer( QWidget *parent, const char *name )
	: QDialog( parent, name, true )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Transfer to URI:"), vbox );
	touri = new QLineEdit( vbox );
	touri->setMinimumWidth( fontMetrics().maxWidth() * 20 );

	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	helpPushButton = new QPushButton( this, tr("help button") );
	helpPushButton->setText( tr("&help...") );
	helpPushButton->setEnabled( FALSE );
	buttonBox->addWidget( helpPushButton );
	QSpacerItem *spacer = new QSpacerItem(
		0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
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

KCallTransfer::~KCallTransfer( void )
{
}

void KCallTransfer::slotOk( void )
{
	if( touri->text().stripWhiteSpace() != QString::null ) {
		pleaseTransfer( touri->text() );
	}
	touri->clear();
	QDialog::accept();
}

void KCallTransfer::slotCancel( void )
{
	touri->clear();
	QDialog::reject();
}

KCallWidget::KCallWidget( KSipAuthentication *auth, CallAudio *callaudio, SipCall *initcall, KPhoneView *parent, const char * /*name*/ )
	: call( initcall ), phoneView( parent ), dtmfsenderdelay( 200 )
{
	sipauthentication = auth;
	hided = false;
	phoneBook = 0;
	curstate = PreDial;
	member = 0;
	audio = callaudio;
	connect( audio, SIGNAL( outputDead() ),
		this, SLOT( audioOutputDead() ) );
	connect( audio, SIGNAL( statusUpdated() ),
		this, SLOT( updateAudioStatus() ) );

	ringCount = 0;
	ringTimer = new QTimer();
	connect( ringTimer, SIGNAL( timeout() ),
		this, SLOT( ringTimeout() ) );
	acceptCallTimer = new QTimer();
	connect( acceptCallTimer, SIGNAL( timeout() ),
		this, SLOT( acceptCallTimeout() ) );

	dtmfsenderTimer = new QTimer();
	connect( dtmfsenderTimer, SIGNAL( timeout() ),
		this, SLOT( dtmfsenderTimeout() ) );

	switchCall( initcall );
}

KCallWidget::~KCallWidget( void )
{
	delete ringTimer;
	delete acceptCallTimer;
	if( call ) {
		delete call;
	}
}

void KCallWidget::createKeypad(void)
{

}

void KCallWidget::setRemote( QString newremote )
{
	uriToCall = newremote;
}

SipCall *KCallWidget::getCall()
{
	return call;
}

void KCallWidget::switchCall( SipCall *newcall )
{
	ringTimer->stop();
	printf( "KCallWidget: Switching calls...\n" );
	call = newcall;
	audio->setBodyMask( call->getSdpMessageMask() );
	if( member ) disconnect( member, 0, this, 0 );
	member = call->getMemberList().toFirst();
	if( member ) {
		if( member->getState() == SipCallMember::state_Disconnected ) {
			forceDisconnect();
     	printf( "KCallWidget: Switching calls forceDisconnect();...\n" );
     	printf( "KCallWidget: Switching calls return \n" );
			return;
		}
		if( member->getState() == SipCallMember::state_Redirected ) {
     	printf( "KCallWidget: Switching calls Redirect();...\n" );
     	printf( "KCallWidget: Switching calls return \n" );
			handleRedirect();
			return;
		}
  // member->getLocalStatusDescription() );
		connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
			this, SLOT( callMemberStatusUpdated() ) );
	//	setCaption( getUserPrefix() + QString( tr("Call: ") ) + call->getCallId() );
		if( call->getSubject() == tr("Incoming call") ) {


    if( call->getCallType() == SipCall::videoCall ) {

           printf( "KCallWidget: Switching calls video inComing();...\n" );

       } else {

           printf( "KCallWidget: Switching calls audio inComing();...\n" );


      }
			QString ss = member->getUri().uri();
			QDateTime t = QDateTime::currentDateTime();
			incomingCall = new IncomingCall( ss, t );

      // Ringing tone
			QSettings settings;
			QString p = "/kphone/General/";
			isRingingTone = ( settings.readEntry(
				p + "/ringingtone", "No" ).upper() == "YES" );

			ringCount = 0;
			ringTimer->start( ringTime_1, TRUE );
		}


    if( member->getState() == SipCallMember::state_InviteRequested ) {

      printf( "KCallWidget: Switching calls invite received i.e outGoing();...\n" );
      curstate = Calling;

    } else if( member->getState() == SipCallMember::state_RequestingInvite ) {

    printf( "KCallWidget: Switching calls inComing : Accept or reject ...\n" );

    //  Accept or Reject

      curstate = Called;

    } else {

      curstate = Connected;
     printf( "KCallWidget: Switching calls  :Call in connected state \n" );

		}

  } else {


    if( call->getCallType() == SipCall::videoCall ) {


  } else {


      }
		//subject = call->getSubject();
		curstate = PreDial;
	}
	updateAudioStatus();
  printf( "KCallWidget: Switching calls return \n" );

}

void KCallWidget::dialClicked( void )
{
	// Multi-purpose buttons hack
	if( curstate == Called ) {
    // Accept
		acceptCall();
		return;
	}
  // Need to have some vakid destination
  // Check if remote->currentText(); is non zero length

  QString strRemoteUri;
	QString s = uriToCall;//"676576"; // USer part of the SIP url // it was s= remote->currentText();

  if( s.contains( '[' ) && s.contains( ']' ) ) {
		strRemoteUri = s.mid( s.find( '[' ) + 1, s.find( ']' ) - s.find( '[' ) - 1 );
	}
  else {
	if( s.left( 4 ).lower() != "tel:" ) {
		if( s.left( 4 ).lower() != "sip:" ) {
			s = "sip:" + s;
		}
		if( !s.contains( '@' ) ) {
			s = s + "@" + call->getHostname();
		}
	}
	strRemoteUri = s;
   }

        SipUri remoteuri( strRemoteUri );
	audio->setRtpCodec( codecUnknown );
	audio->setVideoRtpCodec( codecUnknown );
	member = new SipCallMember( call, remoteuri );
	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
		this, SLOT( callMemberStatusUpdated() ) );
	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
		sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
	audio->setCurrentCall( call );
	member->requestInvite(
		audio->audioOut().message( audio->getRtpCodec(), audio->getVideoRtpCodec(), audio->getBodyMask() ),
		MimeContentType( "application/sdp" ) );
	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
		this, SLOT( callMemberStatusUpdated( ) ) );
	audio->attachToCallMember( member );
	curstate = Calling;
}

void KCallWidget::callMemberStatusUpdated( void )
{

  printf("KCallWidget::callMemberStatusUpdated - inside\n") ;
	SdpMessage sdpm;
	SdpMessage rsdp;
	if(pVoipUserAgent)
	{	pVoipUserAgent->CallStatusCallback(this);
	}
	if( member->getState() == SipCallMember::state_Disconnected ) {
		if( member->getLocalStatusDescription().left( 2 ) == "!!" ) {
			// PD TBDIMP check if handled in adaptation layer?
            // curstat->setText( tr("Call Failed") );
			//QMessageBox::critical( this, "KPhone",
			//	member->getLocalStatusDescription().remove(0,2) );
			//setHide();
		} else {
			//curstat->setText( member->getLocalStatusDescription()
			//		+ " at " + QDateTime::currentDateTime().toString());
		}
		forceDisconnect();
	} else if( member->getState() == SipCallMember::state_Redirected ) {
		//curstat->setText( member->getLocalStatusDescription() );
		//handleRedirect();
	} else if( member->getState() == SipCallMember::state_Refer ) {
		//curstat->setText( member->getLocalStatusDescription() );
		//member->setState( SipCallMember::state_Refer_handling );
		//handleRefer();
	} else if( member->getState() == SipCallMember::state_Connected ) {
		//curstat->setText( member->getLocalStatusDescription() );
		curstate = Connected;
		//if (!dtmfsenderTimer->isActive())
		//	dtmfsenderTimer->start(dtmfsenderdelay);
	} else {
		//curstat->setText( member->getLocalStatusDescription() );
	}
  printf("KCallWidget::callMemberStatusUpdated - out \n");  ;

 
}

void KCallWidget::audioOutputDead( void )
{
	printf( "KCallAudio: Broken output pipe, disconnecting unpolitely\n" );
	forceDisconnect();
}

void KCallWidget::hangupCall( void )
{
	ringTimer->stop();

	// Reject call if that's our current state
	if( curstate == Called ) {
		member->declineInvite();
		setHide();
		return;
	}
	if( call->getCallStatus() != SipCall::callDead ) {

    if( member->getState() == SipCallMember::state_Connected ) {
			member->requestDisconnect();
		} else {
			member->cancelTransaction();
			if( audio->getCurrentCall() == call ) {
				audio->detachFromCall();
			}
		}
		return;
	}
}

void KCallWidget::acceptCall( void )
{
	ringTimer->stop();
	acceptCallTimer->start( acceptTime, true );
}

void KCallWidget::holdCall( void )
{
    if( curstate != Connected )
		return;
	if( audio->getCurrentCall() == call ) {
		audio->toggleOnHold();
	//	holdbutton->setText( tr("Unhold") );
	} else if( member ) {
		audio->setCurrentCall( call );
		audio->attachToCallMember( member );
		audio->toggleOnHold();
		//holdbutton->setText( tr("Hold") );
	}
}

void KCallWidget::handleRedirect( void )
{
}

void KCallWidget::handleRefer( void )
{
}

void KCallWidget::forceDisconnect( void )
{
	printf( "KCallWidget: Starting force disconnect...\n" );
	if( audio && audio->getCurrentCall() == call ) {
		audio->detachFromCall();
	}
	printf( "KCallWidget: Starting force disconnect...\n" );
	printf( "KCallWidget: Starting force disconnect...\n" );
	if( member )
  disconnect( member, 0, this, 0 );

  member = 0;
 if( call )
  delete call;
	call = 0;

  callDeleted();
 ringTimer->stop();
}

void KCallWidget::updateAudioStatus( void )
{
int DoSomething;

  printf("KCallWidget::updateAudioStatus - inside\n");

  if( audio->getCurrentCall() == call )
  {
		if( audio->isRemoteHold() )
     DoSomething++;
     else 
     DoSomething--;

 }
 else
 {
		//curaudiostat->setText( tr("Unattached") );

 }

  printf("KCallWidget::updateAudioStatus - out\n");
}

void KCallWidget::clickDial()
{
  printf("KCallWidget::clickDial inside\n");
	dialClicked();
  printf("KCallWidget::clickDial out \n");
}

void KCallWidget::clickHangup()
{
  printf("KCallWidget::hangup inside\n");
	hangupCall();
  printf("KCallWidget::hangup out \n") ;
}

void KCallWidget::pleaseDial( const SipUri &/*dialuri*/ )
{
  printf("KCallWidget::pleaseDial inside\n");
//remote->setEditText( dialuri.reqUri() );
 dialClicked();
  printf("KCallWidget::pleaseDial out \n");

}

void KCallWidget::showTransferDialog( void )
{
}

void KCallWidget::pleaseTransfer( const QString &/*transferto*/ )
{
}

void KCallWidget::feedTone(const QString& tones)
{
    dtmfsender.append(tones);
    if (!dtmfsenderTimer->isActive())
        dtmfsenderTimeout();
}

void KCallWidget::show( )
{


}


void KCallWidget::getUri( void )
{
	QSettings settings;
	QString p = "/kphone/" + getUserPrefix() + "/Local";
	QString xmlFile = settings.readEntry( p + "/PhoneBook", "" );
	if( xmlFile.isEmpty() ) {
		if( getUserPrefix().isEmpty() ) {
			xmlFile = QDir::homeDirPath() + "/.kphone-phonebook.xml";
		} else {
			xmlFile = QDir::homeDirPath() +
				"/.kphone" + getUserPrefix() + "phonebook.xml";
		}
	}
	QStringList r;
	//if( !phoneBook ) {
	//	phoneBook = new PhoneBook(xmlFile, this, getUserPrefix() + "Phone Book", receivedCalls, missedCalls, r );
	//}
	//phoneBook->exec();
	//remote->setEditText( phoneBook->getUri() );
	//phoneView->updateContacts( xmlFile );
}

void KCallWidget::updateCallRegister( void )
{
}

void KCallWidget::dtmfsenderTimeout( void )
{
    char c;
    if (dtmfsender.length() == 0 || curstate != Connected) {
        dtmfsenderTimer->stop();
    } else {
        c = dtmfsender.at(0).latin1();
        dtmfsenderTimer->changeInterval(dtmfsenderdelay);
        switch (c) {
            case '0':       case '1':       case '2':       case '3':       case '4':
            case '5':       case '6':       case '7':       case '8':       case '9':
            case '#':       case '*':       case 'A':       case 'a':       case 'B':
            case 'b':       case 'C':       case 'c':       case 'D':       case 'd':
            case 'X':       case 'x':       case 'Y':       case 'y':       case 'Z':
            case 'z':
                audio->startDTMF(c);
                dtmfsender = (dtmfsender.mid(1).prepend('w'));
                break;
            case 'W':
                audio->stopDTMF();
                dtmfsender = (dtmfsender.mid(1).prepend('w'));
                break;
            case 'w':
                audio->stopDTMF();
                dtmfsender = (dtmfsender.mid(1));
                break;
            case ' ':       case '-':       case '(':       case ')':
                /* Quick-pause */
                dtmfsender = (dtmfsender.mid(1));
                dtmfsenderTimer->changeInterval(1);
                break;
            case '/':
            default:
                /* This blocks DTMF... */
                dtmfsenderTimer->stop();
        }
    }
}

void KCallWidget::ringTimeout( void )
{
	if( isRingingTone ) {
		if( ringCount < 5 ) {
			ringCount++;
			audio_fd = ::open( audio->getOSSFilename(), O_WRONLY | O_NONBLOCK );
			if( audio_fd == -1 ) {
				printf( "ERROR: %s\n", "Open Failed" );
				return;
			}
			int flags = fcntl( audio_fd, F_GETFL );
			flags &= ~O_NONBLOCK;
			fcntl( audio_fd, F_SETFL, flags );
			int format = AFMT_S16_LE;
			if( ioctl( audio_fd, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
				return;
			}
			if( format != AFMT_S16_LE ) {
				return;
			}
			int channels = 1;
			if( ioctl( audio_fd, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
				return;
			}
			if( channels != 1 ) {
				return;
			}
			int rate = 8000;
			if( ioctl( audio_fd, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
				return;
			}
			if( rate != 8000) {
				return;
			}
			int size = 4096;
			int samp = size/2;
			int ampl = 16384;
			unsigned char devbuf[size];
			int buf[samp];
			int i, p=0;
			double arg1, arg2;
			arg1 = (double)2 * (double)M_PI * (double)941 / (double)samp;
			arg2 = (double)2 * (double)M_PI * (double)1336 / (double)samp;
			for(int i = 0; i < samp; i++) {
				buf[i] = (short)((double)(ampl) * sin(arg1 * i) +
					(double)(ampl) * sin(arg2 * i));
			};
			for (i=0; i<samp; i+=2) {
				devbuf[p++] = (unsigned char)(buf[i] & 0xff);
				devbuf[p++] = (unsigned char)((buf[i] >> 8) & 0xff);
			}
			for(;;) {
				if( write( audio_fd, devbuf, samp ) != -1 ) {
					break;
				}
			}
			::close( audio_fd );
                        audio_fd = -1;
			ringTimer->start( ringTime_1, TRUE );
		} else {
			ringCount = 0;
			printf( "Ringing tone\n" );
			ringTimer->start( ringTime_2, TRUE );
		}
	}
}

void KCallWidget::acceptCallTimeout( void )
{

printf("\nKCallWidget::acceptCallTimeout - inside\n ");
  if( !audio->isAudioOn() ) {
                if( audio_fd != -1 ) {
                    printf("KCallWidget::acceptCallTimeout - forcing audio off");
                    ::close(audio_fd);
                    audio_fd = -1;
                }
		audio_fd = ::open( audio->getOSSFilename(), O_WRONLY | O_NONBLOCK );
		if( audio_fd == -1 ) {

            // PD TBDIMP
            // Add callback to convey that the hardware is busy

            //QMessageBox::information( this, tr("Accept Call"),
			//	tr("Close any program which might be using soundcard \nand then retry accepting call.") );
                        printf("KCallWidget::acceptCallTimeout - failed to access sound hardware\n");
			return;
		} else {
			::close( audio_fd );
                        audio_fd = -1;
		}
	}


	if( audio->checkCodec( member ) ) {
		audio->setCurrentCall( call );
		audio->attachToCallMember( member );
printf("\nKCallWidget::acceptCallTimeout   Accept Invite - inside\n ");
		member->acceptInvite(
			audio->audioOut().message( audio->getRtpCodec(),
				audio->getVideoRtpCodec(), audio->getBodyMask() ),
			MimeContentType( "application/sdp" ) );
printf("\nKCallWidget::acceptCallTimeout   Came out of invite still inside\n ");


  } else {
		member->notAcceptableHere();
printf("\nKCallWidget::acceptCallTimeout   Wrong codec not acceptable inside\n ");
		//QMessageBox::information( this, tr("Accept Call"),
		//	tr("Accepted codec not found.") );
	}

 printf("\nKCallWidget::acceptCallTimeout   out \n ");
 
}

void KCallWidget::hideCall( void )
{
	if( curstate == PreDial ) {
		delete call;
		call = 0;
	}
	setHide();
}

QString KCallWidget::getUserPrefix( void )
{
	return ((KPhone *)phoneView->parentWidget())->getUserPrefix();
}

void KCallWidget::setHide( void )
{
	hided = true;
	//hide();
}

KInstantMessageWidget::KInstantMessageWidget( KSipAuthentication *auth,
		SipCall *initcall, QWidget *parent, const char *name )
	: QDialog( parent, name ), imcall( initcall )
{
	sipauthentication = auth;
	phoneBook = 0;
	setCaption( getUserPrefix() + tr(" Message - KPhone") );
	QVBoxLayout *vbox = new QVBoxLayout( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QGridLayout *headergrid = new QGridLayout( 6, 3 );
	headergrid->setSpacing( 3 );
	vbox->insertLayout( 0, headergrid );
	QLabel *remotelabel = new QLabel( tr("Remote: "), this );
	remotelabel->setFixedWidth( remotelabel->fontMetrics().width( tr("  Remote:  ")  ) );
	headergrid->addWidget( remotelabel, 0, 0 );
	remote = new QComboBox( true, this );
	remote->setAutoCompletion( true );
	remote->setEditText( "" );
	headergrid->addWidget( remote, 0, 1 );
	QPushButton *loadUri = new QPushButton( "", this );
	QIconSet icon;
	icon.setPixmap(SHARE_DIR "/icons/phonebook.png", QIconSet::Automatic );
	loadUri->setIconSet( icon );
	loadUri->setFixedWidth( loadUri->fontMetrics().maxWidth() * 2 );
	connect( loadUri, SIGNAL( clicked() ), this, SLOT( getUri() ) );
	headergrid->addWidget( loadUri, 0, 2 );
	QLabel *slabel = new QLabel( tr("Status: "), this );
	vbox->insertWidget( 1, slabel );
	status = new QTextView( this );
	vbox->insertWidget( 2, status );
	QLabel *mlabel = new QLabel( tr("Message: "), this );
	vbox->insertWidget( 3, mlabel );
	QGridLayout *mgrid = new QGridLayout( 1, 3 );
	mgrid->setSpacing( 3 );
	vbox->insertLayout( 4, mgrid );
	mEdit = new QMultiLineEdit( this );
	mgrid->addWidget( mEdit, 0,0 );
	mEdit->setFocus();
	mbutton = new QPushButton( tr("Send"), this );
	mbutton->setFixedWidth( mbutton->fontMetrics().maxWidth() * 4 );
	connect( mbutton, SIGNAL( clicked() ), this, SLOT( mClicked() ) );
	mgrid->addWidget( mbutton, 0, 2 );
	QGrid *buttonbox = new QGrid(2, this );
	buttonbox->setSpacing( 5 );
	vbox->insertWidget( 5, buttonbox );
	(void) new QWidget( buttonbox );
	hidebutton = new QPushButton( tr("Hide"), buttonbox );
	connect( hidebutton, SIGNAL( clicked() ), this, SLOT( hide() ) );
	resize( 400, 200 );
}

KInstantMessageWidget::~KInstantMessageWidget( void )
{
	if( phoneBook ) {
		delete phoneBook;
	}
}

void KInstantMessageWidget::instantMessage( SipMessage *message )
{
	QString s = message->getHeaderData( SipHeader::From );
	SipUri u = SipUri( s );
	remote->setEditText( u.getUsername() + "@" + u.getHostname() );
	QString m = message->messageBody();
	status->setText( status->text() + "\n" + tr("Message received from ") + remote->currentText() );
	status->setText( status->text() + " (" + QTime::currentTime().toString().latin1() + ")" );
	status->setText( status->text() + "\n > " + m );
}

SipCall *KInstantMessageWidget::getCall()
{
	return imcall;
}

QString KInstantMessageWidget::getRemote()
{
	return remote->currentText();
}

void KInstantMessageWidget::setRemote( QString newremote )
{
	remote->setEditText( newremote );
}

void KInstantMessageWidget::mClicked( void )
{
	if( remote->currentText().length() == 0 ) {
//		QMessageBox::critical( this, tr("Error: No Destination"),
//			tr("You must specify someone to send message.") );
		return;
	}
	QString strRemoteUri;
	QString s = remote->currentText();
	if( s.contains( '[' ) && s.contains( ']' ) ) {
		strRemoteUri = s.mid( s.find( '[' ) + 1, s.find( ']' ) - s.find( '[' ) - 1 );
	} else {
		if( s.left( 4 ).lower() != "tel:" ) {
			if( s.left( 4 ).lower() != "sip:" ) {
				s = "sip:" + s;
			}
			if( !s.contains( '@' ) ) {
				s = s + "@" + imcall->getHostname();
			}
		}
		strRemoteUri = s;
	}
	for( int i = 0; i < remote->count(); i++ ) {
		if( remote->text( i ).compare( s ) == 0 ) {
			remote->removeItem( i );
		}
	}
	remote->insertItem( s, 0 );
	remote->setCurrentItem( 0 );
	SipUri remoteuri( strRemoteUri );
	member = new SipCallMember( imcall, remoteuri );
	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
		sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
	status->setText( status->text() + "\n" + tr("Message sent to ") + remote->currentText() );
	status->setText( status->text() + " (" + QTime::currentTime().toString().latin1() + ")" );
	status->setText( status->text() + "\n > " + mEdit->text() );

	member->requestMessage( mEdit->text(),
		MimeContentType( "text/plain; charset=UTF-8" ) );
	mEdit->clear();
}

void KInstantMessageWidget::getUri( void )
{
	QSettings settings;
	QString p = "/kphone/" + getUserPrefix() + "/local";
	QString xmlFile = settings.readEntry( p + "/PhoneBook", "" );
	if( xmlFile.isEmpty() ) {
		if( getUserPrefix().isEmpty() ) {
			xmlFile = QDir::homeDirPath() +
				"/.kphone-phonebook.xml";
		} else {
			xmlFile = QDir::homeDirPath() +
				"/.kphone" + getUserPrefix() + "phonebook.xml";
		}
	}
	QStringList r;
	if( !phoneBook ) {
		phoneBook = new PhoneBook( xmlFile, this,
			getUserPrefix() + "Phone Book",
			receivedCalls, missedCalls, r );
	}
	phoneBook->exec();
	remote->setEditText( phoneBook->getUri() );
	((KPhoneView *)parentWidget())->updateContacts( xmlFile );
}


QString KInstantMessageWidget::getUserPrefix( void )
{
	return ((KPhone *)parentWidget()->parentWidget())->getUserPrefix();
}
