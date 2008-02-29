/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include "qtopiaresource_p.h"

#include <desktopsettings.h>

#include <QDebug>
#include <QApplication>
#include <qfsfileengine.h>

static QString szXsz; // STATIC XXX CHANGES NOT HONOURED

QFileResourceFileEngineHandler::QFileResourceFileEngineHandler()
    : QAbstractFileEngineHandler()
{
#if 0
    QSettings config("Trolltech", "qpe");
    config.beginGroup("Appearance");
    QVariant value = config.value("IconSize");

    int size = value.toInt();
    szXsz.sprintf("%dx%d",size,size);
    //qLog(Resource) << "Using icon size " << szXsz;
#endif
    szXsz = "22x22";
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
        QString p = findResourceFile(path);
        if (!p.isNull())
            return new QFSFileEngine(p);
    }
    return 0;
}

void QFileResourceFileEngineHandler::appendSearchDirs(QStringList& dirs,
        const QString& dir, const QString& subdir) const
{
    QString t;
    t = dir+subdir+qApp->applicationName()+'/';
    if ( QFSFileEngine(t).fileFlags(QFSFileEngine::ExistsFlag) )
        dirs.append(t);
    t = dir+subdir;
    if ( QFSFileEngine(t).fileFlags(QFSFileEngine::ExistsFlag) )
        dirs.append(t);
}

QString QFileResourceFileEngineHandler::findResourceFile(const QString &path) const
{
    if ( imagedirs.isEmpty() ) {
        imagedirs = iconpath;
        QStringList p;
        p << DesktopSettings::installedDir();
        foreach (const QString &s, p) {
            appendSearchDirs(imagedirs,s,"pics/");
            appendSearchDirs(sounddirs,s,"sounds/");
        }
    }

    QString r;
    if ( path.left(7)==":image/" ) {
        QString p1 = path.mid(7);
        p1 += ".png";

        QStringList dirs = imagedirs;
        bool useI18n = false;
        if (path.indexOf("/i18n/") == 6) {
            p1 = p1.mid(5); //cut i18n indicator
            useI18n = true;
        }

        if (useI18n ) {
            QString pathFileName;
            QString pathDirectory;
            if (p1.indexOf('/') != 1) {
                pathFileName = p1.section('/', -1);
                pathDirectory = p1.left(p1.length()-pathFileName.length());
                p1 = pathFileName;
            }
            QStringList langs = DesktopSettings::languages();
            langs.append("en_US");
            dirs.clear();
            foreach(const QString &lang, langs) {
                QString subDir = pathDirectory+'/';
                foreach(const QString &dir, imagedirs) {
                    if (!dir.isEmpty()) {
                        dirs.append(dir+pathDirectory+"i18n/"+lang+'/');
                    }
                }
            }
        }

        int ext;
        foreach (const QString &s, dirs) {
            r = s + p1;
            if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
                //qLog(Resource) << "PNG Image Resource" << path << "->" << r;
                break;
            }
            ext = r.length()-3;
            r[ext]='j'; r[ext+1]='p'; r[ext+2]='g';
            if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
                //qLog(Resource) << "JPG Image Resource" << path << "->" << r;
                break;
            }
            r[ext]='m'; r[ext+1]='n'; r[ext+2]='g';
            if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
                //qLog(Resource) << "MNG Image Resource" << path << "->" << r;
                break;
            }
            r = QString();
        }
        /*
        if ( qLogEnabled(Resource) )
            if ( r.isNull() )
                qLog(Resource) << "No PNG, JPG or MNG resource" << path;
        */
    } else if ( path.left(6)==":icon/" ) {
        bool useI18n = false;
        QString newPath = path;
        if (path.indexOf("/i18n/") == 5) {//cut i18n out
            newPath = path.left(6)+path.mid(11);
            useI18n = true;
        }

        QStringList dirs = imagedirs;
        QString p1;
        if (newPath.indexOf('/') != -1)
        {
            // special case path does not directly correspond to file name
            QString pathFileName = newPath.section('/',-1);
            QString pathDirectory = newPath.mid(5,newPath.length()-5-pathFileName.length());
            if (!useI18n) {
                p1 = pathDirectory+"icons/"+szXsz+'/'+ pathFileName+ ".png";
            } else {
                p1 = pathFileName + ".png";
                QStringList i18nDirs;
                QStringList langs = DesktopSettings::languages();
                langs.append("en_US");

                foreach(const QString &lang, langs) {
                    QString subDir = pathDirectory+"icons/"+szXsz+'/';
                    foreach(const QString &dir, dirs) {
                        if (!dir.isEmpty()) {
                            i18nDirs.append(dir+subDir+"i18n/"+lang+'/');
                        }
                    }
                }
                dirs = i18nDirs;
            }
        }
        else
        {
            p1 = "/icons/"+szXsz+newPath.mid(5) + ".png";
        }

        foreach (const QString &s, dirs) {
            r = s + p1;
            if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
                //qLog(Resource) << "Icon Resource" << newPath << "->" << r;
                break;
            }
            r = QString();
        }
        if ( r.isNull() ) {
            // Use a pixmap (will be scaled by IconEngine).
            // This should be acceptable since QIcon does not load the image until needed.
            // (note: Qt 4.1 still stats the file at construction - optimization required there)
            r = findResourceFile(":image"+path.mid(5));
        }
        /*
        if ( r.isNull() )
            qLog(Resource) << "No" << szXsz << "resource" << newPath;
        */
    } else if ( path.left(7)==":sound/" ) {
        QString p1 = "/"+path.mid(7);
        p1 += ".wav";
        foreach (const QString &s, sounddirs) {
            r = s + p1;
            if ( QFSFileEngine(r).fileFlags(QFSFileEngine::ExistsFlag) ) {
                //qLog(Resource) << "WAV Sound Resource" << path << "->" << r;
                break;
            }
            r = QString();
        }
        /*
        if ( qLogEnabled(Resource) )
            if ( r.isNull() )
                qLog(Resource) << "No resource" << path;
        */
    }
    /*
    else {
        qLog(Resource) << "Unsupported resource" << path;
    }
    */
    return r;
}

int QFileResourceFileEngineHandler::fontHeightToIconSize( const int fontHeight )
{
#if 0
    // available icon sizes
    QSettings is("Trolltech", "IconSizes");
    is.beginGroup("IconSizes");
    QVariant available = is.value("Available");
    QList<QVariant> sizes;

    if ( !available.isNull() )
        sizes = available.toList();

    if ( sizes.size() == 0 )
        sizes << "14" << "16" << "22";

    int iconSize;
    if ( sizes.indexOf( fontHeight ) == -1 ) {
        int i = 0;
        int size = sizes.size();
        // find the first icon size that is the same size or bigger than the given value
        while ( i < size && sizes.at( i ).toInt() < fontHeight )
                i++;
        i = i == size ? size - 1 : i;
        iconSize = sizes.at( i ).toInt();
    } else
        iconSize = fontHeight;

    // set as the current size
    szXsz.sprintf("%dx%d",iconSize,iconSize);
    //qLog(Resource) << "Using icon size " << szXsz;
#endif
    szXsz.sprintf("%dx%d",fontHeight,fontHeight);
    return fontHeight;
}
