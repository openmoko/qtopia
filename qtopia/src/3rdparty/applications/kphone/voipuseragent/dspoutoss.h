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
#ifndef DSPOUTOSS_H_INCLUDED
#define DSPOUTOSS_H_INCLUDED

#include <sys/soundcard.h>
#include "dspout.h"

/**
 * @short OSS implementation of DspOut.
 * @author Billy Biggs <vektor@div8.net>
 *
 * This is the OSS implementation of DspOut for the /dev/dsp*
 * devices.  Note that you cannot change how many fragments
 * this class requests, yet.
 */
class DspOutOss : public DspOut
{
public:
	/**
	 * Constructs a DspOutOss object representing the given
	 * filename.  Default is /dev/dsp.
	 */
	DspOutOss( const QString &fileName = "/dev/dsp" );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspOutOss( void );

	bool openDevice( DeviceMode mode );
	bool openDevice( int exist_fs );
	bool writeBuffer( void );
	unsigned int readableBytes( void );
	bool readBuffer( int bytes );
	int audio_fd;

private:
	QString filename;         // device filename
	bool audio_stereo;
};

#endif  // DSPOUTOSS_H_INCLUDED
