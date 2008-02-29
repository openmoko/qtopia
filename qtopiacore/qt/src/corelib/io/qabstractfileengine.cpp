/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractfileengine.h"
#include "private/qabstractfileengine_p.h"
#include "qdatetime.h"
#include "qmutex.h"
// built-in handlers
#include "qfsfileengine.h"

/*!
    \class QAbstractFileEngineHandler
    \reentrant

    \brief The QAbstractFileEngineHandler class provides a way to register
    custom file engines with your application.

    \ingroup io
    \since 4.1

    QAbstractFileEngineHandler is a factory for creating QAbstractFileEngine
    objects (file engines), which are used internally by QFile, QFileInfo, and
    QDir when working with files and directories.

    When you open a file, Qt chooses a suitable file engine by passing the
    file name from QFile or QDir through an internal list of registered file
    engine handlers. The first handler to recognize the file name is used to
    create the engine. Qt provides internal file engines for working with
    regular files and resources, but you can also register your own
    QAbstractFileEngine subclasses.

    To install an application-specific file engine, you subclass
    QAbstractFileEngineHandler and reimplement create(). When you instantiate
    the handler (e.g. by creating an instance on the stack or on the heap), it
    will automatically register with Qt. (The latest registered handler takes
    precedence over existing handlers.)

    For example:

    \code
        class ZipEngineHandler : public QAbstractFileEngineHandler
        {
        public:
            QAbstractFileEngine *create(const QString &fileName) const;
        };

        QAbstractFileEngine *ZipEngineHandler::create(const QString &fileName) const
        {
            // ZipEngineHandler returns a ZipEngine for all .zip files
            return fileName.toLower().endsWith(".zip") ? new ZipEngine(fileName) : 0;
        }

        int main(int argc, char **argv)
        {
            QApplication app(argc, argv);

            ZipEngineHandler engine;

            MainWindow window;
            window.show();

            return app.exec();
        }
    \endcode

    When the handler is destroyed, it is automatically removed from Qt.

    The most common approach to registering a handler is to create an instance
    as part of the start-up phase of your application. It is also possible to
    limit the scope of the file engine handler to a particular area of
    interest (e.g. a special file dialog that needs a custom file engine). By
    creating the handler inside a local scope, you can precisely control the
    area in which your engine will be applied without disturbing file
    operations in other parts of your application.

    \sa QAbstractFileEngine, QAbstractFileEngine::create()
*/

/*! \typedef QAbstractFileEngine::Iterator
    \internal
*/

/*
    All application-wide handlers are stored in this list. The mutex must be
    acquired to ensure thread safety.
 */
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, fileEngineHandlerMutex, (QMutex::Recursive))
static bool qt_abstractfileenginehandlerlist_shutDown = false;
class QAbstractFileEngineHandlerList : public QList<QAbstractFileEngineHandler *>
{
public:
    ~QAbstractFileEngineHandlerList()
    {
        QMutexLocker locker(fileEngineHandlerMutex());
        qt_abstractfileenginehandlerlist_shutDown = true;
    }
};
Q_GLOBAL_STATIC(QAbstractFileEngineHandlerList, fileEngineHandlers)

/*!
    Constructs a file handler and registers it with Qt. Once created this
    handler's create() function will be called (along with all the other
    handlers) for any paths used. The most recently created handler that
    recognizes the given path (i.e. that returns a QAbstractFileEngine) is
    used for the new path.

    \sa create()
 */
QAbstractFileEngineHandler::QAbstractFileEngineHandler()
{
    QMutexLocker locker(fileEngineHandlerMutex());
    fileEngineHandlers()->prepend(this);
}

/*!
    Destroys the file handler. This will automatically unregister the handler
    from Qt.
 */
QAbstractFileEngineHandler::~QAbstractFileEngineHandler()
{
    QMutexLocker locker(fileEngineHandlerMutex());
    // Remove this handler from the handler list only if the list is valid.
    if (!qt_abstractfileenginehandlerlist_shutDown)
        fileEngineHandlers()->removeAll(this);
}

/*!
    \fn QAbstractFileEngine *QAbstractFileEngineHandler::create(const QString &fileName) const

    Creates a file engine for file \a fileName. Returns 0 if this
    file handler cannot handle \a fileName.

    Example:

    \code
        QAbstractSocketEngine *ZipEngineHandler::create(const QString &fileName) const
        {
            // ZipEngineHandler returns a ZipEngine for all .zip files
            return fileName.toLower().endsWith(".zip") ? new ZipEngine(fileName) : 0;
        }
    \endcode

    \sa QAbstractFileEngine::create()
*/    

/*!
    Creates and returns a QAbstractFileEngine suitable for processing \a
    fileName.

    You should not need to call this function; use QFile, QFileInfo or QDir
    directly instead.

    \sa QAbstractFileEngineHandler
*/
QAbstractFileEngine *QAbstractFileEngine::create(const QString &fileName)
{
    QMutexLocker locker(fileEngineHandlerMutex());

    // check for registered handlers that can load the file
    for (int i = 0; i < fileEngineHandlers()->size(); i++) {
        if (QAbstractFileEngine *ret = fileEngineHandlers()->at(i)->create(fileName))
            return ret;
    }

    // fall back to regular file engine
    return new QFSFileEngine(fileName);
}

/*!
    \class QAbstractFileEngine
    \reentrant

    \brief The QAbstractFileEngine class provides an abstraction for accessing
    the filesystem.

    \ingroup io
    \since 4.1

    The QDir, QFile, and QFileInfo classes all make use of a
    QAbstractFileEngine internally. If you create your own QAbstractFileEngine
    subclass (and register it with Qt by creating a QAbstractFileEngineHandler
    subclass), your file engine will be used when the path is one that your
    file engine handles.

    A QAbstractFileEngine refers to one file or one directory. If the referent
    is a file, the setFileName(), rename(), and remove() functions are
    applicable. If the referent is a directory the mkdir(), rmdir(), and
    entryList() functions are applicable. In all cases the caseSensitive(),
    isRelativePath(), fileFlags(), ownerId(), owner(), and fileTime()
    functions are applicable.

    A QAbstractFileEngine subclass can be created to do synchronous network I/O
    based file system operations, local file system operations, or to operate
    as a resource system to access file based resources.

   \sa QAbstractFileEngineHandler
*/

/*!
    \enum QAbstractFileEngine::FileName

    These values are used to request a file name in a particular
    format.

    \value DefaultName The same filename that was passed to the
    QAbstractFileEngine.
    \value BaseName The name of the file excluding the path.
    \value PathName The path to the file excluding the base name.
    \value AbsoluteName The absolute path to the file (including
    the base name).
    \value AbsolutePathName The absolute path to the file (excluding
    the base name).
    \value LinkName The full file name of the file that this file is a
link to. (This will be empty if this file is not a link.)
    \value CanonicalName Often very similar to LinkName. Will return the true path to the file.
    \value CanonicalPathName Same as CanonicalName, excluding the base name.

    \sa fileName(), setFileName()
*/

/*!
    \enum QAbstractFileEngine::FileFlag

    The permissions and types of a file, suitable for OR'ing together.

    \value ReadOwnerPerm The owner of the file has permission to read
    it.
    \value WriteOwnerPerm The owner of the file has permission to
    write to it.
    \value ExeOwnerPerm The owner of the file has permission to
    execute it.
    \value ReadUserPerm The current user has permission to read the
    file.
    \value WriteUserPerm The current user has permission to write to
    the file.
    \value ExeUserPerm The current user has permission to execute the
    file.
    \value ReadGroupPerm Members of the current user's group have
    permission to read the file.
    \value WriteGroupPerm Members of the current user's group have
    permission to write to the file.
    \value ExeGroupPerm Members of the current user's group have
    permission to execute the file.
    \value ReadOtherPerm All users have permission to read the file.
    \value WriteOtherPerm All users have permission to write to the
    file.
    \value ExeOtherPerm All users have permission to execute the file.

    \value LinkType The file is a link to another file (or link) in
    the file system (i.e. not a file or directory).
    \value FileType The file is a regular file to the file system
    (i.e. not a link or directory)
    \value DirectoryType The file is a directory in the file system
    (i.e. not a link or file).

    \value HiddenFlag The file is hidden.
    \value ExistsFlag The file actually exists in the file system.
    \value RootFlag  The file or the file pointed to is the root of the filesystem.
    \value LocalDiskFlag The file resides on the local disk and can be passed to standard file functions.

    \omitvalue PermsMask
    \omitvalue TypesMask
    \omitvalue FlagsMask
    \omitvalue FileInfoAll

    \sa fileFlags(), setFileName()
*/

/*!
    \enum QAbstractFileEngine::FileTime

    These are used by the fileTime() function.

    \value CreationTime When the file was created.
    \value ModificationTime When the file was most recently modified.
    \value AccessTime When the file was most recently accessed (e.g.
    read or written to).

    \sa setFileName()
*/

/*!
    \enum QAbstractFileEngine::FileOwner

    \value OwnerUser The user who owns the file.
    \value OwnerGroup The group who owns the file.

    \sa owner(), ownerId(), setFileName()
*/

/*!
   Constructs a new QAbstractFileEngine that does not refer to any file or directory.

   \sa setFileName()
 */
QAbstractFileEngine::QAbstractFileEngine() : d_ptr(new QAbstractFileEnginePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
   \internal

   Constructs a QAbstractFileEngine.
 */
QAbstractFileEngine::QAbstractFileEngine(QAbstractFileEnginePrivate &dd) : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the QAbstractFileEngine.
 */
QAbstractFileEngine::~QAbstractFileEngine()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
    \fn bool QAbstractFileEngine::open(QIODevice::OpenMode mode)

    Opens the file in the specified \a mode. Returns true if the file
    was successfully opened; otherwise returns false.

    The \a mode is an OR combination of QIODevice::OpenMode and
    QIODevice::HandlingMode values.
*/
bool QAbstractFileEngine::open(QIODevice::OpenMode openMode)
{
    Q_UNUSED(openMode);
    return false;
}

/*!
    Closes the file, returning true if successful; otherwise returns false.

    The default implementation always returns false.
*/
bool QAbstractFileEngine::close()
{
    return false;
}

/*!
    Flushes the open file, returning true if successful; otherwise returns
    false.

    The default implementation always returns false.
*/
bool QAbstractFileEngine::flush()
{
    return false;
}

/*!
    Returns the size of the file.
*/
qint64 QAbstractFileEngine::size() const
{
    return 0;
}

/*!
    Returns the current file position.

    This is the position of the data read/write head of the file.
*/
qint64 QAbstractFileEngine::pos() const
{
    return 0;
}

/*!
    \fn bool QAbstractFileEngine::seek(qint64 offset)

    Sets the file position to the given \a offset. Returns true if
    the position was successfully set; otherwise returns false.

    The offset is from the beginning of the file, unless the
    file is sequential.

    \sa isSequential()
*/
bool QAbstractFileEngine::seek(qint64 pos)
{
    Q_UNUSED(pos);
    return false;
}

/*!
    Returns true if the file is a sequential access device; returns
    false if the file is a direct access device.

    Operations involving size() and seek(int) are not valid on
    sequential devices.
*/
bool QAbstractFileEngine::isSequential() const
{
    return false;
}

/*!
    Requests that the file is deleted from the file system. If the
    operation succeeds return true; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() rmdir()
 */
bool QAbstractFileEngine::remove()
{
    return false;
}

/*!
    Copies the contents of this file to a file with the name \a newName.
    Returns true on success; otherwise, false is returned.
*/
bool QAbstractFileEngine::copy(const QString &newName)
{
    Q_UNUSED(newName);
    return false;
}

/*!
    Requests that the file be renamed to \a newName in the file
    system. If the operation succeeds return true; otherwise return
    false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */
bool QAbstractFileEngine::rename(const QString &newName)
{
    Q_UNUSED(newName);
    return false;
}

/*!
    Creates a link from the file currently specified by fileName() to
    \a newName. What a link is depends on the underlying filesystem
    (be it a shortcut on Windows or a symbolic link on Unix). Returns
    true if successful; otherwise returns false.
*/
bool QAbstractFileEngine::link(const QString &newName)
{
    Q_UNUSED(newName);
    return false;
}

/*!
    Requests that the directory \a dirName be created. If
    \a createParentDirectories is true, then any sub-directories in \a dirName
    that don't exist must be created. If \a createParentDirectories is false then
    any sub-directories in \a dirName must already exist for the function to
    succeed. If the operation succeeds return true; otherwise return
    false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() rmdir() isRelativePath()
 */
bool QAbstractFileEngine::mkdir(const QString &dirName, bool createParentDirectories) const
{
    Q_UNUSED(dirName);
    Q_UNUSED(createParentDirectories);
    return false;
}

/*!
    Requests that the directory \a dirName is deleted from the file
    system. When \a recurseParentDirectories is true, then any empty
    parent-directories in \a dirName must also be deleted. If
    \a recurseParentDirectories is false, only the \a dirName leaf-node
    should be deleted. In most file systems a directory cannot be deleted
    using this function if it is non-empty. If the operation succeeds
    return true; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() remove() mkdir() isRelativePath()
 */
bool QAbstractFileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
{
    Q_UNUSED(dirName);
    Q_UNUSED(recurseParentDirectories);
    return false;
}

/*!
    Requests that the file be set to size \a size. If \a size is larger
    than the current file then it is filled with 0's, if smaller it is
    simply truncated. If the operations succceeds return true; otherwise
    return false;

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/
bool QAbstractFileEngine::setSize(qint64 size)
{
    Q_UNUSED(size);
    return false;
}

/*!
    Should return true if the underlying file system is case-sensitive;
    otherwise return false.

    This virtual function must be reimplemented by all subclasses.
 */
bool QAbstractFileEngine::caseSensitive() const
{
    return false;
}

/*!
    Return true if the file referred to by this file engine has a
    relative path; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */
bool QAbstractFileEngine::isRelativePath() const
{
    return false;
}

/*!
    Requests that a list of all the files matching the \a filters
    list based on the \a filterNames in the file engine's directory
    are returned.

    Should return an empty list if the file engine refers to a file
    rather than a directory, or if the directory is unreadable or does
    not exist or if nothing matches the specifications.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */
QStringList QAbstractFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    Q_UNUSED(filters);
    Q_UNUSED(filterNames);
    return QStringList();
}

/*!
    This function should return the set of OR'd flags that are true
    for the file engine's file, and that are in the \a type's OR'd
    members.

    In your reimplementation you can use the \a type argument as an
    optimization hint and only return the OR'd set of members that are
    true and that match those in \a type; in other words you can
    ignore any members not mentioned in \a type, thus avoiding some
    potentially expensive lookups or system calls.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
*/
QAbstractFileEngine::FileFlags QAbstractFileEngine::fileFlags(FileFlags type) const
{
    Q_UNUSED(type);
    return 0;
}

/*!
    Requests that the file's permissions be set to \a perms. The argument
    perms will be set to the OR-ed together combination of
    QAbstractFileEngine::FileInfo, with only the QAbstractFileEngine::PermsMask being
    honored. If the operations succceeds return true; otherwise return
    false;

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/
bool QAbstractFileEngine::setPermissions(uint perms)
{
    Q_UNUSED(perms);
    return false;
}

/*!
    Return  the file engine's current file name in the format
    specified by \a file.

    If you don't handle some \c FileName possibilities, return the
    file name set in setFileName() when an unhandled format is
    requested.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName(), FileName
 */
QString QAbstractFileEngine::fileName(FileName file) const
{
    Q_UNUSED(file);
    return QString();
}

/*!
    If \a owner is \c OwnerUser return the ID of the user who owns
    the file. If \a owner is \c OwnerGroup return the ID of the group
    that own the file. If you can't determine the owner return -2.

    This virtual function must be reimplemented by all subclasses.

    \sa owner() setFileName(), FileOwner
 */
uint QAbstractFileEngine::ownerId(FileOwner owner) const
{
    Q_UNUSED(owner);
    return 0;
}

/*!
    If \a owner is \c OwnerUser return the name of the user who owns
    the file. If \a owner is \c OwnerGroup return the name of the group
    that own the file. If you can't determine the owner return
    QString().

    This virtual function must be reimplemented by all subclasses.

    \sa ownerId() setFileName(), FileOwner
 */
QString QAbstractFileEngine::owner(FileOwner owner) const
{
    Q_UNUSED(owner);
    return QString();
}

/*!
    If \a time is \c CreationTime, return when the file was created.
    If \a time is \c ModificationTime, return when the file was most
    recently modified. If \a time is \c AccessTime, return when the
    file was most recently accessed (e.g. read or written).
    If the time cannot be determined return QDateTime() (an invalid
    date time).

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName(), QDateTime, QDateTime::isValid(), FileTime
 */
QDateTime QAbstractFileEngine::fileTime(FileTime time) const
{
    Q_UNUSED(time);
    return QDateTime();
}

/*!
    Sets the file engine's file name to \a file. This file name is the
    file that the rest of the virtual functions will operate on.

    This virtual function must be reimplemented by all subclasses.

    \sa rename()
 */
void QAbstractFileEngine::setFileName(const QString &file)
{
    Q_UNUSED(file);
}

/*!
    Returns the native file handle for this file engine. This handle must be
    used with care; its value and type are platform specific, and using it
    will most likely lead to non-portable code.
*/
int QAbstractFileEngine::handle() const
{
    return -1;
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *QAbstractFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    Q_UNUSED(filters);
    Q_UNUSED(filterNames);
    return 0;
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *QAbstractFileEngine::endEntryList()
{
    return 0;
}

/*!
    Reads a number of characters from the file into \a data. At most
    \a maxlen characters will be read.

    Returns -1 if a fatal error occurs, or 0 if there are no bytes to
    read.
*/
qint64 QAbstractFileEngine::read(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

/*!
    Writes \a len bytes from \a data to the file. Returns the number
    of characters written on success; otherwise returns -1.
*/
qint64 QAbstractFileEngine::write(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return -1;
}

/*!
    This function reads one line, terminated by a '\n' character, from the
    file info \a data. At most \a maxlen characters will be read. The
    end-of-line character is included.
*/
qint64 QAbstractFileEngine::readLine(char *data, qint64 maxlen)
{
    qint64 readSoFar = 0;
    while (readSoFar < maxlen) {
        char c;
        qint64 readResult = read(&c, 1);
        if (readResult <= 0)
            return (readSoFar > 0) ? readSoFar : readResult;
        ++readSoFar;
        *data++ = c;
        if (c == '\n')
            return readSoFar;
    }
    return readSoFar;
}

/*!
   \internal
   \enum QAbstractFileEngine::Extension
*/

/*!
   \internal
   \class QAbstractFileEngine::ExtensionOption
*/

/*!
   \internal
   \class QAbstractFileEngine::ExtensionReturn
*/

/*!
    \internal
*/
bool QAbstractFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    Q_UNUSED(extension);
    Q_UNUSED(option);
    Q_UNUSED(output);
    return false;
}

/*!
    \internal
*/
bool QAbstractFileEngine::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
  Returns the QFile::FileError that resulted from the last failed
  operation. If QFile::UnspecifiedError is returned, QFile will
  use its own idea of the error status.

  \sa QFile::FileError, errorString()
 */
QFile::FileError QAbstractFileEngine::error() const
{
    Q_D(const QAbstractFileEngine);
    return d->fileError;
}

/*!
  Returns the human-readable message appropriate to the current error
  reported by error(). If no suitable string is available, an
  empty string is returned.

  \sa error()
 */
QString QAbstractFileEngine::errorString() const
{
    Q_D(const QAbstractFileEngine);
    return d->errorString;
}

/*!
    Sets the error type to \a error, and the error string to \a errorString.
    Call this function to set the error values returned by the higher-level
    classes.

    \sa QFile::error(), QIODevice::errorString(), QIODevice::setErrorString()
*/
void QAbstractFileEngine::setError(QFile::FileError error, const QString &errorString)
{
    Q_D(QAbstractFileEngine);
    d->fileError = error;
    d->errorString = errorString;
}
