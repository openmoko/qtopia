/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <qwsembedwidget.h>

#include <qabstractipcinterface.h>

#include "qmediavideocontrol.h"



// {{{ QMediaVideoControlPrivate
class QMediaVideoControlPrivate : public QAbstractIpcInterface
{
public:

    QMediaVideoControlPrivate(QUuid const& id):
        QAbstractIpcInterface("/MediaServer", QMediaVideoControl::name(), id.toString())
    {
    }

    bool hasVideo() const
    {
        return value("hasVideo").toBool();
    }

    WId getWId() const
    {
        return value("WId").toInt();        // XXX: knowledge
    }

signals:
    void videoAvailable();
};
// }}}

/*!
    \class QMediaVideoControl
    \brief The QMediaVideoControl class is an interface to videos playing through
    the Qtopia Media System.

    If a prepared media resource has associated video content, when
    that content is available to be played, this inteface can be used to contruct
    a QWidget which will contain the rendering of the video content.

    A QMediaVideoControl is available after the QMediaControl::controlAvailable signal
    has been emitted with the controls name.
    When the QMediaControl::controlUnavailable signal is emitted, there is no
    longer any valid video to be played. The QMediaControl related to the
    prepared media content is used to start, stop and perform other operations
    on the video.
*/

/*!
    Create a QMediaVideoControl with the handle to a prepared media resource

    The QMediaControl needs to be constructed with a valid QMediaHandle passed in by
    the \a mediaHandle parameter, obtained from a QMediaContent instance.

    The \a parent is the Parent QObject.
*/

QMediaVideoControl::QMediaVideoControl(QMediaHandle const& mediaHandle, QObject* parent):
    QObject(parent)
{
    d = new QMediaVideoControlPrivate(mediaHandle.id());
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
        rc = new QWSEmbedWidget(d->getWId(), parent);
    }

    return rc;
}

/*!
    Return the name of this control.
*/

QString QMediaVideoControl::name()
{
    return "VideoControl";
}


