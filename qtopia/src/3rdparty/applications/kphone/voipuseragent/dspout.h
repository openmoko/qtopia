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
#ifndef DSPOUT_H_INCLUDED
#define DSPOUT_H_INCLUDED

#include <qstring.h>

#include "audiobuffer.h"

/**
 * @short Base class for DSP audio devices.
 * @author Billy Biggs <vektor@div8.net>
 *
 * This is the base class for DSP output devices, such as
 * /dev/dsp for OSS, or /dev/audio for Sun audio.
 */ 
class DspOut
{
public:
	/**
	 * DspOut constructor just does simple initialization.
	 */
	DspOut( void );

	/**
	 * DspOut destructor.
	 */
	virtual ~DspOut( void );

	enum DeviceState {
		DeviceOpened,
		DeviceClosed };

	enum DeviceMode {
		ReadOnly,
		WriteOnly,
		ReadWrite };

	/**
	 * This is the call to actually open the device for
	 * audio output.  All setup (samplerate, format) must be
	 * done before the call to openDevice().
	 */
	virtual bool openDevice( DeviceMode mode ) = 0;

	/**
	 * Return the state of the device, opened or closed.
	 */
	DeviceState getDeviceState( void ) const { return devstate; }

	/**
	 * This provides the caller with the next audio buffer to
	 * fill.  A call to @ref #writeBuffer () will cause this
	 * to actually be queued by the soundcard.
	 *
	 * The format of the audio buffer is that of the internal
	 * program format.  The call to @ref #writeBuffer () will
	 * perform the appropriate conversion to the output format,
	 * if any is required.
	 */
	AudioBuffer &getBuffer( void ) { return audio_buf; }

	/**
	 * Send the current audio buffer (available using @ref #getBuffer () )
	 * to the soundcard.  This applies any conversion necessary to go
	 * from the internal format to the format required by the device.
	 */
	virtual bool writeBuffer( void ) = 0;

	/**
	 * Returns the number of currently readable bytes.
	 */
	virtual unsigned int readableBytes( void ) = 0;

	/**
	 * Reads an incoming audio buffer from the soundcard.
	 */
	virtual bool readBuffer( int bytes = 0 ) = 0;

	/**
	 * Returns the name of the current output device.  This is simply
	 * for the user to be able to identify the device, and does not
	 * need to correspond to anything real, such as the filename.
	 */
	QString getDeviceName( void ) const { return devname; }

	/**
	 * Sets the current output device name.  This is simply for the user
	 * to be able to identify the device, and does not need to correspond
	 * to anything real, such as the filename.
	 */
	void setDeviceName( const QString &newDevName );


protected:
	DeviceState devstate;  // Current state
	DeviceMode devmode;    // Current mode
	QString lasterror;     // Informative text output of last error
	QString devname;       // Device Name, (not necessarily filename!)
	AudioBuffer audio_buf; // Buffer that the application fills
	int rate;              // Sampling rate (in fps)
};

#endif  // DSPOUT_H_INCLUDED
