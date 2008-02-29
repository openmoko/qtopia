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
#ifndef AUDIOBUFFER_H_INCLUDED
#define AUDIOBUFFER_H_INCLUDED

#include <stdlib.h>

/**
 * Small class for passing around buffers of audio data. Does not
 * specify a format, so the responsibility is on the programmer to
 * know what the datatype of the buffer really is.
 */
class AudioBuffer
{
public:
	/**
	 * Creates an audio buffer of @param length bytes.
	 */
	AudioBuffer( size_t length = 4096 );

	/**
	 * Deletes the audio buffer, freeing the data.
	 */
	~AudioBuffer( void );

	/**
	 * Returns a pointer to the audio data.
	 */
	void *getData( void ) const { return data; }

	/**
	 * Returns the size of the buffer.
	 */
	size_t getSize( void ) const { return size; }

	/**
	 * Resizes the buffer to size newlength. Will only allocate new memory
	 * if the size is larger than what has been previously allocated.
	 */
	void resize( size_t newsize );

private:
	void *data;
	size_t size;
	size_t realsize;
};

#endif // AUDIOBUFFER_H_INCLUDED
