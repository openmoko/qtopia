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

#include <gst/gst.h>

#include <QMediaVideoControlServer>

#include <qmediahandle_p.h>

#include "gstreamerbushelper.h"
#include "gstreamermessage.h"
#include "gstreamerdirectpainterwidget.h"

#include "gstreamerplaybinsession.h"


namespace gstreamer
{

// {{{ PlaybinSessionPrivate
class PlaybinSessionPrivate
{
public:
    bool                    haveStreamInfo;
    bool                    muted;
    gdouble                 volume;
    quint32                 position;
    quint32                 length;
    QtopiaMedia::State      state;
    QMediaHandle            id;
    QString                 domain;
    QUrl                    url;
    BusHelper*              busHelper;
    Engine*                 engine;
    GstElement*             playbin;
    GstBus*                 bus;
    DirectPainterWidget*    sinkWidget;
    QStringList             interfaces;

    QMediaVideoControlServer*   videoControlServer;
};
// }}}

// {{{ PlaybinSession

/*!
    \class gstreamer::PlaybinSession
    \internal
*/

PlaybinSession::PlaybinSession
(
 Engine*        engine,
 QUuid const&   id,
 QUrl const&    url
):
    d(new PlaybinSessionPrivate)
{
    d->haveStreamInfo = false;
    d->muted = false;
    d->volume = 5.0;
    d->position = 0;
    d->length  = -1;
    d->state = QtopiaMedia::Stopped;
    d->engine = engine;
    d->id = QMediaHandle(id);
    d->url = url;
    d->playbin = 0;
    d->bus = 0;
    d->sinkWidget = 0;
    d->videoControlServer = 0;

    d->interfaces << "Basic";

    readySession();
}

PlaybinSession::~PlaybinSession()
{
    if (d->playbin != 0)
    {
        stop();
        gst_object_unref(GST_OBJECT(d->playbin));
    }

    delete d->sinkWidget;
    delete d->videoControlServer;

    delete d;
}

bool PlaybinSession::isValid() const
{
    return true;
}

void PlaybinSession::start()
{
    if (d->playbin != 0)
        gst_element_set_state(d->playbin, GST_STATE_PLAYING);
}

void PlaybinSession::pause()
{
    if (d->playbin != 0)
        gst_element_set_state(d->playbin, GST_STATE_PAUSED);
}

void PlaybinSession::stop()
{
    if (d->playbin != 0)
        gst_element_set_state(d->playbin, GST_STATE_NULL);
}

void PlaybinSession::suspend()
{
}

void PlaybinSession::resume()
{
}

void PlaybinSession::seek(quint32 ms)
{
    if (d->playbin != 0)
    {
        gint64  position = (gint64)ms * 1000000;

        gst_element_seek_simple(d->playbin, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, position);
    }
}

quint32 PlaybinSession::length()
{
    return d->length;
}

void PlaybinSession::setVolume(int volume)
{
    if (d->playbin != 0)
    {
        d->volume = gdouble(volume) / 10;

        if (!d->muted)
        {
            g_object_set(G_OBJECT(d->playbin), "volume", d->volume, NULL);

            emit volumeChanged(volume);
        }
    }
}

int PlaybinSession::volume() const
{
    return int(d->volume * 10);
}

void PlaybinSession::setMuted(bool mute)
{
    if (d->playbin != 0)
    {
        d->muted = mute;

        g_object_set(G_OBJECT(d->playbin), "volume", mute ? 0 : d->volume, NULL);

        emit volumeMuted(mute);
    }
}

bool PlaybinSession::isMuted() const
{
    return d->muted;
}

QtopiaMedia::State PlaybinSession::playerState() const
{
    return d->state;
}

QString PlaybinSession::errorString()
{
    return QString();
}

void PlaybinSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString PlaybinSession::domain() const
{
    return d->domain;
}

QStringList PlaybinSession::interfaces()
{
    return d->interfaces;
}

QString PlaybinSession::id() const
{
    return d->id.toString();
}

QString PlaybinSession::reportData() const
{
    return d->url.toString();
}

void PlaybinSession::busMessage(Message const& msg)
{
    GstMessage* gm = msg.rawMessage();

    if (gm == 0)
    {   // Null message, query current position
        GstFormat   format = GST_FORMAT_TIME;
        gint64      position = 0;

        if (gst_element_query_position(d->playbin, &format, &position) == TRUE)
        {
            quint32 pos = position / 1000000;

            if (pos != d->position)
            {
                d->position = pos;
                emit positionChanged(d->position);
            }
        }
    }
    else
    {
        switch (GST_MESSAGE_TYPE(gm))
        {
        case GST_MESSAGE_DURATION:
            break;

        case GST_MESSAGE_STATE_CHANGED:
            {
                GstState    oldState;
                GstState    newState;
                GstState    pending;

                gst_message_parse_state_changed(gm, &oldState, &newState, &pending);

                switch (newState)
                {
                    case GST_STATE_VOID_PENDING:
                    case GST_STATE_NULL:
                    case GST_STATE_READY:
                        break;

                    case GST_STATE_PAUSED:
                        if (oldState == GST_STATE_READY)
                            getStreamsInfo();
                        break;

                    case GST_STATE_PLAYING:
                        if (d->state != QtopiaMedia::Playing)
                            emit playerStateChanged(d->state = QtopiaMedia::Playing);
                        break;
                }
            }
            break;

        case GST_MESSAGE_EOS:
            if (d->state != QtopiaMedia::Stopped)
                emit playerStateChanged(d->state = QtopiaMedia::Stopped);
            break;

        case GST_MESSAGE_STREAM_STATUS:
        case GST_MESSAGE_UNKNOWN:
        case GST_MESSAGE_ERROR:
        case GST_MESSAGE_WARNING:
        case GST_MESSAGE_INFO:
        case GST_MESSAGE_TAG:
        case GST_MESSAGE_BUFFERING:
        case GST_MESSAGE_STATE_DIRTY:
        case GST_MESSAGE_STEP_DONE:
        case GST_MESSAGE_CLOCK_PROVIDE:
        case GST_MESSAGE_CLOCK_LOST:
        case GST_MESSAGE_NEW_CLOCK:
        case GST_MESSAGE_STRUCTURE_CHANGE:
        case GST_MESSAGE_APPLICATION:
        case GST_MESSAGE_ELEMENT:
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
        case GST_MESSAGE_LATENCY:
        case GST_MESSAGE_ANY:
            break;
        }
    }
}


void PlaybinSession::getStreamsInfo()
{
    // Get Length if have not before.
    if (qint32(d->length) == -1)
    {
        GstFormat   format = GST_FORMAT_TIME;
        gint64      duration = 0;

        if (gst_element_query_duration(d->playbin, &format, &duration) == TRUE)
        {
            d->length = duration / 1000000;
            emit lengthChanged(d->length);
        }
    }

    // Audio/Video
    if (!d->haveStreamInfo)
    {
        enum {
            GST_STREAM_TYPE_UNKNOWN,
            GST_STREAM_TYPE_AUDIO,
            GST_STREAM_TYPE_VIDEO,
            GST_STREAM_TYPE_TEXT,
            GST_STREAM_TYPE_SUBPICTURE,
            GST_STREAM_TYPE_ELEMENT
        };

        GList*      streamInfo;

        g_object_get(G_OBJECT(d->playbin), "stream-info", &streamInfo, NULL);

        for (; streamInfo != 0; streamInfo = g_list_next(streamInfo))
        {
            gint        type;
            GObject*    obj = G_OBJECT(streamInfo->data);

            g_object_get(obj, "type", &type, NULL);

            switch (type)
            {
            case GST_STREAM_TYPE_AUDIO:
                break;

            case GST_STREAM_TYPE_VIDEO:
                d->videoControlServer = new QMediaVideoControlServer(d->id);
                d->videoControlServer->setRenderTarget(d->sinkWidget->winId());
                d->interfaces << "Video";
                emit interfaceAvailable("Video");
                break;

            case GST_STREAM_TYPE_UNKNOWN:
                break;

            case GST_STREAM_TYPE_TEXT:
                break;

            case GST_STREAM_TYPE_SUBPICTURE:
                break;

            case GST_STREAM_TYPE_ELEMENT:
                break;
            }
        }

        d->haveStreamInfo = true;
    }
}

void PlaybinSession::readySession()
{
    d->playbin = gst_element_factory_make("playbin", NULL);
    if (d->playbin != 0)
    {
        g_object_set(G_OBJECT(d->playbin), "uri", d->url.toString().toLocal8Bit().constData(), NULL);

        // Pre-set video element, even if no video
        d->sinkWidget = new DirectPainterWidget;
        g_object_set(G_OBJECT(d->playbin), "video-sink", d->sinkWidget->element(), NULL);

        // Sort out messages
        d->bus = gst_element_get_bus(d->playbin);

        d->busHelper = new BusHelper(d->bus, this);

        connect(d->busHelper, SIGNAL(message(Message)),
                this, SLOT(busMessage(Message)));

        g_object_get(G_OBJECT(d->playbin), "volume", &d->volume, NULL);
    }
}

// }}}

}   // ns gstreamer

