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
#include <qtopia/private/qtopiaresource_p.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <QImageIOHandler>
#include <QImageIOPlugin>
#include <QImage>
#include <QPair>
#include <QList>
#include <QCache>

#if QT_VERSION >= 0x040300
#include <QResource>
#endif

#include <QDebug>
#include <QApplication>
#include <QStyle>
#include <qfsfileengine.h>
#include <QImageReader>

QFileResourceFileEngineHandler::QFileResourceFileEngineHandler()
    : QAbstractFileEngineHandler()
{
}

QFileResourceFileEngineHandler::~QFileResourceFileEngineHandler()
{
}

void QFileResourceFileEngineHandler::setIconPath(const QStringList& p)
{
    if (!p.isEmpty())
        iconpath = p;
    imagedirs.clear();
    sounddirs.clear();
}

QAbstractFileEngine *QFileResourceFileEngineHandler::create(const QString &path) const
{
    if ( path.length() > 0 && path[0] == ':' ) {
        if ( imagedirs.isEmpty() ) {
            imagedirs = iconpath;
            QStringList p = Qtopia::installPaths();
            foreach (QString s, p) {
                appendSearchDirs(imagedirs,s,"pics/");
                appendSearchDirs(sounddirs,s,"sounds/");
            }
        }

#if QT_VERSION >= 0x040300
        QAbstractFileEngine * e = findArchivedResourceFile(path);
        if(e)
            return e;
#endif

        QString p = findDiskResourceFile(path);
        if (!p.isNull())
            return new QFSFileEngine(p);
    }
    return 0;
}

void QFileResourceFileEngineHandler::appendSearchDirs(QStringList& dirs,
        const QString& dir, const QString& subdir) const
{
    QString t = dir + subdir;
    if ( QFSFileEngine(t).fileFlags(QFSFileEngine::ExistsFlag) )
        dirs.append(t);
}

/*!
  \page qtopia_resource_system.html
  \title Qtopia Resource System

  The Qtopia resource system allows application programmers to access common
  application resources such as images, icons and sounds without having to
  concern themselves with the exact installation location or file types.  The
  Qtopia resource system is built on top of the Qtopia Core resource model.

  Rather than accessing resource files directly from disk, applications should
  use resource syntax inplace of a regular file name.  For example, the
  following lines refer to the same image:

  \code
  QPixmap pix1("/opt/Qtopia/pics/addressbook/email.png");
  QPixmap pix2(":image/addressbook/email");
  \endcode

  When Qtopia detects the use of the special ":" prefix, searches in various
  locations - depending on the resource type - and for various file types
  to locate the actual resource.  In addition to improving the efficiency of
  reference for the programmer, the Qtopia resource system improves the
  efficiency of access for the system.  Using a special file, known as a
  resource database, Qtopia can bundle many separate images and icons into a
  single archive that is both quick to access and efficiently shared across
  processes.

  A resource database is created using the Qtopia Core \c {rcc} tool in binary
  mode.  Any type of Qtopia supported image and icon can be added to a resource
  database.  A special image type, known as a \i {QRAW} image, is also
  exclusively supported in resource databases.  A \i {QRAW} image is an
  uncompressed raw image that can be mmap'ed directly from disk and efficiently
  displayed on screen with no resident in-memory copies.  As the \i {QRAW} format
  is uncompressed, it is best reserved for small images, very frequently used images,
  or images stored on compressed filesystems.  \i {QRAW} images can be created
  from other image types using the \c {mkqraw} tool included with Qtopia.

  Resource databases are always named \c {qtopia.rdb} and stored in the location
  dictated by the resouces they contain.  The list of search directories
  outlined for each resource type below can be used to determine where a
  resource database be placed.  In general, resource databases are placed in
  either the \c {/opt/Qtopia/pics} or \c {/opt/Qtopia/pics/<application name>}
  directories, but may be located elsewhere in the case of installable software
  or resources.

  Resource databases are only supported in the Qtopia 4.2 series and later.

  The specifics for each resource type are outlined below.

  \section1 Images

  When requesting an image, applications use a "filename" of the form
  \c {<path> := :image/[i18n/][<app name>/]<image>}.  For each search directory
  listed in the $QTOPIA_PATHS environment variable as well as Qtopia's install
  location, the following sub-locations are tried:

  \c {pics/<QApplication::applicationName()>/qtopia.rdb#<app name>/<image>}

  \c {pics/qtopia.rdb#<QApplication::applicationName()>/<app name>/<image>}

  \c {pics/<app name>/qtopia.rdb#<image>}

  \c {pics/qtopia.rdb#<app name>/<image>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/i18n/<language>_<locale>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<app name>/i18n/<language>_<locale>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/i18n/<language>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<app name>/i18n/<language>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/i18n/en_US/<image>.<image extension>}

  \i {i18n only:} \c {pics/<app name>/i18n/en_US/<image>.<image extension>}

  \c {pics/<QApplication::applicationName()>/<app name>/<image>.<image extension>}

  \c {pics/<app name>/<image>.<image extension>}

  In the listing above, \c {<language>} corresponds to Qtopia's configured
  language and \c {<locale>} its configured locale.  The supported
  <image extensions> are currently "png", "jpg", "mng" and no extension.

  For example, in the "addressbook" application

  \code
  // /opt/Qtopia/pics/addressbook/qtopia.rdb#email
  // /opt/Qtopia/pics/qtopia.rdb#addressbook/email
  // /opt/Qtopia/pics/qtopia.rdb#email
  // /opt/Qtopia/pics/addressbook/email.png
  // /opt/Qtopia/pics/addressbook/email.jpg
  // /opt/Qtopia/pics/addressbook/email.mng
  // /opt/Qtopia/pics/addressbook/email
  // /opt/Qtopia/pics/email.png
  // /opt/Qtopia/pics/email.jpg
  // /opt/Qtopia/pics/email.mng
  // /opt/Qtopia/pics/email
  QPixmap pix(":image/email");
  \endcode

  \section1 Icons

  When requesting an icon, applications use a "filename" of the form
  \c {<path> := :icon/[i18n/][<app name>/]<icon>}.  For each search directory
  listed in the $QTOPIA_PATHS environment variable as well as Qtopia's install
  location, the following sub-locations are tried:

  \c {pics/<QApplication::applicationName()>/qtopia.rdb#<app name>/icons/<icon size>/<icon>}

  \c {pics/qtopia.rdb#<QApplication::applicationName()>/<app name>/icons/<icon size>/<icon>}

  \c {pics/<app name>/qtopia.rdb#icons/<icon size>/icon}

  \c {pics/qtopia.rdb#<app name>/icons/<icon size>/icon}

  \c {pics/<QApplication::applicationName()>/<app name>/icons/<icon size>/i18n/<language>_<locale>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<app name>/icons/<icon size>/i18n/<language>_<locale>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/icons/<icon size>/i18n/<language>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<app name>/icons/<icon size>/i18n/<language>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/icons/<icon size>/i18n/en_US/icon.<icon extension>}

  \i {i18n only:} \c {pics/<app name>/icons/<icon size>/i18n/en_US/icon.<icon extension>}

  \c {pics/<QApplication::applicationName()>/icons/<icon size>/<app name>/icon>.<icon extension>}

  \c {pics/<app name>/icons/<icon size>/<icon>.<icon extension>}

  \i {If none found, search for :image/[i18n/][<app name>/]<icon> as though the icon was requested as an image}

  In the listing above, \c {<language>} corresponds to Qtopia's configured
  language and \c {<locale>} its configured locale.  The supported
  <icon extensions> are currently "png", "jpg", "mng" and no extension.

  \section1 Sounds

  When requesting a sound, applications use a "filename" of the form
  \c {<path> := :sound/<sound>}.  For each search directory listed in the
  $QTOPIA_PATHS environment variable as well as Qtopia's install location, the
  following sub-locations are tried:

  \c {sounds/<QApplication::applicationName()>/<sound>.wav}

  \c {sounds/<sound>.wav}
 */

#if QT_VERSION >= 0x040300
QAbstractFileEngine *QFileResourceFileEngineHandler::findArchivedResourceFile(const QString &path) const
{
    if ( path.left(7 /* ::strlen(":image/") */)==":image/" ) {

        QString p1 = path.mid(7 /* ::strlen(:image/") */);
        return findArchivedImage(p1);

    } else if ( path.left(6 /* ::strlen(":icon/") */)==":icon/" ) {

        QString p1 = path.mid(6 /* ::strlen(":icon/") */);
        return findArchivedIcon(p1);

    }

    return 0;
}

/*! Returns an archive identifier if succeeds, empty string if not */
QString QFileResourceFileEngineHandler::loadArchive(const QString &archive) const
{
    static int nextId = 0;

    QMap<QString, QString>::ConstIterator iter =
        m_registeredArchives.find(archive);
    if(iter != m_registeredArchives.end())
        return *iter;

    QString myId = QString("/Qtopia/%1").arg(nextId++);
    if(!QResource::registerResource(archive, myId))
        myId = QString();

    m_registeredArchives.insert(archive, myId);

    return myId;
}

QAbstractFileEngine *QFileResourceFileEngineHandler::findArchivedImage(const QString &_path) const
{
    QString path = _path;

    if(path.startsWith("i18n/"))
        path.remove(0, 5 /* ::strlen("i18n/") */);

    QString myApp = QApplication::applicationName();
    QString app;
    QString image;
    {
        int index = path.indexOf('/');
        if(index != -1) {
            app = path.left(index);
            image = path.mid(index + 1);
        } else {
            image = path;
        }
    }

    QList<QPair<QString, QString > > searchNames;
    if(app.isEmpty()) {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb", image));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/" + image));
        searchNames.append(qMakePair(QString("qtopia.rdb"), image));
    } else {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb", app + "/" + image));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/" + app + "/" + image));
        searchNames.append(qMakePair(app + "/qtopia.rdb", image));
        searchNames.append(qMakePair(QString("qtopia.rdb"), app + "/" + image));
    }

    foreach (QString searchBase, imagedirs) {
        typedef QPair<QString, QString> SearchName;
        foreach (SearchName searchName, searchNames) {
            QString id = loadArchive(searchBase + "/" + searchName.first);
            if(!id.isEmpty()) {
                QString resName = ":" + id + "/" + searchName.second;
                QResource resource(resName);
                if(resource.isValid()) {
                    qLog(Resource) << "Archived Image Resource " << _path << "->" << resName;
                    return new QResourceFileEngine(resName);
                }
            }
        }
    }

    return 0;
}

/*
     <app name>/qtopia.rdb/icons/<icon size>/<icon>
     qtopia.rdb/<app name>/icons/<icon size>/<icon>
 */
QAbstractFileEngine *QFileResourceFileEngineHandler::findArchivedIcon(const QString &_path) const
{
    QString path = _path;

    if(path.startsWith("i18n/"))
        path.remove(0, 5 /* ::strlen("i18n/") */);

    QString myApp = QApplication::applicationName();
    QString app;
    QString icon;
    {
        int index = path.indexOf('/');
        if(index != -1) {
            app = path.left(index);
            icon = path.mid(index + 1);
        } else {
            icon = path;
        }
    }

    QList<QPair<QString, QString > > searchNames;
    if(app.isEmpty()) {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb",
                                     QString("icons/") + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/icons/" + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     QString("icons/") + icon));
    } else {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb",
                                     app + "/icons/" + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/" + app + "/icons/" + icon));
        searchNames.append(qMakePair(app + "/" + "qtopia.rdb",
                                     QString("icons/") + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     app + "/icons/" + icon));
    }

    foreach (QString searchBase, imagedirs) {
        typedef QPair<QString, QString> SearchName;
        foreach (SearchName searchName, searchNames) {
            QString id = loadArchive(searchBase + "/" + searchName.first);
            if(!id.isEmpty()) {
                QString resName = ":" + id + "/" + searchName.second;
                QResource resource(resName);
                if(resource.isValid()) {
                    qLog(Resource) << "Archived Icon Resource " << _path << "->" << resName;
                    return new QResourceFileEngine(resName);
                }
            }
        }
    }

    return findArchivedImage(path);
}
#endif

QString QFileResourceFileEngineHandler::findDiskResourceFile(const QString &path) const
{
    // Caching makes sense.  We often look for the same small number of files.
    static QCache<QString,QString> fileCache(25);

    if (QString *cFile = fileCache.object(path))
        return *cFile;

    static const QString image(QLatin1String(":image/"));
    static const QString icon(QLatin1String(":icon/"));
    static const QString sound(QLatin1String(":sound/"));

    QString r;
    if (path.startsWith(image)) {

        QString p1 = path.mid(7 /* ::strlen(:image/") */);
        r = findDiskImage(p1, QString());

    } else if (path.startsWith(icon)) {

        QString p1 = path.mid(6 /* ::strlen(":icon/") */);
        static const QString icons(QLatin1String("icons/"));
        r = findDiskImage(p1, icons);
        if ( r.isEmpty() )
            r = findDiskImage(p1, QString());

    } else if (path.startsWith(sound)) {

        QString p1 = path.mid(7 /* ::strlen(":sound/") */);
        r = findDiskSound(p1);

    } else {

        qLog(Resource) << "Unsupported resource" << path;
    }

    fileCache.insert(path, new QString(r));

    return r;
}

/* _path does NOT include the ":image/" prefix; _subdir is either empty or MUST include trailing "/" */
QString QFileResourceFileEngineHandler::findDiskImage(const QString &_path, const QString& _subdir) const
{
    static QStringList commonFormats;
    static QStringList otherFormats;
    static const QLatin1Char sep('/');
    static const QString i18nDir("i18n/");

    if (!commonFormats.count()) {
        //XXX Only 3 letter extensions supported for common formats.
        commonFormats.append("svg");
#ifndef QT_NO_PICTURE
        commonFormats.append("pic");
#endif
        commonFormats.append("png");
        commonFormats.append("jpg");
        commonFormats.append("mng");

        // Get the rest of the formats Qt supports.
        QList<QByteArray> suppFormats = QImageReader::supportedImageFormats();
        foreach (QByteArray format, suppFormats) {
            if (!commonFormats.contains(format)) {
                otherFormats.append(format);
            }
        }
    }

    QString path = _path;
    QStringList searchNames; // List of names to search for

    bool i18n = false;
    if(path.startsWith(i18nDir)) {
        path.remove(0, 5 /* ::strlen("i18n/") */);
        i18n = true;
    }

    QString myApp = QApplication::applicationName();
    QString app;
    QString image;
    bool knownExtn = false;

    {
        int slash = path.indexOf(sep);
        if(slash != -1) {
            app = path.left(slash);
            image = path.mid(slash + 1);
        } else {
            image = path;
        }
        int dot = image.lastIndexOf('.');
        if ( dot >= 0 ) {
            slash = image.lastIndexOf(sep);
            if (slash < 0 || dot > slash) {
                QString img_extn = image.mid(dot+1);
                if (commonFormats.contains(img_extn)
                    || otherFormats.contains(img_extn)) {
                    knownExtn = true;
                }
            }
        }
    }

    if(i18n) {
        QStringList langs = Qtopia::languageList();
        langs.append(QLatin1String("en_US"));

        foreach(QString lang, langs) {
            if(app.isEmpty()) {
                searchNames.append(myApp + sep + _subdir + i18nDir + lang + sep + image);
                searchNames.append(_subdir + i18nDir + lang + sep + image);
            } else {
                searchNames.append(myApp + sep + app + sep + _subdir + i18nDir + lang + sep + image);
                searchNames.append(app + sep + _subdir + i18nDir + lang + sep + image);
            }
        }
    }
    if(app.isEmpty()) {
        searchNames.append(myApp + sep + _subdir + image);
        searchNames.append(_subdir + image);
    } else {
        searchNames.append(myApp + sep + app + sep + _subdir + image);
        searchNames.append(app + sep + _subdir + image);
    }

    foreach (QString searchBase, imagedirs) {
        searchBase += sep;
        foreach (QString searchName, searchNames) {
            QString r(searchBase + searchName);
            if (!knownExtn) {
                QString fn = r + QLatin1String("....");
                int ext = fn.length()-3;
                // Try our common formats first.
                foreach (QString extn, commonFormats) {
                    fn[ext]=extn[0]; fn[ext+1]=extn[1]; fn[ext+2]=extn[2];
                    if (QFSFileEngine(fn).fileFlags(QFSFileEngine::ExistsFlag)) {
                        qLog(Resource) << extn << "Image Resource" << path << "->" << fn;
                        return fn;
                    }
                }
                // Then anything else Qt supports
                foreach (QString extn, otherFormats) {
                    fn = r + QLatin1Char('.') + extn;
                    if (QFSFileEngine(fn).fileFlags(QFSFileEngine::ExistsFlag)) {
                        qLog(Resource) << extn << "Image Resource" << path << "->" << fn;
                        return fn;
                    }
                }
            } else {
                // File has an extension we know
                if (QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag)) {
                    qLog(Resource) << "Found Image Resource" << path << "->" << r;
                    return r;
                }
            }
        }
    }

    qLog(Resource) << "No resource" << path;
    qLog(Resource) << "  Tried directories:" << imagedirs;
    qLog(Resource) << "  Tried files:" << searchNames;

    return QString();
}

/*!
  \internal
 */
QString QFileResourceFileEngineHandler::findDiskSound(const QString &path) const
{
    QString myApp = QApplication::applicationName();
    QString p1 = path;
    p1 += ".wav";
    foreach (QString s, sounddirs) {
        QString r = s + p1;
        if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
            qLog(Resource) << "WAV Sound Resource" << path << "->" << r;
            return r;
        }

        r = s + "/" + myApp + "/" + p1;
        if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
            qLog(Resource) << "WAV Sound Resource" << path << "->" << r;
            return r;
        }
    }

    qLog(Resource) << "No resource" << path;

    return QString();
}

#if QT_VERSION >= 0x040300
// declare QRawImageIOPlugin
class QRawImageIOPlugin : public QImageIOPlugin
{
public:
    virtual Capabilities capabilities(QIODevice *device,
                                      const QByteArray &format) const;
    virtual QStringList keys() const;
    virtual QImageIOHandler *create(QIODevice *device,
                                    const QByteArray &format) const;
};

// declare QRawImageIOHandler
class QRawImageIOHandler : public QImageIOHandler
{
public:
    QRawImageIOHandler(QIODevice *);

    virtual bool canRead() const;
    virtual bool read(QImage * image);
private:
    struct RawImage {
        const char qraw[4];
        int format;
        int width;
        int height;
        const uchar data[0];
    };
    const RawImage * m_data;
};

// device QRawImageIOHandler
QRawImageIOHandler::QRawImageIOHandler(QIODevice *device)
: m_data(0)
{
    // We only support QRAW images in resource archives
    QFile *file = qobject_cast<QFile *>(device);
    if(!file) return;

    QResource resource(file->fileName());
    if(!resource.isValid()) return;

    m_data = reinterpret_cast<const RawImage *>(resource.data());
}

bool QRawImageIOHandler::canRead() const
{
    return (m_data != 0);
}

bool QRawImageIOHandler::read(QImage * image)
{
    Q_ASSERT(image);
    if(!m_data)
        return false;
    *image = QImage(m_data->data, m_data->width,
                    m_data->height, (QImage::Format)m_data->format);
    return true;
}

// define QRawImageIOPlugin
QObject * qt_plugin_instance_qraw_image()
{
    return new QRawImageIOPlugin();
}
Q_IMPORT_PLUGIN(qraw_image);

QRawImageIOPlugin::Capabilities
QRawImageIOPlugin::capabilities(QIODevice *device,
                                const QByteArray &format) const
{
    if(!device) return 0;

    device->seek(0);
    QByteArray type = device->read(4 /* ::strlen("QRAW") */);

    return ("QRAW" == type)?CanRead:(QRawImageIOPlugin::Capabilities)(0);
}

QStringList QRawImageIOPlugin::keys() const
{
    QStringList rv;
    rv << "qraw";
    return rv;
}

QImageIOHandler *QRawImageIOPlugin::create(QIODevice *device,
                                           const QByteArray &format) const
{
    device->seek(4);
    return new QRawImageIOHandler(device);
}
#endif
