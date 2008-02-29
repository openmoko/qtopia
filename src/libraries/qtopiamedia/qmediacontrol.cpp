/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <quuid.h>
#include <qtopialog.h>
#include <qabstractipcinterface.h>
#include <qtopiaservices.h>

#include "qmediacontrol.h"


// {{{ QMediaControlPrivate
class QMediaControlPrivate : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    QMediaControlPrivate(QUuid const& _id):
        QAbstractIpcInterface("/MediaServer", QMediaControl::name(), _id.toString()),
        id(_id)
    {
    }

    ~QMediaControlPrivate()
    {
    }

    QtopiaMedia::State playerState() const
    {
        return static_cast<QtopiaMedia::State>(value(QLatin1String("playerState")).toUInt());
    }

    quint32 length() const
    {
        return static_cast<quint32>(value(QLatin1String("length")).toUInt());
    }

    quint32 position() const
    {
        return static_cast<quint32>(value(QLatin1String("position")).toUInt());
    }

    bool isMuted()
    {
        return value(QLatin1String("muted")).toBool();
    }

    int volume() const
    {
        return value(QLatin1String("volume")).toInt();
    }

    QString errorString() const
    {
        return value(QLatin1String("errorString")).toString();
    }

    QStringList interfaces() const
    {
        return value(QLatin1String("interfaces")).toStringList();
    }

public slots:
    void start()
    {
        invoke(SLOT(start()));
    }

    void pause()
    {
        invoke(SLOT(pause()));
    }

    void stop()
    {
        invoke(SLOT(stop()));
    }

    void seek(quint32 ms)
    {
         invoke(SLOT(seek(quint32)), ms);
    }

    void setVolume(int volume)
    {
        invoke(SLOT(setVolume(int)), volume);
    }

    void setMuted(bool mute)
    {
        invoke(SLOT(setMuted(bool)), mute);
    }

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);
    void controlAvailable(const QString& control);
    void controlUnavailable(const QString& control);

public:
    QUuid id;
};
// }}}


/*!
    \class QMediaControl
    \brief The QMediaControl class is used to manipulate a media resource in the
    Qtopia media system.

    \ingroup multimedia
*/


// {{{ QMediaControl

/*!
    Construct a QMediaControl.

    The QMediaControl needs to be constructed with a valid QMediaHandle
    obtained from a QMediaContent instance. The QMediaControl can be
    constructed only after the QMediaContent instance has signaled that it is
    available via the QMediaContenet::controlAvailable signal.

    The \a mediaHandle parameter is the handle of the media content for which
    the control is being constructed
    The \a parent is the Parent QObject.
*/

QMediaControl::QMediaControl(QMediaHandle const& mediaHandle, QObject* parent):
    QObject(parent)
{
    d = new QMediaControlPrivate(mediaHandle.id());

    connect(d, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SIGNAL(playerStateChanged(QtopiaMedia::State)));

    connect(d, SIGNAL(positionChanged(quint32)),
            this, SIGNAL(positionChanged(quint32)));

    connect(d, SIGNAL(lengthChanged(quint32)),
            this, SIGNAL(lengthChanged(quint32)));

    connect(d, SIGNAL(volumeChanged(int)),
            this, SIGNAL(volumeChanged(int)));

    connect(d, SIGNAL(volumeMuted(bool)),
            this, SIGNAL(volumeMuted(bool)));

    connect(d, SIGNAL(controlAvailable(const QString&)),
            this, SIGNAL(controlAvailable(const QString&)));

    connect(d, SIGNAL(controlUnavailable(const QString&)),
            this, SIGNAL(controlUnavailable(const QString&)));
}

/*!
    Destroy the QMediaControl object.
*/

QMediaControl::~QMediaControl()
{
    delete d;
}

/*!
    Return the current state of the media content
*/

QtopiaMedia::State QMediaControl::playerState() const
{
    return d->playerState();
}

/*!
    Return the length of the media content.
*/

quint32 QMediaControl::length() const
{
    return d->length();
}

/*!
    Return the current position in the media content.
*/

quint32 QMediaControl::position() const
{
    return d->position();
}

/*!
    Mute or unmute the volume for the media content.

    The \a mute parameter enables or disable muting of the media.
*/

void QMediaControl::setMuted(bool mute)
{
    d->setMuted(mute);
}

/*!
    Return the mute state of the media content.
*/

bool QMediaControl::isMuted() const
{
    return d->isMuted();
}

/*!
    Set the volume of the current media content. The acceptable
    volume range is 1-100.

    The \a volume is the value of the volume to be set for this media.

*/

void QMediaControl::setVolume(int volume)
{
    return d->setVolume(volume);
}

/*!
    Return the current volume for the media content.
*/

int QMediaControl::volume() const
{
    return d->volume();
}

/*!
    When an error occurs, indicated by the QtopiaMedia::Errpr state,
    this function will return a QString with appropriate information
    regarding the error.
*/

QString QMediaControl::errorString() const
{
    return d->errorString();
}

/*!
    Return a list of Controls that this Media supports.
*/

QStringList QMediaControl::controls() const
{
    return d->interfaces();
}

/*!
    Return the handle of the media content that this control is manipulating.
*/

QMediaHandle QMediaControl::handle() const
{
    return QMediaHandle(d->id);
}

/*!
    Returns the name of this control.
*/

QString QMediaControl::name()
{
    return "MediaControl";
}


/*!
    Start playing the media content.
*/

void QMediaControl::start()
{
    d->start();
}


/*!
    Pause the media content.
*/

void QMediaControl::pause()
{
    d->pause();
}

/*!
    Stop playing the media content.
*/

void QMediaControl::stop()
{
    d->stop();
}

/*!
    Seek to the location in the media content. The location is specified with
    the \a ms parameter which is the number of milliseconds from the beginning
    of the media.
*/

void QMediaControl::seek(quint32 ms)
{
    d->seek(ms);
}

/*!
    \fn QMediaControl::playerStateChanged(QtopiaMedia::State state);

    Signals a change in the state of the media content. The new state
    is represented by the \a state parameter.
*/

/*!
    \fn QMediaControl::positionChanged(quint32 ms);

    Signals an update to the current position of a media content.  This signal
    is emitted while the media is playing and represents the current
    milliseconds \a ms from the beginning of the media.
*/

/*!
    \fn QMediaControl::lengthChanged(quint32 ms);

    Signals a change in the length of the media. For some media content the
    length may not be available until after playback has started, it is
    advisable to hook onto this signal to ensure clients have the correct
    length information, The length information is available as the total number
    of milliseconds from the parameter \a ms.
*/

/*!
    \fn QMediaControl::volumeChanged(int volume);

    Signals a change in volume of the media content. The new volume is has the
    value \a volume, ranging from 1 - 100.
*/

/*!
    \fn QMediaControl::volumeMuted(bool muted);

    Signals if the media content has been muted or not. The \a muted paramter
    is true when muted, false when not.
*/

/*!
    \fn QMediaControl::controlAvailable(const QString& control);

    Signals that a control with identity \a control is available in the system.
*/

/*!
    \fn QMediaControl::controlUnavailable(const QString& control);

    Signals that a control with identity \a control is no longer available in the system.
*/

// }}}


#include "qmediacontrol.moc"

