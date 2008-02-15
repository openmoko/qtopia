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
#include "qfscontentengine_p.h"
#include <QtDebug>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QFileSystem>
#include <qtopia/private/qcontentstore_p.h>

static const char *qFileErrors[] =
{
    "QFile::NoError ",
    "QFile::ReadError ",
    "QFile::WriteError ",
    "QFile::FatalError ",
    "QFile::ResourceError ",
    "QFile::OpenError ",
    "QFile::AbortError ",
    "QFile::TimeOutError ",
    "QFile::UnspecifiedError ",
    "QFile::RemoveError ",
    "QFile::RenameError ",
    "QFile::PositionError ",
    "QFile::ResizeError ",
    "QFile::PermissionsError ",
    "QFile::CopyError "
};


/*!
    \class QFSContentEngine
    \mainclass
    \brief QFSContentEngine is the default content for non DRM protected files.

    \internal
*/

/*!
    Constructs a new unpopulated QFSContentEngine.
*/
QFSContentEngine::QFSContentEngine()
    : QContentEngine( QLatin1String( "*/*" ) )
{
}

/*!
    Constructs a new QFSContentEngine with the engine mime type \a engineType.
*/
QFSContentEngine::QFSContentEngine( const QString &engineType )
    : QContentEngine( engineType )
{
}

/*!
    Destroys a QFSContentEngine.
*/
QFSContentEngine::~QFSContentEngine()
{
}

/*!
    \reimp
*/
QContentEngine *QFSContentEngine::copyTo( const QString &newPath )
{
    if( fileName().isEmpty() )
    {
        setError( "Empty destination path" );

        return 0;
    }

    QFile source( fileName() );

    if( source.copy( newPath ) )
    {
        QFSContentEngine *copy = new QFSContentEngine;

        copy->copy( *this );

        copy->setFileName( newPath );

        return copy;
    }

    setError( qFileErrors[ source.error() ] + source.errorString() );

    return 0;
}

/*!
    \reimp
*/
bool QFSContentEngine::moveTo( const QString &newPath )
{
    if( fileName().isEmpty() )
    {
        setError( "Empty destination path" );

        return false;
    }

    QString oldName = fileName();

    QFile source( oldName );

    if( source.copy( newPath ) )
    {
        if( source.remove() )
        {
            setFileName( newPath );

            return true;
        }

        QFile::remove( newPath );
    }

    setError( qFileErrors[ source.error() ] + source.errorString() );

    return false;
}

/*!
    \reimp
*/
bool QFSContentEngine::execute( const QStringList &arguments ) const
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if( role() == QContent::Application )
    {
        qLog(DocAPI) << "QFSContentEngine::execute" << fileName() << arguments;

        Qtopia::execute( fileName(), arguments.count() ? arguments[0] : QString() );

        return true;
    }
    else
    {
        QContent app = mimeType().application();

        if ( app.isValid() )
        {
            app.execute( QStringList() << arguments << fileName() );

            return true;
        }
}
#else
    Q_UNUSED(arguments)
#endif

    return false;
}

/*!
    \reimp
*/
QIODevice *QFSContentEngine::open( QIODevice::OpenMode mode )
{
    if( name().isEmpty() )
        return 0;

    if( fileName().isEmpty() )
    {
        if( mode & QIODevice::WriteOnly )
        {
            if( mimeType().isNull() )
                setMimeType( QMimeType::fromFileName( name() ) );

            QString media = this->media();

            QFileSystem fs = !media.isNull()
                    ? QFileSystem::fromFileName( media )
                    : QFileSystem::documentsFileSystem();

            setFileName( newFileName( name(), mimeType().id(), fs.documentsPath() ) );

            QString extension = determineExtension( name(), mimeType() );

            if( !extension.isEmpty() && name().endsWith( extension, Qt::CaseInsensitive ) )
            {
                QString baseName = name();

                baseName.chop( extension.length() );

                setName( baseName );
            }
        }
        else
            return 0;
    }

    QFile *file = new QFile( fileName() );

    if( !file->open( mode ) )
    {
        setError( qFileErrors[ file->error() ] + file->errorString() );

        delete file;

        file = 0;
    }

    return file;
}

/*!
    \reimp
*/
QContentEngine *QFSContentEngine::createCopy() const
{
    QFSContentEngine *engine = new QFSContentEngine;

    engine->copy( *this );
    engine->setId( id() );

    return engine;
}

bool QFSContentEngine::isOutOfDate() const
{
    return lastUpdated().isNull() || lastUpdated() != QFileInfo( fileName() ).lastModified();
}

/*!
    \reimp
*/
qint64 QFSContentEngine::querySize()
{
    return QFile( fileName() ).size();
}

/*!
    \reimp
*/
bool QFSContentEngine::queryValidity()
{
    return !fileName().isEmpty() && ( role() == QContent::Application || QFile::exists( fileName() ) );
}

/*!
    Constructs a new file name for the content with the user visible name \a name, and mime type \a type in the
    the directory \a location.
*/
QString QFSContentEngine::newFileName( const QString &name, const QString &type, const QString &location )
{
    if( name.isEmpty() || type.isEmpty() || location.isEmpty() )
        return QString();

    QString extension = determineExtension( name, QMimeType::fromId( type ) );

    QString baseName = name;

    if( !extension.isEmpty() && baseName.endsWith( extension, Qt::CaseInsensitive ) )
        baseName.chop( extension.length() );

    QDir dir( location +  QLatin1Char( '/' ) + type );

    if( !dir.exists() )
        QDir::root().mkpath( dir.absolutePath() );

    QString safeName;

    foreach( QChar c, baseName )
        safeName += (c.isLetterOrNumber() || c == QLatin1Char( '.' )) ? c : QLatin1Char( '_' );

    QString possibleName = safeName + extension;

    int number = 0;

    while( dir.exists( possibleName ) )
        possibleName = safeName + QLatin1Char( '_' ) + QString::number( number++ ) + extension;

    return dir.filePath( possibleName );
}

QString QFSContentEngine::determineExtension( const QString &name, const QMimeType &type )
{
    foreach( QString extension, type.extensions() )
    {
        extension.prepend( QLatin1Char( '.' ) );

        if( name.endsWith( extension, Qt::CaseInsensitive ) )
            return extension;
    }

    QString extension = type.extension();

    if( !extension.isEmpty() )
    {
        extension.prepend( QLatin1Char( '.' ) );
    }
    else
    {
        int index = name.indexOf( QLatin1Char( '.' ), name.length() - 6 );

        if( index != -1 )
            extension = name.mid( index );
    }

    return extension;
}

/*!
    \reimp
*/
bool QFSContentEngine::remove()
{
    QFile file( fileName() );

    if( !file.remove() )
    {
        setError( qFileErrors[ file.error() ] + file.errorString() );

        return false;
    }
    else
        return true;
}

