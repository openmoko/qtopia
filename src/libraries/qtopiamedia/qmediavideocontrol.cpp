/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QObject>

#if defined(Q_WS_QWS)
#include <QWSEmbedWidget>
#elif defined(Q_WS_X11)
#include <QX11EmbedWidget>
#endif

#include "qmediaabstractcontrol.h"
#include "qmediacontent.h"

#include "qmediavideocontrol.h"



// {{{ QMediaVideoControlPrivate
class QMediaVideoControlPrivate : public QMediaAbstractControl
{
    Q_OBJECT

public:
    QMediaVideoControlPrivate(QMediaContent* mediaContent):
        QMediaAbstractControl(mediaContent, QMediaVideoControl::name())
    {
        proxyAll();
    }

    bool hasVideo() const
    {
        return value("hasVideo").toBool();
    }

    WId getWindowId() const
    {
        return value("windowId").toInt();        // XXX: knowledge
    }

signals:
    void videoTargetAvailable();
    void videoTargetRemoved();
};
// }}}


/*!
    \class QMediaVideoControl
    \mainclass
    \brief The QMediaVideoControl class is an interface to videos playing through
    the Qtopia Media System.

    If a prepared media resource has associated video content, when
    that content is available to be played, this inteface can be used to contruct
    a QWidget which will contain the rendering of the video content.

    Use the \l QMediaControlNotifier class to listen for when video content is
    available.

    \code
        QMediaControlNotifier *notifier = new QMediaControlNotifier( QMediaVideoControl::name() );
        connect( notifier, SIGNAL(valid()), this, SLOT(showVideo()) );

        QMediaContent *content = new QMediaContent( "video.3gp" );
        notifier->setMediaContent( content );
    \endcode

    If video content is available createVideoWidget() can be used to construct a
    new video widget for the media content.

    \code
        QMediaVideoControl control( content );

        QWidget *video = control.createVideoWidget();
        videolayout->addWidget( video );
    \endcode

    Simply delete video widgets when they are no longer needed or when the video
    control becomes invalid.

    \ingroup multimedia

    \sa QMediaContent, QMediaControlNotifier
*/

/*!
    Create a QMediaVideoControl with the handle to a prepared media resource

    The QMediaControl needs to be constructed with \a mediaContent that
    represents the video to be played.
*/

QMediaVideoControl::QMediaVideoControl(QMediaContent* mediaContent):
    QObject(mediaContent)
{
    d = new QMediaVideoControlPrivate(mediaContent);

    connect(d, SIGNAL(valid()), this, SIGNAL(valid()));
    connect(d, SIGNAL(invalid()), this, SIGNAL(invalid()));

    connect(d, SIGNAL(videoTargetAvailable()), this, SIGNAL(videoTargetAvailable()));
    connect(d, SIGNAL(videoTargetRemoved()), this, SIGNAL(videoTargetRemoved()));
}

/*!
    Destroy a QMediaVideoControl
*/

QMediaVideoControl::~QMediaVideoControl()
{
    delete d;
}


/*!
    Create a QWidget that displays the rendered video content.

    This function is used to create a QWidget that will display the video
    content, it is parented to the widget passed in with the \a parent
    parameter. This widget can be deleted when the application is finished with it.
*/

QWidget* QMediaVideoControl::createVideoWidget(QWidget* parent) const
{
    QWidget*    rc = 0;

    if (d->hasVideo())
    {
#if defined(Q_WS_QWS)
        rc = new QWSEmbedWidget(d->getWindowId(), parent);
#elif defined(Q_WS_X11)
        QX11EmbedWidget *embed = new QX11EmbedWidget(parent);
        embed->embedInto(d->getWindowId());
        rc = embed;
#endif
    }

    return rc;
}

/*!
    Return the name of this control.
*/

QString QMediaVideoControl::name()
{
    return "Video";
}

/*!
    \fn void QMediaVideoControl::valid();

    Signal that is emitted when the control is valid and available for use.
*/

/*!
    \fn void QMediaVideoControl::invalid();

    Signal that is emitted when the control is invalid and no longer available for use.
*/

/*!
    \fn void QMediaVideoControl::videoTargetAvailable();

    This signal is emitted when there is a video available to be viewed. The client can
    call createVideoWidget() at this time.

    \sa createVideoWidget()
*/

/*!
    \fn void QMediaVideoControl::videoTargetRemoved();

    This signal is emitted when there is no longer video available to be viewed. The client should
    delete any QWidget obtained from createVideoWidget()

    \sa createVideoWidget()
*/

#include "qmediavideocontrol.moc"

