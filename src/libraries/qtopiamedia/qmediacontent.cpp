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

#include <qstringlist.h>

#include "private/mediaserverproxy.h"
#include "qmediacontrol.h"
#include "qmediacontent.h"

using namespace mlp;


// {{{ QMediaContentPrivate
class QMediaContentPrivate : public MediaServerCallback
{
public:
    QUuid               id;
    QMediaContent*      mediaContent;
    QString             domain;
    QMediaControl*      control;

    void mediaReady(int key, QUuid const& mediaId)
    {
        Q_UNUSED(key);

        id = mediaId;

        control = new QMediaControl(QMediaHandle(id), mediaContent);
        QObject::connect( control, SIGNAL(controlAvailable(const QString&)),
            mediaContent, SIGNAL(controlAvailable(const QString&)) );

        foreach (QString const& controlName, control->controls())
        {
            mediaContent->controlAvailable(controlName);
        }
    }

    void mediaError(int key, QString const& errorString)
    {
        Q_UNUSED(key);

        mediaContent->mediaError(errorString);
    }
};
// }}}


/*!
    \class QMediaContent
    \brief The QMediaContent class is used to prepare a media resource
    for playing in Qtopia.

    A QMediaContent is constructed with a URL, that may refer to a local or
    remote resource, or a QContent that refers to a local media resource.

    If the QMediaContent is constructed with a valid piece of media a signal
    will be emitted with for each of the controls available to manipulate that
    media.

    During the life-time of a piece of media, the controls available may
    change, due to different formats being available in the content, each time
    a control is usable or unusable a controlAvailable or controlUnavailable
    signal will be emitted.

    If there is an error preparing the media resource, the mediaError signal will
    be emitted, with a QString detailing the error information.

    Example:
    {
        QContent        beep = findBeepSound();

        m_mediaContent = new QMediaContent(mediaContent(beep));

        connect(mediaContent, SIGNAL(controlAvailable(QString)),
                this, SLOT(mediaControlAvailable(QString)));

    }

    void mediaControlAvailable(QString const& id)
    {
        if (id == QMediaControl::name())
        {
            // The basic media control is available, all media content
            // will have this control available.
            m_mediaControl = new QMediaControl(m_mediaContent.handle());

            // Now do something play/get length
            m_mediaControl->start();
        }
    }
*/

/*!
    Create a QMediaContent from a URL.

    The URL may point to a local or remote media resource.

    The \a url paramter is a URL representing the location of the media content.
    The \a domain is the audio domain in which this media content should exist.
    The \a parent is the Parent QObject.
*/

QMediaContent::QMediaContent
(
 QString const& url,
 QString const& domain,
 QObject* parent
):
    QObject(parent),
    d(new QMediaContentPrivate)
{
    d->mediaContent = this;
    d->domain = domain;
    d->control = 0;

    MediaServerProxy::instance()->prepareContent(d, d->domain, url);
}

/*!
    Create a QMediaContent from a QContent.

    This creates a QMediaContent from a local resource known by Qtopia's
    Document System.

    The \a content is the QContent representing the location of the media content.
    The \a domain is the audio domain in which this media content should exist.
    The \a parent is the Parent QObject.
*/

QMediaContent::QMediaContent
(
 QContent const& content,
 QString const& domain,
 QObject* parent
):
    QObject(parent),
    d(new QMediaContentPrivate)
{
    d->mediaContent = this;
    d->domain = domain;
    d->control = 0;

    QString url = content.file();

    if (content.drmState() == QContent::Protected)
        url.prepend("qtopia://");
    else
        url.prepend("file://");

    MediaServerProxy::instance()->prepareContent(d, d->domain, url);
}

/*!
    Destroy the QMediaContent object.
*/

QMediaContent::~QMediaContent()
{
    if (!d->id.isNull())
        MediaServerProxy::instance()->destroySession(d->id);

    delete d;
}


/*!
    \fn QMediaContent::handle() const;
    Return a handle that can be used to construct a control
    to interact with the media resource.
*/

QMediaHandle QMediaContent::handle() const
{
    return QMediaHandle(d->id);
}

/*!
    \fn QMediaContent::controls() const;
    Return a list of Controls that this Media supports.
*/

QStringList QMediaContent::controls() const
{
    if (d->control)
        return d->control->controls();

    return QStringList();
}

/*!
    \fn QMediaContent::controlAvailable(const QString& id);

    Signals that a control with identity \a id is now available for use.
*/

/*!
    \fn QMediaContent::controlUnavailable(const QString& id);

    Signals that a control with identity \a id is no longer usable.
*/

/*!
    \fn QMediaContent::mediaError(const QString& mediaError);

    Signals that there was an error trying to access a piece of media content.
    Details of the error are contained in the \a mediaError parameter.
*/

