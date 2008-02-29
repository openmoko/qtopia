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
#ifndef DSPTHREAD_H_INCLUDED
#define DSPTHREAD_H_INCLUDED

#include "../config.h"

#ifdef QT_THREAD_SUPPORT
#include <qthread.h>
#else
#include <qtimer.h>
#include <qobject.h>
#endif

#include "../dissipate2/sdp.h"
#include "audiobuffer.h"
#include "dtmfgenerator.h"


class DspOut;

#ifdef QT_THREAD_SUPPORT
class DspAudio : public QThread
#else
class DspAudio : public QObject
#endif
{
public:
	DspAudio( DspOut *in, DspOut *out );
	virtual ~DspAudio( void );
#ifdef QT_THREAD_SUPPORT
	virtual void run();
#else
	virtual void start( void ) = 0;
	bool running( void ) { return false; }
	void wait( void ) { return; }
	void exit( void ) { return; }
#endif
	virtual void timerTick( void ) = 0;
	void startTone(char code);
	void stopTone(void);

	/*
	 * Generate DTMF if required
	 * returns 1 if DTMF was generated
	 * and 0 if not
	*/
	bool generateDTMF(short* buffer, size_t n);
	void setCancel( void ) { cancel = true; }
	void setCodec( const codecType newCodec, int newCodecNum );

protected:
	bool isCanceled( void ) { return cancel; }
	DspOut *input;
	DspOut *output;
	bool broken;
	bool cancel;
	DTMFGenerator dtmf;
	AudioBuffer copybuffer;
	unsigned int dtmfSamples;
	char newTone;      // New DTMF tone to be generated (0 if none)
	char currentTone;  // Tone currently being generated (0 if none)

#ifndef QT_THREAD_SUPPORT
	QTimer *timer;
#endif

};

#endif // DSPTHREAD_H_INCLUDED
