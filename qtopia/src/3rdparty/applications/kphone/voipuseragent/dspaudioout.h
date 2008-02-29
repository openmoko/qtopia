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
#ifndef DSPAUDIOOUT_H_INCLUDED
#define DSPAUDIOOUT_H_INCLUDED

#include "../config.h"
#include "dspaudio.h"

class DspOut;

class DspAudioOut : public DspAudio
{
#ifndef QT_THREAD_SUPPORT
	Q_OBJECT
#endif
public:
	DspAudioOut( DspOut *in, DspOut *out );
	~DspAudioOut( void );

	/**
	 * Just do one tick, for non-threaded implementations.
	 */
	virtual void timerTick( void );

#ifndef QT_THREAD_SUPPORT
	virtual void start( void );

private slots:
	virtual void timeout( void );

#endif

private:
	unsigned int curpos;
};

#endif // DSPAUDIOOUT_H_INCLUDED
