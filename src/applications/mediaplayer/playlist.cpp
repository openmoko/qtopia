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

#include "playlist.h"

#include <qcontent.h>

void BasicPlaylist::setPlaying( const QModelIndex& index )
{
    if( m_playing != index ) {
        emit playingChanged( m_playing = index );
    }
}

QModelIndex BasicPlaylist::playing() const
{
    return m_playing;
}

void BasicPlaylist::cue( Playlist* playlist )
{
    int cuepos = m_urls.count();
    beginInsertRows( QModelIndex(), cuepos, cuepos + playlist->rowCount() );

    for( int i = 0; i < playlist->rowCount(); ++i ) {
        m_urls.append( playlist->data( playlist->index( i ), Playlist::Url ).toString() );
    }

    endInsertRows();
}

void BasicPlaylist::remove( const QModelIndex& index )
{
    if( index == m_playing ) {
        emit playingChanged( m_playing = QModelIndex() );
    }

    beginRemoveRows( QModelIndex(), index.row(), index.row() );

    m_urls.removeAt( index.row() );

    endRemoveRows();
}

void BasicPlaylist::clear()
{
    beginRemoveRows( QModelIndex(), 0, rowCount() - 1 );

    m_urls.clear();

    endRemoveRows();

    emit playingChanged( m_playing = QModelIndex() );
}

QString BasicPlaylist::suggestedName() const
{
    return tr( "Saved Playlist" );
}

void BasicPlaylist::save( const QString& name )
{
    QContent file;
    file.setName( name );
    file.setType( "audio/mpegurl" );

    QIODevice *io = file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream stream( io );
    foreach( QString url, m_urls ) {
        stream << url << endl;
    }
    stream.flush();
    delete io;

    file.commit();
}

QVariant BasicPlaylist::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() ) {
        return QVariant();
    }

    if( index.row() < 0 || index.row() >= m_urls.count() ) {
        return QVariant();
    }

    QString item = m_urls[index.row()];

    QContent content;
    if( !item.contains( "://" ) ) {
        content = QContent( item );
    }

    if( content.isValid() ) {
        switch( role )
        {
        case Playlist::Title:
            return content.name();
        case Playlist::Url:
            return item;
        case Playlist::Artist:
            return content.property( QContent::Artist );
        case Playlist::Album:
            return content.property( QContent::Album );
        case Playlist::Genre:
            return content.property( QContent::Genre );
        case Playlist::AlbumCover:
            {
            QString coverfile = QFileInfo( item ).path() + "/cover.jpg";
            if( QFile::exists( coverfile ) ) {
                return coverfile;
            }
            }
            break;
        default:
            // Ignore
            break;
        }
    } else {
        switch( role )
        {
        case Playlist::Title:
        case Playlist::Url:
            return item;
        default:
            // Ignore
            break;
        }
    }

    return QVariant();
}

int BasicPlaylist::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent);
    return m_urls.count();
}

static QStringList parse_m3u_playlist( const QString& filename )
{
    QStringList urls;

    // Open file and populate playlist
    QFile file( filename );

    if( file.open( QIODevice::ReadOnly ) ) {
        // For each line of playlist
        QTextStream stream( &file );
        while( !stream.atEnd() ) {
            QString line = stream.readLine();

            // Ignore blank lines and comments
            if( !line.isEmpty() && line[0] != '#' ) {
                // Add item to list of items
                urls << line;
            }
        }
    }

    return urls;
}

M3UPlaylist::M3UPlaylist( const QString& filename )
    : BasicPlaylist( parse_m3u_playlist( filename ) )
{

}

static inline QString extract_value( const QString& string )
{
    return string.right( string.length() - string.indexOf( '=' ) - 1 );
}

PLSPlaylist::PLSPlaylist( const QString& filename )
{
    QFile file( filename );

    if( file.open( QIODevice::ReadOnly ) ) {
        // For each line of playlist
        QTextStream stream( &file );
        enum { FILE, TITLE } state = FILE;
        Item item;
        while( !stream.atEnd() ) {
            QString line = stream.readLine();

            switch( state )
            {
            case FILE:
                if( line.toLower().startsWith( "file" ) ) {
                    item.url = extract_value( line );
                    state = TITLE;
                }
                break;
            case TITLE:
                if( line.toLower().startsWith( "title" ) ) {
                    item.title = extract_value( line );
                } else {
                    item.title = item.url;
                }

                m_items.append( item );

                state = FILE;
                break;
            }
        }
        // ### TODO clean up here, eof while in title
    }
}

QVariant PLSPlaylist::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() ) {
        return QVariant();
    }

    if( index.row() < 0 || index.row() >= m_items.count() ) {
        return QVariant();
    }

    switch( role )
    {
    case Qt::DisplayRole:
        return m_items[index.row()].title;
    case Qt::UserRole:
        return m_items[index.row()].url;
    default:
        // Ignore
        break;
    }

    return QVariant();
}

int PLSPlaylist::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent);
    return m_items.count();
}
