/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "qmimetype.h"
#include "contentlnksql_p.h"
#include <qtopiaglobal.h>
#include <qcontent.h>
#include <qcontentset.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#else
#include <QApplication>
#include <qtopianamespace.h>
#endif

#include <QHash>
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QPixmapCache>
#include <QDir>
#include <QMutexLocker>
#include <QStyle>
#include <QSettings>
#include <QtGlobal>
#include <qtopialog.h>

#include "drmcontent_p.h"

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

class QMimeTypeAppManager : public QObject
{
    Q_OBJECT
public:
    QMimeTypeAppManager() : QObject() {
        apps = new QContentSet(QContentFilter(QContent::Application), this);
        connect(apps, SIGNAL(changed(const QContentIdList&,QContent::ChangeType)),
            this, SLOT(appsChanged(const QContentIdList&,QContent::ChangeType)));
        connect(apps, SIGNAL(changed()), this, SLOT(appsChanged()));
    }

private slots:
    void appsChanged(const QContentIdList &cids, QContent::ChangeType ct) {
        if (ct == QContent::Added) {
            foreach (QContentId cid, cids)
                QMimeType::registerApp(QContent(cid));
        } else {
            QMimeType::updateApplications();
        }
    }
    void appsChanged()
    {
        QMimeType::updateApplications();
    }

private:
    QContentSet *apps;
};

Q_GLOBAL_STATIC(QMimeTypeAppManager, appManager);

class QMimeTypeData {
public:
    QMimeTypeData(const QString& i) :
        id(i)
    {
    }
    QString id;
    QString extension;
    QContentList apps;
    QStringList icons;
    QList<QDrmRights::Permission> permissions;

    ~QMimeTypeData()
    {
    }

    const QString &description()
    {
#ifndef QTOPIA_CONTENT_INSTALLER
        if ( desc.isEmpty() )
            desc = QtopiaApplication::tr("%1 document").arg(apps.last().name());
#endif
        return desc;
    }

    const QIcon &icon( QMimeType::IconType type )
    {
        switch( type )
        {
            case QMimeType::DrmValid:
            {
                if ( drmValidIcon.isNull() )
                    loadPixmaps( type );
                return drmValidIcon;
            }
            case QMimeType::DrmInvalid:
            {
                if ( drmInvalidIcon.isNull() )
                    loadPixmaps( type );
                return drmInvalidIcon;
            }
            default:    // QMimeType::Default. Not explicit as compiler will warn about not all
            {           // code paths returning a value.
                if ( icn.isNull() )
                    loadPixmaps( type );
                return icn;
            }
        }
    }

private:
    void loadPixmaps( QMimeType::IconType type )
    {
        if ( icons.count() ) {
            QString icon = icons.first();
            QContent content = apps.first();

            int smallSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize);
            int bigSize   = qApp->style()->pixelMetric(QStyle::PM_LargeIconSize);

            if ( icon.isNull() ) {
                icn = content.icon();

                if( type == QMimeType::DrmValid )
                    drmValidIcon = DrmContentPrivate::createIcon( icn, smallSize, bigSize, true );
                else if( type == QMimeType::DrmInvalid )
                    drmInvalidIcon = DrmContentPrivate::createIcon( icn, smallSize, bigSize, false );
            } else {
                icn = QIcon(QLatin1String(":icon/") + icon);
                switch( type )
                {
                    case QMimeType::DrmValid:
                        drmValidIcon = DrmContentPrivate::createIcon( icn, smallSize, bigSize, true );
                        break;
                    case QMimeType::DrmInvalid:
                        drmInvalidIcon = DrmContentPrivate::createIcon( icn, smallSize, bigSize, false );
                    default:
                        break;
                }
            }
        }
    }

    QIcon icn;
    QIcon drmValidIcon;
    QIcon drmInvalidIcon;
    QString desc;
};

class QMimeType::Private : public QHash<QString,QMimeTypeData*> {
public:
    Private() {}
    ~Private()
    {   // implement "autoDelete"
        QStringList keylist = keys();
        foreach ( QString k, keylist )
        {
            delete take( k );
        }
    }

    // ...
};


QMimeType::Private* QMimeType::d=0;
typedef QHash<QString, QString> typeForType;
Q_GLOBAL_STATIC(typeForType, typeFor);
typedef QHash<QString, QStringList> extForType;
Q_GLOBAL_STATIC(extForType, extFor);
typedef QHash<QString, QDateTime> loadedTimesType;
Q_GLOBAL_STATIC(loadedTimesType, loadedTimes);

QMutex QMimeType::staticsGuardMutex(QMutex::Recursive);

QMimeType::Private& QMimeType::data()
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    if ( !d ) {
        d = new Private;
        static bool setCleanup = false;
        if ( !setCleanup ) {
            qAddPostRoutine( QMimeType::clear );
            setCleanup = true;
        }
    }
    return *d;
}

/*!
    \class QMimeType
    \brief The QMimeType class provides MIME type information.

    A QMimeType object is a light-weight value which
    provides MIME type information.

    \ingroup qtopiaemb
*/

/*!
    \enum QMimeType::IconType
    IconType describes the properties of the content an icon represents.

    \value Default The icon is displayed for content with no special properties.
    \value DrmValid The icon is displayed for drm protected content with current valid rights.
    \value DrmInvalid The icon is displayed for drm protected content with no current valid rights.
*/

/*!
    The QMimeType class Constructs a QMimeType.  The parameter \a ext_or_id is a MIME type, however, it
    is interpreted as a filename if all of the following conditions hold:
    \list
    \o it is not in the known list of MIME types
    \o if either QFile( \a ext_or_id).exists() is true, or if ext_or_id starts with a '/' or contains no
    '/'
    \endlist
    If it is interpreted as a file name, the file extension (for exmaple, .txt)
    is used as the MIME type.
    If none of the above is true, then it is treated as an unknown  MIME type.
*/
QMimeType::QMimeType( const QString& ext_or_id )
{
    init(ext_or_id);
}

/*!
    Constructs a QMimeType from the type() of \a lnk.
*/
QMimeType::QMimeType( const QContent& lnk )
{
    init(lnk.type());
}

/*!
    Returns the MIME type identifier.
*/
QString QMimeType::id() const
{
    return mimeId;
}

/*!
    Returns a description of the MIME Type. This is usually based
    on the application() associated with the type.
*/
QString QMimeType::description() const
{
    QMimeTypeData* mtd = data(mimeId);
    return mtd ? mtd->description() : QString::null;
}

/*!
    Returns an \a iconType icon appropriate for the MIME type.
*/
QIcon QMimeType::icon( IconType iconType ) const
{
    QMimeTypeData* mtd = data(mimeId);

    return mtd ? mtd->icon( iconType ) : QIcon();
}


/*!
    \internal
    This function is not generally available.
*/
QString QMimeType::extension() const
{
    QStringList exts = extensions();
    if (exts.count())
        return extensions().first();
    return QString::null;
}

/*!
    Returns the list of file extensions associated with the MIME type.
*/
QStringList QMimeType::extensions() const
{
    loadExtensions();
    QHash<QString,QStringList>::const_iterator it = extFor()->find(mimeId);
    if (it != extFor()->end())
        return *it;
    return QStringList();
}

/*!
    \internal
    This function is not generally available.
*/
QContentList QMimeType::applications() const
{
    QMimeTypeData* mtd = data(mimeId);
    return mtd ? mtd->apps : QContentList();
}

/*!
    Returns the QContent defining the application associated
    with this MIME type, or an invalid QContent if none is associated.

    \sa QtopiaService::binding()
*/
QContent QMimeType::application() const
{
    QMimeTypeData* mtd = data(mimeId);
    return mtd ? mtd->apps.last() : QContent();
}

/*!
    Returns the permission type used by the associated application to open
    a file of this MIME type.  If no application is associated or the application
    does not support DRM QDrmContent::Unrestricted is returned.
*/
QDrmRights::Permission QMimeType::permission() const
{
    QMimeTypeData* mtd = data(mimeId);
    return mtd && !mtd->permissions.isEmpty() ? mtd->permissions.first() : QDrmRights::Unrestricted;
}

/*!
    \internal
    Returns the permissions used by the associated applications.
    Permissions are mapped one to one with  the list returned by applications().
*/
QList< QDrmRights::Permission > QMimeType::permissions() const
{
    QMimeTypeData* mtd = data(mimeId);
    return mtd ? mtd->permissions : QList< QDrmRights::Permission >();
}

static QString serviceBinding(const QString& service)
{
    // Copied from qtopiaservices
    QString svrc = service;
    for (int i=0; i<(int)svrc.length(); i++)
        if ( svrc[i]=='/' ) svrc[i] = '-';
    return "Service-"+svrc; // No tr
}

/*!
    \internal
*/
void QMimeType::registerApp( const QContent& lnk )
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    QStringList list = lnk.mimeTypes();
    QStringList icons = lnk.mimeTypeIcons();
    QList< QDrmRights::Permission > permissions = lnk.mimeTypePermissions();
    int index = 0;
    foreach (QString type, list) {
        if( type.isEmpty() )
            continue;

        QMimeTypeData* cur = NULL;

        if( data().contains( type ) )
            cur = data()[ type ];

        if( !cur )
        {
            cur = new QMimeTypeData( type );
            data().insert( type, cur );
            cur->apps.append( lnk );
            cur->permissions.append( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
            cur->icons.append( index < icons.count() ? icons[ index ] : QString() );
        }
        else if( cur->apps.count() )
        {
            if( type.right(2)==QLatin1String("/*") )
                type.truncate( type.length()-2 );
            QSettings binding( QLatin1String("Trolltech"), serviceBinding( QLatin1String("Open/") + type ) );
            QString def;
            if( binding.status()==QSettings::NoError )
            {
                binding.beginGroup(QLatin1String("Service"));
                def = binding.value(QLatin1String("default")).toString();
            }
            if( lnk.executableName() == def )
            {
                cur->apps.prepend( lnk );
                cur->permissions.prepend( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
                cur->icons.prepend( index < icons.count() ? icons[ index ] : QString() );
            }
            else
            {
                cur->apps.append( lnk );
                cur->permissions.append( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
                cur->icons.append( index < icons.count() ? icons[ index ] : QString() );
            }
        }
        else
        {
            cur->apps.append( lnk );
            cur->permissions.append( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
            cur->icons.append( index < icons.count() ? icons[ index ] : QString() );
        }

        index++;
    }
}

/*!
    \internal
*/
void QMimeType::clear()
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    delete d;
    d = 0;
    extFor()->clear();
    typeFor()->clear();
    loadedTimes()->clear();
}

void QMimeType::loadExtensions()
{
    QStringList paths;
    paths << QLatin1String("/") << Qtopia::installPaths();

    const QString etcMimeTypesConst = QLatin1String("etc/mime.types");

    QMutexLocker staticsGuard(&staticsGuardMutex);
    QFileInfo fi;
    foreach(QString path, paths)
    {
        QString file = path+etcMimeTypesConst;
        fi.setFile(file);
        if(!loadedTimes()->contains(path) || (fi.exists() && fi.created() > loadedTimes()->value(path)))
        {
            loadExtensions(file);
            loadedTimes()->insert(path, fi.created());
        }
    }
}

static QString nextString( const char *line, int& posn )
{
    if ( line[posn] == '\0' )
        return QString::null;
    int end = posn;
    char ch;
    for (;;) {
        ch = line[end];
        if ( ch == '\0' || ch == ' ' || ch == '\t' ||
             ch == '\r' || ch == '\n' ) {
            break;
        }
        ++end;
    }
    const char *result = line + posn;
    int resultLen = end - posn;
    for (;;) {
        ch = line[end];
        if ( ch == '\0' )
            break;
        if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' )
            break;
        ++end;
    }
    posn = end;
    return QString::fromLocal8Bit(result, resultLen);
}

void QMimeType::loadExtensions(const QString& filename)
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    QFile file(filename);
    if ( file.open(QIODevice::ReadOnly) ) {
        char line[1024];
        int posn;
        QStringList newexts, exts;
        QString ext, id;
        while (file.readLine(line, sizeof(line)) >= 0) {
            if (line[0] == '\0' || line[0] == '#')
                continue;
            posn = 0;
            id = nextString(line, posn);
            if ( id.isEmpty() )
                continue;
            id = id.toLower();
            if(extFor()->contains(id))
            exts = extFor()->value(id);
            else
                exts.clear();
            ext = nextString(line, posn);
            newexts.clear();
            while(!ext.isEmpty()) {
                exts.removeAll(ext);
                if ( !newexts.contains(ext) )
                    newexts.append(ext);
                typeFor()->insert(ext, id);
                ext = nextString(line, posn);
            }
            if(!newexts.isEmpty())
                extFor()->insert(id, exts + newexts);
        }
    }
}

void QMimeType::init( const QString& ext_or_id )
{
    if (ext_or_id.isEmpty())
        return;

    loadExtensions();
    initializeAppManager();
    QFile ef( ext_or_id );
    QString mime_sep = QLatin1String("/");
#ifdef Q_OS_WIN32
    mime_sep = QLatin1String(":");
#endif
    // either it doesnt have exactly one mime-separator, or it has
    // a path separator at the beginning
    bool doesntLookLikeMimeString =
        ext_or_id.count( mime_sep ) != 1 ||
        ext_or_id[0] == QDir::separator();

    // do a case insensitive search for a known mime type.
    QString lwrExtOrId = ext_or_id.toLower();
    QHash<QString,QStringList>::const_iterator it = extFor()->find(lwrExtOrId);
    if ( it != extFor()->end() ) {
        mimeId = lwrExtOrId;
    } else if ( doesntLookLikeMimeString || ef.exists() )
    {
        int dot = ext_or_id.lastIndexOf('.');
        QString ext = dot >= 0 ? ext_or_id.mid(dot+1) : ext_or_id;
        mimeId = typeFor()->value(ext.toLower());
#ifdef Q_OS_UNIX
        if ( mimeId.isNull() && ef.exists() )  // try to find from magic
        {
            char buf[ 10 ];
            int fd = ::open( ef.fileName().toLocal8Bit().constData(), O_RDONLY );
            (void)::read( fd, buf, 10 );
            ::close( fd );
            const char elfMagic[] = { '\177', 'E', 'L', 'F', '\0' };
            const char *mptr = elfMagic;
            int e;
            for ( e = 0; e < 4; e++, mptr++ )
                if ( *mptr != buf[e] )
                    break;
            if ( e == 4 )
                mimeId = QLatin1String("application/x-executable");  // could be a shared library or an exe
        }
#endif
        if ( mimeId.isNull() )
            mimeId = QLatin1String("application/octet-stream");
    }
    else  // could be something like application/vnd.oma.rights+object
    {
        mimeId = lwrExtOrId;
    }
    if ( !d )
        updateApplications();
}

QMimeTypeData* QMimeType::data(const QString& id)
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    QMimeTypeData* mtd = NULL;
    if ( !d )
        updateApplications();
    if (data().contains(id)) {
        mtd = data()[id];
    } else {
        int s = id.indexOf('/');
        QString idw = id.left(s)+QLatin1String("/*");
        if (data().contains(idw))
            mtd = data()[idw];
    }
    return mtd;
}

/*!
    Returns a Qtopia folder containing application definitions.
*/
QString QMimeType::appsFolderName()
{
    return Qtopia::qtopiaDir() + QLatin1String("apps");
}

/*!
    Reloads application definitions.
*/
void QMimeType::updateApplications()
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    initializeAppManager();

    QList< ContentLinkSql::MimeData > mimeData = QContent::database()->getMimeData();

    foreach( ContentLinkSql::MimeData mime, mimeData )
    {
        QStringList types = mime.mimeTypes.split( ';' );
        QStringList icons = mime.icons.split( ';' );
        QList< QDrmRights::Permission > permissions = ContentLinkPrivate::extractPermissions( mime.permissions );

        int index = 0;

        foreach( QString type, types )
        {
            if( type.isEmpty() )
                continue;

            QMimeTypeData* cur = NULL;

            if( data().contains( type ) )
                cur = data()[ type ];

            QContent app( mime.cid );

            if ( !cur )
            {
                cur = new QMimeTypeData( type );
                data().insert( type, cur );
                cur->apps.append( app );
                cur->permissions.append( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
                cur->icons.append( index < icons.count() ? icons[ index ] : QString() );
            }
            else if( cur->apps.count() )
            {
                if( type.right(2)==QLatin1String("/*") )
                    type.truncate( type.length()-2 );
                QSettings binding( QLatin1String("Trolltech"), serviceBinding( QLatin1String("Open/") + type ) );
                QString def;
                if( binding.status()==QSettings::NoError )
                {
                    binding.beginGroup(QLatin1String("Service"));
                    def = binding.value(QLatin1String("default")).toString();
                }
                if( app.executableName() == def )
                {
                    cur->apps.prepend( app );
                    cur->permissions.prepend( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
                    cur->icons.prepend( index < icons.count() ? icons[ index ] : QString() );
                }
                else
                {
                    cur->apps.append( app );
                    cur->permissions.append( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
                    cur->icons.append( index < icons.count() ? icons[ index ] : QString() );
                }
            }
            else
            {
                cur->apps.append( app );
                cur->permissions.append( index < permissions.count() ? permissions[ index ] : QDrmRights::Unrestricted );
                cur->icons.append( index < icons.count() ? icons[ index ] : QString() );
            }

            index++;
        }
    }
}

void QMimeType::initializeAppManager()
{
    QMimeTypeAppManager *appMan = appManager();
    if(appMan == NULL)
        // should never hit the below, simply there for if we go fubar, and to stop optimising out the call
        qLog(DocAPI) << "MimeType App Manager initialisation failed";
}

#include "qmimetype.moc"
