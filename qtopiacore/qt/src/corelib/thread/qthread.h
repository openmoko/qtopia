/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTHREAD_H
#define QTHREAD_H

#include <QtCore/qobject.h>

#include <limits.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QThreadData;
class QThreadPrivate;

#ifndef QT_NO_THREAD
class Q_CORE_EXPORT QThread : public QObject
{
public:
    static Qt::HANDLE currentThreadId();
    static QThread *currentThread();

    explicit QThread(QObject *parent = 0);
    ~QThread();

    enum Priority {
        IdlePriority,

        LowestPriority,
        LowPriority,
        NormalPriority,
        HighPriority,
        HighestPriority,

        TimeCriticalPriority,

        InheritPriority
    };

    void setPriority(Priority priority);
    Priority priority() const;

    bool isFinished() const;
    bool isRunning() const;

    void setStackSize(uint stackSize);
    uint stackSize() const;

    void exit(int retcode = 0);

public Q_SLOTS:
    void start(Priority = InheritPriority);
    void terminate();
    void quit();

public:
    // default argument causes thread to block indefinately
    bool wait(unsigned long time = ULONG_MAX);

Q_SIGNALS:
    void started();
    void finished();
    void terminated();

protected:
    virtual void run() = 0;
    int exec();

    static void setTerminationEnabled(bool enabled = true);

    static void sleep(unsigned long);
    static void msleep(unsigned long);
    static void usleep(unsigned long);

#ifdef QT3_SUPPORT
public:
    inline QT3_SUPPORT bool finished() const { return isFinished(); }
    inline QT3_SUPPORT bool running() const { return isRunning(); }
#endif

protected:
    QThread(QThreadPrivate &dd, QObject *parent = 0);

private:
    Q_OBJECT
    Q_DECLARE_PRIVATE(QThread)

    static void initialize();
    static void cleanup();

    friend class QCoreApplication;
    friend class QThreadData;
};

#else // QT_NO_THREAD

class Q_CORE_EXPORT QThread : public QObject
{
public:
    static Qt::HANDLE currentThreadId() { return Qt::HANDLE(currentThread()); }
    static QThread* currentThread()
    { if (!instance) instance = new QThread(); return instance; }

private:
    QThread();
    static QThread *instance;

    friend class QCoreApplication;
    friend class QThreadData;
    Q_DECLARE_PRIVATE(QThread)
};

#endif // QT_NO_THREAD

QT_END_HEADER

#endif // QTHREAD_H
