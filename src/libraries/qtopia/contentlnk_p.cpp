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

#include "contentlnk_p.h"
#include "contentserverinterface_p.h"
#include "drmcontent_p.h"

#include <qcontent.h>
#include <qcontentset.h>
#include <qmimetype.h>
#include <qtranslatablesettings.h>
#include <qtopianamespace.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiasql.h>
#include <qtopia/private/contentlnksql_p.h>

#include <qtopiaipcadaptor.h>
#include <qcategorymanager.h>
#include <qstorage.h>
#include <qtopiaservices.h>
#include <qtopiaabstractservice.h>
#include <qdrmcontentplugin.h>

#include <QFileInfo>
#include <QPixmapCache>
#include <QPainter>
#include <QDir>
#include <QDebug>
#include <QStyle>

#include <stdlib.h>  // for rand()

////////////////////////////////////////////////////////////////////////
/////
/////   ContentLinkPrivate - container for normal doc content

QHash<QContentId, QString> ContentLinkPrivate::errors;
QThreadStorage<ContentLinkSql *> ContentLinkPrivate::dbs;

static QString safeFileName(const QString& n)
{
    static const char SAFE_SPACE = '_';

    QString safename;

    // Remove all special ASCII characters and ensure that name does not start with a number
    QByteArray ascii = n.toAscii();
    for ( int i = 0; i < ascii.length(); i++ ){
        QChar c = ascii.at(i);
        if ( c.isLetterOrNumber() )
            safename += c;
        else if ( c.isSpace() )
            safename += SAFE_SPACE;
    }
    if ( safename.isEmpty() )
        safename = SAFE_SPACE;
    else if ( safename.at(0).isNumber() )
        safename.prepend( SAFE_SPACE );

    return safename;
}

bool ContentLinkPrivate::prepareDirectories(const QString& lf)
{
    if (lf.isNull() || lf.isEmpty())
        return true;

    QFileInfo fi(lf);
    if (!fi.isDir()){
        fi = QFileInfo(fi.absolutePath());
    }

    QString dirPath = fi.absoluteFilePath();
    if ( !fi.exists() ) {
        // mkpath should be static, but it isn't, so use an empty object.
        if ( !QDir().mkpath( dirPath ) ) {
            qLog(DocAPI) << "QContent::prepareDirectories : System failed to create directory path " << dirPath;
             return false;
        }
    }
    return true;
}

QList< QDrmRights::Permission > ContentLinkPrivate::extractPermissions( const QString &permissionList )
{
    QStringList permissionStrings = permissionList.toLower().split( ';' );

    QList< QDrmRights::Permission > permissions;

    foreach( QString permission, permissionStrings )
    {
        if( permission.isEmpty() )
            continue;

        if( permission == QLatin1String( "play" ) )
            permissions.append( QDrmRights::Play );
        else if( permission == QLatin1String( "display" ) )
            permissions.append( QDrmRights::Display );
        else if( permission == QLatin1String( "execute" ) )
            permissions.append( QDrmRights::Execute );
        else if( permission == QLatin1String( "print" ) )
            permissions.append( QDrmRights::Print );
        else if( permission == QLatin1String( "export" ) )
            permissions.append( QDrmRights::Export );
        else if( permission == QLatin1String( "browsecontents" ) )
            permissions.append( QDrmRights::BrowseContents );
        else
            permissions.append( QDrmRights::Unrestricted );
    }

    return permissions;
}

QString ContentLinkPrivate::packPermissions( const QList< QDrmRights::Permission > &permissions )
{
    QStringList permissionStrings;

    foreach( QDrmRights::Permission permission, permissions )
    {
        switch( permission )
        {
            case QDrmRights::Play:
                permissionStrings.append( QLatin1String( "play" ) );
            case QDrmRights::Display:
                permissionStrings.append( QLatin1String( "display" ) );
            case QDrmRights::Execute:
                permissionStrings.append( QLatin1String( "execute" ) );
            case QDrmRights::Print:
                permissionStrings.append( QLatin1String( "print" ) );
            case QDrmRights::Export:
                permissionStrings.append( QLatin1String( "export" ) );
            case QDrmRights::BrowseContents:
                permissionStrings.append( QLatin1String( "browsecontents" ) );
            default:
                permissionStrings.append( QLatin1String( "unrestricted" ) );
        }
    }

    return permissionStrings.join( QLatin1String( ";" ) );
}

/*!
  Default constructor
*/
ContentLinkPrivate::ContentLinkPrivate()
    : QSharedData()
    , valid( UnknownValidity )
    , cSize(-1)
    , cPermissions( QDrmRights::InvalidPermission )
    , cContentState( New )
    , cId( QContent::InvalidId)
    , um( QContent::UnknownUsage )
    , drm( QContent::Unprotected )
    , cNameChanged( false )
    , cExtraLoaded(0)
    , batchLoading( false )
{
}

/*!
  Destructor
*/
ContentLinkPrivate::~ContentLinkPrivate()
{
}

/*!
  Copy constructor
*/
ContentLinkPrivate::ContentLinkPrivate( const ContentLinkPrivate &copy )
    : QSharedData()
{
    (*this) = copy;  // use assignment operator
}

/*!
  Return the user visible name for this content object
*/

const QString &ContentLinkPrivate::name() const
{
    if (!(cExtraLoaded & NameTranslated)) {
        ContentLinkPrivate *that = const_cast<ContentLinkPrivate*>(this);
        if (!linkFile().isEmpty()) {
            QString trFile = property( QLatin1String("File"), QLatin1String("Translation") );
            QString trContext = property( QLatin1String("Context"), QLatin1String("Translation") );
            if (!trFile.isEmpty() && !trContext.isEmpty())
                that->cName = Qtopia::translate( trFile, trContext, cName );
        }
        that->cExtraLoaded |= NameTranslated;
    }
    return cName;
}

/*!
  Return the RFC2045 mime-type for the content
*/
const QString &ContentLinkPrivate::type() const
{
    return cType;
}

const QString &ContentLinkPrivate::comment() const
{
    if (cComment.isEmpty())
        const_cast<ContentLinkPrivate *>(this)->cComment = QMimeType( cType ).description();
    return cComment;
}

ContentLinkPrivate::ContentState ContentLinkPrivate::contentState() const
{
    return cContentState;
}

const QString &ContentLinkPrivate::file() const
{
    // QTOPIA_DOCAPI_TODO: cleanup .desktop file usage...
    if ( cPath.isNull() && !type().startsWith(QLatin1String("Folder/"))) {
        ContentLinkPrivate* that = (ContentLinkPrivate*)this;
        QString ext = QMimeType(cType).extension();
        if ( !ext.isEmpty() )
            ext = QLatin1String(".") + ext;
        if ( !cLinkFile.isEmpty() ) {
            that->cPath =
                    cLinkFile.right(8)==QLatin1String(".desktop") // 8 = strlen(".desktop")
                    ? cLinkFile.left(cLinkFile.length()-8) : cLinkFile;
        } else if ( cType.contains('/') ) {
            QString documentPath;

            if( cMedia.isEmpty() )
                documentPath = Qtopia::documentDir();
            else
            {
                QFileSystem fs = QFileSystem::fromFileName( cMedia );

                if( !fs.isNull() )
                    documentPath = fs.documentsPath() + '/';
                else
                    documentPath = Qtopia::documentDir();
            }

            that->cPath =
                    QString(documentPath+cType+QLatin1String("/")+safeFileName(that->cName));
            if ( QFile::exists(that->cPath+ext) || QFile::exists(that->cPath+QLatin1String(".desktop")) ) {
                int n=1;
                QString nn;
                while (QFile::exists((nn=(that->cPath+QLatin1String("_")+QString::number(n)))+ext)
                       || QFile::exists(nn+QLatin1String(".desktop")))
                    n++;
                that->cPath = nn;
            }
            that->cPath += ext;
        }
        prepareDirectories(that->cPath);
        if ( !that->cPath.isEmpty() ) {
            qLog(DocAPI) << "File does not exist - creating" << that->cPath;
            QFile f(that->cPath);
            if ( !f.open(QIODevice::WriteOnly) ) {
                that->cPath = QString::null;
            } else {
                that->valid = Valid;
                that->drm = QContent::Unprotected;
            }
            return that->cPath;
        }
    }
    return cPath;
}

void ContentLinkPrivate::syncFileName()
{
    if( !cNameChanged )
        return;

    cNameChanged = false;

    QString path;
    QString file;
    QString ext;

    int index = cPath.lastIndexOf( QDir::separator() );

    if( index != -1 )
    {
        path = cPath.left( index );
        file = cPath.mid( index + 1 );
    }
    else
        file = cPath;

    index = file.indexOf( '.' );

    if( index != -1 )
        ext = file.mid( index );

    QString baseFile = path + QDir::separator() + safeFileName( cName );
    file = baseFile + ext;

    index = 0;

    while( QFile::exists( file ) )
        file = baseFile + '_' + QString::number( index++ ) + ext;

    if( QFile::rename( cPath, file ) )
        cPath = file;
}

ContentLinkPrivate &ContentLinkPrivate::operator=( const ContentLinkPrivate &rhs )
{
    this->cId = rhs.cId;
    this->cName = rhs.cName;
    this->cIconPath = rhs.cIconPath;
    this->cType = rhs.cType;
    this->cComment = rhs.cComment;
    this->cPath = rhs.cPath;
    this->valid = rhs.valid;
    this->cRotation = rhs.cRotation;
    this->cMimeTypes = rhs.cMimeTypes;
    this->cMimeTypeIcons = rhs.cMimeTypeIcons;
    this->cMimeTypePermissions = rhs.cMimeTypePermissions;
    this->cIcon = rhs.cIcon;
    this->cLinkFile = rhs.cLinkFile;
    this->cCategories = rhs.cCategories;
    this->cExtraLoaded = rhs.cExtraLoaded;
    this->cPropertyList = rhs.cPropertyList;
    this->batchLoading = rhs.batchLoading;
    this->cLastUpdated = rhs.cLastUpdated;
    this->cPermissions = rhs.cPermissions;
    this->cContentState = rhs.cContentState;
    this->cSize = rhs.cSize;
    this->um = rhs.um;
    this->drm = rhs.drm;
    this->cNameChanged = rhs.cNameChanged;

    return (*this);
}

void ContentLinkPrivate::loadIcon( bool force )
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    if ( QApplication::type() == QApplication::Tty )
        return;

    if( !cIcon.isNull() && !force )
        return;

    if( iconName().isEmpty() )
    {
        if( !type().isEmpty() )
        {
            // Use the icon associated with the mime-type.
            QMimeType mt( type() );
            cIcon = mt.icon();
            if (!cIcon.isNull())
                return;
        }
    }
    else
    {
        cIcon = QIcon(QLatin1String(":icon/") + iconName());
        if (!cIcon.isNull())
            return;
    }

    cIcon = unknownDocIcon();
}

QIcon ContentLinkPrivate::unknownDocIcon()
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    static QIcon icon;

    if( icon.isNull() && QApplication::type() != QApplication::Tty )
        icon = QIcon( QLatin1String(":image/qpe/UnknownDocument") );

    return icon;
}

QPixmap ContentLinkPrivate::unknownDocPixmap( int size )
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    QString suffix = QString::number(size);
    QPixmap pm;

    QString name = QLatin1String("_QPE_Global_UnknownDocument") + suffix;
    if( QPixmapCache::find(name, pm) )
        return pm;

        // Create unknown document icons as required
    pm = QPixmap( QLatin1String(":image/qpe/UnknownDocument") ).scaled(size,size);
    QPixmapCache::insert(name, pm);

    return pm;
}

void ContentLinkPrivate::load( QHash<QString, QVariant> const&m )
{
    // TODO - these hardcoded field names are yuk
    cName = m.value(QLatin1String("uiName")).toString();
    cType = m.value(QLatin1String("mimeType")).toString();
    cIconPath = m.value(QLatin1String("icon")).toString();
    cComment = m.value(QLatin1String("comment")).toString();
    cPath = m.value(QLatin1String("path")).toString();
    cLinkFile = m.value(QLatin1String("linkFile")).toString();
    cMimeTypes = m.value(QLatin1String("mimeTypes")).toString().split(QLatin1String(";"));
    if (m.contains(QLatin1String("lastUpdated")))
        cLastUpdated.setTime_t( m.value( QLatin1String("lastUpdated") ).toUInt() );
}

/*!
  \internal
  note the f.exists call causes the QFileInfo to do a stat, so once
  that cost is incurred also check the size
*/
bool ContentLinkPrivate::isValid(bool force) const
{
    if (um == QContent::Application)
        return true;
    if (!force && valid != UnknownValidity)
        return valid == Valid;
    if(type().startsWith(QLatin1String("Folder/")))
        return true;

    if(!linkFile().isEmpty() && QFileInfo(linkFile()).exists())
    {
        ContentLinkPrivate *that=const_cast<ContentLinkPrivate *>(this);
        that->valid = Valid;
        //QTOPIA_DOCAPI_TODO check if we're in the app path, if so, no pathing info on cPath needed.
        if(linkFile().isEmpty())
            qLog(DocAPI) << "isValid called on empty file!!";
        qLog(DocAPI) << "isValid linkfile" << linkFile() << valid;
    }
    else
    {
        QFileInfo f( cPath );
        ContentLinkPrivate *that=const_cast<ContentLinkPrivate *>(this);
        that->valid = f.exists() ? Valid : Invalid;
        if(cPath.isEmpty())
            qLog(DocAPI) << "isValid called on empty file!!";
        qLog(DocAPI) << "isValid file" << cPath << valid;
    }
    return valid == Valid;
}

/*!
  Return the plaintext size of the content in bytes
*/
qint64 ContentLinkPrivate::size() const
{
    if (cSize == -1)
    {
        if (fileKnown() && isValid())
        {
            QFileInfo fi;
            if (!cLinkFile.isEmpty())
                fi = QFileInfo(cLinkFile);
            else if (!cPath.isEmpty())
                fi = QFileInfo(cPath);
            else
                return -1;
            const_cast<ContentLinkPrivate *>(this)->cSize = fi.size();
            return cSize;
        }
        else
            return -1;
    }
    else
        return cSize;
}

/*!
  \internal
*/
const QIcon &ContentLinkPrivate::icon() const
{
    if (cIcon.isNull()) {
        ContentLinkPrivate *that=const_cast<ContentLinkPrivate *>(this);
        that->loadIcon();
    }
    return cIcon;
}

/*!
  \internal
 */
const QIcon &ContentLinkPrivate::icon( const QDrmRights::Permission permission ) const
{
    Q_UNUSED( permission );

    return icon();
}

/*!
  \overload
  Executes the application associated with this AppLnk.

  \sa exec()
 */
void ContentLinkPrivate::execute() const
{
    execute(QStringList());
}

/*!
  Executes the application associated with this AppLnk, with
  \a args as arguments.

  \sa exec()
 */
void ContentLinkPrivate::execute(const QStringList& args) const
{
#ifdef Q_WS_QWS
    if (um == QContent::Application)
    {
        qLog(DocAPI) << "ContentLinkPrivate::invoke" << cPath << args;
        Qtopia::execute( cPath, args.count() ? args[0] : QString::null );
    }
    else
    {
        QMimeType mt(type());
        QContent app = mt.application();
        if ( app.isValid() ) {
            QStringList a = args;
            if ( !linkFile().isNull() && QFile::exists( linkFile() ) )
                a.append(linkFile());
            else
                a.append(file());
            app.execute(a);
        }
    }
#else
    Q_UNUSED(args)
#endif
}

const QString &ContentLinkPrivate::iconName() const
{
    return cIconPath;
}

const QList< QDrmRights::Permission > &ContentLinkPrivate::mimeTypePermissions() const
{
    if( !(cExtraLoaded & MimePermissions) && um == QContent::Application )
    {
        ContentLinkPrivate *that=const_cast<ContentLinkPrivate *>(this);

        that->cMimeTypePermissions = extractPermissions( property( QLatin1String("MimeTypePermissions"), QLatin1String("DRM") ) );

        that->cExtraLoaded |= MimePermissions;
    }

    return cMimeTypePermissions;
}

void ContentLinkPrivate::setProperty(const QString& key, const QString& value, const QString& group)
{
    if( !(cExtraLoaded & Properties) )
    {
        cPropertyList = persistenceEngine()->readProperties( cId );

        cExtraLoaded |= Properties;
    }

    QPair<QString, QString> realKey(key,group);

    cPropertyList[realKey] = value;
}

QString ContentLinkPrivate::property(const QString& key, const QString& group) const
{
    QPair<QString, QString> realKey(key,group);
    if (cExtraLoaded & Properties) {
        if (cPropertyList.contains(realKey))
            return cPropertyList[realKey];
    } else {
        ContentLinkPrivate *that = const_cast<ContentLinkPrivate*>(this);
        that->cPropertyList = that->persistenceEngine()->readProperties(cId);
        that->cExtraLoaded |= Properties;
        if (cPropertyList.contains(realKey))
            return cPropertyList[realKey];
    }

    return QString::null;
}

void ContentLinkPrivate::removeFiles()
{
    QFile::remove( cPath );
}

QDateTime ContentLinkPrivate::createdDate() const
{
    if (fileKnown() && isValid())
    {
        QFileInfo fi;
        if (!cLinkFile.isEmpty())
            fi = QFileInfo(cLinkFile);
        else if (!cPath.isEmpty())
            fi = QFileInfo(cPath);
        else
            return QDateTime();
        return fi.created();
    }
    else
        return QDateTime();
}

QDateTime ContentLinkPrivate::modifiedDate() const
{
    if (fileKnown() && isValid())
    {
        QFileInfo fi;
        if (!cLinkFile.isEmpty())
            fi = QFileInfo(cLinkFile);
        else if (!cPath.isEmpty())
            fi = QFileInfo(cPath);
        else
            return QDateTime();
        return fi.lastModified();
    }
    else
        return QDateTime();
}

ContentLinkSql *ContentLinkPrivate::persistenceEngine()
{
    if (!ContentLinkPrivate::dbs.hasLocalData())
        ContentLinkPrivate::dbs.setLocalData(new ContentLinkSql);
    return ContentLinkPrivate::dbs.localData();
}

const QString &ContentLinkPrivate::media() const
{
    static const QString nullMedia;

    if( !cMedia.isEmpty() )
        return cMedia;
    else if( !cPath.isEmpty() )
    {
        QStorageMetaInfo storage;

        const QFileSystem *system = storage.fileSystemOf( cPath );

        if( system )
        {
            const_cast< ContentLinkPrivate * >( this )->cMedia = system->path();

            return cMedia;
        }
    }

    return nullMedia;
}

bool ContentLinkPrivate::setMedia( const QString &media )
{
    if( !media.isEmpty() && !fileKnown()  )
    {
        QString newMedia = media;

        if( newMedia.endsWith( "/" ) )
            newMedia.chop( 1 );

        QStorageMetaInfo storage;

        QFileSystemFilter fsf;

        fsf.documents = QFileSystemFilter::Set;

        foreach( QFileSystem *system, storage.fileSystems( &fsf ) )
        {
            if( system->path() == newMedia )
            {
                cMedia = newMedia;

                return true;
            }
        }
    }
    return false;
}


QIODevice *ContentLinkPrivate::open(QIODevice::OpenMode mode) const
{
    QString fn = file();
    if (mode & QIODevice::WriteOnly || mode & QIODevice::Append
        || mode & QIODevice::Truncate) {
        prepareDirectories(fn);
    }
    QFile* fl = new QFile(fn);
    if (!fl->open(mode)) {
        delete fl;
        fl = 0;
        qWarning() << "Failed to open" << fn;
    }
    return fl;
}

const QStringList &ContentLinkPrivate::mimeTypeIcons() const
{
    if( !(cExtraLoaded & MimeIcons) )
    {
        ContentLinkPrivate *that=const_cast<ContentLinkPrivate *>(this);

        that->cMimeTypeIcons = property( QLatin1String("MimeTypeIcons"), QLatin1String("Desktop Entry") ).split( ';' );
        that->cExtraLoaded |= MimeIcons;
    }

    return cMimeTypeIcons;
}

QDebug &operator<<(QDebug &debug, const ContentLinkPrivate *contentLink)
{
    debug << "(cId:" << contentLink->cId
            << "cName:" << contentLink->cName
            << "cIconPath:" << contentLink->cIconPath
            << "cType:" << contentLink->cType
            << "cComment:" << contentLink->cComment
            << "cPath:" << contentLink->cPath
            << "valid:" << contentLink->valid
            << "cRotation:" << contentLink->cRotation
            << "cMimeTypes:" << contentLink->cMimeTypes
            << "cMimeTypeIcons:" << contentLink->cMimeTypeIcons
            << "cMimeTypePermissions:" << contentLink->cMimeTypePermissions
            << "cLinkFile:" << contentLink->cLinkFile
            << "cPropertyList:" << contentLink->cPropertyList
            << "cCategories:" << contentLink->cCategories
            << "cSize:" << contentLink->cSize
            << "cLastUpdated:" << contentLink->cLastUpdated
            << "cPermissions:" << contentLink->cPermissions
            << "um:" << contentLink->um
            << "drm:" << contentLink->drm
            << "batchLoading:" << contentLink->batchLoading
            << ")";
    return debug;
}

////////////////////////////////////////////////////////////////////////
/////
/////   ContentLinkPrivateDRM - container for DRM content


ContentLinkPrivateDRM::ContentLinkPrivateDRM()
    : ContentLinkPrivate()
{
    cPermissions = QDrmRights::InvalidPermission;
}

ContentLinkPrivateDRM::ContentLinkPrivateDRM( const ContentLinkPrivateDRM &copy )
    : ContentLinkPrivate()
{
    *this = copy;
}

ContentLinkPrivateDRM::ContentLinkPrivateDRM( const ContentLinkPrivate &copy )
    : ContentLinkPrivate( copy )
{
}

ContentLinkPrivateDRM::~ContentLinkPrivateDRM()
{
}

ContentLinkPrivateDRM &ContentLinkPrivateDRM::operator =( const ContentLinkPrivateDRM &rhs )
{
    ContentLinkPrivate::operator =( rhs );

    cInvalidIcon = rhs.cInvalidIcon;

    return *this;
}

const QIcon &ContentLinkPrivateDRM::icon() const
{
    return icon( QMimeType( cType ).permission() );
}

const QIcon &ContentLinkPrivateDRM::icon( QDrmRights::Permission permission ) const
{
    return ( permission != QDrmRights::Unrestricted && permissions() & permission )
            ? ContentLinkPrivate::icon()
            : invalidIcon();
}

const QIcon &ContentLinkPrivateDRM::invalidIcon() const
{
    if( cInvalidIcon.isNull() )
        const_cast< ContentLinkPrivateDRM * >( this )->loadInvalidIcon();

    return cInvalidIcon;
}

const QString &ContentLinkPrivateDRM::comment() const
{
        return ContentLinkPrivate::comment();
}

QDrmRights ContentLinkPrivateDRM::rights( const QDrmRights::Permission permission ) const
{
    if( drm == QContent::Unprotected )
        return ContentLinkPrivate::rights( permission );

    return DrmContentPrivate::getRights( cPath, permission );
}

QDrmRights::Permissions ContentLinkPrivateDRM::permissions( bool force ) const
{
    if( force || cPermissions == QDrmRights::InvalidPermission )
        const_cast< ContentLinkPrivateDRM * >( this )->cPermissions = DrmContentPrivate::permissions( cPath );

    return cPermissions;
}

qint64 ContentLinkPrivateDRM::size() const
{
    if( cSize < 0 )
        const_cast< ContentLinkPrivateDRM * >( this )->cSize = DrmContentPrivate::unencryptedSize( cPath );

    return cSize;
}

#ifdef Q_WS_QWS
void ContentLinkPrivateDRM::execute(const QStringList& args) const
{
    if (um == QContent::Application)
    {
        if( DrmContentPrivate::activate( QContent(cPath), QDrmRights::Execute ) )
            Qtopia::execute( cPath, args.count() ? args[0] : QString::null );
    }
    else
    {
        QMimeType mt(type());
        QContent app = mt.application();
        if (app.isValid())
        {
            QString path = !linkFile().isEmpty() && QFile::exists( linkFile() ) ? linkFile() : file();

            if( DrmContentPrivate::activate( QContent(path), mt.permission() ) )
                app.execute( QStringList() << path );
        }
    }
}
#endif

void ContentLinkPrivateDRM::loadIcon( bool force )
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    if( !cIcon.isNull() && !force )
        return;

    if( iconName().isEmpty() && !type().isEmpty() )
    {
        // Use the icon associated with the mime-type.
        QMimeType mt( type() );
        cIcon = mt.icon( QMimeType::DrmValid );
        if (!cIcon.isNull())
            return;
    }

    if( !cIcon.isNull() )
        cIcon = QIcon();

    QPixmap smallPixmap = pixmap( qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ), true );
    QPixmap largePixmap = pixmap( qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ), true );

    if( !smallPixmap.isNull() ) cIcon.addPixmap( smallPixmap );
    if( !largePixmap.isNull() ) cIcon.addPixmap( largePixmap );

    if( cIcon.isNull() )
        cIcon = unknownDocIcon( true );
}

void ContentLinkPrivateDRM::loadInvalidIcon( bool force )
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    if( !cInvalidIcon.isNull() && !force )
        return;

    if( iconName().isEmpty() && !type().isEmpty() )
    {
        // Use the icon associated with the mime-type.
        QMimeType mt( type() );
        cInvalidIcon = mt.icon( QMimeType::DrmInvalid );
        if (!cInvalidIcon.isNull())
        return;
    }

    if( !cInvalidIcon.isNull() )
        cIcon = QIcon();

    QPixmap smallPixmap = pixmap( qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ), false );
    QPixmap largePixmap = pixmap( qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ), false );

    if( !smallPixmap.isNull() ) cInvalidIcon.addPixmap( smallPixmap );
    if( !largePixmap.isNull() ) cInvalidIcon.addPixmap( largePixmap );

    if( cInvalidIcon.isNull() )
        cInvalidIcon = unknownDocIcon( false );
}

QPixmap ContentLinkPrivateDRM::pixmap( int size, bool valid ) const
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    QString suffix = QString::number(size);
    QPixmap pm;

    if( !iconName().isEmpty() )
        {
        QString key = QLatin1String("_QPE_DrmValid") + cPath + '_' + cIconPath + suffix;

        // Applications have icons associated with them.
        if ( QPixmapCache::find(key, pm) )
            return pm;

        // Load up the application's icon.
        pm = ContentLinkPrivate::icon().pixmap(size, size);

        if ( !pm.isNull() )
        {
            pm = DrmContentPrivate::compositeDrmIcon( pm, size, valid );
            QPixmapCache::insert(key, pm);
        }
        }

    return pm;
}

QIcon ContentLinkPrivateDRM::unknownDocIcon( bool valid )
{
    QIcon icon;

    icon.addPixmap( unknownDocPixmap( qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ), valid ) );
    icon.addPixmap( unknownDocPixmap( qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ), valid ) );

    return icon;
}

QPixmap ContentLinkPrivateDRM::unknownDocPixmap( int size, bool valid )
{
    static QMutex guardMutex(QMutex::Recursive);   // QTOPIA_DOCAPI_TODO move this to a read/write locker.
    QMutexLocker guard(&guardMutex);

    QString suffix = QString::number(size);
    QPixmap pm;

    QString name = (valid
            ? QLatin1String("_QPE_Global_UnknownValidDrmDocument")
            : QLatin1String("_QPE_Global_UnknownInvalidDrmDocument") ) + suffix;

    if( QPixmapCache::find(name, pm) )
        return pm;

        // Create unknown document icons as required
    pm = DrmContentPrivate::compositeDrmIcon( ContentLinkPrivate::unknownDocPixmap( size ), size, valid );
    QPixmapCache::insert(name, pm);

    return pm;
}

void ContentLinkPrivateDRM::removeFiles()
{
    DrmContentPrivate::deleteFile( cPath );
}

