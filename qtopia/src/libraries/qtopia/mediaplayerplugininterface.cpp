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
#include <qtopia/mediaplayerplugininterface.h>

/*!
    \class MediaPlayerDecoder <qtopia/mediaplayerplugininterface.h>

    \brief The MediaPlayerDecoder class provides an abstract base class for
    Qtopia MediaPlayer decoder plugins.

    Writing a decoder plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions and exporting the class with
    the \c Q_EXPORT_PLUGIN macro. See the \link pluginintro.html Plugins
    documentation\endlink for details.

    \ingroup qtopiaemb
*/

/*!
    \fn virtual MediaPlayerDecoder::~MediaPlayerDecoder() = 0

    This function is called when a decoder plugin object is deleted.
    It closes any open streams, and releases any memory which has been
    allocated for decoding.
*/

/*!
    \fn virtual const char *MediaPlayerDecoder::pluginName() = 0

    This function returns a string which is used to identify the plugin.
    The primary use of this string is for providing debug information.

    The output of this function is never used in any user interface, so
    it is safe to provide a non-internationalized string. (Future decoder
    APIs will provide a replacement/additional function which will be
    declared const and will expect an internationalized QString return
    value that will be able to be used in user interfaces if nessaccery).
*/

/*!
    \fn virtual const char *MediaPlayerDecoder::pluginComment() = 0

    This function returns a string which provides copyright or a small
    amount of text that gives a brief description of the plugin.
    This string may be helpful for developers using the plugin
    to find out how the plugin authors might be able to be contacted,
    or a pointer to a web page which provides more information about
    the plugin.

    The output of this function is not intented to be used in any
    user interface. Future decoder APIs may be extended to provide
    an additional function which should be internationalized and
    appropriate to be used in a user interface if nessaccery.
*/

/*!
    \fn virtual double MediaPlayerDecoder::pluginVersion() = 0

    This function returns a number which represents the version
    of the decoder plugin. For example, if it is version 1.6 of
    the plugin, then a value of (double)1.6 would be returned.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::isFileSupported( const QString& file ) = 0

    If the given \a file can be decoded successfully by this plugin, a return
    value of TRUE is returned, otherwise FALSE is returned.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::open( const QString& file ) = 0

    If the given \a file is opened successfully by this plugin, a return
    value of TRUE is returned, otherwise FALSE is returned. After this has been
    called, functions which return information about the stream can be used.
    For example, after a call to open() it is possible to query the channels,
    frequency, video width and height with calls to the appropriate functions.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::close() = 0

    This function closes all open streams. If there is a failure or no streams
    are open, this function returns FALSE;
*/

/*!
    \fn virtual bool MediaPlayerDecoder::isOpen() = 0

    If a stream has been successfully opened with a call to open() and has not yet
    been closed with a call to close(), this function returns TRUE, otherwise it
    returns FALSE;
*/

/*!
    \fn virtual const QString &MediaPlayerDecoder::fileInfo() = 0

    This function returns an internationalized QString which describes
    information fields which are comma seperated and paired with colons
    about the currently open stream.

    An example would be:
    \code
	"Format: MPEG1 Layer3, Year: 1998"
    \endcode

    The internationalization is achieved inside the plugin using the
    qApp->translate() function around the translatable strings.

    For the above example, the code to generate the string might be written like this:
    \code
	qApp->translate( "PluginName", "Format: MPEG1 Layer3" ) + ", " +
	    qApp->translate( "PluginName", "Year: " + "1998";
    \endcode

    Please refer to the Qt Internationalization documentation for more detailed
    information on creating code which makes good translations possible.

    The translation files are loaded by the Qtopia MediaPlayer for the decoder
    plugin by searching the language directories for files matching
    $QPEDIR/i18n/[LANG]/[PLUGIN].qm for the supported languages.
*/

/*!
    \fn virtual int MediaPlayerDecoder::audioStreams() = 0

    If the opened file contains audio streams, the number of audio streams
    are returned here, otherwise zero is returned. The Qtopia MediaPlayer currently
    only supports opening the first stream (stream 0) from a decoder if there is
    more than one stream.

    The return value is normally 1 or 0.
*/

/*!
    \fn virtual int MediaPlayerDecoder::audioChannels( int stream ) = 0

    Returns the number of audio channels for the given \a stream. A mono
    file will return 1, stereo returns 2, and so on.

    (The Qtopia MediaPlayer currently only supports decoding up to the first
    2 channels of audio, however it is quite possible for a \a stream to have
    up to 6 channels, and for those to be decoded when audioReadSamples is
    called with 6 channels requested.)
*/

/*!
    \fn virtual int MediaPlayerDecoder::audioFrequency( int stream ) = 0

    Returns the frequency of the audio \a stream. A typical value is 44100
    which represents that there are 44100 audio samples per second per channel
    (for example, if it is 16-bit stereo, then there will be a total of 44100*2*2
    bytes available per second).
*/

/*!
    \fn virtual int MediaPlayerDecoder::audioSamples( int stream ) = 0

    Returns the total number of audio samples in the given \a stream.

    As an example, if there is 10 seconds of music at 44100Hz with stereo
    samples, this function would return 441000 samples (each sample in a
    stereo pair is only counted once).

    If the media file is being streamed, it may not always be possible to
    determine the length of the streamed data (for example a radio webcast
    which continuously streams audio data), in which case the return value
    is -1 which indicates that the \a stream is not seekable. In such cases
    attempts to call audioSetSample() will most likely fail.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::audioSetSample( long sample, int stream ) = 0

    This function causes the decoder to seek to the given \a sample postion in the
    media file.
    
    If the open \a stream is not a file, but is instead for example a URL, then
    it may not be possible to seek to the given \a sample and FALSE will be returned, otherwise
    the next time either audioReadSamples() or videoReadFrame() is called, decoding will
    continue from the set position in the file.

    Note this function repositions both the audio and video if there are also video streams
    to the appropriate positions.

    \sa videoSetFrame()
*/

/*!
    \fn virtual long MediaPlayerDecoder::audioGetSample( int stream ) = 0

    This function retrieves the decoders sample postion in the given \a stream. The
    sample position can change due to audioReadSamples() being called or if supported,
    from audioSetSample() being called.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::audioReadSamples( short *samples, int channels, long sampleCount, long& samplesRead, int stream ) = 0

    This function reads \a sampleCount number of samples in to the \a samples buffer from the given \a stream.

    The \a samples buffer is filled with 16 bit wide values, therefore if the samples are only
    8 bit, they are expanded to shorts before being stored by the decoder. The \a channels value
    specifies how many channels to decoded.

    The samples for multiple channels are placed together, for example in the following order:

	sample1channel1, sample1channel2, sample2channel1, sample2channel2 etc.

    On success, TRUE is returned, otherwise FALSE is returned which indicates
    the end of the file has been reached. The value in \a samplesRead on return contains the number
    of samples actually read.

    In the case of streamed media, when no input is currently available, a return value of TRUE
    is returned, however \a samplesRead is set to 0 after a suitable timeout has elapsed and
    no samples have been able to be read.
*/

/*!
    \fn virtual int MediaPlayerDecoder::videoStreams() = 0

    If the opened file contains video streams, the number of video streams
    is returned, otherwise zero is returned.

    (The Qtopia MediaPlayer currently only supports opening the first
    stream (stream 0) from a decoder if there is more than one stream.)

    The return value is normally 1 or 0.
*/

/*!
    \fn virtual int MediaPlayerDecoder::videoWidth( int stream ) = 0

    Returns the width of video frames for the given \a stream.
*/

/*!
    \fn virtual int MediaPlayerDecoder::videoHeight( int stream ) = 0

    Returns the height of video frames for the given \a stream.
*/

/*!
    \fn virtual double MediaPlayerDecoder::videoFrameRate( int stream ) = 0

    Returns the frames per second of the given video \a stream.

    (The Qtopia MediaPlayer makes its best effort to play the video frames at the specified rate.)
*/

/*!
    \fn virtual int MediaPlayerDecoder::videoFrames( int stream ) = 0

    Returns the total number of video frames in the given \a stream.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::videoSetFrame( long frame, int stream ) = 0

    This function causes the decoder to seek to the given \a frame postion in the
    given media file's \a stream. If the open stream is not a file, but is instead for example a URL,
    then it may not be possible to seek to the given frame and FALSE will be returned, otherwise
    the next time either audioReadSamples() or videoReadFrame() is called, decoding will
    continue from the set position in the file.

    Note this function repositions both the audio and video if there are also audio streams
    to the appropriate positions.

    \sa audioSetSample()
*/

/*!
    \fn virtual long MediaPlayerDecoder::videoGetFrame( int stream ) = 0

    This function retrieves the decoders frame postion in the given \a stream.

    The frame position can change due to videoReadScaledFrame() being called or if supported,
    from videoSetFrame() being called.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int ) = 0

    This function is deprecated.

    Please use videoReadScaledFrame() instead.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::videoReadYUVFrame( char *, char *, char *, int, int, int, int, int ) = 0

    This function is deprecated.

    Please use videoReadScaledFrame() instead.
    (Future decoder interfaces may provide a similar function to this in the future).
*/

/*!
    \fn virtual bool MediaPlayerDecoder::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream ) = 0

    This function decodes the current video frame from the given \a stream in to the
    buffer at \a output_rows.

    The \a in_x and \a in_y parameters specify where to decode from and the \a in_w and \a in_h
    parameters specify the width and height of the input stream to decode.

    The \a out_w and \a out_h parameters specify the width and height to scale the input to.

    The \a color_model specifies the format of the bits for color in the output_rows.
    A typical value for the color_model is RGB565.

    This function can be used (as is done in the Qtopia MediaPlayer)
    to decode video directly to a frame buffer device using the QDirectPainter class or
    directly to a QImage. In the case of decoding to a QImage, the
    QImage::jumpTable() of the QImage is provided as the first parameter
    to this function. In the QDirectPainter case, a jump table for the frame
    buffer's scanlines needs to be constructed, but this is only required to be done
    once, and could be reused by the caller for every subsequent call to this function.
*/

/*!
    \fn virtual double MediaPlayerDecoder::getTime() = 0

    This function is for profiling decoders. It returns the last decoded timestamp from
    the media stream.

    Please refer to getPlayTime() for a discussion on how best to query the elapsed time.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::setSMP( int CPUs ) = 0

    This function is used as a hint to the decoder for how many decoding threads
    it should consider using based on the provided number of \a CPUs.

    (Currently the Qtopia MediaPlayer doesn't use this function.)
*/

/*!
    \fn virtual bool MediaPlayerDecoder::setMMX( bool useMMX ) = 0

    This function is used as a hint to the decoder to use MMX specific
    optimizations if being used on an x86 processor and \a useMMX is true. On other binary
    incompatible processors, this function is interpreted to mean use the MMX equivelant
    optimizations for that processor family. For example on MIPS, this function
    translates to using MMI optimizations.

    (Currently the Qtopia MediaPlayer doesn't use this function.)
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsAudio() = 0

    This function returns the audio capability of the decoder. If audio can
    be decoded, this function returns TRUE, otherwise it returns FALSE.
    If audio is not supported, all virtual audio functions are still provided,
    however the return values for those functions will always be FALSE or 0 as
    applicable.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsVideo() = 0

    This function returns the video capability of the decoder. If video can
    be decoded, this function returns TRUE, otherwise it returns FALSE.
    If video is not supported, all virtual video functions are still provided,
    however the return values for those functions will always be FALSE or 0 as
    applicable.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsYUV() = 0

    This function is deprecated.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsSMP() = 0

    This function is deprecated.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsMMX() = 0

    This function is deprecated.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsStereo() = 0

    This function is deprecated.
*/

/*!
    \fn virtual bool MediaPlayerDecoder::supportsScaling() = 0

    This function is deprecated.
*/

/*!
    \fn virtual long MediaPlayerDecoder::getPlayTime() = 0

    This function is deprecated.

    The most accurate way to get the elapsed play time for files which contain
    audio is to use the audioGetSample() function and derive the elapsed time
    from the sample rate, or for files without audio is to use videoGetFrame() and
    derive the elapsed time from the frame rate.
*/

/*!
    \class MediaPlayerDecoder_1_6 <qtopia/mediaplayerplugininterface.h>

    \brief The MediaPlayerDecoder_1_6 class provides an extension to the
    MediaPlayerDecoder class for Qtopia MediaPlayer decoder plugins.

    \ingroup qtopiaemb
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::supportsStreaming() = 0

    Returns TRUE if the decoder supports streaming.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::canStreamURL( const QUrl& url, const QString& mimetype ) = 0

    Returns TRUE if the decoder can support the given \a mimetype with the \a url provided.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::openURL( const QUrl& url, const QString& mimetype ) = 0

    Opens the \a url of given \a mimetype. Typically this information is provided by a web
    server to a web browser which could pass this on directly to the plugin or indirectly to the
    Qtopia Media Player through a QCop signal.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::streamed() = 0

    Returns TRUE if the decoder is playing a streamed file opened using openURL(), or FALSE if the file was
    opened using the MediaPlayerDecoder::open() function used for files.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::syncAvailable() = 0

    Returns TRUE if the decoder supports for the given stream syncing the audio and
    video together with the sync() function.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::sync() = 0

    Sync up the video to match the timestamp of where the audio currently is playing.
    If the video is behind, this will skip forward to the correctly matching frame.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::seekAvailable() = 0

    Returns TRUE if the stream is seekable or not. For streamed media, it is possible
    that seeking may not be able to be implemented in the plugin therefore
    the plugin returns FALSE, otherwise if it can, it returns TRUE.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::seek( long pos ) = 0

    Seeks to \a pos byte offset from the beginning of the stream.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::tellAvailable() = 0

    Returns TRUE if the decoder can support the tell() function for the given stream.
*/

/*!
    \fn virtual long MediaPlayerDecoder_1_6::tell() = 0

    Returns the current byte offset from the beginning of the stream.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::lengthAvailable() = 0

    Returns TRUE if the decoder can support the length() function for the given stream.
*/

/*!
    \fn virtual long MediaPlayerDecoder_1_6::length() = 0

    Returns the length in bytes of the stream. For streamed data, the protocol may
    not always return how long the stream will be, therefore it should be assumed
    that this may not always be available with all decoder plugins.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::totalTimeAvailable() = 0

    Returns TRUE if the decoder can support the totalTime() function for the given stream.
*/

/*!
    \fn virtual long MediaPlayerDecoder_1_6::totalTime() = 0

    Returns the total time in milliseconds of the stream. For some file formats,
    the total play time may not be easily obtainable. Some formats use VBR (variable
    bit rate) encoding without timestamps making it uncertain as to exactly how long
    it takes to play the file without actually decoding or indexing it.
*/

/*!
    \fn virtual bool MediaPlayerDecoder_1_6::currentTimeAvailable() = 0

    Returns TRUE if the decoder can support the currentTime() function for the given stream.
*/

/*!
    \fn virtual long MediaPlayerDecoder_1_6::currentTime() = 0

    Returns the current millisecond being played in the stream. The accuracy should only
    be considered good enough for the use in the display of the current play time and
    not used for the precise timing of syncing playback. Not all formats contain timestamps,
    therefore after seeking, not all formats and decoders will be able to support this function.
*/



/*!
    \class MediaPlayerPluginInterface <qtopia/mediaplayerplugininterface.h>

    \brief The MediaPlayerPluginInterface class is used to access MediaPlayerDecoder objects.
*/

/*!
    \fn virtual MediaPlayerDecoder *MediaPlayerPluginInterface::decoder() = 0

    This function returns a MediaPlayerDecoder object (or a MediaPlayerDecoder_1_6
    extended MediaPlayerDecoder object if this interface returns that it supports
    the 1_6 API extensions).
*/

/*!
    \fn virtual MediaPlayerEncoder *MediaPlayerPluginInterface::encoder() = 0

    This function returns a MediaPlayerEncoder object, however 
    the MediaPlayerEncoder interface is yet to be defined. Please refer to the
    documentation on the MediaRecoderEncoder documentation which provides an
    audio encoder API.
*/


