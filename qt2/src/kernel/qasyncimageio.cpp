/****************************************************************************
** $Id: qt/src/kernel/qasyncimageio.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of asynchronous image/movie loading classes
**
** Created : 970617
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qasyncimageio.h"

#ifndef QT_NO_ASYNC_IMAGE_IO

#include "qpainter.h"
#include "qlist.h"
#include "qgifimageformat_p.h"
#include <stdlib.h>

extern void qt_init_image_handlers();


// NOT REVISED
/*!
  \class QImageConsumer qasyncimageio.h
  \brief An abstraction used by QImageDecoder.

  \ingroup images

  A QImageConsumer consumes information about changes to the QImage
  maintained by a QImageDecoder.  It represents the a view of the
  image which the decoder produces.

  \sa QImageDecoder
*/

/*!
  \fn void QImageConsumer::changed(const QRect&)

  Called when the given area of the image has changed.
*/

/*!
  \fn void QImageConsumer::end()

  Called when all data of all frames has been decoded and revealed
  as changed().
*/

/*!
  \fn void QImageConsumer::frameDone()

  One of the two frameDone() functions will be called
  when a frame of an animated image has ended and been revealed
  as changed().

  When this function is called, the current image should be displayed.

  The decoder will not make
  any further changes to the image until the next call to
  QImageFormat::decode().
*/

/*!
  \fn void QImageConsumer::frameDone( const QPoint& offset, const QRect& rect )

  One of the two frameDone() functions will be called
  when a frame of an animated image has ended and been revealed
  as changed().

  When this function is called, the area \a rect in the current image
  should be moved by \a offset and displayed.

  The decoder will not make
  any further changes to the image until the next call to
  QImageFormat::decode().
*/

/*!
  \fn void QImageConsumer::setLooping(int n)

  Called to indicate that the sequence of frames in the image
  should be repeated \a n times, including the sequence during
  decoding.

  <ul>
    <li> 0 = Forever
    <li> 1 = Only display frames the first time through
    <li> 2 = Repeat once after first pass through images
    <li> etc.
  </ul>

  To make the QImageDecoder
  do this just delete it and pass the information to it again
  for decoding (setLooping() will be called again of course, but
  that can be ignored), or keep copies of the
  changed areas at the ends of frames.
*/

/*!
  \fn void QImageConsumer::setFramePeriod(int milliseconds)

  Notes that the frame about to be decoded should not be displayed until
  the given number of \a milliseconds after the time that this function
  is called.  Of course, the image may not have been decoded by then, in
  which case the frame should not be displayed until it is complete.
  A value of -1 (the assumed default) indicates that the image should
  be displayed even while it is only partially loaded.
*/

/*!
  \fn void QImageConsumer::setSize(int, int)

  This function is called as soon as the size of the image has
  been determined.
*/


/*!
  \class QImageDecoder qasyncimageio.h
  \brief Incremental image decoder for all supported image formats.

  \ingroup images

  New formats are installed by creating objects of class
  QImageFormatType, and the QMovie class can be used for using
  all installed incremental image formats; QImageDecoder is only
  useful for creating new ways of feeding data to an QImageConsumer.
  
  \mustquote

  Qt supports GIF reading, if it is configured that way during
  installation (see qgif.h). If it is, we are required to state that
  "The Graphics Interchange Format(c) is the Copyright property of
  CompuServe Incorporated. GIF(sm) is a Service Mark property of
  CompuServe Incorporated."
*/

static const int max_header = 32;


struct QImageDecoderPrivate {
    QImageDecoderPrivate()
    {
	count = 0;
    }

    static void cleanup();

    static void ensureFactories()
    {
	if ( !factories ) {
	    factories = new QList<QImageFormatType>;
// See qgif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
	    gif_decoder_factory = new QGIFFormatType;
#endif
	    qAddPostRoutine( cleanup );
	}
    }

    static QList<QImageFormatType> * factories;

// See qgif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
    static QGIFFormatType * gif_decoder_factory;
#endif

    uchar header[max_header];
    int count;
};

QList<QImageFormatType> * QImageDecoderPrivate::factories = 0;
// See qgif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
QGIFFormatType * QImageDecoderPrivate::gif_decoder_factory = 0;
#endif


void QImageDecoderPrivate::cleanup()
{
    delete factories;
    factories = 0;
// See qgif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
    delete gif_decoder_factory;
    gif_decoder_factory = 0;
#endif
}


/*!
  Constructs a QImageDecoder which will send change information to
  a given QImageConsumer.
*/
QImageDecoder::QImageDecoder(QImageConsumer* c)
{
    qt_init_image_handlers();
    d = new QImageDecoderPrivate;
    CHECK_PTR(d);
    consumer = c;
    actual_decoder = 0;
}

/*!
  Destroys a QImageDecoder.  The image it built is destroyed.  The decoder
  built by the factory for the file format is destroyed. The consumer
  for which it decoded the image is \e not destroyed.
*/
QImageDecoder::~QImageDecoder()
{
    delete d;
    delete actual_decoder;
}

/*!
  \fn const QImage& QImageDecoder::image()

  Returns the image currently being decoded.
*/

/*!
  Call this function to decode some data into image changes.  The data
  will be decoded, sending change information to the QImageConsumer of
  this QImageDecoder, until one of the change functions of the consumer
  returns FALSE.

  Returns the number of bytes consumed, 0 if consumption is complete,
  and -1 if decoding fails due to invalid data.
*/
int QImageDecoder::decode(const uchar* buffer, int length)
{
    if (!actual_decoder) {
	int i=0;

	while (i < length && d->count < max_header)
	    d->header[d->count++] = buffer[i++];

	QImageDecoderPrivate::ensureFactories();

	for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	    f && !actual_decoder;
	    f = QImageDecoderPrivate::factories->next())
	{
	    actual_decoder = f->decoderFor(d->header, d->count);
	}

	if (!actual_decoder) {
	    if ( d->count < max_header ) {
		// not enough info yet
		return i;
	    } else {
		// failure - nothing matches max_header bytes
		return -1;
	    }
	}
    }
    return actual_decoder->decode(img, consumer, buffer, length);
}

/*!
  Returns a QImageFormatType by name. This might be used in cases where
  the user needs to force data to be interpreted as being in a certain
  format.  \a name is one of the formats listed by
  QImageDecoder::inputFormats(). Note that you will still need to supply
  decodable data to result->decoderFor() before you can begin decoding
  the data.
*/
QImageFormatType* QImageDecoder::format( const char* name )
{
    for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	f;
	f = QImageDecoderPrivate::factories->next())
    {
	if ( qstricmp(name,f->formatName())==0 )
	    return f;
    }
    return 0;
}

/*!
  Call this function to find the name of the format of the given header.
  The returned string is statically allocated.

  Returns 0 if the format is not recognized.
*/
const char* QImageDecoder::formatName(const uchar* buffer, int length)
{
    QImageDecoderPrivate::ensureFactories();

    const char* name = 0;
    for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	f && !name;
	f = QImageDecoderPrivate::factories->next())
    {
	QImageFormat *decoder = f->decoderFor(buffer, length);
	if (decoder) {
	    name = f->formatName();
	    delete decoder;
	}
    }
    return name;
}

/*!
  Returns a sorted list of formats for which asynchronous loading is supported.
*/
QStrList QImageDecoder::inputFormats()
{
    QImageDecoderPrivate::ensureFactories();

    QStrList result;

    for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	 f;
	 f = QImageDecoderPrivate::factories->next())
    {
	if ( !result.contains(  f->formatName() ) ) {
	    result.inSort(  f->formatName() );
	}
    }

    return result;
}

/*!
  Registers a new QImageFormatType.  This is not needed in
  application code as factories call this themselves.
*/
void QImageDecoder::registerDecoderFactory(QImageFormatType* f)
{
    QImageDecoderPrivate::ensureFactories();

    QImageDecoderPrivate::factories->insert(0,f);
}

/*!
  Unregisters a new QImageFormatType.  This is not needed in
  application code as factories call this themselves.
*/
void QImageDecoder::unregisterDecoderFactory(QImageFormatType* f)
{
    if ( !QImageDecoderPrivate::factories )
	return;

    QImageDecoderPrivate::factories->remove(f);
}

/*!
  \class QImageFormat qasyncimageio.h
  \brief Incremental image decoder for a specific image format.

  \ingroup images

  By making a derived classes of QImageFormatType which in turn
  creates objects that are a subclass of QImageFormat, you can add
  support for more incremental image formats, allowing such formats to
  be sources for a QMovie, or for the first frame of the image stream
  to be loaded as a QImage or QPixmap.

  Your new subclass must reimplement the decode() function in order to
  process your new format.

  New QImageFormat objects are generated by new QImageFormatType factories.
*/

/*!
  Destructs the object.

  \internal
  More importantly, destructs derived classes.
*/
QImageFormat::~QImageFormat()
{
}

/*!
  \fn int QImageFormat::decode(QImage& img, QImageConsumer* consumer,
	    const uchar* buffer, int length)

  New subclasses must reimplement this method.

  It should decode some or all of the bytes from \a buffer into
  \a img, calling the methods of \a consumer as the decoding proceeds to
  inform that consumer of changes to the image.
  The consumer may be 0, in which case the function should just process
  the data into \a img without telling any consumer about the changes.
  Note that the decoder must store enough state
  to be able to continue in subsequent calls to this method - this is
  the essence of the incremental image loading.

  The function should return without processing all the data if it
  reaches the end of a frame in the input.

  The function must return the number of bytes it has processed.
*/

/*!
  \class QImageFormatType qasyncimageio.h
  \brief Factory that makes QImageFormat objects.

  \ingroup images

  While the QImageIO class allows for \e complete loading of images,
  QImageFormatType allows for \e incremental loading of images.

  New image file formats are installed by creating objects of derived
  classes of QImageFormatType.  They must implement decoderFor()
  and formatName().

  QImageFormatType is a very simple class.  Its only task is to
  recognize image data in some format and make a new object, subclassed
  from QImageFormat, which can decode that format.

  The factories for formats built into Qt
  are automatically defined before any other factory is initialized.
  If two factories would recognize an image format, the factory created
  last will override the earlier one, thus you can override current
  and future built-in formats.
*/

/*!
  \fn virtual QImageFormat* QImageFormatType::decoderFor(const
	    uchar* buffer, int length)

  Returns a decoder for decoding an image which starts with the give bytes.
  This function should only return a decoder if it is definite that the
  decoder applies to data with the given header.  Returns 0 if there is
  insufficient data in the header to make a positive identification,
  or if the data is not recognized.
*/

/*!
  \fn virtual const char* QImageFormatType::formatName() const

  Returns the name of the format supported by decoders from this factory.
  The string is statically allocated.
*/

/*!
  Constructs a factory.  It automatically registers itself with QImageDecoder.
*/
QImageFormatType::QImageFormatType()
{
    QImageDecoder::registerDecoderFactory(this);
}

/*!
  Destroys a factory.  It automatically unregisters itself from QImageDecoder.
*/
QImageFormatType::~QImageFormatType()
{
    QImageDecoder::unregisterDecoderFactory(this);
}

#endif // QT_NO_ASYNC_IMAGE_IO
