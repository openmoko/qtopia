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
#include <QCache>
#include <QReadWriteLock>

#include <qcontent.h>
#include <qtopia/private/contentlnk_p.h>
#include <qtopia/private/contentlnksql_p.h>
#include <qtopia/private/drmcontent_p.h>

#include <qtranslatablesettings.h>
#include <qtopiasql.h>
#include <qmimetype.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtopiaipcenvelope.h>
#include <qtopia/private/contentserverinterface_p.h>
#include <qtopia/private/contentpluginmanager_p.h>
#include <qcategorymanager.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopia/private/qcontent_p.h>
#endif
#include <QValueSpaceObject>

static const uint ContentCacheSize = 100;

/*!
    \variable QContent::InvalidId
    \brief A constant representing an invalid QContent identifier
*/
const QContentId QContent::InvalidId = QContentId(0xffffffff, Q_UINT64_C(0xffffffffffffffff));

Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QContentId);
Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QContentId, QContentId);
Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QContentIdList, QContentIdList);
Q_IMPLEMENT_USER_METATYPE_ENUM(QContent::ChangeType);

typedef QCache<QContentId, QContent> QContentCache;
Q_GLOBAL_STATIC_WITH_ARGS(QContentCache, contentCache, (ContentCacheSize));
static QMutex contentCacheMutex(QMutex::Recursive);
static QReadWriteLock databaseLock;

/*!
    \relates QContent
    \typedef QtopiaDatabaseId

    A QtopiaDatabaseId is a globally unique identifier for a QContent backing store.
    Synonym for quint32.
*/

/*!
    \relates QContent
    \typedef QContentId

    A QContentId is a globally unique identifier for a QContent record.
    Synonym for QPair<QtopiaDatabaseId, quint64>.
*/

/*!
    \relates QContent
    \typedef QContentIdList

    Synonym for QList<QContentId>.
*/

/*!
  \class QContent
  \mainclass
  \brief The QContent class represents a content carrying entity in the
  Qtopia system, where content includes resources which an end-user may
  view or manage.

  By creating content with a \l {Role} of \c Data using the setRole() method,
  resources not intended to be directly consumed by the end-user may also be
  managed.

  An instance of a QContent may, for example, represent any one of the following
  types of content:
  \list
  \o a stream
  \o a file
  \o a temporary file
  \o a DRM controlled file (possibly encrypted).
  \o an item in a container, eg an archive or DRM multi-part file
  \endlist

  The QContent class is responsible for
  providing access to metadata about the content contained in a file
  or stream.

  As a system-wide invariant, the backing store is authoritative for information
  about content available on the device.  For example content stored in a DRM file
  or a stream may not have a logical file on the file-system.

  In general applications should use the QContent and QContentSet interface to
  manage, and search for resources, instead of calling through to the file-system.

  The backing store is typically an SQL database, although it could be some other
  kind of persistent storage.

  The static methods install() and uninstall()
  are used to update records of metadata in the backing store in response to hardware and
  software events such as:
  \list
  \o removable storage being removed
  \o new content arriving via WAP or MMS
  \o DRM expiry.
  \endlist

  To be notified of these events, create a QContentSet object and connect
  to its \l{QContentSet::changed()} signal.

  \sa QContentSet, QContentFilter

  \ingroup content
*/

/*!
    \enum QContent::ChangeType

    This enum specifies the type of change made to a QContent.

    \value Added the QContent has been added.
    \value Removed the QContent has been deleted.
    \value Updated the QContent has been modified.

    \sa QContentSet::changed()
*/

/*!
  \typedef QContent::UsageMode
    \deprecated
    \c UsageMode has been deprecated, use \c Role instead.

    \sa QContent::Role
 */

/*!
    \enum QContent::Role

    This enum specifies the type of document this QContent object appears to be.

    \value UnknownUsage unknown mode (default)
    \value Document a user-visible document file suitable for "open"
    \value Data a data or config file not suitable for "open"
    \value Application an application (possibly Java, possibly DRM controlled)
 */


/*!
  \enum QContent::DrmState

    This enum specifies the DRM State of this QContent object.

    \value Unprotected plain text "legacy" file, not subject to drm
    \value Protected file subject to drm.
 */

/*!
  \fn bool QContent::linkFileKnown() const

   Return true if the file associated with this QContent object is already
   known. If false, calling file() will generate a file name.

   \sa QContent::fileKnown(), QContent::linkFile(), QContent::file()
 */

/*!
  Constructs an empty invalid content object.
*/
QContent::QContent()
    : d( 0 )
{
    d = new ContentLinkPrivate;
    Q_ASSERT(d != NULL);
}

/*!
  Create a \c QContent object by fetching the metadata from the backing
  store specified by \a id.  This method will be fast if the object
  has already been referenced by \c QContent and \c QContentSet
  classes loaded in the current process, since internally the results
  of calls to the backing store are cached.
*/
QContent::QContent( QContentId id )
    : d( 0 )
{
    /*
       This uses in-process caching with statics in the inner private
       class to save on SQL/calls to the backing store.  */
    if(id==QContent::InvalidId)
    {
        d = new ContentLinkPrivate;
        return;
    }
    contentCacheMutex.lock();
    QContent *cached = contentCache()->object(id);

    if (cached) {
        *this = *cached;
        contentCacheMutex.unlock();
        return;
    }
    contentCacheMutex.unlock();

    databaseLock.lockForRead();

    QHash<QString, QVariant> databaseLink = database()->linkById( id );

    if( !databaseLink.isEmpty() )
    {
        create( databaseLink );
    }
    else
    {
        d = new ContentLinkPrivate;
        qWarning() << "No content for id" << id;
    }

    databaseLock.unlock();

    contentCacheMutex.lock();
    contentCache()->insert(id, new QContent(*this));
    contentCacheMutex.unlock();
    Q_ASSERT(d != NULL);
}

/*!
  Create a \c QContent based on the content contained in the file represented by \a fi.

  Passing \a store specifies whether this content object is stored into the backing store,
  or only used as a local object.
*/
QContent::QContent( const QFileInfo &fi, bool store )
    : d( 0 )
{
    d = new ContentLinkPrivate;

    init( fi, store );
    Q_ASSERT(d != NULL);
}

/*!
  Create a \c QContent based on the content contained in the file represented by \a fileName.

  Passing \a store specifies whether this content object is stored into the backing store database,
  or only used as a local object.
*/
QContent::QContent( const QString &fileName, bool store )
    : d( 0 )
{
    d = new ContentLinkPrivate;

    init( QFileInfo( fileName ), store );
    Q_ASSERT(d != NULL);
}

/*!
  \internal
  Initialise this QContent object with the QContent point at by \a fileName, \a store in the backing database if necessary
 */
void QContent::init( const QFileInfo &fi, bool store )
{
    if ( fi.fileName().isEmpty() )
    {
        return;
    }
    // note - this method is slow on Unix (see doco for QFileInfo)
    const QString filepath=fi.absoluteFilePath();
    const bool isLink = fi.suffix() == QLatin1String("desktop") || filepath.endsWith(QLatin1String(".directory"));
    databaseLock.lockForRead();
    QHash<QString, QVariant> databaseLink = QContent::database()->linkByPath(filepath, isLink);
    databaseLock.unlock();

    if( !databaseLink.isEmpty() )
    {
        QContentId id(databaseLink[QLatin1String("database")].toUInt(), databaseLink[QLatin1String("cid")].toULongLong());
        if(isCached(id))
            *this = QContent(id);
        else
        {
            create( databaseLink );

            static const char *unknownMime =  "application/octet-stream";

            if( ( lastUpdated().isNull() || lastUpdated() < fi.lastModified() ||
                ( type() == unknownMime && QMimeType( file() ).id() != unknownMime ) ) && updateContent() )
            {
                d->cContentState = ContentLinkPrivate::Uncommited;
            }
            else
                d->cContentState = ContentLinkPrivate::Committed;
        }
    }
    else if( installContent( filepath ) )
    {
        d->cContentState = ContentLinkPrivate::Uncommited;
    }
    else
        return;

    if( store )
        commit();
}

/*!
    Reads the properties of a file in preperation for an initial write to the database.

    If the \c file() property of the QContent has been set the data will be read from the file
    at that location, otherwise the file at \a filePath will be used.  If no file exists at either
    location the method will fail.
*/
bool QContent::installContent( const QString &filePath )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;

    QString path = !d->fileKnown() ? filePath : d->file();

    if( path.isEmpty() || !QFile::exists( path ) )
        return false;

    if( DrmContentPrivate::installContent( path, this ) )
    {
        d = new ContentLinkPrivateDRM( *d );

        d->drm = QContent::Protected;
    }
    else if( !QContentFactory::installContent( path, this ) && QFileInfo( path ).isDir() )
    {   // If neither a DRM or content plugin can identify a folder as a content item ignore it.
        return false;
    }

    d->cExtraLoaded |= ContentLinkPrivate::Properties;

    if( !d->fileKnown() && !linkFileKnown() )
        setFile( path );

    if( d->name().isEmpty() )
        setName( QFileInfo( path ).baseName() );

    if( d->type().isEmpty() )
        setType( QMimeType( path ).id() );

    if( d->um == QContent::UnknownUsage )
        setRole( QContent::Document );

    d->cLastUpdated = QDateTime::currentDateTime();
    d->cContentState = ContentLinkPrivate::Edited;

    return true;
}

/*!
    Updates the content meta-data when the associated file has changed.
*/
bool QContent::updateContent()
{
    bool changed = false;

    d->cContentState = ContentLinkPrivate::New;

    if( DrmContentPrivate::updateContent( this ) || d->drm != Protected && QContentFactory::updateContent( this ) )
    {
        return true;
    }
    else if( fileKnown() && type() == "application/octet-stream" )
    {
        QMimeType mime( file() );

        if( mime.id() != "application/octet-stream" )
        {
            setType( mime.id() );

            changed = true;
        }
    }

    return changed;
}

/*!
  Create a \c QContent by copying the \a other content object.
*/
QContent::QContent( const QContent &other )
    : d( 0 )
{
    (*this) = other;  // use assignment operator
    Q_ASSERT(d != NULL);
}

/*!
  \internal
  Create a content link by copying the \a link private content object.
 */
QContent::QContent( ContentLinkPrivate *link  )
    : d( link )
{
    Q_ASSERT(d != NULL);
}

/*!
  Destroys the content object.
*/
QContent::~QContent()
{
    // destruction of QSharedDataPointer d not required
}

/*!
  Content is invalid if the backing file is unavailable, either
  due to removal of media or deletion of the file. If \a force is true,
  the content will be revalidated even if this value has been previously
  cached. Returns true if content is valid; otherwise returns false.

  Note: this method can be expensive.
*/
bool QContent::isValid(bool force) const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;
    else
        return d->isValid(force);
}

/*!
  Return the user-visible name for this content object
*/
QString QContent::name() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->name();
}

/*!
  Return the RFC2045 mime-type for the content
  If this is an application, instead return "Application" (the
  underlying QContent's type)
*/
QString QContent::type() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->type();
}

/*!
  Return the plaintext size of the content in bytes
*/
qint64 QContent::size() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return 0;
    else
        return d->size();
}

/*!
  Return the icon for this object.  If the object is an application, then it
  will be the application icon, otherwise it will be a generic icon for the
  mime-type of the object.

  If the object is a DRM controlled,  a key emblem is super-imposed on the
  icon. If the content does not have current rights for the default application
  the icon is shown greyed out (using the QIcon dynamic routines).
 */
QIcon QContent::icon() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QIcon();
    else
        return d->icon();
}

/*!
  Return the icon for this object.  If the object is an application, then it
  will be the application icon, otherwise it will be a generic icon for the
  mime-type of the object.

  If the object is a DRM controlled,  a key emblem is super-imposed on the
  icon. If the content does not have current rights for \a permission the icon is shown
  greyed out (using the QIcon dynamic routines).
*/
QIcon QContent::icon( QDrmRights::Permission permission ) const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QIcon();
    else
        return d->icon( permission );
}



/*!
  \internal
  Retrieve an instance of the backing store API.
  To change to a different backing store when customizing Qtopia, change this
  method to return a custom backing store implementation.
*/
ContentLinkSql *QContent::database()
{
    return ContentLinkPrivate::persistenceEngine();
}

/*!
  Takes a \c QFileInfo reference \a fi to a content object, and installs the
  metadata for it in the backing store/database.  The path must be an
  absolute path and the file must exist.  If the file is a .desktop file
  referring to an application or a document a \c QContent record is created.

  If mimetype data is not available, it is determined by file extension, or
  failing that, the magic number.

  If the object is a DRM-controlled file (is a .dcf file, or is otherwise
  shown as drm controlled) the DRM subsystem is queried for the DRM status.

  \sa QContentSet::scan()
*/
QContentId QContent::install( const QFileInfo &fi )
{
    if (!fi.exists())
    {
        qWarning() << "Attempt to install a file into the content database that doesn't exist:" << fi.fileName();
        return QContent::InvalidId;
    }
    QContent cl( fi.filePath(), true );
    updateSets(cl.id(), QContent::Added);
    return cl.id();
}

/*!
  Remove the Content with \a id from the backing store
*/
void QContent::uninstall( QContentId id )
{
    if(id == QContent::InvalidId)
    {
        qLog(DocAPI) << "Cannot uninstall QContent::InvalidId";
        return;
    }
    qLog(DocAPI) << "Uninstalling QContent for id " << id;
    databaseLock.lockForWrite();
    QContent::database()->removeLink( id );
    databaseLock.unlock();
    invalidate(id);
    updateSets(id, QContent::Removed);
}

/*!
  Installs a set of files in a \a batch, committing to the database only once. This achieves a significant
  performance boost over multiple calls to install.

  \sa install()
 */

void QContent::installBatch( const QList<QFileInfo> &batch )
{
    //QTOPIA_DOCAPI_TODO, wrap this all in a transaction, this code is simply placeholder atm
    qLog(DocAPI) << "Entering QContent::installBatch";
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->beginInstall();
#endif
    QMultiHash<QtopiaDatabaseId, QContent> list;
    foreach(const QFileInfo &fi, batch)
        list.insert(QtopiaSql::databaseIdForPath(fi.absolutePath()), QContent(fi, false));
    foreach(QtopiaDatabaseId dbid, list.uniqueKeys())
    {
        QMap<QContentId, ChangeType> cidctypemap;
        QSqlDatabase db=QtopiaSql::database(dbid);
        if(!db.transaction())
            qWarning("QContent::installBatch: couldn't start transaction");
        foreach(QContent content, list.values(dbid))
        {
            content.d->batchLoading = true;
            ChangeType ctype=Updated;
            content.commit(ctype);
            if(content.id() != QContent::InvalidId)
                cidctypemap[content.id()]=ctype;
        }
        if(!db.commit())
            qWarning("QContent::installBatch: couldn't commit transaction");
        foreach(QContentId id, cidctypemap.keys())
            updateSets(id, cidctypemap[id]);
    }
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->endInstall();
#endif
    qLog(DocAPI) << "Leaving QContent::installBatch";
}

/*!
  Uninstalls a set of files in a \a batch, committing to the database only once. This achieves a significant
  performance boost over multiple calls to uninstall().

  \sa uninstall()
 */

void QContent::uninstallBatch( const QList<QContentId> &batch )
{
    //QTOPIA_DOCAPI_TODO, wrap this all in a transaction, this code is simply placeholder atm
    qLog(DocAPI) << "Entering QContent::uninstallBatch";
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->beginInstall();
#endif
    QMultiHash<QtopiaDatabaseId, QContentId> list;
    foreach(const QContentId& id, batch)
        list.insert(id.first, id);
    foreach(QtopiaDatabaseId dbid, list.uniqueKeys())
    {
        QSqlDatabase db=QtopiaSql::database(dbid);
        if(!db.transaction())
            qWarning("QContent::installBatch: couldn't start transaction");
        foreach(QContentId contentId, list.values(dbid))
            uninstall(contentId);
        if(!db.commit())
            qWarning("QContent::installBatch: couldn't commit transaction");
    }
#ifndef QTOPIA_CONTENT_INSTALLER
    QContentUpdateManager::instance()->endInstall();
#endif
    qLog(DocAPI) << "Leaving QContent::uninstallBatch";
}


/*!
  Clear all error flags and errors strings on all \c QContent objects.
  Note: this method clears the global error cache for all \c QContent objects
  in this process.
*/
void QContent::clearErrors()
{
    ContentLinkPrivate::errors.clear();
}

/*!
  Given an application binary name \a bin, return \c QContentId pointing to the
  QContent item representing an application with that binary.

  A path name may be supplied for \a bin (eg \a bin may contain "/"
  characters).

  Note that binary names are unique across qtopia.
*/
QContentId QContent::execToContent( const QString& bin )
{
    return database()->idByExecutable( bin );
}

/*!
  Return the DRM status of this object, as per the \c DRMState enum.  This
  value is a cache of the real value available from the DRM agent.  When
  an object expires an event should be fired to update the database but be aware
  that stale information may be displayed.
*/
QContent::DrmState QContent::drmState() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return Unprotected;
    else
        return d->drm;
}

/*!
    \deprecated
  Return the document status of the object as per the \c UsageMode enum.  This
  value does not usually change.  The status is used to determine what to display
  to the user. \c Data objects make no sense to display to the user, as
  they cannot be launched or categorized. These files are only of use to the
  applications which operate on them.

    \sa role()
*/
QContent::UsageMode QContent::usageMode() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return UnknownUsage;
    else
        return d->um;
}

/*!
  Return the document status of the object as per the \c Role enum.  This
  value does not usually change.  The status is used to determine what to display
  to the user. \c Data objects make no sense to display to the user, as
  they cannot be launched or categorized. These files are only of use to the
  applications which operate on them.
 */
QContent::Role QContent::role() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return UnknownUsage;
    else
        return d->um;
}

/*!
    Set the document status of the object to \a role as per the \c Role enum.
*/
void QContent::setRole( Role role )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;

    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->um = role;
}


/*!
  Return the comment for this object, typically used on ToolTips
  For DRM controlled objects this will include a summary of the
  rights and DRM status
*/
QString QContent::comment() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->comment();
}

/*!
    Sets a string \a comment for this object.
*/
void QContent::setComment( const QString &comment )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;

    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->cComment = comment;
}

/*!
  Return the error string of this object.  If \c error() is false
  then this method returns an empty string.  Use \c error() instead
  of checking for an empty error string. The error string is not translated
  and therefore should not be used in a user interface.

  \internal
  Internally the error is stored in a static hash by record id.  In the
  general case there wont be an error and if 1000's of \c QContent
  objects are created having the extra storage for a string and a
  bool which are almost always empty is inefficient.
*/
QString QContent::errorString() const
{
    return (error() && id() != InvalidId) ? ContentLinkPrivate::errors[id()] : QString();
}

/*!
  Return true if this \c QContent object is in an error state.
  Call the \c errorString() method to return a text description of the error.
*/
bool QContent::error() const
{
    return id() != InvalidId && ContentLinkPrivate::errors.contains( id() );
}

/*!
    Returns all the DRM permissions the content currently has rights for.
    A true value of \a force will cause the rights to be requeried otherwise a
    cached value may be returned.
*/
QDrmRights::Permissions QContent::permissions( bool force ) const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QDrmRights::InvalidPermission;
    else
        return d->permissions( force );
}

/*!
  Return a QDrmRights representation of any rights for this object for the
  given \a permission.
  If the drmStatus() returns Unprotected, then this method returns a QDrmRights
  object with valid rights for all permissions.  Use drmStatus() != Unprotected
  to test for unprotected content and permissions() & [permission] to test for
  a specific permission.
*/
QDrmRights QContent::rights( QDrmRights::Permission permission ) const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QDrmRights();
    else
        return d->rights( permission );
}

/*!
   Return true if the file associated with this QContent object is already
   known. If false, calling file() will generate a file name.
*/
bool QContent::fileKnown() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;
    else
        return d->fileKnown();
}

/*!
  Deprecated functionality: Return the path of the .desktop link file
  this metainfo ContentLink was generated from if available.
*/
QString QContent::linkFile() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->linkFile();
}

/*!
  Returns the file associated with the QContent.

  \sa executableName(), name()
*/
QString QContent::file() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    return d->file();
}

/*!
    Returns MimeTypePermissions property of the QContent.
*/
QList< QDrmRights::Permission > QContent::mimeTypePermissions() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QList<QDrmRights::Permission>();
    return d->mimeTypePermissions();
}

/*!
  Returns the MimeTypeIcons property of the QContent.
*/

QStringList QContent::mimeTypeIcons() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QStringList();
    else
        return d->mimeTypeIcons();
}

/*!
  Returns the MimeTypes property. This is the list of MIME types
  that the application can view or edit.
*/
QStringList QContent::mimeTypes() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QStringList();
    else
        return d->mimeTypes();
}

/*!
  Sets the MimeTypes property tp \a mimeTypes. This is the list of MIME types
  that the application can view or edit.
 */
void QContent::setMimeTypes( const QStringList &mimeTypes )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;

    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->cMimeTypes = mimeTypes;
}

/*!
  Return a list of strings representing the categories on this object.
  Usually it will make more sense to create a \c QContentSet and set
  category filters on it.
*/
QStringList QContent::categories() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QStringList();
    
    return d->categories();
}

/*!
  Assignment operator.  Sets data on this to be what is set on \a other.
*/
QContent &QContent::operator=( const QContent &other )
{
    d = other.d;   // shallow QSharedData copy
    Q_ASSERT(d.constData() != NULL);
    return (*this);
}

/*!
  Equality operator.  Return true if this \c Content object is the same as
  the \a other; otherwise returns false.  Is true if both have the same Id number, or if both are
  empty.
*/
bool QContent::operator==( const QContent &other ) const
{
    if(id() == QContent::InvalidId && other.id() == QContent::InvalidId) {
        return *d == *other.d;
    } else {
        return (id() == other.id());
    }
}

/*!
    Queries the launcher configuration settings and returns true if this object is in the systems PreloadApps list; otherwise returns false.
*/
bool QContent::isPreloaded() const
{
    // Preload information is stored in the Launcher config in v1.5.
    if (isDocument())
        return false;
    QSettings cfg("Trolltech","Launcher");
    cfg.beginGroup("AppLoading");
    QStringList apps = cfg.value("PreloadApps").toString().split(',');
    if (apps.contains(executableName()))
        return true;
    return false;
}

/*!
  Returns the executable name property. This is the name of the executable
  program associated with the QContent target.

  \sa setExecutableName(), execute()
*/
QString QContent::executableName() const
{
    if (role() == Application)
        return file();
    else
    {
        QMimeType mt(type());
        QContent app = mt.application();
        if ( app.id() != InvalidId )
            return app.executableName();
        else
            return QString();
    }
}

/*!
    Sets the executable name property to \a exec.
    The property will not be written to the backing store until commit()
    is called.

  \sa executableName(), execute()
*/

void QContent::setExecutableName(const QString &exec)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    
    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    if( d->cPath.isEmpty() )
        d->cPath = exec;
}

/*!
  \overload
  Executes the application associated with this QContent.

  \sa executableName(), setExecutableName()
*/

void QContent::execute() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    else
        d->execute();
}

/*!
  Executes the application associated with this QContent, with
  \a args as arguments.

  \sa executableName(), setExecutableName()
*/

void QContent::execute(const QStringList& args) const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    else
        d->execute(args);
}

/*!
    Return the path to the icon for this file.

    \sa icon()
*/

QString QContent::iconName() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->iconName();
}

/*!
    Sets the Name property to \a docname.
    The property will not be written to the backing store until commit()
    is called.

  \sa name()
 */
void QContent::setName(const QString& docname)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    else if( d->cName != docname )
    {
        if( d->contentState() == ContentLinkPrivate::Committed )
            d->cContentState = ContentLinkPrivate::Edited;

        if( d->contentState() != ContentLinkPrivate::New )
            d->cNameChanged = true;

        d->cName = docname;

        d->loadIcon( true );
    }
}

/*!
    Sets the Type property to \a doctype.
    The property will not be written to the backing store until commit()
    is called.

  \sa name()
 */
void QContent::setType(const QString& doctype)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    
    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->cType = doctype;
    d->loadIcon();

    if( d->um == UnknownUsage )
    {
        if (doctype == "application/x-executable") {
            d->um = Application;
        } else {
            d->um = Document;
        }
    }
}

/*!
    Sets the icons associated with this content to \a iconpath.
    The property will not be written to the backing store until commit()
    is called.

    \sa icon(), commit()
*/
void QContent::setIcon(const QString& iconpath)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    
    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->cIconPath = iconpath;
    d->loadIcon( true );
}

/*!
    Convenience function to test if this QContent is a Document or an Application.
    Returns true if this QContent is a Document or an Application; otherwise returns false.

    \sa usageMode()
*/
bool QContent::isDocument() const
{
    return usageMode() != Application;
}

/*!
    \enum QContent::Property

    Convenience enum provides some of the more common content properties.

    \value Album  The name of the album which the content is part of.
    \value Artist The name of the artist who produced the content.
    \value Author The author of the content.
    \value Composer The composer of the content.
    \value ContentUrl A URL from which a copy of the content can be downloaded.  Typically associated with OMA DRM content.
    \value Copyright A copyright notice for the content.
    \value CopyrightUrl A URL with copyright/legal information.
    \value Description A description of the content.
    \value Genre The genre the content belongs to.
    \value InformationUrl A URL where additional information about the content can be obtained.  Typically associated with OMA DRM content.
    \value PublisherUrl The URL for the content publisher's website.
    \value RightsIssuerUrl A URL where rights for DRM protected content may be obtained.  Typically associated with OMA DRM content.
    \value Track The content's position on an album.
    \value Version The content's version number.
*/

/*!
    Set a property associated with the content with key \a key and value
    \a value.  If \a group is specified then the property will be stored in
    the specified group, otherwise it will be stored in the default group.
    The property will not be written to the backing store until
    commit() is called.

    \sa property(), commit()
*/
void QContent::setProperty(const QString& key, const QString& value, const QString &group)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    
    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->setProperty(key, value, group);
}

/*!
    Set a property associated with the content with key \a key and value
    \a value.  The property will not be written to the backing store until
    commit() is called.

    \sa property(), commit()
*/
void QContent::setProperty( Property key, const QString &value )
{
    setProperty( propertyKey( key ), value );
}

/*!
    Return a property associated with this content with key \a key and
    group \a group.  If \a group is not specifed the default group will be
    used.

    \sa setProperty()
*/
QString QContent::property(const QString& key, const QString &group) const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->property(key, group);
}

/*!
    Return a property associated with this content with key \a key.

    \sa setProperty()
*/
QString QContent::property( Property key ) const
{
    return property( propertyKey( key ) );
}

/*!
    Returns the string key for a given \a property.
*/
QString QContent::propertyKey( Property property )
{
    switch( property )
    {
    case Album:           return QLatin1String( "Album"           );
    case Artist:          return QLatin1String( "Artist"          );
    case Author:          return QLatin1String( "Author"          );
    case Composer:        return QLatin1String( "Composer"        );
    case ContentUrl:      return QLatin1String( "ContentUrl"      );
    case Copyright:       return QLatin1String( "Copyright"       );
    case CopyrightUrl:    return QLatin1String( "CopyrightUrl"    );
    case Description:     return QLatin1String( "Description"     );
    case Genre:           return QLatin1String( "Genre"           );
    case InformationUrl:  return QLatin1String( "InformationUrl"  );
    case PublisherUrl:    return QLatin1String( "PublisherUrl"    );
    case RightsIssuerUrl: return QLatin1String( "RightsIssuerUrl" );
    case Track:           return QLatin1String( "Track"           );
    case Version:         return QLatin1String( "Version"         );
    default:
         return QLatin1String( "Unknown"         );
    }
}

/*!
    Set the categories associated with this content to \a categoryList.
    The categores will not be written to the backing store until commit()
    is called.

    \sa categories(), commit()
*/
void QContent::setCategories( const QStringList &categoryList )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;

    if( d->contentState() == ContentLinkPrivate::Committed )
        d->cContentState = ContentLinkPrivate::Edited;

    d->cExtraLoaded |= ContentLinkPrivate::Categories;
    d->cCategories = categoryList;
}
/*!
  \fn bool QContent::commit()

  Writes the changes to the QContent object to the backing store. Returns true if successfull
 */


/*!
    Writes the changes to the QContent to the backing store. Returns true if successfull and sets
    \a change to the type of change committed.
*/
bool QContent::commit(ChangeType &change)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;

#ifndef QTOPIA_CONTENT_INSTALLER
    if ( !d->isValid() )
        return false;
#endif

    switch( d->cContentState )
    {
    case ContentLinkPrivate::New:
        if( !installContent( linkFileKnown() ? linkFile() : file() ) )
            return false;
        else
            break;
    case ContentLinkPrivate::Committed:
        if( !lastUpdated().isNull() && lastUpdated() >= QFileInfo( linkFileKnown() ? linkFile() : file() ).lastModified() )
            return true;
    case ContentLinkPrivate::Edited:
        {
            if(d->cNameChanged)
                d->syncFileName();

            updateContent();
        }
        break;
    case ContentLinkPrivate::Uncommited:
        break;
    }

    databaseLock.lockForWrite();
    d->cId = database()->postLink( d.data(), change );
    if( id() != InvalidId )
    {
        invalidate(d->cId);
        if( d->cExtraLoaded & ContentLinkPrivate::Categories )
        {
            database()->removeCategoryMap(id());
            foreach(QString cat, d->cCategories)
                database()->appendNewCategoryMap(cat, d->um == Application ? QLatin1String("Applications") : (d->um == Data ? QLatin1String("Data") : QLatin1String("Documents")), id(), QString::null, !d->batchLoading);
        }
        QMap<QPair<QString,QString>,QString>::const_iterator pit = d->cPropertyList.constBegin();
        while (pit != d->cPropertyList.constEnd()) {
            QPair<QString, QString> realKey = pit.key();
            database()->writeProperty(id(), realKey.first, pit.value(), realKey.second);
            pit++;
        }
        if( !d->cMimeTypeIcons.isEmpty() )
            database()->writeProperty( d->cId, QLatin1String("MimeTypeIcons"), d->cMimeTypeIcons.join( QLatin1String(";") ), QLatin1String("Desktop Entry") );
        databaseLock.unlock();
        d->cContentState = ContentLinkPrivate::Committed;
        if(!d->batchLoading)
            updateSets(id(), change);

        return true;
    }
    else
    {
        databaseLock.unlock();

        return false;
    }
}

/*!
    Set the file that this content references to \a filename
*/
void QContent::setFile( const QString& filename )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;

#ifndef QTOPIA_CONTENT_INSTALLER
    if( d->contentState() == ContentLinkPrivate::New )
    {
#endif
        d->valid = ContentLinkPrivate::UnknownValidity;

        d->cPath = filename;
#ifndef QTOPIA_CONTENT_INSTALLER
    }
#endif
}

/*!
    Uninstall this object from the database, and remove it from the filesystem.
 */
void QContent::removeFiles()
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;
    else if(QFileInfo(file()).isWritable() && !file().startsWith(Qtopia::qtopiaDir()+"/"))
    {
        if ( !type().startsWith("Folder/") && type() != "Applications"
            && type() != "Games" && type() != "Settings" && fileKnown()) {
            d->removeFiles();
        }
        uninstall(id());
    }
}

/*!
    \deprecated
    Uninstall the .desktop/link file for this object from the database, and remove it from the filesystem.
 */
void QContent::removeLinkFile()
{
    QFile fi(linkFile());
    if (fi.exists()  && fi.isWritable() && !linkFile().startsWith(Qtopia::qtopiaDir()+"/"))
        fi.remove();
}

/*!
    \deprecated
    Set the link/.desktop file that this content references to \a filename.
 */
void QContent::setLinkFile( const QString& filename )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return;

#ifndef QTOPIA_CONTENT_INSTALLER
    if( d->contentState() == ContentLinkPrivate::New )
    {
#endif
        d->valid = ContentLinkPrivate::UnknownValidity;

        d->cLinkFile = filename;
#ifndef QTOPIA_CONTENT_INSTALLER
    }
#endif
}

/*!
    Returns the root path of the media the content is stored on.
*/
QString QContent::media() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QString();
    else
        return d->media();
}

/*!
    Sets the root path of the \a media the file is stored on.  Once a QContent has been commited to the database
    the media cannot be changed. Returns true if successful; otherwise false.
*/
bool QContent::setMedia( const QString &media )
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;
    else
    {
        if( d->contentState() == ContentLinkPrivate::Committed )
            d->cContentState = ContentLinkPrivate::Edited;
        return d->setMedia( media );
    }
}

/*!
    Returns the Id of this QContent.  If this QContent is not present in
    the backing store its value will be QContent::InvalidId.
*/
QContentId QContent::id() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QContent::InvalidId;
    else
        return d->cId;
}

/*!
    Opens a QIODevice using mode \a mode.

    Returns the QIODevice if successful, otherwise returns 0.  It is the
    caller's responsibility to delete the return value.
*/
QIODevice *QContent::open(QIODevice::OpenMode mode)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return NULL;

    QIODevice *dev = d ? d->open(mode) : 0;
    if (dev && mode & (QIODevice::WriteOnly | QIODevice::Append | QIODevice::Truncate)) {
        commit();
    }

    return dev;
}

/*!
    Opens a read only QIODevice.

    Returns the QIODevice if successful, otherwise returns 0.  It is the
    caller's responsibility to delete the return value.
 */
QIODevice *QContent::open() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return NULL;
    else
        return fileKnown() ? d->open( QIODevice::ReadOnly ) : 0;
}

/*!
    Saves the data \a data.

    Returns true is successful; otherwise false.
*/
bool QContent::save(const QByteArray &data)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;

    QIODevice *dev = open(QIODevice::WriteOnly);
    if (!dev)
        return false;

    int bytesWritten = dev->write(data);
    dev->close();
    delete dev;

    if (bytesWritten != data.size()) {
        removeFiles();
        return false;
    }

    if( d->contentState() != ContentLinkPrivate::New )
        d->cContentState = ContentLinkPrivate::Edited;

    commit();

    return true;
}

/*!
    Loads the content into \a data.

    Returns true if successful; otherwise false.
*/
bool QContent::load(QByteArray &data) const
{
    QIODevice *dev = open();
    if (!dev)
        return false;

    if (dev->size() > 0)
        data = dev->readAll();
    dev->close();
    delete dev;

    return true;
}

/*!
    Copy the contents of \a from to this QContent.

    Returns true is successful, otherwise false.
*/
bool QContent::copyContent(const QContent &from)
{
    QFile source( from.file() );

    if( source.copy( file() ) )
        return true;

    return false;
}

/*!
    Copy the contents of the file and the metainfo from this QContent to
    \a newPath.

    Returns true is successful, otherwise false.
 */

bool QContent::copyTo(const QString &newPath)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;

    bool ok;
    if(!isValid())
        return false;
    QContent newCopy(*this);
    newCopy.d->cId = QContent::InvalidId;
    newCopy.d->cPath = newPath;
    newCopy.commit();
    ok = newCopy.copyContent(*this);
    if(!ok)
        newCopy.removeFiles();
    return ok;
}

/*!
    Move the contents of the file and the metainfo from this QContent to
    \a newPath.

    Returns true if the contents is successfully moved, otherwise false.

    Note: The id() of the original file will be invalid and should not be used.
 */

bool QContent::moveTo(const QString &newPath)
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return false;

    if(copyTo(newPath)) {
        removeFiles();
        *this = QContent(newPath);
        return true;
    }
    return false;
}

void QContent::invalidate(QContentId id)
{
    contentCacheMutex.lock();
    contentCache()->remove(id);
    contentCacheMutex.unlock();
}

bool QContent::isCached(QContentId id)
{
    return contentCache()->contains(id);
}

void QContent::cache(const QContentIdList &idList)
{
    databaseLock.lockForRead();
    QContentIdList internalidList;
    QMutexLocker guard(&contentCacheMutex);
    foreach(QContentId id, idList)
        if(!contentCache()->contains(id))
            internalidList.append(id);
    QList< QHash<QString, QVariant> > results = QContent::database()->linksById(internalidList);
    databaseLock.unlock();
    if (results.count()) {
        QHash<QString, QVariant> result;
        foreach( result, results ) {
            QContent *c = new QContent();
            c->create( result );
            contentCache()->insert(c->id(), c);
        }
    }
}

void QContent::create(QHash<QString, QVariant> result)
{
    bool correctionNeeded = false;
    static const QString docStatusString("docStatus"), drmFlagsString("drmFlags");

    QContentId id(result[QLatin1String("database")].toUInt(), result[QLatin1String("cid")].toULongLong());
    QChar docStatus = result[ docStatusString ].toString()[0];
    bool isProtected = result[ drmFlagsString ].toInt() != 65536;
    QString mimeType=result[ QLatin1String("mimeType") ].toString();

    if ( result.count() == 0
         || !result.contains( drmFlagsString )
         || !result.contains( docStatusString ))
    {
        QString error;
        ContentLinkPrivate::errors.insert( id, QString("Record (%1,%2) for QContent does not contain both drm & doc status"
                                                      ).arg( result[QLatin1String("database")].toUInt() ).arg( result[QLatin1String("cid")].toULongLong() ));
        d = new ContentLinkPrivate;
        return;
    }
    if ( (mimeType == QLatin1String("Applications") || mimeType == QLatin1String("Games") || mimeType == QLatin1String("Settings")) && docStatus != 'a' )
    {
        qLog (DocAPI) << "database has invalid doc/drm state data, fixing ( to be done, internal values corrected, database not corrected)";
        result[ docStatusString ] = QLatin1String("a");
        // QTOPIA_DOCAPI_TODO mark for write back to the database
        correctionNeeded = true;
    }

    if( isProtected )
    {
        d = new ContentLinkPrivateDRM;

        d->drm = Protected;
    }
    else
    {
        d = new ContentLinkPrivate;

        d->drm = Unprotected;
    }

    switch ( docStatus.toAscii() )
    {
    case 'a':
        d->um = Application;
        break;
    case 'd':
        d->um = Document;
        break;
    default:
        d->um = Data;
    }

    d->cId = id;
    d->load( result );

    d->cContentState = ContentLinkPrivate::Committed;

    if (correctionNeeded)
    {

        //ChangeType change = QContent::Updated;
        //d->cId = database()->postLink( d.data(), change );
    }
    QMutexLocker guard(&contentCacheMutex);
    contentCache()->insert(id, new QContent(*this));
}

/*!
  Update all the QContentSets this content is a member of
*/
void QContent::updateSets(QContentId id, QContent::ChangeType c)
{
#ifdef QTOPIA_CONTENT_INSTALLER
    Q_UNUSED(id);
    Q_UNUSED(c);
#else
    if(id != InvalidId)
        QContentUpdateManager::instance()->addUpdated(id, c);
#endif
}

/*!
  Return the value of the last time underlying file was updated.
 */

QDateTime QContent::lastUpdated() const
{
    Q_ASSERT(d != NULL);
    if (d == NULL)
        return QDateTime();
    else
        return d->lastUpdated();
}

QDataStream& operator<<(QDataStream& ds, const QContentId &id)
{
    ds << id.first << id.second;
    return ds;
}

QDataStream& operator>>(QDataStream& ds, QContentId &id)
{
    ds >> id.first >> id.second;
    return ds;
}

#if defined(QTOPIA_DBUS_IPC)
const QDBusArgument &operator>>(const QDBusArgument &a, QContentId &id)
{
    a.beginStructure();
    a >> id.first >> id.second;
    a.endStructure();

    return a;
}

QDBusArgument &operator<<(QDBusArgument &a, const QContentId &id)
{
    a.beginStructure();
    a << id.first << id.second;
    a.endStructure();

    return a;
}
#endif

QTOPIA_EXPORT QDebug &operator<<(QDebug &debug, const QContent &content)
{
    return debug << content.d.data();
}

/*!
  Return a uint suitable for use as a hash value.  This allows QContentId
  to be stored in a QHash
*/
uint qHash(const QContentId &id)
{
    int n;
    uint h = 0;
    uint g = 0;

    const uchar *p = reinterpret_cast<const uchar *>(&id.first);
    n = sizeof(id.first);
    while (n--) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }

    const uchar *q = reinterpret_cast<const uchar *>(&id.second);
    n = sizeof(id.second);
    while (n--) {
        h = (h << 4) + *q++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }

    return h;
}
