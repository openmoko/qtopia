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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtGui>

class Playlist : public QAbstractListModel
{
    Q_OBJECT
public:
    enum DataRole { Title = Qt::DisplayRole, Url = Qt::UserRole, Artist, Album, Genre, AlbumCover };

    virtual QModelIndex playing() const = 0;
    virtual void setPlaying( const QModelIndex& index ) = 0;

signals:
    void playingChanged( const QModelIndex& index );
};

class PlaylistCue
{
public:
    virtual ~PlaylistCue() { }

    virtual void cue( Playlist* playlist ) = 0;
};

Q_DECLARE_INTERFACE(PlaylistCue,
    "com.trolltech.Qtopia.MediaPlayer.PlaylistCue/1.0")

class PlaylistRemove
{
public:
    virtual ~PlaylistRemove() { }

    // Remove item given by index from playlist
    virtual void remove( const QModelIndex& index ) = 0;
    // Remove all items from playlist
    virtual void clear() = 0;
};

Q_DECLARE_INTERFACE(PlaylistRemove,
    "com.trolltech.Qtopia.MediaPlayer.PlaylistRemove/1.0")

class PlaylistSave
{
public:
    virtual ~PlaylistSave() { }

    // Suggest name for playlist
    virtual QString suggestedName() const = 0;
    // Save playlist using given name
    virtual void save( const QString& name ) = 0;
};

Q_DECLARE_INTERFACE(PlaylistSave,
    "com.trolltech.Qtopia.MediaPlayer.PlaylistSave/1.0")

class BasicPlaylist : public Playlist,
    public PlaylistCue,
    public PlaylistRemove,
    public PlaylistSave
{
    Q_OBJECT
    Q_INTERFACES(PlaylistCue)
    Q_INTERFACES(PlaylistRemove)
    Q_INTERFACES(PlaylistSave)
public:
    BasicPlaylist( const QStringList& urls = QStringList() )
        : m_urls( urls )
    { }

    // Playlist
    void setPlaying( const QModelIndex& index );
    QModelIndex playing() const;

    // PlaylistCue
    void cue( Playlist* playlist );

    // PlaylistRemove
    void remove( const QModelIndex& index );
    void clear();

    // PlaylistSave
    QString suggestedName() const;
    void save( const QString& name );

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

private:
    QStringList m_urls;
    QPersistentModelIndex m_playing;
};

class M3UPlaylist : public BasicPlaylist
{
public:
    M3UPlaylist( const QString& filename );
};

class PLSPlaylist : public Playlist
{
public:
    PLSPlaylist( const QString& filename );

    void setPlaying( const QModelIndex& index ) { m_playing = index; }
    QModelIndex playing() const { return m_playing; }

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

private:
    struct Item
    {
        QString url;
        QString title;
    };

    QPersistentModelIndex m_playing;
    QList<Item> m_items;
};

#endif // PLAYLIST_H
