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

#include "qfsfileengine_p.h"
#include "qdatetime.h"

#include <errno.h>
#include <stdio.h>

#ifdef Q_OS_WIN
#  ifndef S_ISREG
#    define S_ISREG(x)   (((x) & S_IFMT) == S_IFREG)
#  endif
#  ifndef S_ISCHR
#    define S_ISCHR(x)   (((x) & S_IFMT) == S_IFCHR)
#  endif
#  ifndef S_ISFIFO
#    define S_ISFIFO(x) false
#  endif
#  ifndef S_ISSOCK
#    define S_ISSOCK(x) false
#  endif
#endif

/*! \class QFSFileEngine
    \brief The QFSFileEngine class implements Qt's default file engine.
    \since 4.1

    This class is part of the file engine framework in Qt. If you only want to
    access files or directories, use QFile, QFileInfo or QDir instead.

    QFSFileEngine is the default file engine for accessing regular files. It
    is provided for convenience; by subclassing this class, you can alter its
    behavior slightly, without having to write a complete QAbstractFileEngine
    subclass. To install your custom file engine, you must also subclass
    QAbstractFileEngineHandler and create an instance of your handler.

    It can also be useful to create a QFSFileEngine object directly if you
    need to use the local file system inside QAbstractFileEngine::create(), in
    order to avoid recursion (as higher-level classes tend to call
    QAbstractFileEngine::create()).
*/

//**************** QFSFileEnginePrivate
QFSFileEnginePrivate::QFSFileEnginePrivate() : QAbstractFileEnginePrivate()
{
    sequential = 0;
    tried_stat = 0;
#ifdef Q_OS_UNIX
    need_lstat = 1;
    is_link = 0;
#endif
    is_readonly = 0;
    fd = -1;
    fh = 0;
    lastIOCommand = IOFlushCommand;
    lastFlushFailed = false;
    closeFileHandle = false;
    init();
}

/*!
    Constructs a QFSFileEngine for the file name \a file.
*/
QFSFileEngine::QFSFileEngine(const QString &file) : QAbstractFileEngine(*new QFSFileEnginePrivate)
{
    Q_D(QFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
}

/*!
    Constructs a QFSFileEngine.
*/
QFSFileEngine::QFSFileEngine() : QAbstractFileEngine(*new QFSFileEnginePrivate)
{
}

/*!
    \internal
*/
QFSFileEngine::QFSFileEngine(QFSFileEnginePrivate &dd)
    : QAbstractFileEngine(dd)
{
}

/*!
    Destructs the QFSFileEngine.
*/
QFSFileEngine::~QFSFileEngine()
{
    Q_D(QFSFileEngine);
    if (d->closeFileHandle) {
        if (d->fh) {
            int ret;
            do {
                ret = fclose(d->fh);
            } while (ret == EOF && errno == EINTR);
        } else if (d->fd != -1) {
            int ret;
            do {
                ret = QT_CLOSE(d->fd);
            } while (ret == -1 && errno == EINTR);
        }
    }
}

/*!
    \reimp
*/
void QFSFileEngine::setFileName(const QString &file)
{
    Q_D(QFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
    d->tried_stat = 0;
}

static QByteArray openModeToFopenMode(QIODevice::OpenMode flags, const QString &fileName = QString())
{
    QByteArray mode;
    if ((flags & QIODevice::ReadOnly) && !(flags & QIODevice::Truncate)) {
        mode = "rb";
        if (flags & QIODevice::WriteOnly) {
            if (!fileName.isEmpty() &&QFile::exists(fileName))
                mode = "rb+";
            else
                mode = "wb+";
        }
    } else if (flags & QIODevice::WriteOnly) {
        mode = "wb";
        if (flags & QIODevice::ReadOnly)
            mode += "+";
    }
    if (flags & QIODevice::Append) {
        mode = "ab";
        if (flags & QIODevice::ReadOnly)
            mode += "+";
    }
    return mode;
}

/*!
    \reimp
*/
bool QFSFileEngine::open(QIODevice::OpenMode flags)
{
    Q_D(QFSFileEngine);
    d->lastFlushFailed = false;

    if (d->file.isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        setError(QFile::OpenError, QLatin1String("No file name specified"));
        return false;
    }

    if (flags & QFile::Append)
        flags |= QFile::WriteOnly;

#ifdef Q_OS_WIN
    if (d->file.startsWith("//./")) {
#endif
    do {
#if defined(Q_OS_WIN) && defined(_MSC_VER) && _MSC_VER >= 1400
        d->fh = 0;
        fopen_s(&d->fh, QFile::encodeName(QDir::toNativeSeparators(d->file)).constData(),
                         openModeToFopenMode(flags, d->file).constData());
#else
        d->fh = QT_FOPEN(QFile::encodeName(QDir::toNativeSeparators(d->file)).constData(),
                         openModeToFopenMode(flags, d->file).constData());
#endif
    } while (!d->fh && errno == EINTR);
    if (!d->fh) {
        setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                 qt_error_string(int(errno)));
        return false;
    }

    if ((flags & QIODevice::WriteOnly) == 0)
        d->is_readonly = 1;

    if (flags & QIODevice::Unbuffered)
        setvbuf(d->fh, 0, _IONBF, 0);

    if (flags & QIODevice::Append) {
        int ret;
        do {
            ret = QT_FSEEK(d->fh, 0, SEEK_END);
        } while (ret == -1 && errno == EINTR);
    }

    d->closeFileHandle = true;
    d->fd = QT_FILENO(d->fh);

    QT_STATBUF st;
    if (QT_FSTAT(QT_FILENO(d->fh), &st) != 0)
#ifdef Q_OS_UNIX
        return false;
#endif
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
    return true;

#ifdef Q_OS_WIN
    } else {
#endif

    int oflags = QT_OPEN_RDONLY;
    if ((flags & QFile::ReadWrite) == QFile::ReadWrite) {
        oflags = QT_OPEN_RDWR | QT_OPEN_CREAT;
    } else if (flags & QFile::WriteOnly) {
        oflags = QT_OPEN_WRONLY | QT_OPEN_CREAT;
    }

    if (flags & QFile::Append) {
        oflags |= QT_OPEN_APPEND;
    } else if (flags & QFile::WriteOnly) {
        if ((flags & QFile::Truncate) || !(flags & QFile::ReadOnly))
            oflags |= QT_OPEN_TRUNC;
    }

#if defined(Q_OS_MSDOS) || defined(Q_OS_WIN32) || defined(Q_OS_OS2)
    oflags |= QT_OPEN_BINARY; // we handle all text translations our self.
#endif

    d->fd = d->sysOpen(d->file, oflags);
    if(d->fd != -1) {
        // Before appending, seek to the end of the file to allow
        // at() to return the correct position before ::write()
        //  has been called.
        if (flags & QFile::Append)
            QT_LSEEK(d->fd, 0, SEEK_END);

        d->closeFileHandle = true;
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
        if ((st.st_mode & S_IFMT) != S_IFREG)
            d->sequential = 1;
        return true;
    }
    setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError, qt_error_string(errno));
    return false;

#ifdef Q_OS_WIN
    }
#endif
}

/*!
    Opens the file descriptor \a fd to the file engine, using the open mode \a
    flags.
*/
bool QFSFileEngine::open(QIODevice::OpenMode flags, int fd)
{
    Q_D(QFSFileEngine);
    d->lastFlushFailed = false;

    d->closeFileHandle = false;
#ifdef Q_OS_UNIX
    d->fh = fdopen(fd, openModeToFopenMode(flags).constData());
    if (!d->fh) {
        setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                 qt_error_string(int(errno)));
        return false;
    }

    if ((flags & QIODevice::WriteOnly) == 0)
        d->is_readonly = 1;

    if (flags & QIODevice::Unbuffered)
        setvbuf(d->fh, 0, _IONBF, 0);

    if (flags & QIODevice::Append) {
        int ret;
        do {
            ret = QT_FSEEK(d->fh, 0, SEEK_END);
        } while (ret == -1 && errno == EINTR);
    }

    d->fd = QT_FILENO(d->fh);

    QT_STATBUF st;
    if (QT_FSTAT(QT_FILENO(d->fh), &st) != 0)
        return false;
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
    return true;
#else
    Q_UNUSED(flags);
    d->fd = fd;
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
        if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG || !fd) //stdin is non seekable
            d->sequential = 1;
        return true;
    }
    return false;
#endif
}

/*!
    Opens the file handle \a fh using the open mode \a flags.
*/
bool QFSFileEngine::open(QIODevice::OpenMode flags, FILE *fh)
{
    Q_D(QFSFileEngine);
    d->lastFlushFailed = false;

    Q_UNUSED(flags);
    d->fh = fh;
    d->fd = QT_FILENO(fh);
    QT_STATBUF st;
    if (QT_FSTAT(QT_FILENO(fh), &st) != 0)
        return false;
#ifdef Q_OS_WIN32
    HANDLE hnd = (HANDLE)_get_osfhandle(d->fd);
    if (hnd == INVALID_HANDLE_VALUE)
        return false;

    DWORD ftype = ::GetFileType(hnd);
    d->sequential = ftype == FILE_TYPE_CHAR || ftype == FILE_TYPE_PIPE;
#else
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
#endif
    d->closeFileHandle = false;
#ifdef Q_OS_UNIX
    if ((flags & QIODevice::WriteOnly) == 0)
        d->is_readonly = 1;
#endif
    return true;
}

/*!
    \reimp
*/
bool QFSFileEngine::close()
{
    Q_D(QFSFileEngine);

    bool flushed = !d->lastFlushFailed && flush();

    d->tried_stat = 0;
    if (d->fh) {
        bool closed = true;
        if (d->closeFileHandle) {
            int ret;
            do {
                ret = fclose(d->fh);
            } while (ret == EOF && errno == EINTR);
            closed = (ret == 0);
        }
        d->fh = 0;
        d->fd = -1;
        return flushed && closed;
    }

    if (d->fd == -1)
        return false;

    int ret;
    if (d->closeFileHandle) {
        do {
            ret = QT_CLOSE(d->fd);
        } while (ret == -1 && errno == EINTR);
    } else {
        ret = 0;
    }
    d->fd = -1;
    if (ret == -1) {
        setError(QFile::UnspecifiedError, qt_error_string(errno));
        return false;
    }
    return true;
}

/*!
    \reimp
*/
bool QFSFileEngine::flush()
{
    Q_D(QFSFileEngine);
#ifdef Q_OS_UNIX
    if (d->is_readonly)
        return true;
#endif

    d->ungetchBuffer.clear();
    if (d->lastFlushFailed)
        return false;

    if (!d->fh) {
        // There's no write buffer when using an fd.
        return d->fd != -1;
    }

#ifdef Q_OS_WIN
    QT_FPOS_T pos;
    int gotPos = QT_FGETPOS(d->fh, &pos);
#endif
    int ret = fflush(d->fh);
    d->lastFlushFailed = (ret != 0);

#ifdef Q_OS_WIN
    if (gotPos == 0) {
        int ret;
        do {
            ret = QT_FSETPOS(d->fh, &pos);
        } while (ret == -1 && errno == EINTR);
    }
#endif
    d->lastIOCommand = QFSFileEnginePrivate::IOFlushCommand;
    if (ret != 0) {
        setError(errno == ENOSPC ? QFile::ResourceError : QFile::WriteError,
                 qt_error_string(errno));
        return false;
    }
    return true;
}

/*!
    \reimp
*/
qint64 QFSFileEngine::read(char *data, qint64 len)
{
    Q_D(QFSFileEngine);

    if (d->fh) {
        if (d->lastIOCommand != QFSFileEnginePrivate::IOReadCommand) {
            flush();
            d->lastIOCommand = QFSFileEnginePrivate::IOReadCommand;
        }

        if (feof(d->fh))
            return 0;

        size_t readBytes = 0;
#ifdef Q_OS_UNIX
        if (d->sequential) {
            int oldFlags = fcntl(fileno(d->fh), F_GETFL);

            for (int i = 0; i < 2; ++i) {
                // Make the underlying file descriptor non-blocking
                int v = 1;
                if ((oldFlags & O_NONBLOCK) == 0)
                    fcntl(fileno(d->fh), F_SETFL, oldFlags | O_NONBLOCK, &v, sizeof(v));

                size_t read = 0;
                do {
                    read = fread(data + readBytes, 1, size_t(len - readBytes), d->fh);
                } while (read == 0 && !feof(d->fh) && errno == EINTR);
                if (read > 0) {
                    readBytes += read;
                    break;
                } else {
                    if (readBytes)
                        break;
                    readBytes = read;
                }

                // Restore the blocking state of the underlying socket
                if ((oldFlags & O_NONBLOCK) == 0) {
                    int v = 1;
                    fcntl(fileno(d->fh), F_SETFL, oldFlags, &v, sizeof(v));
                    if (readBytes == 0) {
                        int readByte = 0;
                        do {
                            readByte = fgetc(d->fh);
                        } while (readByte == -1 && errno == EINTR);
                        if (readByte != -1) {
                            *data = uchar(readByte);
                            readBytes += 1;
                        } else {
                            break;
                        }
                    }
                }
            }
            if ((oldFlags & O_NONBLOCK) == 0) {
                int v = 1;
                fcntl(fileno(d->fh), F_SETFL, oldFlags, &v, sizeof(v));
            }
        } else
#endif
        do {
            readBytes = fread(data, 1, size_t(len), d->fh);
        } while (readBytes == 0 && !feof(d->fh) && errno == EINTR);
        qint64 ret = qint64(readBytes);
        if (ret == 0) {
            setError(QFile::ReadError, qt_error_string(int(errno)));
            if (!feof(d->fh))
                ret = -1;
        }
        return ret;
    }

    qint64 ret = 0;
    if (!d->ungetchBuffer.isEmpty()) {
        qint64 l = d->ungetchBuffer.size();
        while(ret < l) {
            *data = d->ungetchBuffer.at(l - ret - 1);
            data++;
            ret++;
        }
        d->ungetchBuffer.resize(l - ret);
        len -= ret;
    }
    if(len && ret != len) {
        int result;
        qint64 read = 0;
        do {
            qint64 bytesToRead = len - read;
#ifdef Q_OS_WIN
            // Reading on Windows fails with ERROR_NO_SYSTEM_RESOURCES
            // when the chunks are too large, so we limit the block
            // size to 32MB.
            const qint64 MaxBlockSize = 32 * 1024 * 1024;
            bytesToRead = qMin(bytesToRead, MaxBlockSize);
#endif
            do {
                result = QT_READ(d->fd, data + read, int(bytesToRead));
            } while (result == -1 && errno == EINTR);
            if (result > 0)
                read += result;
        } while (result > 0 && read < len);
        if (read > 0) {
            ret += read;
        } else {
            if (!ret)
                ret = -1;
            setError(QFile::ReadError, qt_error_string(errno));
        }
    }
    return ret;
}

/*!
    \reimp
*/
qint64 QFSFileEngine::readLine(char *data, qint64 maxlen)
{
    Q_D(QFSFileEngine);
    if (!d->fh)
        return QAbstractFileEngine::readLine(data, maxlen);

    if (d->lastIOCommand != QFSFileEnginePrivate::IOReadCommand) {
        flush();
        d->lastIOCommand = QFSFileEnginePrivate::IOReadCommand;
    }
    if (feof(d->fh))
        return 0;

    // QIODevice::readLine() passes maxlen - 1 to QFile::readLineData()
    // because it has made space for the '\0' at the end of data.  But fgets
    // does the same, so we'd get two '\0' at the end - passing maxlen + 1
    // solves this.
    if (!fgets(data, int(maxlen + 1), d->fh)) {
        setError(QFile::ReadError, qt_error_string(int(errno)));
        return 0;
    }
    return qstrlen(data);
}

/*!
    \reimp
*/
qint64 QFSFileEngine::write(const char *data, qint64 len)
{
    Q_D(QFSFileEngine);

    if (d->fh) {
        if (d->lastIOCommand != QFSFileEnginePrivate::IOWriteCommand) {
            flush();
            d->lastIOCommand = QFSFileEnginePrivate::IOWriteCommand;
        }
    }

    qint64 result;
    qint64 written = 0;
#ifdef Q_OS_WIN
    qint64 posbefore;
    if (!d->sequential) {
        posbefore = pos();
        if (posbefore == -1) {
            setError(QFile::WriteError, qt_error_string(errno));
            return qint64(-1);
        }
    }
#endif
    do {
        qint64 bytesToWrite = len - written;
#ifdef Q_OS_WIN
        // Writing on Windows fails with ERROR_NO_SYSTEM_RESOURCES
        // when the chunks are too large, so we limit the block size
        // to 32MB.
        const qint64 MaxChunkSize = 32 * 1024 * 1024;
        bytesToWrite = qMin<qint64>(bytesToWrite, MaxChunkSize);
#endif
        if (d->fh) {
            do {
                result = qint64(fwrite(data + written, 1, size_t(bytesToWrite), d->fh));
            } while (result == 0 && errno == EINTR);
            if (bytesToWrite > 0 && result == 0)
                result = -1;
        } else {
            do {
                result = QT_WRITE(d->fd, data + written, bytesToWrite);
            } while (result == -1 && errno == EINTR);
        }
        if (result > 0)
            written += qint64(result);
    } while (written < len && ((result > 0 || (result == 0 && errno == EINTR))));

#ifdef Q_OS_WIN
    if (!d->sequential) {
        qint64 currentpos = pos();
        if (currentpos == -1) {
            result = -1;
        } else {
            written = currentpos - posbefore;
        }
    }
#endif
    if (result > 0)
        return written;
    setError(errno == ENOSPC ? QFile::ResourceError : QFile::WriteError, qt_error_string(errno));
    return qint64(result);
}

/*!
    \reimp
*/
qint64 QFSFileEngine::pos() const
{
    Q_D(const QFSFileEngine);
    if (d->fh)
        return qint64(QT_FTELL(d->fh));
    return QT_LSEEK(d->fd, 0, SEEK_CUR);
}

/*!
    \reimp
*/
int QFSFileEngine::handle() const
{
    Q_D(const QFSFileEngine);
    return d->fd;
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *QFSFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    Q_UNUSED(filters);
    Q_UNUSED(filterNames);
    return 0;
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *QFSFileEngine::endEntryList()
{
    return 0;
}

/*!
    \reimp
*/
bool QFSFileEngine::seek(qint64 pos)
{
    Q_D(QFSFileEngine);
    if (d->lastIOCommand != QFSFileEnginePrivate::IOFlushCommand && !flush())
        return false;

    if (d->fh) {
        int ret;
        do {
            ret = QT_FSEEK(d->fh, QT_OFF_T(pos), SEEK_SET);
        } while (ret == -1 && errno == EINTR);
        if (ret == -1) {
            setError(QFile::ReadError, qt_error_string(int(errno)));
            return false;
        }
        return true;
    }

    if(QT_LSEEK(d->fd, pos, SEEK_SET) == -1) {
        qWarning("QFile::at: Cannot set file position %lld", pos);
        setError(QFile::PositionError, qt_error_string(errno));
        return false;
    }
    d->ungetchBuffer.clear();
    return true;
}

/*!
    \reimp
*/
bool QFSFileEngine::isSequential() const
{
    Q_D(const QFSFileEngine);
    return d->sequential;
}

/*!
    \reimp
*/
bool QFSFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    Q_UNUSED(extension);
    Q_UNUSED(option);
    Q_UNUSED(output);
    return false;
}

/*!
    \reimp
*/
bool QFSFileEngine::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}
