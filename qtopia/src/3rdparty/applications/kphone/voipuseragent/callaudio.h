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
#ifndef CALLAUDIO_H_INCLUDED
#define CALLAUDIO_H_INCLUDED

#include <qobject.h>
#include <qstring.h>
#include <sys/types.h>

#include "../dissipate2/sdp.h"
#include "../dissipate2/udpmessagesocket.h"

class DspAudio;
class SipCall;
class SipCallMember;
class KPhoneView;
class DspAudioJackOut;
class DspAudioJackIn;


enum audiomodeType {
	oss,
	alsa,
	jack };


class CallAudio : public QObject
{
	Q_OBJECT
public:
	CallAudio( KPhoneView *phoneview );
	~CallAudio( void );
	void setAudiomode( QString str );
	void setOSSFilename( const QString &devname );
	void setOSSFilename2( const QString &devname );
	const QString &getOSSFilename( void ) const { return ossfilename; }
	const QString &getOSSFilename2( void ) const { return ossfilename2; }
	void setVideoSW( const QString &sw );
	const QString &getVideoSW( void ) const { return videoSW; }

	// Call attachment
	void attachToCallMember( SipCallMember *newmember );
	void detachFromCall( void );
	void toggleOnHold( void );
	void renegotiateCall( void );

	void startDTMF(char code); // Start DTMF tone generation
	void stopDTMF(void);  // Stop DTMF tone generation

	// Returns a session description for sending in responses
	SdpMessage audioOut( void );

	// Call tracking
	void setCurrentCall( SipCall *newcall );
	SipCall *getCurrentCall( void ) const { return curcall; }

	bool isRemoteHold( void );

	// Settings
	void saveAudioSettings( void );
	void readAudioSettings( void );
	void readVideoSettings( void );
	void setPayload( int newPayload ) { payload = newPayload; }
	bool isAudioOn( void );

	// Set/reset symmetric mode
	void setSymMediaMode( bool yesno ) { symMedia = yesno; }

	void setCodec( codecType newCodec ) { codec = newCodec; }
	void setRtpCodec( codecType newCodec ) { rtpCodec = newCodec; }
	void setVideoCodec( codecType newCodec ) { videoCodec = newCodec; }
	void setVideoRtpCodec( codecType newCodec ) { videoRtpCodec = newCodec; }
	codecType getRtpCodec( void );
	int getRtpCodecNum( void );
	short getRtpBlockLength( void );
	codecType getVideoRtpCodec( void );
	int getVideoRtpCodecNum( void );
	QString getVideoRtpCodecName( void );
	void setBodyMask( QString body ) { bodyMask = body; }
	QString getBodyMask( void ) { return bodyMask; }
	bool checkCodec( SipCallMember *member );
	void setStunSrv( QString newStunSrv );
	
signals:
	void outputDead( void );
	void statusUpdated( void );

private slots:
	void memberStatusUpdated(SipCallMember *member);

private:
	KPhoneView *view;
	DspAudio *input;
	DspAudio *output;
	DspAudioJackOut *jack_audioout;
	DspAudioJackIn  *jack_audioin;
	int audio_fd;
	SdpMessage local;
	SdpMessage remote;
	audiomodeType audiomode;
	QString ossfilename;
	QString ossfilename2;
	QString videoSW;
	SipCall *curcall;
	SipCallMember *curmember;
	void audioIn( void );
	void stopListeningAudio( void );
	void stopSendingAudio( void );
	void detachAndHold( void );
	int payload;
	codecType codec;
	codecType rtpCodec;
	codecType videoCodec;
	codecType videoRtpCodec;
	int rtpCodecNum;
	int videoRtpCodecNum;
	QString bodyMask;
	pid_t pidVideo;
	bool useStun;
	bool symMedia;
	QString stunSrv;

	/*
	 * Common socket for both streams, used only in symmetric mode
	 */
	UDPMessageSocket socket;
};


#endif // CALLAUDIO_H_INCLUDED
