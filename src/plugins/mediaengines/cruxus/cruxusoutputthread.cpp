/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <math.h>

#include <QList>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QMediaDecoder>
#include <QAudioOutput>
#include <QDebug>
#include <QFile>

#include <qtopialog.h>

#include <custom.h>

#include "cruxusoutputthread.h"

//#define DEBUG_ENGINE

namespace cruxus
{

// {{{ OutputThreadPrivate
class OutputThreadPrivate : public QThread
{
    Q_OBJECT

    static const int MAX_VOLUME = 100;

#ifdef CRUXUS_OUTPUT_FREQ
    static const int request_frequency = CRUXUS_OUTPUT_FREQ;
#else
    static const int request_frequency = 44100;
#endif

    static const int request_bitsPerSample = 16;
    static const int request_channels = 2;

#ifdef CRUXUS_FRAMESIZE
    static const int frame_milliseconds = CRUXUS_FRAMESIZE;
#else
    static const int frame_milliseconds = 100;
#endif

    static const int default_frame_size = (48000 / (1000 / frame_milliseconds)) *
                                          (request_bitsPerSample / 8) *
                                          request_channels;

public:
    bool                    opened;
    bool                    running;
    bool                    quit;
    bool                    paused;
    QMutex                  mutex;
    QWaitCondition          condition;
    QAudioOutput*           audioOutput;
    QMediaDevice::Info      inputInfo;
    QList<QMediaDevice*>    activeSessions;

    void suspend();
    void resume();

protected:
    void run();

private:
    int readFromDevice(QMediaDevice* device, QMediaDevice::Info const& info, char* working);
    int resampleAndMix(QMediaDevice::Info const& deviceInfo, char* src, int dataAmt, bool first);
    qint32 addSamples(qint32 s1,qint32 s2);

    char    mixbuf[default_frame_size];
};

void OutputThreadPrivate::run()
{
    unsigned long   timeout = 30000;

    quit = false;
    paused = false;

    audioOutput = new QAudioOutput;

    audioOutput->setFrequency(request_frequency);
    audioOutput->setChannels(request_channels);
    audioOutput->setBitsPerSample(request_bitsPerSample);

    inputInfo.type = QMediaDevice::Info::PCM;
    inputInfo.frequency = audioOutput->frequency();
    inputInfo.bitsPerSample = audioOutput->bitsPerSample();
    inputInfo.channels = audioOutput->channels();

    qLog(Media) << "OutputThreadPrivate::run(); opened device with " <<
                inputInfo.frequency << inputInfo.bitsPerSample << inputInfo.channels;
#if defined(DEBUG_ENGINE)
    QFile* file = new QFile("/tmp/test.raw");
#endif

    if(!paused)
        if(!audioOutput->isOpen()) {
            qLog(Media) <<"OutputThreadPrivate::run(); open output";
            audioOutput->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
        }

    do
    {
        QMutexLocker    conditionLock(&mutex);

        int             sc = activeSessions.size();

        if (sc == 0) {
            if(!condition.wait(&mutex, timeout) && opened) {
                audioOutput->close();
                opened = false;
                timeout = ULONG_MAX;
            }
        } else {
            if (!paused) {
                bool    mixed = false;
                bool    first = true;
                int     mixLength = 0;
                char    working[default_frame_size];

                for (int i = 0; i < sc; ++i) {
                    QMediaDevice* input = activeSessions.at(i);
                    QMediaDevice::Info const& info = input->dataType();

                    int read = readFromDevice(input, info, working);
                    if (read > 0) {
                        if(mixLength > 0)
                            mixed = true;
                        mixLength = qMax(resampleAndMix(info, working, read, first), mixLength);
                        first = false;
                    }
                    else {
                        activeSessions.removeAt(i);
                        --sc;
                    }
                }
                if (mixLength > 0) {

#if defined(DEBUG_ENGINE)
                    file->open(QIODevice::WriteOnly | QIODevice::Append );
                    file->write(mixbuf, mixLength);
                    file->close();
#endif
                    audioOutput->write(mixbuf, mixLength);
                }
                timeout = activeSessions.size() > 0 ? 0 : 30000;
            } else {
                qLog(Media) <<"OutputThreadPrivate::run(); wait";
                condition.wait(&mutex,250);
            }
            condition.wait(&mutex,5); // allow locking, need wait here
        }
    } while (!quit);

    delete audioOutput;
    audioOutput = 0;

    qLog(Media) << "OutputThreadPrivate::run(); exiting";
}

void OutputThreadPrivate::suspend()
{
    if(audioOutput && opened) {
        audioOutput->close();
        opened = false;
    }
    paused = true;
    qLog(Media) << "OutputThreadPrivate::suspend()";
}

void OutputThreadPrivate::resume()
{
    if(audioOutput && !opened)
        opened = audioOutput->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    paused = false;
    qLog(Media) << "OutputThreadPrivate::resume()";
}

inline int OutputThreadPrivate::readFromDevice
(
 QMediaDevice* device,
 QMediaDevice::Info const& info,
 char* working
)
{
    return device->read(working, (info.frequency / (1000 / frame_milliseconds)) *
                                 (info.bitsPerSample / 8) *
                                 info.channels);
}

inline qint32 getNextSamplePart(char*& working, const int sampleSize)
{
    qint32  rc;

    switch (sampleSize)
    {
    case 1: rc = qint32(*(quint8*)working) - 128; break;
    case 2: rc = *(qint16*)working; break;
    case 4: rc = *(qint32*)working; break;
    default: rc = 0;
    }

    working += sampleSize;

    return rc;
}

inline void setNextSamplePart(char*& working, qint32 sample, const int sampleSize)
{
    switch (sampleSize)
    {
    case 1: *(quint8*)working = quint8(sample + 128); break;
    case 2: *(qint16*)working = qint16(sample); break;
    case 4: *(qint32*)working = sample; break;
    }

    working += sampleSize;
}

inline qint32 OutputThreadPrivate::addSamples(qint32 s1, qint32 s2)
{
    qint64  ret = s1 + s2;

    if(qAbs(s1+s2) > 0x7FFF) {
        if(s1 > 0) ret = 0x7FFF;
        else ret = -0x7FFF;
    }
    return (qint32)ret;
}

int OutputThreadPrivate::resampleAndMix
(
 QMediaDevice::Info const& deviceInfo,
 char* src,
 int dataAmt,
 bool first
)
{
#if defined(DEBUG_ENGINE)
    qLog(Media) << "resampleAndMix();begin" << deviceInfo.bitsPerSample << deviceInfo.frequency << deviceInfo.channels;
#endif

    const int   oss = inputInfo.bitsPerSample / 8;   // output sample size
    const int   iss = deviceInfo.bitsPerSample / 8; // input sample size
    char*       mix = mixbuf;                   // src of data to mix
    char*       dst = mixbuf;                   // dst of all data (mixed or not)
    int         converted, counter = 0;

    if (deviceInfo.frequency == inputInfo.frequency &&
        deviceInfo.bitsPerSample == inputInfo.bitsPerSample &&
        deviceInfo.channels == inputInfo.channels)
    {   // Just send off
        converted = dataAmt;

        if (first)
        {
            for (;dataAmt > 0; dataAmt -= iss)
                setNextSamplePart(dst, getNextSamplePart(src, iss) * deviceInfo.volume / MAX_VOLUME, oss);
        }
        else
        {
            for (;dataAmt > 0; dataAmt -= iss)
                setNextSamplePart(dst, addSamples((getNextSamplePart(src,iss)*deviceInfo.volume/MAX_VOLUME), getNextSamplePart(mix, oss)), oss);
        }
    }
    else
    {   // re-sample (forgive me)
        const int rsr = deviceInfo.frequency > inputInfo.frequency  ? deviceInfo.frequency/inputInfo.frequency : 1;
        const int srcChannelRate = deviceInfo.channels > inputInfo.channels  ? 2 : 1;
        const int dstChannelRate = inputInfo.channels  > deviceInfo.channels ? 2 : 1;
        const int resshift = 1 << qAbs(deviceInfo.bitsPerSample - inputInfo.bitsPerSample);

        int convertedX = (inputInfo.frequency*dataAmt/deviceInfo.frequency*oss/iss*inputInfo.channels/deviceInfo.channels);

        const int rdr = convertedX/(dataAmt/iss);

        converted = rdr * dataAmt;

        bool   firstUse = true;
        qint32 prevSample[inputInfo.channels];
        memset(prevSample, 0, sizeof(prevSample));

#if defined(DEBUG_ENGINE)
        qLog(Media)<<"input="<<dataAmt<<", rdr="<<rdr<<", converted="<<converted<<", convertedX="<<convertedX;
#endif

        int offset = 0;

        while (dataAmt > 0)
        {
            int     requiredSrcSamples = rsr;

            qint32  sample[inputInfo.channels];
            memset(sample, 0, sizeof(sample));

            // 1. Create 1 sample from input stream
            while (requiredSrcSamples-- > 0 && dataAmt > 0)
            {
                qint32 cs[inputInfo.channels];
                memset(cs, 0, sizeof(cs));

                if (srcChannelRate > 1) {
                    // Stereo to Mono
                    for (int i = 0; i < deviceInfo.channels; ++i) {
                        qint32 s = addSamples(cs[i % inputInfo.channels], getNextSamplePart(src, iss));
                        cs[i % inputInfo.channels] = s;
                    }
                } else if (dstChannelRate > 1) {
                    // Mono to Stereo
                    qint32  tmp = getNextSamplePart(src, iss);
                    for (int i = 0; i < inputInfo.channels; ++i)
                        cs[i] = tmp;
                } else {
                    // Input channels = Output channels
                    for (int i = 0; i < inputInfo.channels; ++i)
                        cs[i] = getNextSamplePart(src, iss);
                }
                // Resolution
                if (iss == oss)
                    memcpy(sample, cs, sizeof(sample));
                else {
                    for (int i = 0; i < inputInfo.channels; ++i)
                        sample[i] = iss > oss ? cs[i] / resshift : cs[i] * resshift;
                }
                dataAmt -= iss * deviceInfo.channels;
            }

            offset++;

            /* Special case: When going from 8000Hz to 44100Hz conversion
               Input: 800 samples for 100ms, Output: 17600 samples for 100ms (this should be 17640).
               Because it doesn't multiply up nicely we just duplicate a sample every so often
               to make sure the playback frequency is correct.
            */
            int extraSample = (convertedX >= converted+oss*inputInfo.channels && offset > rdr);

            if(extraSample) {
                converted = converted + oss*inputInfo.channels;
                offset=0;
            }
            if(inputInfo.frequency < deviceInfo.frequency) {
                // Handle 48000Hz case, TODO: need to drop samples to output @ 44100Hz
                // currently playback is a little fast.
                extraSample=1;
            }

            // 2. Output samples resulting from the 1 sample created above
            qint32 step[2];
            qint32 samplei;

            if(firstUse) {
                memcpy(prevSample, sample, sizeof(prevSample));
                firstUse = false;
            }
            step[0] = (sample[0] - prevSample[0])/(rdr+extraSample);
            if(inputInfo.channels > 1)
                step[1] = (sample[1] - prevSample[1])/(rdr+extraSample);

            int j=1;
            for (int i = (rdr/dstChannelRate+extraSample); i > 0; --i) {
                counter+=oss;
                samplei = ((prevSample[i % inputInfo.channels] / rsr) + step[i % inputInfo.channels]*j) * deviceInfo.volume / MAX_VOLUME;
                if (first)
                    setNextSamplePart(dst, samplei, oss);
                else
                    setNextSamplePart(dst, addSamples(samplei, getNextSamplePart(mix, oss)), oss);
                j++;
            }
            // Store sample for next iteration
            memcpy(prevSample, sample, sizeof(prevSample));
        }
        converted = counter;
    }

#if defined(DEBUG_ENGINE)
    qLog(Media) << "resampleAndMix();end returned="<<converted<<", counter="<<counter;
#endif

    return converted;
}
// }}}

// {{{ OutputThread

/*!
    \class cruxus::OutputThread
    \internal
*/

OutputThread::OutputThread():
    d(new OutputThreadPrivate)
{
    qLog(Media) << "OutputThread::OutputThread()";
    d->opened = false;
    d->paused = false;
    d->start(QThread::HighPriority);
}

OutputThread::~OutputThread()
{
    QMutexLocker    lock(&d->mutex);

    qLog(Media) << "OutputThread::~OutputThread()";

    d->quit = true;

    d->condition.wakeOne();
    d->wait();

    delete d;
}

QMediaDevice::Info const& OutputThread::dataType() const
{
    Q_ASSERT(false);        // Should never be called

    QMutexLocker    lock(&d->mutex);
    return d->inputInfo;
}

bool OutputThread::connectToInput(QMediaDevice* input)
{
    QMutexLocker    lock(&d->mutex);

    if (input->dataType().type != QMediaDevice::Info::PCM)
        return false;

    connect(input, SIGNAL(readyRead()), SLOT(deviceReady()));
    return true;
}

void OutputThread::disconnectFromInput(QMediaDevice* input)
{
    QMutexLocker    lock(&d->mutex);

    input->disconnect(this);

    d->activeSessions.removeAll(input);
}

bool OutputThread::open(QIODevice::OpenMode mode)
{
    QMutexLocker    lock(&d->mutex);

    if (!d->opened)
        d->opened = QIODevice::open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    return d->opened;

    Q_UNUSED(mode);
}

void OutputThread::close()
{
}

// private slots:
void OutputThread::deviceReady()
{
    QMutexLocker    lock(&d->mutex);

    if(!d->opened) {
        d->audioOutput->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        d->opened = true;
    }

    d->activeSessions.append(qobject_cast<QMediaDevice*>(sender()));

    d->condition.wakeOne();
}

// private:
qint64 OutputThread::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);

    return 0;
}

qint64 OutputThread::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}
// }}}

}   // ns cruxus

#include "cruxusoutputthread.moc"



