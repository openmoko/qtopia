
#include <QTimer>
#include <QFileInfo>

#include <qtopianamespace.h>
#include <qtopialog.h>

#include <qtopiamedia/qmediadevice.h>
#include <qtopiamedia/qmediapipe.h>

#include <timidity.h>

#include "mididecoder.h"

#define MIDI_BUFFER_SIZE    1024 * 8
#define MAX_AMPLIFICATION   800         // From libtimidity


// {{{ MidiDecoderPrivate
class MidiDecoderPrivate
{
public:
    bool                initialized;
    bool                muted;
    int                 volume;
    quint32             length;
    quint32             position;
    QMediaPipe*         inputPipe;
    QMediaPipe*         outputPipe;
    MidSong*            song;
    MidIStream*         midiStream;
    MidSongOptions      options;
    QtopiaMedia::State  state;
};
// }}}


// {{{ MidiDecoder
MidiDecoder::MidiDecoder():
    d(new MidiDecoderPrivate)
{
    // init
    d->initialized = false;
    d->muted = false;
    d->volume = MAX_AMPLIFICATION / 8;
    d->length = 0;
    d->position = 0;
    d->state = QtopiaMedia::Stopped;

    // Load .cfg
    foreach (QString configPath, Qtopia::installPaths())
    {
        configPath += QLatin1String("etc/timidity/timidity.cfg");

        qLog(Media) << "MidiDecoder::MidiDecoder(); searching for config -" << configPath;

        if (QFileInfo(configPath).exists())
        {
            qLog(Media) << "MidiDecoder::MidiDecoder(); found timidity.cfg";

            if (mid_init(configPath.toLocal8Bit().data()) == -1)
                qLog(Media) << "MidiDecoder::MidiDecoder(); Invalid config file";

            break;
        }
    }
}

MidiDecoder::~MidiDecoder()
{
    mid_song_free(d->song);
    mid_exit();

    delete d;
}

void MidiDecoder::connectInputPipe(QMediaPipe* inputPipe)
{
    d->inputPipe = inputPipe;
}

void MidiDecoder::connectOutputPipe(QMediaPipe* outputPipe)
{
    d->outputPipe = outputPipe;
}

void MidiDecoder::disconnectInputPipe(QMediaPipe* inputPipe)
{
    Q_UNUSED(inputPipe);

    d->inputPipe = 0;
}

void MidiDecoder::disconnectOutputPipe(QMediaPipe* outputPipe)
{
    Q_UNUSED(outputPipe);

    d->outputPipe = 0;
}

void MidiDecoder::setValue(QString const& name, QVariant const& value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

QVariant MidiDecoder::value(QString const& name)
{
    Q_UNUSED(name);

    return QVariant();
}

void MidiDecoder::start()
{
    if (!d->initialized)
    {
        QIODevice::open(QIODevice::ReadWrite | QIODevice::Unbuffered);

        d->options.rate        = 44100;
        d->options.format      = MID_AUDIO_S16LSB;
        d->options.channels    = 2;
        d->options.buffer_size = MIDI_BUFFER_SIZE / (16 * 2 / 8);

        d->midiStream = mid_istream_open_callbacks(midiReadCallback,
                                                  midiCloseCallback,
                                                  this);

        d->song = mid_song_load(d->midiStream, &d->options);

        if (d->song != 0)
        {
            d->state = QtopiaMedia::Playing;

            d->length = mid_song_get_total_time(d->song);
            emit lengthChanged(d->length);

            mid_song_set_volume(d->song, d->muted ? 0 : d->volume * 8);

            mid_song_start(d->song);

            d->initialized = true;

        }
        else
            qLog(Media) << "MidiDecoder::start(); Failed to load MIDI file";
    }
    else
        d->state = QtopiaMedia::Playing;

    if (d->initialized)
    {
        emit readyRead();
        emit playerStateChanged(d->state);
    }
}

void MidiDecoder::stop()
{
    d->state = QtopiaMedia::Stopped;
    seek(0);
}

void MidiDecoder::pause()
{
    d->state = QtopiaMedia::Paused;
}

quint32 MidiDecoder::length()
{
    return d->length;
}

bool MidiDecoder::seek(qint64 ms)
{
    if (d->initialized)
        mid_song_seek(d->song, ms);

    return true;
}

void MidiDecoder::setVolume(int volume)
{
    d->volume = volume;
    if (d->initialized && !d->muted)
        mid_song_set_volume(d->song, d->volume * 8);

    emit volumeChanged(d->volume);
}

int MidiDecoder::volume()
{
    return d->volume;
}

void MidiDecoder::setMuted(bool mute)
{
    d->muted = mute;

    if (d->initialized)
        mid_song_set_volume(d->song, mute ? 0 : d->volume * 8);

    emit volumeMuted(d->muted);
}

bool MidiDecoder::isMuted()
{
    return d->muted;
}

//private:
qint64 MidiDecoder::readData(char *data, qint64 maxlen)
{
    qint64      rc = maxlen;

    if (maxlen > 0)
    {
        if (d->state == QtopiaMedia::Playing)
        {
            quint32 position = (mid_song_get_time(d->song) / 1000) * 1000;
            if (d->position != position)
            {
                d->position = position;
                emit positionChanged(d->position);
            }

            qint64 rc = (qint64) mid_song_read_wave(d->song, data, maxlen);

            if (rc == 0)
                d->state = QtopiaMedia::Stopped;
        }
        else
        {
            rc = 0;
        }
    }

    return rc;
}

qint64 MidiDecoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

size_t MidiDecoder::readCallback(void* dst, size_t itemSize, size_t numItems)
{
    size_t rc = d->inputPipe->read(reinterpret_cast<char*>(dst), itemSize * numItems);

    if (rc > 0)
        rc /= itemSize;

    return rc;
}

int MidiDecoder::closeCallback()
{
    return 0;
}

// static
size_t MidiDecoder::midiReadCallback(void* ctx, void* ptr, size_t size, size_t nmemb)
{
    return reinterpret_cast<MidiDecoder*>(ctx)->readCallback(ptr, size, nmemb);
}

int MidiDecoder::midiCloseCallback(void* ctx)
{
    return reinterpret_cast<MidiDecoder*>(ctx)->closeCallback();
}
// }}}

