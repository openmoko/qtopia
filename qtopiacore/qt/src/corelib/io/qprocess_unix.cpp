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

//#define QPROCESS_DEBUG
#include "qdebug.h"

#ifndef QT_NO_PROCESS

#if defined QPROCESS_DEBUG
#include "qstring.h"
#include <ctype.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(c)) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            QString tmp;
            tmp.sprintf("\\%o", c);
            out += tmp.toLatin1();
        }
    }

    if (len < maxSize)
        out += "...";

    return out;
}
#endif

#include "qplatformdefs.h"

#include "qprocess.h"
#include "qprocess_p.h"

#ifdef Q_OS_MAC
#include <private/qcore_mac_p.h>
#endif

#include <private/qcoreapplication_p.h>
#include <private/qthread_p.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qmap.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qsocketnotifier.h>
#include <qthread.h>

#include <errno.h>
#include <stdlib.h>

static qint64 qt_native_read(int fd, char *data, qint64 maxlen)
{
    qint64 ret = 0;
    do {
        ret = ::read(fd, data, maxlen);
    } while (ret == -1 && errno == EINTR);
    return ret;
}

static qint64 qt_native_write(int fd, const char *data, qint64 len)
{
    qint64 ret = 0;
    do {
        ret = ::write(fd, data, len);
    } while (ret == -1 && errno == EINTR);
    return ret;
}

static void qt_native_close(int fd)
{
    int ret;
    do {
        ret = ::close(fd);
    } while (ret == -1 && errno == EINTR);
}

static void qt_native_sigaction(int signum, const struct sigaction *act,
                                struct sigaction *oldact)
{
    int ret;
    do {
        ret = ::sigaction(signum, act, oldact);
    } while (ret == -1 && errno == EINTR);
}

static void qt_native_dup2(int oldfd, int newfd)
{
    int ret;
    do {
        ret = ::dup2(oldfd, newfd);
    } while (ret == -1 && errno == EINTR);
}

static void qt_native_chdir(const char *path)
{
    int ret;
    do {
        ret = ::chdir(path);
    } while (ret == -1 && errno == EINTR);
}

static void qt_native_execve(const char *filename, char *const argv[],
                              char *const envp[])
{
    int ret;
    do {
        ret = ::execve(filename, argv, envp);
    } while (ret == -1 && errno == EINTR);
}

static void qt_native_execv(const char *path, char *const argv[])
{
    int ret;
    do {
        ret = ::execv(path, argv);
    } while (ret == -1 && errno == EINTR);
}

static void qt_native_execvp(const char *file, char *const argv[])
{
    int ret;
    do {
        ret = ::execvp(file, argv);
    } while (ret == -1 && errno == EINTR);
}

static int qt_qprocess_deadChild_pipe[2];
static void (*qt_sa_old_sigchld_handler)(int) = 0;
static void qt_sa_sigchld_handler(int signum)
{
    qt_native_write(qt_qprocess_deadChild_pipe[1], "", 1);
#if defined (QPROCESS_DEBUG)
    fprintf(stderr, "*** SIGCHLD\n");
#endif

    if (qt_sa_old_sigchld_handler && qt_sa_old_sigchld_handler != SIG_IGN)
        qt_sa_old_sigchld_handler(signum);
}

struct QProcessInfo {
    QProcess *process;
    int deathPipe;
    int exitResult;
    pid_t pid;
    int serialNumber;
};

class QProcessManager : public QThread
{
    Q_OBJECT
public:
    QProcessManager();
    ~QProcessManager();

    void run();
    void catchDeadChildren();
    void add(pid_t pid, QProcess *process);
    void remove(QProcess *process);
    void lock();
    void unlock();

private:
    QMutex mutex;
    QMap<int, QProcessInfo *> children;
};

Q_GLOBAL_STATIC(QProcessManager, processManager)

QProcessManager::QProcessManager()
{
#if defined (QPROCESS_DEBUG)
    qDebug() << "QProcessManager::QProcessManager()";
#endif
    // initialize the dead child pipe and make it non-blocking. in the
    // extremely unlikely event that the pipe fills up, we do not under any
    // circumstances want to block.
    ::pipe(qt_qprocess_deadChild_pipe);
    ::fcntl(qt_qprocess_deadChild_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(qt_qprocess_deadChild_pipe[1], F_SETFD, FD_CLOEXEC);
    ::fcntl(qt_qprocess_deadChild_pipe[0], F_SETFL,
	    ::fcntl(qt_qprocess_deadChild_pipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(qt_qprocess_deadChild_pipe[1], F_SETFL,
	    ::fcntl(qt_qprocess_deadChild_pipe[1], F_GETFL) | O_NONBLOCK);

    // set up the SIGCHLD handler, which writes a single byte to the dead
    // child pipe every time a child dies.
    struct sigaction oldAction;
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = qt_sa_sigchld_handler;
    action.sa_flags = SA_NOCLDSTOP;
    qt_native_sigaction(SIGCHLD, &action, &oldAction);
    if (oldAction.sa_handler != qt_sa_sigchld_handler)
	qt_sa_old_sigchld_handler = oldAction.sa_handler;
}

QProcessManager::~QProcessManager()
{
    // notify the thread that we're shutting down.
    qt_native_write(qt_qprocess_deadChild_pipe[1], "@", 1);
    qt_native_close(qt_qprocess_deadChild_pipe[1]);
    wait();

    // on certain unixes, closing the reading end of the pipe will cause
    // select in run() to block forever, rather than return with EBADF.
    qt_native_close(qt_qprocess_deadChild_pipe[0]);

    qt_qprocess_deadChild_pipe[0] = -1;
    qt_qprocess_deadChild_pipe[1] = -1;

    qDeleteAll(children.values());
    children.clear();
}

void QProcessManager::run()
{
    forever {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(qt_qprocess_deadChild_pipe[0], &readset);

#if defined (QPROCESS_DEBUG)
        qDebug() << "QProcessManager::run() waiting for children to die";
#endif

        // block forever, or until activity is detected on the dead child
        // pipe. the only other peers are the SIGCHLD signal handler, and the
        // QProcessManager destructor.
        int nselect = select(qt_qprocess_deadChild_pipe[0] + 1, &readset, 0, 0, 0);
        if (nselect < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        // empty only one byte from the pipe, even though several SIGCHLD
        // signals may have been delivered in the meantime, to avoid race
        // conditions.
        char c;
        if (qt_native_read(qt_qprocess_deadChild_pipe[0], &c, 1) < 0 || c == '@')
            break;

        // catch any and all children that we can.
        catchDeadChildren();
    }
}

void QProcessManager::catchDeadChildren()
{
    QMutexLocker locker(&mutex);

    // try to catch all children whose pid we have registered, and whose
    // deathPipe is still valid (i.e, we have not already notified it).
    QMap<int, QProcessInfo *>::Iterator it = children.begin();
    while (it != children.end()) {
        // notify all children that they may have died. they need to run
        // waitpid() in their own thread.
        QProcessInfo *info = it.value();
        qt_native_write(info->deathPipe, "", 1);

#if defined (QPROCESS_DEBUG)
        qDebug() << "QProcessManager::run() sending death notice to" << info->process;
#endif
        ++it;
    }
}

static QBasicAtomic idCounter = Q_ATOMIC_INIT(1);
static int qt_qprocess_nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
}

void QProcessManager::add(pid_t pid, QProcess *process)
{
#if defined (QPROCESS_DEBUG)
    qDebug() << "QProcessManager::add() adding pid" << pid << "process" << process;
#endif

    // insert a new info structure for this process
    QProcessInfo *info = new QProcessInfo;
    info->process = process;
    info->deathPipe = process->d_func()->deathPipe[1];
    info->exitResult = 0;
    info->pid = pid;

    int serial = qt_qprocess_nextId();
    process->d_func()->serial = serial;
    children.insert(serial, info);
}

void QProcessManager::remove(QProcess *process)
{
    QMutexLocker locker(&mutex);

    int serial = process->d_func()->serial;
    QProcessInfo *info = children.value(serial);
    if (!info)
        return;

#if defined (QPROCESS_DEBUG)
    qDebug() << "QProcessManager::remove() removing pid" << info->pid << "process" << info->process;
#endif

    children.remove(serial);
    delete info;
}

void QProcessManager::lock()
{
    mutex.lock();
}

void QProcessManager::unlock()
{
    mutex.unlock();
}

static void qt_create_pipe(int *pipe)
{
    if (pipe[0] != -1)
        qt_native_close(pipe[0]);
    if (pipe[1] != -1)
        qt_native_close(pipe[1]);
#ifdef Q_OS_IRIX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, pipe) == -1) {
        qWarning("QProcessPrivate::createPipe: Cannot create pipe %p: %s",
                 pipe, qPrintable(qt_error_string(errno)));
    }
#else
    if (::pipe(pipe) != 0) {
        qWarning("QProcessPrivate::createPipe: Cannot create pipe %p: %s",
                 pipe, qPrintable(qt_error_string(errno)));
    }
#endif
    ::fcntl(pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(pipe[1], F_SETFD, FD_CLOEXEC);
}

void QProcessPrivate::destroyPipe(int *pipe)
{
    if (pipe[1] != -1) {
        qt_native_close(pipe[1]);
        pipe[1] = -1;
    }
    if (pipe[0] != -1) {
        qt_native_close(pipe[0]);
        pipe[0] = -1;
    }
}

/*
    Create the pipes to a QProcessPrivate::Channel.

    This function must be called in order: stdin, stdout, stderr
*/
bool QProcessPrivate::createChannel(Channel &channel)
{
    Q_Q(QProcess);

    if (&channel == &stderrChannel && processChannelMode == QProcess::MergedChannels) {
        channel.pipe[0] = -1;
        channel.pipe[1] = -1;
        return true;
    }

    if (channel.type == Channel::Normal) {
        // we're piping this channel to our own process
        qt_create_pipe(channel.pipe);

        // create the socket notifiers
        if (threadData->eventDispatcher) {
            if (&channel == &stdinChannel) {
                channel.notifier = new QSocketNotifier(channel.pipe[1],
                                                       QSocketNotifier::Write, q);
                channel.notifier->setEnabled(false);
                QObject::connect(channel.notifier, SIGNAL(activated(int)),
                                 q, SLOT(_q_canWrite()));
            } else {
                channel.notifier = new QSocketNotifier(channel.pipe[0],
                                                       QSocketNotifier::Read, q);
                const char *receiver;
                if (&channel == &stdoutChannel)
                    receiver = SLOT(_q_canReadStandardOutput());
                else
                    receiver = SLOT(_q_canReadStandardError());
                QObject::connect(channel.notifier, SIGNAL(activated(int)),
                                 q, receiver);
            }
        }

        return true;
    } else if (channel.type == Channel::Redirect) {
        // we're redirecting the channel to/from a file
        QByteArray fname = QFile::encodeName(channel.file);

        if (&channel == &stdinChannel) {
            // try to open in read-only mode
            channel.pipe[1] = -1;
            if ( (channel.pipe[0] = open(fname, O_RDONLY)) != -1)
                return true;    // success

            q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Could not open input redirection for reading")));
        } else {
            int mode = O_WRONLY | O_CREAT;
            if (channel.append)
                mode |= O_APPEND;
            else
                mode |= O_TRUNC;

            channel.pipe[0] = -1;
            if ( (channel.pipe[1] = open(fname, mode, 0666)) != -1)
                return true; // success

            q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Could not open output redirection for writing")));
        }

        // could not open file
        processError = QProcess::FailedToStart;
        emit q->error(processError);
        cleanup();
        return false;
    } else {
        Q_ASSERT_X(channel.process, "QProcess::start", "Internal error");

        Channel *source;
        Channel *sink;

        if (channel.type == Channel::PipeSource) {
            // we are the source
            source = &channel;
            sink = &channel.process->stdinChannel;

            Q_ASSERT(source == &stdoutChannel);
            Q_ASSERT(sink->process == this && sink->type == Channel::PipeSink);
        } else {
            // we are the sink;
            source = &channel.process->stdoutChannel;
            sink = &channel;

            Q_ASSERT(sink == &stdinChannel);
            Q_ASSERT(source->process == this && source->type == Channel::PipeSource);
        }

        if (source->pipe[1] != INVALID_Q_PIPE || sink->pipe[0] != INVALID_Q_PIPE) {
            // already created, do nothing
            return true;
        } else {
            Q_ASSERT(source->pipe[0] == INVALID_Q_PIPE && source->pipe[1] == INVALID_Q_PIPE);
            Q_ASSERT(sink->pipe[0] == INVALID_Q_PIPE && sink->pipe[1] == INVALID_Q_PIPE);

            Q_PIPE pipe[2] = { -1, -1 };
            qt_create_pipe(pipe);
            sink->pipe[0] = pipe[0];
            source->pipe[1] = pipe[1];

            return true;
        }
    }
}

void QProcessPrivate::startProcess()
{
    Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::startProcess()");
#endif

    processManager()->start();

    // Initialize pipes
    qt_create_pipe(childStartedPipe);
    if (threadData->eventDispatcher) {
        startupSocketNotifier = new QSocketNotifier(childStartedPipe[0],
                                                    QSocketNotifier::Read, q);
        QObject::connect(startupSocketNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_startupNotification()));
    }

    qt_create_pipe(deathPipe);
    ::fcntl(deathPipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(deathPipe[1], F_SETFD, FD_CLOEXEC);
    if (threadData->eventDispatcher) {
        deathNotifier = new QSocketNotifier(deathPipe[0],
                                            QSocketNotifier::Read, q);
        QObject::connect(deathNotifier, SIGNAL(activated(int)),
                         q, SLOT(_q_processDied()));
    }

    if (!createChannel(stdinChannel) ||
        !createChannel(stdoutChannel) ||
        !createChannel(stderrChannel))
        return;

    // Start the process (platform dependent)
    processState = QProcess::Starting;
    emit q->stateChanged(processState);

    QByteArray encodedProg = QFile::encodeName(program);
    processManager()->lock();
    pid_t childPid = fork();
    if (childPid < 0) {
        // Cleanup, report error and return
        processManager()->unlock();
        processState = QProcess::NotRunning;
        emit q->stateChanged(processState);
        processError = QProcess::FailedToStart;
        q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Resource error (fork failure)")));
        emit q->error(processError);
        cleanup();
        return;
    }

    if (childPid == 0) {
        execChild(encodedProg);
        ::_exit(-1);
    }
    processManager()->add(childPid, q);
    pid = Q_PID(childPid);
    processManager()->unlock();

    // parent
    // close the ends we don't use and make all pipes non-blocking
    ::fcntl(deathPipe[0], F_SETFL, ::fcntl(deathPipe[0], F_GETFL) | O_NONBLOCK);
    qt_native_close(childStartedPipe[1]);
    childStartedPipe[1] = -1;

    if (stdinChannel.pipe[0] != -1) {
        qt_native_close(stdinChannel.pipe[0]);
        stdinChannel.pipe[0] = -1;
    }

    if (stdinChannel.pipe[1] != -1)
        ::fcntl(stdinChannel.pipe[1], F_SETFL, ::fcntl(stdinChannel.pipe[1], F_GETFL) | O_NONBLOCK);

    if (stdoutChannel.pipe[1] != -1) {
        qt_native_close(stdoutChannel.pipe[1]);
        stdoutChannel.pipe[1] = -1;
    }

    if (stdoutChannel.pipe[0] != -1)
        ::fcntl(stdoutChannel.pipe[0], F_SETFL, ::fcntl(stdoutChannel.pipe[0], F_GETFL) | O_NONBLOCK);

    if (stderrChannel.pipe[1] != -1) {
        qt_native_close(stderrChannel.pipe[1]);
        stderrChannel.pipe[1] = -1;
    }
    if (stderrChannel.pipe[0] != -1)
        ::fcntl(stderrChannel.pipe[0], F_SETFL, ::fcntl(stderrChannel.pipe[0], F_GETFL) | O_NONBLOCK);
}

void QProcessPrivate::execChild(const QByteArray &programName)
{
    ::signal(SIGPIPE, SIG_DFL);         // reset the signal that we ignored

    QByteArray encodedProgramName = programName;
    Q_Q(QProcess);

    // create argument list with right number of elements, and set the
    // final one to 0.
    char **argv = new char *[arguments.count() + 2];
    argv[arguments.count() + 1] = 0;

    // allow invoking of .app bundles on the Mac.
#ifdef Q_OS_MAC
    QFileInfo fileInfo(encodedProgramName);
    if (encodedProgramName.endsWith(".app") && fileInfo.isDir()) {
        QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0,
                                                          QCFString(fileInfo.absoluteFilePath()),
                                                          kCFURLPOSIXPathStyle, true);
        QCFType<CFBundleRef> bundle = CFBundleCreate(0, url);
        url = CFBundleCopyExecutableURL(bundle);
        if (url) {
            QCFString str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            encodedProgramName += "/Contents/MacOS/" + static_cast<QString>(str).toUtf8();
        }
    }
#endif

    // add the program name
    argv[0] = ::strdup(encodedProgramName.constData());

    // add every argument to the list
    for (int i = 0; i < arguments.count(); ++i) {
        QString arg = arguments.at(i);
#ifdef Q_OS_MAC
        argv[i + 1] = ::strdup(arg.toUtf8().constData());
#else
        argv[i + 1] = ::strdup(arg.toLocal8Bit().constData());
#endif
    }

    // copy the stdin socket
    qt_native_dup2(stdinChannel.pipe[0], fileno(stdin));

    // copy the stdout and stderr if asked to
    if (processChannelMode != QProcess::ForwardedChannels) {
        qt_native_dup2(stdoutChannel.pipe[1], fileno(stdout));

        // merge stdout and stderr if asked to
        if (processChannelMode == QProcess::MergedChannels) {
            qt_native_dup2(fileno(stdout), fileno(stderr));
        } else {
            qt_native_dup2(stderrChannel.pipe[1], fileno(stderr));
        }
    }

    // make sure this fd is closed if execvp() succeeds
    qt_native_close(childStartedPipe[0]);
    ::fcntl(childStartedPipe[1], F_SETFD, FD_CLOEXEC);

    // enter the working directory
    if (!workingDirectory.isEmpty())
        qt_native_chdir(QFile::encodeName(workingDirectory).constData());

    // this is a virtual call, and it base behavior is to do nothing.
    q->setupChildProcess();

    // execute the process
    if (environment.isEmpty()) {
        qt_native_execvp(argv[0], argv);
    } else {
        // if LD_LIBRARY_PATH exists in the current environment, but
        // not in the environment list passed by the programmer, then
        // copy it over.
#if defined(Q_OS_MAC)
        static const char libraryPath[] = "DYLD_LIBRARY_PATH";
#else
        static const char libraryPath[] = "LD_LIBRARY_PATH";
#endif
        const QString libraryPathString = QLatin1String(libraryPath);
        QStringList matches = environment.filter(
                QRegExp(QLatin1Char('^') + libraryPathString + QLatin1Char('=')));
        const QString envLibraryPath = QString::fromLocal8Bit(::getenv(libraryPath));
        if (matches.isEmpty() && !envLibraryPath.isEmpty()) {
            QString entry = libraryPathString;
            entry += QLatin1Char('=');
            entry += envLibraryPath;
            environment << libraryPathString + QLatin1Char('=') + envLibraryPath;
        }

        char **envp = new char *[environment.count() + 1];
        envp[environment.count()] = 0;

        for (int j = 0; j < environment.count(); ++j) {
            QString item = environment.at(j);
            envp[j] = ::strdup(item.toLocal8Bit().constData());
        }

        if (!encodedProgramName.contains("/")) {
            const QString path = QString::fromLocal8Bit(::getenv("PATH"));
            if (!path.isEmpty()) {
                QStringList pathEntries = path.split(QLatin1Char(':'));
                for (int k = 0; k < pathEntries.size(); ++k) {
                    QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                    if (!tmp.endsWith('/')) tmp += '/';
                    tmp += encodedProgramName;
                    argv[0] = tmp.data();
#if defined (QPROCESS_DEBUG)
                    fprintf(stderr, "QProcessPrivate::execChild() searching / starting %s\n", argv[0]);
#endif
                    qt_native_execve(argv[0], argv, envp);
                }
            }
        } else {
#if defined (QPROCESS_DEBUG)
            fprintf(stderr, "QProcessPrivate::execChild() starting %s\n", argv[0]);
#endif
            qt_native_execve(argv[0], argv, envp);
        }
    }

    // notify failure
#if defined (QPROCESS_DEBUG)
    fprintf(stderr, "QProcessPrivate::execChild() failed, notifying parent process\n");
#endif
    qt_native_write(childStartedPipe[1], "", 1);
    qt_native_close(childStartedPipe[1]);
    childStartedPipe[1] = -1;
}

bool QProcessPrivate::processStarted()
{
    char c;
    int i = qt_native_read(childStartedPipe[0], &c, 1);
    if (startupSocketNotifier) {
        startupSocketNotifier->setEnabled(false);
        delete startupSocketNotifier;
        startupSocketNotifier = 0;
    }
    qt_native_close(childStartedPipe[0]);
    childStartedPipe[0] = -1;

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::processStarted() == %s", i <= 0 ? "true" : "false");
#endif
    return i <= 0;
}

qint64 QProcessPrivate::bytesAvailableFromStdout() const
{
    size_t nbytes = 0;
    qint64 available = 0;
    if (::ioctl(stdoutChannel.pipe[0], FIONREAD, (char *) &nbytes) >= 0)
        available = (qint64) *((int *) &nbytes);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::bytesAvailableFromStdout() == %lld", available);
#endif
    return available;
}

qint64 QProcessPrivate::bytesAvailableFromStderr() const
{
    size_t nbytes = 0;
    qint64 available = 0;
    if (::ioctl(stderrChannel.pipe[0], FIONREAD, (char *) &nbytes) >= 0)
        available = (qint64) *((int *) &nbytes);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::bytesAvailableFromStderr() == %lld", available);
#endif
    return available;
}

qint64 QProcessPrivate::readFromStdout(char *data, qint64 maxlen)
{
    qint64 bytesRead = qt_native_read(stdoutChannel.pipe[0], data, maxlen);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::readFromStdout(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, bytesRead, 16).constData(), maxlen, bytesRead);
#endif
    return bytesRead;
}

qint64 QProcessPrivate::readFromStderr(char *data, qint64 maxlen)
{
    qint64 bytesRead = qt_native_read(stderrChannel.pipe[0], data, maxlen);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::readFromStderr(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, bytesRead, 16).constData(), maxlen, bytesRead);
#endif
    return bytesRead;
}

static void qt_ignore_sigpipe()
{
    // Set to ignore SIGPIPE once only.
    static QBasicAtomic atom = Q_ATOMIC_INIT(0);
    if (atom.testAndSet(0, 1)) {
        struct sigaction noaction;
        memset(&noaction, 0, sizeof(noaction));
        noaction.sa_handler = SIG_IGN;
        qt_native_sigaction(SIGPIPE, &noaction, 0);
    }
}

qint64 QProcessPrivate::writeToStdin(const char *data, qint64 maxlen)
{
    qt_ignore_sigpipe();

    qint64 written = qt_native_write(stdinChannel.pipe[1], data, maxlen);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::writeToStdin(%p \"%s\", %lld) == %lld",
           data, qt_prettyDebug(data, maxlen, 16).constData(), maxlen, written);
#endif
    return written;
}

void QProcessPrivate::terminateProcess()
{
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::killProcess()");
#endif
    if (pid)
        ::kill(pid_t(pid), SIGTERM);
}

void QProcessPrivate::killProcess()
{
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::killProcess()");
#endif
    if (pid)
        ::kill(pid_t(pid), SIGKILL);
}

static int qt_native_select(fd_set *fdread, fd_set *fdwrite, int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    int ret;
    do {
        ret = select(FD_SETSIZE, fdread, fdwrite, 0, timeout < 0 ? 0 : &tv);
    } while (ret < 0 && (errno == EINTR));
    return ret;
}

/*
   Returns the difference between msecs and elapsed. If msecs is -1,
   however, -1 is returned.
*/
static int qt_timeout_value(int msecs, int elapsed)
{
    if (msecs == -1)
        return -1;

    int timeout = msecs - elapsed;
    return timeout < 0 ? 0 : timeout;
}

bool QProcessPrivate::waitForStarted(int msecs)
{
    Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForStarted(%d) waiting for child to start (fd = %d)", msecs,
	   childStartedPipe[0]);
#endif

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(childStartedPipe[0], &fds);
    int ret;
    do {
        ret = qt_native_select(&fds, 0, msecs);
    } while (ret < 0 && errno == EINTR);
    if (ret == 0) {
        processError = QProcess::Timedout;
        q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
#if defined (QPROCESS_DEBUG)
        qDebug("QProcessPrivate::waitForStarted(%d) == false (timed out)", msecs);
#endif
        return false;
    }

    bool startedEmitted = _q_startupNotification();
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForStarted() == %s", startedEmitted ? "true" : "false");
#endif
    return startedEmitted;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
    Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForReadyRead(%d)", msecs);
#endif

    QTime stopWatch;
    stopWatch.start();

    forever {
        fd_set fdread;
        fd_set fdwrite;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);

        if (processState == QProcess::Starting)
            FD_SET(childStartedPipe[0], &fdread);

        if (stdoutChannel.pipe[0] != -1)
            FD_SET(stdoutChannel.pipe[0], &fdread);
        if (stderrChannel.pipe[0] != -1)
            FD_SET(stderrChannel.pipe[0], &fdread);

        FD_SET(deathPipe[0], &fdread);

        if (!writeBuffer.isEmpty() && stdinChannel.pipe[1] != -1)
            FD_SET(stdinChannel.pipe[1], &fdwrite);

        int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
        int ret = qt_native_select(&fdread, &fdwrite, timeout);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (ret == 0) {
            processError = QProcess::Timedout;
            q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
	    return false;
	}

	if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
            if (!_q_startupNotification())
                return false;
	}

        bool readyReadEmitted = false;
	if (stdoutChannel.pipe[0] != -1 && FD_ISSET(stdoutChannel.pipe[0], &fdread)) {
	    bool canRead = _q_canReadStandardOutput();
            if (processChannel == QProcess::StandardOutput && canRead)
                readyReadEmitted = true;
	}
	if (stderrChannel.pipe[0] != -1 && FD_ISSET(stderrChannel.pipe[0], &fdread)) {
	    bool canRead = _q_canReadStandardError();
            if (processChannel == QProcess::StandardError && canRead)
                readyReadEmitted = true;
	}
        if (readyReadEmitted)
            return true;

	if (stdinChannel.pipe[1] != -1 && FD_ISSET(stdinChannel.pipe[1], &fdwrite))
	    _q_canWrite();

	if (FD_ISSET(deathPipe[0], &fdread)) {
            if (_q_processDied())
                return false;
        }
    }
    return false;
}

bool QProcessPrivate::waitForBytesWritten(int msecs)
{
    Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForBytesWritten(%d)", msecs);
#endif

    QTime stopWatch;
    stopWatch.start();

    while (!writeBuffer.isEmpty()) {
        fd_set fdread;
        fd_set fdwrite;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);

        if (processState == QProcess::Starting)
            FD_SET(childStartedPipe[0], &fdread);

        if (stdoutChannel.pipe[0] != -1)
            FD_SET(stdoutChannel.pipe[0], &fdread);
        if (stderrChannel.pipe[0] != -1)
            FD_SET(stderrChannel.pipe[0], &fdread);

        FD_SET(deathPipe[0], &fdread);

        if (!writeBuffer.isEmpty() && stdinChannel.pipe[1] != -1)
            FD_SET(stdinChannel.pipe[1], &fdwrite);

	int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
	int ret = qt_native_select(&fdread, &fdwrite, timeout);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        if (ret == 0) {
	    processError = QProcess::Timedout;
	    q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
	    return false;
	}

	if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
	    if (!_q_startupNotification())
		return false;
	}

	if (stdinChannel.pipe[1] != -1 && FD_ISSET(stdinChannel.pipe[1], &fdwrite))
	    return _q_canWrite();

	if (stdoutChannel.pipe[0] != -1 && FD_ISSET(stdoutChannel.pipe[0], &fdread))
	    _q_canReadStandardOutput();

	if (stderrChannel.pipe[0] != -1 && FD_ISSET(stderrChannel.pipe[0], &fdread))
	    _q_canReadStandardError();

	if (FD_ISSET(deathPipe[0], &fdread)) {
            if (_q_processDied())
                return false;
        }
    }

    return false;
}

bool QProcessPrivate::waitForFinished(int msecs)
{
    Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
    qDebug("QProcessPrivate::waitForFinished(%d)", msecs);
#endif

    QTime stopWatch;
    stopWatch.start();

    forever {
        fd_set fdread;
        fd_set fdwrite;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);

        if (processState == QProcess::Starting)
            FD_SET(childStartedPipe[0], &fdread);

        if (stdoutChannel.pipe[0] != -1)
            FD_SET(stdoutChannel.pipe[0], &fdread);
        if (stderrChannel.pipe[0] != -1)
            FD_SET(stderrChannel.pipe[0], &fdread);

        if (processState == QProcess::Running)
            FD_SET(deathPipe[0], &fdread);

        if (!writeBuffer.isEmpty() && stdinChannel.pipe[1] != -1)
            FD_SET(stdinChannel.pipe[1], &fdwrite);

	int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
	int ret = qt_native_select(&fdread, &fdwrite, timeout);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
	if (ret == 0) {
	    processError = QProcess::Timedout;
	    q->setErrorString(QLatin1String(QT_TRANSLATE_NOOP(QProcess, "Process operation timed out")));
	    return false;
	}

	if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
	    if (!_q_startupNotification())
		return false;
	}
	if (stdinChannel.pipe[1] != -1 && FD_ISSET(stdinChannel.pipe[1], &fdwrite))
	    _q_canWrite();

	if (stdoutChannel.pipe[0] != -1 && FD_ISSET(stdoutChannel.pipe[0], &fdread))
	    _q_canReadStandardOutput();

	if (stderrChannel.pipe[0] != -1 && FD_ISSET(stderrChannel.pipe[0], &fdread))
	    _q_canReadStandardError();

	if (FD_ISSET(deathPipe[0], &fdread)) {
            if (_q_processDied())
                return true;
	}
    }
    return false;
}

bool QProcessPrivate::waitForWrite(int msecs)
{
    fd_set fdwrite;
    FD_ZERO(&fdwrite);
    FD_SET(stdinChannel.pipe[1], &fdwrite);

    int ret;
    do {
        ret = qt_native_select(0, &fdwrite, msecs < 0 ? 0 : msecs) == 1;
    } while (ret < 0 && errno == EINTR);
    return ret == 1;
}

void QProcessPrivate::findExitCode()
{
    Q_Q(QProcess);
    processManager()->remove(q);
}

bool QProcessPrivate::waitForDeadChild()
{
    Q_Q(QProcess);

    // read a byte from the death pipe
    char c;
    qt_native_read(deathPipe[0], &c, 1);

    // check if our process is dead
    int exitStatus;
    pid_t waitResult = waitpid(pid_t(pid), &exitStatus, WNOHANG);
    if (waitResult > 0) {
        processManager()->remove(q);
        crashed = !WIFEXITED(exitStatus);
        exitCode = WEXITSTATUS(exitStatus);
#if defined QPROCESS_DEBUG
        qDebug() << "QProcessPrivate::waitForDeadChild() dead with exitCode"
                 << exitCode << ", crashed?" << crashed;
#endif
        return true;
    }
#if defined QPROCESS_DEBUG
    qDebug() << "QProcessPrivate::waitForDeadChild() not dead!";
#endif
    return false;
}

void QProcessPrivate::_q_notified()
{
}

/*! \internal
 */
bool QProcessPrivate::startDetached(const QString &program, const QStringList &arguments)
{
    processManager()->start();

    // To catch the startup of the child
    int startedPipe[2];
    ::pipe(startedPipe);

    pid_t childPid = fork();
    if (childPid == 0) {
        ::setsid();
        ::signal(SIGHUP, SIG_IGN);
        qt_native_close(startedPipe[0]);
        ::signal(SIGPIPE, SIG_DFL);

        pid_t doubleForkPid = fork();
        if (doubleForkPid == 0) {
            ::fcntl(startedPipe[1], F_SETFD, FD_CLOEXEC);

            char **argv = new char *[arguments.size() + 2];
            for (int i = 0; i < arguments.size(); ++i) {
#ifdef Q_OS_MAC
                argv[i + 1] = ::strdup(arguments.at(i).toUtf8().constData());
#else
                argv[i + 1] = ::strdup(arguments.at(i).toLocal8Bit().constData());
#endif
            }
            argv[arguments.size() + 1] = 0;

            if (!program.contains(QLatin1Char('/'))) {
                const QString path = QString::fromLocal8Bit(::getenv("PATH"));
                if (!path.isEmpty()) {
                    QStringList pathEntries = path.split(QLatin1Char(':'));
                    for (int k = 0; k < pathEntries.size(); ++k) {
                        QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                        if (!tmp.endsWith('/')) tmp += '/';
                        tmp += QFile::encodeName(program);
                        argv[0] = tmp.data();
                        qt_native_execv(argv[0], argv);
                    }
                }
            } else {
                QByteArray tmp = QFile::encodeName(program);
                argv[0] = tmp.data();
                qt_native_execv(argv[0], argv);
            }

            struct sigaction noaction;
            memset(&noaction, 0, sizeof(noaction));
            noaction.sa_handler = SIG_IGN;
            qt_native_sigaction(SIGPIPE, &noaction, 0);

            // '\1' means execv failed
            char c = '\1';
            qt_native_write(startedPipe[1], &c, 1);
            qt_native_close(startedPipe[1]);
            ::_exit(1);
        } else if (doubleForkPid == -1) {
            struct sigaction noaction;
            memset(&noaction, 0, sizeof(noaction));
            noaction.sa_handler = SIG_IGN;
            qt_native_sigaction(SIGPIPE, &noaction, 0);

            // '\2' means internal error
            char c = '\2';
            qt_native_write(startedPipe[1], &c, 1);
        }

        qt_native_close(startedPipe[1]);
        qt_native_chdir("/");
        ::_exit(1);
    }

    qt_native_close(startedPipe[1]);

    if (childPid == -1) {
        qt_native_close(startedPipe[0]);
        return false;
    }

    char reply = '\0';
    int startResult = qt_native_read(startedPipe[0], &reply, 1);
    int result;
    qt_native_close(startedPipe[0]);
    ::waitpid(childPid, &result, 0);
    return startResult != -1 && reply == '\0';
}

void QProcessPrivate::initializeProcessManager()
{
    (void) processManager();
}

#include "qprocess_unix.moc"

#endif // QT_NO_PROCESS
