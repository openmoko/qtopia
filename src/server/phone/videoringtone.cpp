/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "videoringtone.h"

#ifdef MEDIA_SERVER
#include <qmediacontent.h>
#include <qmediacontrol.h>
#include <qmediavideocontrol.h>
#endif




class VideoRingtonePrivate
{
public:
#ifdef MEDIA_SERVER
    QMediaContent*      videoContent;
    QMediaVideoControl* videoControl;
#endif
};

/*!
  \class VideoRingtone
  \brief The VideoRingtone class provides an interface
  to the Qtopia media system to play the video tones for incoming calls.

  The VideoRingtone acts as a bridge between RingControl
  and other part of the system that is interested in displaying the video widget,
  for example, CallScreen.

  RingControl calls playVideo() when the video ring tone is preferred.
  CallScreen should be listening to the videoWidgetReady() signal
  to retreive the video widget object by calling videoWidget().

  RingControl is responsible for responding to the videoRingtoneFailed() signal.

  The signal videoRingtoneStopped() is emitted when the tone finished playing.

  \code
        CallScreen::CallScreen() {
            ...
            connect( VideoRingtone::instance(), SIGNAL(videoWidgetReady()),
                this, SLOT(showVideo()) );
            ...
        }

        CallScreen::showVideo() {
            QWidget *widget = videoTone->videoWidget();

            // set the new parent to manage resource
            widget->setParent( this );

            layout()->addWidget( widget );
        }
  \endcode

  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
*/

/*!
  Destroys the VideoRingtone object.
*/
VideoRingtone::~VideoRingtone()
{
    delete d;
}

/*!
  Attempts to play the video \a fileName.
*/
void VideoRingtone::playVideo(const QString& fileName)
{
#ifdef MEDIA_SERVER
    d->videoContent = new QMediaContent( QContent( fileName ) );
    connect(d->videoContent, SIGNAL(mediaError(QString)), this, SIGNAL(videoRingtoneFailed()));

    QMediaControl* mediaControl = new QMediaControl(d->videoContent);
    connect(mediaControl, SIGNAL(valid()), mediaControl, SLOT(start()));

    d->videoControl = new QMediaVideoControl(d->videoContent);
    connect(d->videoControl, SIGNAL(valid()), this, SIGNAL(videoWidgetReady()));
#endif
}


/*!
  Stops the media content.
*/
void VideoRingtone::stopVideo()
{
#ifdef MEDIA_SERVER
    delete d->videoContent;

    d->videoControl = 0;

    emit videoRingtoneStopped();
#endif
}

/*!
  Returns the video widget instance.
*/
QWidget* VideoRingtone::videoWidget()
{
#ifdef MEDIA_SERVER
    if (d->videoControl != 0)
        return d->videoControl->createVideoWidget();

#endif
    return NULL;
}

/*!
  Returns the VideoRingtone instance.
*/
VideoRingtone* VideoRingtone::instance()
{
    static VideoRingtone    videoRing;

    return &videoRing;
}

/*!
  \internal

  Creates a new VideoRingtone instance with the specified \a parent.
 */
VideoRingtone::VideoRingtone()
{
    d = new VideoRingtonePrivate;

#ifdef MEDIA_SERVER
    d->videoControl = 0;
#endif
}

/*!
  \fn void VideoRingtone::videoRingtoneFailed()

  This signal is emitted when the media system fails to play the video tone.
*/

/*!
  \fn void VideoRingtone::videoWidgetReady()

  This signal is emitted when the video widget is created and ready to be used.
*/

/*!
  \fn void VideoRingtone::videoRingtoneStopped()

  This signal is emitted when the video tone is stopped.
*/

