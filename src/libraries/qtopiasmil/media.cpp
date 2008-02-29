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

#include "media.h"
#include "system.h"
#include "transfer.h"

#include <qsound.h>

#include <qtopianamespace.h>
#include <qsoundcontrol.h>
#include <qpainter.h>
#include <qxml.h>
#include <qmovie.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qfontmetrics.h>
#include <qtoolbutton.h>
#include <QHBoxLayout>
#include <QBuffer>
#include <QDebug>


//---------------------------------------------------------------------------

SmilMedia::SmilMedia(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
    setVisible(false);
}

void SmilMedia::setState(State s)
{
    SmilElement::setState(s);
    sys->update(rect());
}

void SmilMedia::reset()
{
    SmilElement::reset();
    vis = false;
}

SmilMediaParam *SmilMedia::findParameter(const QString &name)
{
    SmilElementList::Iterator it;
    for (it = chn.begin(); it != chn.end(); ++it) {
        if ((*it)->name() == "param") {
            SmilMediaParam *p = (SmilMediaParam *)*it;
            if (p->name == name)
                return p;
        }
    }

    return 0;
}

//---------------------------------------------------------------------------

SmilText::SmilText(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilMedia(sys, p, n, atts), textColor(Qt::black), waiting(false)
{
    source = atts.value("src");
}

void SmilText::addCharacters(const QString &ch)
{
    text += ch;
}

void SmilText::setData(const QByteArray &data, const QString &type)
{
    text = QString::fromUtf8(data.data(), data.size());
    SmilMedia::setData(data, type);
}

void SmilText::process()
{
    SmilMedia::process();
    SmilMediaParam *p = findParameter("foregroundColor");
    if (!p)
        p = findParameter("foreground-color");
    if (p)
        textColor = parseColor(p->value);
    if (!source.isEmpty() && !waiting) {
        waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

Duration SmilText::implicitDuration()
{
    return Duration(0);
}

void SmilText::paint(QPainter *p)
{
    SmilMedia::paint(p);
    if (vis) {
        QPen oldPen = p->pen();
        p->setPen(textColor);
        p->drawText(rect(), Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere, text);
        p->setPen(oldPen);
    }
}

//---------------------------------------------------------------------------

class ImgPrivate : public QObject
{
    Q_OBJECT
public:
    ImgPrivate(SmilSystem *s, const QRect &r);

public slots:
    void status(QMovie::MovieState);
    void update(const QRect &);

public:
    SmilSystem *sys;
    QMovie *movie;
    QRect rect;
};

ImgPrivate::ImgPrivate(SmilSystem *s, const QRect &r)
    : QObject(), sys(s), movie(0), rect(r)
{
}

void ImgPrivate::status(QMovie::MovieState)
{
}

void ImgPrivate::update(const QRect &imgrect)
{
    QRect urect = imgrect;
    urect.translate(rect.x(), rect.y());
    sys->update(urect);
}


SmilImg::SmilImg(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilMedia(sys, p, n, atts), d(0), waiting(false)
{
    source = atts.value("src");
}

SmilImg::~SmilImg()
{
    sys->transferServer()->endData(this, source);
    if (d)
        delete d->movie;
    delete d;
}

void SmilImg::process()
{
    SmilMedia::process();
    if (!source.isEmpty() && !waiting) {
        waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

void SmilImg::setState(State s)
{
    SmilMedia::setState(s);
    if (d && d->movie) {
        switch (s) {
            case Startup:
                d->movie->start();
                break;
            case Active:
                d->movie->setPaused(false);
                break;
            case Idle:
            case End:
                d->movie->setPaused(true);
                break;
            default:
                break;
        }
    }
}

void SmilImg::setData(const QByteArray &data, const QString &type)
{
    char head[7];
    memcpy(head, data.data(), qMin(6, data.size()));
    head[6] = '\0';

    if (QString(head) == "GIF89a") {
        d = new ImgPrivate(sys, rect());
        QByteArray _data( data );
        QBuffer* buf = new QBuffer(&_data,d);
        d->movie = new QMovie(buf);
        d->movie->setPaused(true);
        QObject::connect(d->movie, SIGNAL(updated(const QRect&)), d, SLOT(update(const QRect&)));
        QObject::connect(d->movie, SIGNAL(stateChanged(QMovie::MovieState)), d, SLOT(status(QMovie::MovieState)));
    } else {
        QImage img;
        img.loadFromData(data);
        img = img.scaled(rect().size(), Qt::KeepAspectRatio);
        pix = QPixmap::fromImage(img);
    }
    waiting = false;
    sys->transferServer()->endData(this, source);
    SmilMedia::setData(data, type);
}

Duration SmilImg::implicitDuration()
{
    return Duration(0);
}

void SmilImg::paint(QPainter *p)
{
    SmilMedia::paint(p);
    if (vis) {
        if (d && d->movie) {
            QPixmap pm = d->movie->currentPixmap();
            if (pm.width() > rect().width() || pm.height() > rect().height()) {
                QImage img = d->movie->currentImage();
                img = img.scaled(rect().size(), Qt::KeepAspectRatio);
                pm = QPixmap::fromImage(img);
            }
            p->drawPixmap(rect().x(), rect().y(), pm);
        } else {
            p->drawPixmap(rect().x(), rect().y(), pix);
        }
    }
}

//---------------------------------------------------------------------------

class AudioPlayer : public QObject
{
public:
    AudioPlayer(QObject *parent = 0);
    ~AudioPlayer();

    void play(const QByteArray &data, const QString &type);
    void stop();

private:
    QString audioFile;
    QSoundControl *soundControl;
};


AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent), soundControl(0)
{
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

void AudioPlayer::play(const QByteArray &data, const QString &type)
{
    stop();
    QString ext;
    if (type == "audio/amr") {
        ext = ".amr";
    } else if (type == "audio/x-wav") {
        ext = ".wav";
    } else if (type == "audio/mpeg") {
        ext = ".mp3";
    } else {
        // guess
        char buf[7];
        memcpy(buf, data.data(), qMin(6, data.size()));
        buf[6] = '\0';
        QString head(buf);
        if (head == "#!AMR") {
            ext = ".amr";
        } else if (head == "RIFF") {
            ext = ".wav";
        }
    }

    if (!ext.isEmpty()) {
        if (!audioFile.isEmpty())
            QFile::remove(audioFile);
        audioFile = Qtopia::tempDir() + "MMSaudio" + ext;
        QFile f(audioFile);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(data);
            f.close();
            soundControl = new QSoundControl(new QSound(audioFile));
            soundControl->sound()->play();
        }
    }
}

void AudioPlayer::stop()
{
    if ( soundControl ) {
        if (!soundControl->sound()->isFinished())
            soundControl->sound()->stop();
        delete soundControl->sound();
        delete soundControl;
        soundControl = 0;
    }
    if (!audioFile.isEmpty()) {
        QFile::remove(audioFile);
        audioFile = QString();
    }
}

AudioPlayer *SmilAudio::player = 0;

SmilAudio::SmilAudio(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilMedia(sys, p, n, atts), waiting(false)
{
    source = atts.value("src");
}

SmilAudio::~SmilAudio()
{
    sys->transferServer()->endData(this, source);
}

void SmilAudio::process()
{
    SmilMedia::process();
    if (!source.isEmpty() && !waiting) {
        if (!player)
            player = new AudioPlayer();
        waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

void SmilAudio::setState(State s)
{
    SmilMedia::setState(s);
    switch(s) {
        case Active:
            if (player) {
                player->play(audioData, audioType);
            }
            break;
        case Idle:
        case End:
            if (player) {
                player->stop();
            }
            break;
        default:
            break;
    }
}

void SmilAudio::setData(const QByteArray &data, const QString &type)
{
    waiting = false;
    sys->transferServer()->endData(this, source);
    audioData = data;
    audioType = type;
    SmilMedia::setData(data, type);
}

Duration SmilAudio::implicitDuration()
{
    Duration dur;
    dur.setIndefinite(true);
    return dur;
}

void SmilAudio::paint(QPainter *p)
{
    SmilMedia::paint(p);
    if (vis) {
        // may want to use this to display volume control.
    }
}

//---------------------------------------------------------------------------

SmilMediaParam::SmilMediaParam(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilElement(sys, p, n, atts), valueType(Data)
{
    name = atts.value("name");
    value = atts.value("value");
    type = atts.value("type");
    QString tv = atts.value("typeval");
    if (tv == "ref")
        valueType = Ref;
    else if (tv == "object")
        valueType = Object;
}

//===========================================================================

SmilMediaModule::SmilMediaModule()
    : SmilModule()
{
}

SmilMediaModule::~SmilMediaModule()
{
}

SmilElement *SmilMediaModule::beginParseElement(SmilSystem *sys, SmilElement *e, const QString &qName, const QXmlAttributes &atts)
{
    if (qName == "text") {
        return new SmilText(sys, e, qName, atts);
    } else if (qName == "img") {
        return new SmilImg(sys, e, qName, atts);
    } else if (qName == "audio") {
        return new SmilAudio(sys, e, qName, atts);
    } else if (qName == "ref") {
        // try to guess the actual type.
        QString source = atts.value("src");
        if ( source.contains("image/"))
            return new SmilImg(sys, e, qName, atts);
        else if (source.contains("audio/"))
            return new SmilImg(sys, e, qName, atts);
        else if (source.contains("text/"))
            return new SmilText(sys, e, qName, atts);
    } else if (qName == "param") {
        return new SmilMediaParam(sys, e, qName, atts);
    }

    return 0;
}

bool SmilMediaModule::parseAttributes(SmilSystem *, SmilElement *, const QXmlAttributes &)
{
    return false;
}

void SmilMediaModule::endParseElement(SmilElement *, const QString &)
{
}

QStringList SmilMediaModule::elements() const
{
    QStringList l;
    l.append("text");
    l.append("img");
    l.append("audio");
    l.append("ref");
    l.append("param");
    return l;
}

QStringList SmilMediaModule::attributes() const
{
    //###
    QStringList l;
    return l;
}

#include "media.moc"
