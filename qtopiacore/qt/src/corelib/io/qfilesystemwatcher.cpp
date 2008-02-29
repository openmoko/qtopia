/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfilesystemwatcher.h"
#include "qfilesystemwatcher_p.h"

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qdatetime.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qset.h>
#include <qtimer.h>

#if defined(Q_OS_WIN)
#  include "qfilesystemwatcher_win_p.h"
#elif defined(Q_OS_LINUX)
#  include "qfilesystemwatcher_inotify_p.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
#  include "qfilesystemwatcher_kqueue_p.h"
#endif

enum { PollingInterval = 1000 };

class QPollingFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT

    mutable QMutex mutex;
    QHash<QString, QDateTime> files, directories;

public:
    QPollingFileSystemWatcherEngine();
    
    void run();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private slots:
    void timeout();
};

QPollingFileSystemWatcherEngine::QPollingFileSystemWatcherEngine()
{
    moveToThread(this);
}

void QPollingFileSystemWatcherEngine::run()
{
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), SLOT(timeout()));
    timer.start(PollingInterval);
    (void) exec();
}

QStringList QPollingFileSystemWatcherEngine::addPaths(const QStringList &paths,
                                                      QStringList *files,
                                                      QStringList *directories)
{
    QMutexLocker locker(&mutex);
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        if (!fi.exists())
            continue;
        if (fi.isDir()) {
            if (!directories->contains(path))
                directories->append(path);
            this->directories.insert(path, fi.lastModified());
        } else {
            if (!files->contains(path))
                files->append(path);
            this->files.insert(path, fi.lastModified());
        }
        it.remove();
    }
    start();
    return p;
}

QStringList QPollingFileSystemWatcherEngine::removePaths(const QStringList &paths,
                                                         QStringList *files,
                                                         QStringList *directories)
{
    QMutexLocker locker(&mutex);
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        if (this->directories.remove(path)) {
            directories->removeAll(path);
            it.remove();
        } else if (this->files.remove(path)) {
            files->removeAll(path);
            it.remove();
        }
    }
    if (this->files.isEmpty() && this->directories.isEmpty()) {
        locker.unlock();
        stop();
        wait();
    }
    return p;
}

void QPollingFileSystemWatcherEngine::stop()
{
    QMetaObject::invokeMethod(this, "quit");
}

void QPollingFileSystemWatcherEngine::timeout()
{
    QMutexLocker locker(&mutex);
    QMutableHashIterator<QString, QDateTime> fit(files);
    while (fit.hasNext()) {
        QHash<QString, QDateTime>::iterator x = fit.next();
        QString path = x.key();
        QFileInfo fi(path);
        if (!fi.exists()) {
            fit.remove();
            emit fileChanged(path, true);
        } else if (x.value() != fi.lastModified()) {
            x.value() = fi.lastModified();
            emit fileChanged(path, false);
        }
    }
    QMutableHashIterator<QString, QDateTime> dit(directories);
    while (dit.hasNext()) {
        QHash<QString, QDateTime>::iterator x = dit.next();
        QString path = x.key();
        QFileInfo fi(path);
        if (!fi.exists()) {
            dit.remove();
            emit directoryChanged(path, true);
        } else if (x.value() != fi.lastModified()) {
            x.value() = fi.lastModified();
            emit directoryChanged(path, false);
        }
    }
}




QFileSystemWatcherEngine *QFileSystemWatcherPrivate::createNativeEngine()
{
#if defined(Q_OS_WIN)
    return new QWindowsFileSystemWatcherEngine;
#elif defined(Q_OS_LINUX)
    return QInotifyFileSystemWatcherEngine::create();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
    return QKqueueFileSystemWatcherEngine::create();
#else
    return 0;
#endif
}

QFileSystemWatcherPrivate::QFileSystemWatcherPrivate()
    : native(0), poller(0)
{
}

void QFileSystemWatcherPrivate::init()
{
    Q_Q(QFileSystemWatcher);
    native = createNativeEngine();
    if (native) {
        QObject::connect(native,
                         SIGNAL(fileChanged(QString,bool)),
                         q,
                         SLOT(_q_fileChanged(QString,bool)));
        QObject::connect(native,
                         SIGNAL(directoryChanged(QString,bool)),
                         q,
                         SLOT(_q_directoryChanged(QString,bool)));
    }
}

void QFileSystemWatcherPrivate::_q_fileChanged(const QString &path, bool removed)
{
    Q_Q(QFileSystemWatcher);
    if (!files.contains(path)) {
        // the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed)
        files.removeAll(path);
    emit q->fileChanged(path);
}

void QFileSystemWatcherPrivate::_q_directoryChanged(const QString &path, bool removed)
{
    Q_Q(QFileSystemWatcher);
    if (!directories.contains(path)) {
        // perhaps the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed)
        directories.removeAll(path);
    emit q->directoryChanged(path);
}



/*!
    \class QFileSystemWatcher
    \brief The QFileSystemWatcher class provides an interface for monitoring files and directories for modifications.
    \ingroup io
    \since 4.2
    \reentrant

    QFileSystemWatcher monitors the file system for changes to files
    and directories by watching a list of specified paths.

    Call addPath() to watch a particular file or directory. Multiple
    paths can be added using the addPaths() function. Existing paths can
    be removed by using the removePath() and removePaths() functions.

    QFileSystemWatcher examines each path added to it. Files that have
    been added to the QFileSystemWatcher can be accessed using the
    files() function, and directories using the directories() function.

    The fileChanged() signal is emitted when a file has been modified
    or removed from disk. Similarly, the directoryChanged() signal
    is emitted when a directory is modified or removed. Note that
    QFileSystemWatcher stops monitoring files and directories once they
    have been removed from disk.

    \sa QFile, QDir
*/


/*!
    Constructs a new file system watcher object with the given \a parent.
*/
QFileSystemWatcher::QFileSystemWatcher(QObject *parent)
    : QObject(*new QFileSystemWatcherPrivate, parent)
{
    d_func()->init();
}

/*!
    Constructs a new file system watcher object with the given \a parent
    which monitors the specified \a paths list.
*/
QFileSystemWatcher::QFileSystemWatcher(const QStringList &paths, QObject *parent)
    : QObject(*new QFileSystemWatcherPrivate, parent)
{
    d_func()->init();
    addPaths(paths);
}

/*!
    Destroys the file system watcher.
*/
QFileSystemWatcher::~QFileSystemWatcher()
{
    Q_D(QFileSystemWatcher);
    if (d->native) {
        d->native->stop();
        d->native->wait();
        delete d->native;
        d->native = 0;
    }
    if (d->poller) {
        d->poller->stop();
        d->poller->wait();
        delete d->poller;
        d->poller = 0;
    }
}

/*!
    Adds \a path to the file system watcher if \a path exists. The path is
    not added if it does not exist, or if it is already being monitored by
    the file system watcher.

    If \a path specifies a directory, the directoryChanged() signal
    will be emitted when \a path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when \a path is
    modified or removed.

    \sa addPaths(), removePath()
*/
void QFileSystemWatcher::addPath(const QString &path)
{
    addPaths(QStringList(path));
}

/*!
    Adds each path in \a paths to the file system watcher. Paths are not
    added if they not exist, or if they are already being monitored by the
    file system watcher.

    If a path specifies a directory, the directoryChanged() signal will
    be emitted when the path is modified or removed from disk; otherwise
    the fileChanged() signal is emitted when the path is modified or
    removed.

    \sa addPath(), removePaths()
*/
void QFileSystemWatcher::addPaths(const QStringList &paths)
{
    Q_D(QFileSystemWatcher);
    if (paths.isEmpty())
        return;
    QStringList p = paths;
    if (objectName() != QLatin1String("_qt_autotest_force_engine_poller")) {
        if (d->native)
            p = d->native->addPaths(p, &d->files, &d->directories);
        if (p.isEmpty())
            return;
    } else {
        qDebug() << "QFileSystemWatcher: skipping native engine, using only polling engine";
    }
    if (objectName() != QLatin1String("_qt_autotest_force_engine_native")) {
        // try polling instead
        if (!d->poller) {
            d->poller = new QPollingFileSystemWatcherEngine; // that was a mouthful
            QObject::connect(d->poller,
                             SIGNAL(fileChanged(QString,bool)),
                             this,
                             SLOT(_q_fileChanged(QString,bool)));
            QObject::connect(d->poller,
                             SIGNAL(directoryChanged(QString,bool)),
                             this,
                             SLOT(_q_directoryChanged(QString,bool)));
        }
        p = d->poller->addPaths(p, &d->files, &d->directories);
    } else{
        qDebug("QFileSystemWatcher: skipping polling engine, using only native engine");
    }
    if (!p.isEmpty())
        qWarning("QFileSystemWatcher: failed to add paths: %s",
                 qPrintable(p.join(", ")));
}

/*!
    Removes the specified \a path from the file system watcher.

    \sa removePaths(), addPath()
*/
void QFileSystemWatcher::removePath(const QString &path)
{
    removePaths(QStringList(path));
}

/*!
    Removes the specified \a paths from the file system watcher.

    \sa removePath(), addPaths()
*/
void QFileSystemWatcher::removePaths(const QStringList &paths)
{
    Q_D(QFileSystemWatcher);
    QStringList p = paths;
    if (d->native)
        p = d->native->removePaths(p, &d->files, &d->directories);
    if (d->poller)
        (void) d->poller->removePaths(p, &d->files, &d->directories);
}

/*!
    \fn void QFileSystemWatcher::fileChanged(const QString &path)

    This signal is emitted when the file at the specified \a path is modified
    or removed from disk.

    \sa directoryChanged()
*/

/*!
    \fn void QFileSystemWatcher::directoryChanged(const QString &path)

    This signal is emitted when the directory at the specified \a path is
    modified or removed from disk.

    \sa fileChanged()
*/

/*!
    \fn QStringList QFileSystemWatcher::directories() const

    Returns a list of paths to directories that are being watched.

    \sa files()
*/

/*!
    \fn QStringList QFileSystemWatcher::files() const

    Returns a list of paths to files that are being watched.

    \sa directories()
*/

QStringList QFileSystemWatcher::directories() const
{
    Q_D(const QFileSystemWatcher);
    return d->directories;
}

QStringList QFileSystemWatcher::files() const
{
    Q_D(const QFileSystemWatcher);
    return d->files;
}

#include "moc_qfilesystemwatcher.cpp"
#include "qfilesystemwatcher.moc"

#endif // QT_NO_FILESYSTEMWATCHER
