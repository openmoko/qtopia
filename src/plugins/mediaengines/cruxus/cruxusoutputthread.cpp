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

#include <QList>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QMediaPipe>
#include <QMediaDecoder>
#include <QAudioOutput>

#include "cruxusoutputthread.h"


namespace cruxus
{

class ThreadWaiter
{
public:
    ThreadWaiter()
    {
        reset();
    }

    ~ThreadWaiter()
    {
    }

    void wait()
    {
        m_waitCondition.wait(&m_mutex);
    }

    void signal()
    {
        m_waitCondition.wakeOne();
    }

    void reset()
    {
        m_mutex.lock();
    }

private:
    QMutex          m_mutex;
    QWaitCondition  m_waitCondition;
};

// {{{ OutputThreadPrivate
class OutputThreadPrivate : public QThread
{
    Q_OBJECT

public:
    bool                    running;
    bool                    quit;
    QMutex                  mutex;
    QWaitCondition          condition;
    QAudioOutput*           audioOutput;
    QMediaDevice*           mediaDevice;

    QList<QMediaPipe*>      sessions;
    QList<QMediaPipe*>      activeSessions;

    ThreadWaiter            threadWaiter;

protected:
    void run();
};

void OutputThreadPrivate::run()
{
    unsigned long   timeout = ULONG_MAX;

    quit  = false;

    audioOutput = new QAudioOutput;

    audioOutput->setFrequency(44100);
    audioOutput->setChannels(2);
    audioOutput->setBitsPerSample(16);

    audioOutput->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    threadWaiter.signal();

    do
    {
        QMutexLocker    conditionLock(&mutex);

        condition.wait(&mutex, timeout);
        {
            QList<QMediaPipe*>::iterator it;

            for (it = activeSessions.begin(); it != activeSessions.end(); ++it)
            {
                qint64  length;
                char    buf[1024];

                if ((length = (*it)->read(buf, 1024)) <= 0)
                {
                    activeSessions.erase(it);
                }
                else
                {
                    audioOutput->write(buf, length);
                }
            }
        }

        if (activeSessions.size() > 0)
           timeout = 0;
        else
           timeout = ULONG_MAX;

    } while (!quit);

    delete audioOutput;
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
    QMutexLocker    lock(&d->mutex);

    d->start();
    d->threadWaiter.wait();
}

OutputThread::~OutputThread()
{
    d->quit = true;

    delete d;
}

void OutputThread::connectInputPipe(QMediaPipe* inputPipe)
{
    connect(inputPipe, SIGNAL(readyRead()),
            this, SLOT(readFromPipe()));
}

void OutputThread::connectOutputPipe(QMediaPipe* outputPipe)
{
    Q_UNUSED(outputPipe);

    qWarning("OutputThread is a sink");
}

void OutputThread::disconnectInputPipe(QMediaPipe* inputPipe)
{
    QMutexLocker    lock(&d->mutex);

    inputPipe->disconnect(this);

    d->activeSessions.removeAll(inputPipe);
}

void OutputThread::disconnectOutputPipe(QMediaPipe* outputPipe)
{
    Q_UNUSED(outputPipe);

    qWarning("OutputThread is a sink");
}

void OutputThread::setValue(QString const& name, QVariant const& value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

QVariant OutputThread::value(QString const& name)
{
    Q_UNUSED(name);

    return QVariant();
}

void OutputThread::readFromPipe()
{
    QMutexLocker    lock(&d->mutex);

    d->activeSessions.append(qobject_cast<QMediaPipe*>(sender()));

    d->condition.wakeOne();
}

qint64 OutputThread::readData( char *data, qint64 maxlen )
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);

    return 0;
}

qint64 OutputThread::writeData( const char *data, qint64 len )
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}
// }}}

}   // ns cruxus

#include "cruxusoutputthread.moc"

