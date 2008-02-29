/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
  \class MediaRecorderEncoder mediarecorderplugininterface.h
  \brief Record multimedia data and encode using a specific format

  The abstract MediaRecorderEncoder class represents an interface for
  recording multimedia data to a file or other output device.

  The functions in this class are typically used in the following order
  when recording audio:

  \code
  begin(device, tag);
  addComment("name", "value");
  ...
  addComment("name", "value");
  setAudioChannels(2);
  setAudioFrequency(44100);
  writeAudioSamples(samples, numSamples);
  ...
  writeAudioSamples(samples, numSamples);
  end();
  \endcode

  \sa QIODevice, MediaRecorderPluginInterface, begin()

  \ingroup qtopiaemb
*/

/*!
  \fn int MediaRecorderEncoder::pluginNumFormats() const

  Returns the number of formats that are supported by this plugin.
*/

/*!
  \fn QString MediaRecorderEncoder::pluginFormatName( int format ) const

  Returns the name of a plugin's data format.  For example, "Wav Format".
  This may be displayed to the user in a list of supported recording formats.
*/

/*!
  \fn QString MediaRecorderEncoder::pluginFormatTag( int format ) const

  Returns the tag name of a plugin's data format.  For example, "pcm".
  This value is used with \link MediaRecorderEncoder::begin \endlink .
*/

/*!
  \fn QString MediaRecorderEncoder::pluginComment() const

  Returns a comment that describes the purpose of the plugin.
*/

/*!
  \fn double MediaRecorderEncoder::pluginVersion() const

  Returns the version of the plugin.  Normally 1.0.
*/

/*!
  \fn QString MediaRecorderEncoder::pluginMimeType() const

  Returns the MIME type for the plugin's recommended
  \link MediaRecorderEncoder::pluginExtension() file extension \endlink .
  For example, \c audio/x-wav .
*/

/*!
  \fn bool MediaRecorderEncoder::begin( QIODevice *device, const QString& formatTag )

  Begin recording on the specified output \a device, which must be
  capable of direct access (seeking) if
  \link MediaRecorderEncoder::requiresDirectAccess() \endlink returns
  TRUE.

  Returns TRUE if recording has begun.  Returns FALSE if recording
  is already in progress or if \a device is not capable of seeking.

  This call will typically be followed by calls to set the
  \link setAudioChannels() channels \endlink,
  \link setAudioFrequency() frequency \endlink,
  and \link addComment() file comments \endlink.

  \sa setAudioChannels(), setAudioFrequency(), addComment(),
  writeAudioSample(), requiresDirectAccess()
*/

/*!
  \fn bool MediaRecorderEncoder::end()

  End recording on the current output device.  This function may
  back-patch earlier bytes in the output.  Once it has finished
  outputting the data, it will leave the device positioned after
  all bytes that were written.

  Returns TRUE if recording was successfully terminated.
  Returns FALSE if not currently recording, or there was an
  error writing to the device.
*/

/*!
  \fn bool MediaRecorderEncoder::isActive() const

  Returns TRUE if the recorder is currently active; FALSE otherwise.
*/

/*!
  \fn bool MediaRecorderEncoder::setAudioChannels( int channels )

  Sets the number of audio channels in the recorded data to either 1 or 2.

  Returns TRUE if the channel count was set successfully.  Returns FALSE
  if not recording, the data header has already been written, or if
  \a channels is neither 1 nor 2.

  The data header is considered written upon the first call to
  \link writeAudioSamples() write audio samples \endlink.

  \sa begin(), setAudioFrequency(), writeAudioSamples()
*/

/*!
  \fn bool MediaRecorderEncoder::setAudioFrequency( int frequency )

  Sets the audio sample frequency in the recorded data.

  Returns TRUE if the frequency was set successfuly.  Returns FALSE
  if not recording, the data header has already been written, or if
  \a frequency is less than or equal to zero.

  The data header is considered written upon the first call to
  \link writeAudioSamples() write audio samples \endlink.

  \sa begin(), setAudioChannels(), writeAudioSamples()
*/

/*!
  \fn bool MediaRecorderEncoder::writeAudioSamples( const short *samples, long numSamples )

  Writes a buffer of audio samples to the recorded output.

  Samples are assumed to always be 16-bit and in host byte order.
  It is the responsibility of the caller to rescale other sample sizes.

  The \a numSamples value is the number of 16-bit quantities in the
  \a samples buffer.  This will be a multiple of two for stereo data,
  with alternating channel samples.

  Returns TRUE if the samples were successfully written.  Returns FALSE
  if not recording or there was an error writing to the output device.

  \sa begin(), setAudioChannels(), setAudioFrequency()
*/

/*!
  \fn bool MediaRecorderEncoder::addComment( const QString& tag, const QString& contents )

  Add a tagged comment string to the recorded output.  The plugin may
  ignore tags that it doesn't understand.

  Returns TRUE if the comment was successfully added (or ignored).
  Returns FALSE if not recording or the data header has already been
  written.

  The data header is considered written upon the first call to
  \link writeAudioSamples() write audio samples \endlink.  This is true
  even if plugin's data format places comments at the end of the stream,
  rather than the front.  The plugin should cache the comments until
  it is ready to output them.

  This should only be called if \link supportsComments() comments \endlink
  are supported by the plugin.

  \sa begin(), writeAudioSamples(), supportsComments()
*/

/*!
  \fn long MediaRecorderEncoder::estimateAudioBps( int frequency, int channels, const QString& formatTag )

  Estimate the number of bytes per second that are needed to record
  audio in the \a formatTag format at a given \a frequency with the
  specified number of \a channels.
*/

/*!
  \fn bool MediaRecorderEncoder::supportsAudio() const

  Returns TRUE if this plugin supports audio; FALSE otherwise.
*/

/*!
  \fn bool MediaRecorderEncoder::supportsVideo() const

  Returns TRUE if this plugin supports video; FALSE otherwise.
*/

/*!
  \fn bool MediaRecorderEncoder::supportsComments() const

  Returns TRUE if this plugin supports comments; FALSE otherwise.

  \sa addComment()
*/

/*!
  \fn bool MediaRecorderEncoder::requiresDirectAccess() const

  Returns TRUE if this plugin must be supplied a direct access (seekable)
  output device.

  \sa begin()
*/

/*!
  \class MediaRecorderPluginInterface mediarecorderplugininterface.h
  \brief Obtain an encoder for recording multimedia data.

  The abstract MediaRecorderPluginInterface class allows applications
  to obtain an encoder to record multimedia data to a file or other
  output device.

  \sa MediaRecorderEncoder

  \ingroup qtopiaemb
*/

/*!
  \fn MediaRecorderEncoder *MediaRecorderPluginInterface::encoder()

  Creates and returns a new plugin encoder instance.  It is the
  responsibility of the caller to delete the instance before unloading
  the plugin interface library.
*/
