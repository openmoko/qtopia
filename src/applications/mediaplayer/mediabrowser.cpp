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

#include "mediabrowser.h"

#include "playlist.h"
#include "statewidget.h"
#include "elidedlabel.h"
#include "keyfilter.h"
#include "browser.h"
#include "servicerequest.h"
#include "playerwidget.h"

#include <qmediacontrol.h>
#include <qmediatools.h>
#include <qmediawidgets.h>

#include <qtopialog.h>
#include <qtopiaapplication.h>
#include <qcontentset.h>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#include <custom.h>
#endif

#ifdef Q_WS_QWS
#include <QScreen>
#endif

static const int PLAYLIST_ITEM_FLAG = Qt::UserRole + 0x10;

class CustomPropertyBrowser : public PropertyBrowser
{
public:
    CustomPropertyBrowser( PropertyBrowser::Property property,
        const QContentFilter& filter = QContentFilter(), QObject* parent = 0 )
        : PropertyBrowser( property, filter, parent ), m_nextmenu( 0 )
    { }

    BrowserMenu* nextMenu() const { return m_nextmenu; }
    void setNextMenu( BrowserMenu* menu ) { m_nextmenu = menu; }

    ServiceRequest* action( const QModelIndex& index, ActionContext context ) const;

    // QAbstractListModel
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

private:
    BrowserMenu *m_nextmenu;
};

ServiceRequest* CustomPropertyBrowser::action( const QModelIndex& index, ActionContext context ) const
{
    switch( context )
    {
    case Select:
        {
        m_nextmenu->setFilter( filter() & QContentFilter( QContentFilter::Synthetic, filterString( index ) ) );

        PushMenuRequest pushmenu( m_nextmenu );
        PushTitleRequest pushtitle( data( index ).toString() );

        return new CompoundRequest( QList<ServiceRequest*>() << &pushmenu << &pushtitle );
        }
    case Hold:
        {
        QContentSet set( filter() & QContentFilter( QContentFilter::Synthetic, filterString( index ) ) );
        set.setSortOrder( QStringList()
            << "synthetic/none/Artist"
            << "synthetic/none/Album"
            << "synthetic/none/Track" );

        QStringList urls;
        foreach( QContent content, set.items() ) {
            urls.append( content.fileName() );
        }

        return new CuePlaylistRequest( QExplicitlySharedDataPointer<Playlist>(new BasicPlaylist( urls )) );
        }
    case LongHold:
        {
        QContentSet set( filter() & QContentFilter( QContentFilter::Synthetic, filterString( index ) ) );
        set.setSortOrder( QStringList()
            << "synthetic/none/Artist"
            << "synthetic/none/Album"
            << "synthetic/none/Track" );

        QStringList urls;
        foreach( QContent content, set.items() ) {
            urls.append( content.fileName() );
        }

        return new PlayNowRequest( QExplicitlySharedDataPointer<Playlist>(new BasicPlaylist( urls )) );
        }
    default:
        // Ignore
        break;
    }

    return 0;
}

QVariant CustomPropertyBrowser::data( const QModelIndex& index, int role ) const
{
    if( role == NAVIGATION_HINT_ROLE ) {
        return QVariant( NodeHint );
    }

    if( role == PLAYLIST_ITEM_FLAG ) {
        return QVariant( true );
    }

    return PropertyBrowser::data( index, role );
}

class SongBrowser : public ContentBrowser
{
public:
    SongBrowser( const QContentFilter& filter = QContentFilter(), QObject* parent = 0 )
        : ContentBrowser( filter, parent )
    { }

    ServiceRequest* action( const QModelIndex& index, ActionContext context ) const;

    // QAbstractListModel
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
};

ServiceRequest* SongBrowser::action( const QModelIndex& index, ActionContext context ) const
{
    switch( context )
    {
    case Select:
        {
        QStringList urls;
        for( int i = 0; i < rowCount(); ++i ) {
            urls.append( content( SongBrowser::index( i ) ).fileName() );
        }

        QExplicitlySharedDataPointer<Playlist> playlist(new BasicPlaylist( urls ));
        playlist->setPlaying( playlist->index( index.row() ) );

        return new OpenPlaylistRequest( playlist );
        }
    case Hold:
        return new CuePlaylistRequest( QExplicitlySharedDataPointer<Playlist>( new BasicPlaylist( QStringList( content( index ).fileName() ) ) ) );
    case LongHold:
        return new PlayNowRequest( QExplicitlySharedDataPointer<Playlist>( new BasicPlaylist( QStringList( content( index ).fileName() ) ) ) );
    default:
        // Ignore
        break;
    }

    return 0;
}

QVariant SongBrowser::data( const QModelIndex& index, int role ) const
{
    if( role == NAVIGATION_HINT_ROLE ) {
        return QVariant( LeafHint );
    }

    if( role == PLAYLIST_ITEM_FLAG ) {
        return QVariant( true );
    }

    return ContentBrowser::data( index, role );
}

static const int VISUAL_HINT_ROLE = Qt::UserRole + 0xff;
enum VisualHint { ITEM_DIVIDER = 0x1, GHOST_TEXT = 0x2  };

class PlaylistMenuModel : public MenuModel,
    public PlaylistModel
{
    Q_OBJECT
    Q_INTERFACES(PlaylistModel)
public:
    PlaylistMenuModel( QObject* parent = 0 )
        : MenuModel( parent ), m_ismyshuffle( false )
    { }

    void setPlaylist( QExplicitlySharedDataPointer<Playlist> playlist );
    void setPlaylist( const QString& filename );

    // PlaylistModel
    QExplicitlySharedDataPointer<Playlist> playlist() const { return m_playlist; }

    // MenuModel
    ServiceRequest* action( const QModelIndex& index, ActionContext context = Select ) const;

    // AbstractListModel
    int rowCount( const QModelIndex& = QModelIndex() ) const { return m_playlist->rowCount(); }
    QVariant data( const QModelIndex& index, int role ) const;

public slots:
    void emitDataChanged();

private slots:
    void doBeginInsertRows( const QModelIndex& parent, int start, int end );
    void doEndInsertRows();
    void doBeginRemoveRows( const QModelIndex& parent, int start, int end );
    void doEndRemoveRows();

private:
    QExplicitlySharedDataPointer< Playlist > m_playlist;
    QString m_playlistfile;
    bool m_ismyshuffle;
};

void PlaylistMenuModel::setPlaylist( QExplicitlySharedDataPointer<Playlist> playlist )
{
    // Disconnect from old playlist
    if( m_playlist.data() != NULL ) {
        m_playlist->disconnect( this );
    }

    m_playlist = playlist;

    if ( m_playlist == NULL )
        return;

    // Connect to new playlist
    connect( m_playlist, SIGNAL(playingChanged(QModelIndex)),
        this, SLOT(emitDataChanged()) );
    connect( m_playlist, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
        this, SLOT(doBeginInsertRows(QModelIndex,int,int)) );
    connect( m_playlist, SIGNAL(rowsInserted(QModelIndex,int,int)),
        this, SLOT(doEndInsertRows()) );
    connect( m_playlist, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
        this, SLOT(doBeginRemoveRows(QModelIndex,int,int)) );
    connect( m_playlist, SIGNAL(rowsRemoved(QModelIndex,int,int)),
        this, SLOT(doEndRemoveRows()) );

    // Determine if playlist is a My Shuffle playlist
    m_ismyshuffle = qobject_cast<PlaylistMyShuffle*>( playlist.data() ) != NULL;

    m_playlistfile = QString();
}

void PlaylistMenuModel::setPlaylist( const QString& filename )
{
    setPlaylist( Playlist::construct_playlist( filename ) );
    m_playlistfile = filename;
}

ServiceRequest* PlaylistMenuModel::action( const QModelIndex& index, ActionContext context ) const
{
    switch( context )
    {
    case Select:
        {
        QExplicitlySharedDataPointer<Playlist> playlist = m_playlistfile.isNull() ? m_playlist : Playlist::construct_playlist( m_playlistfile );
        playlist->setPlaying( playlist->index( index.row() ) );
        return new OpenPlaylistRequest( playlist );
        }
    case Hold:
        return new CuePlaylistRequest( QExplicitlySharedDataPointer<Playlist>( new BasicPlaylist( QStringList( m_playlist->data( m_playlist->index( index.row() ), Playlist::Url ).toString() ) ) ) );
    case LongHold:
        return new PlayNowRequest( QExplicitlySharedDataPointer<Playlist>( new BasicPlaylist( QStringList( m_playlist->data( m_playlist->index( index.row() ), Playlist::Url ).toString() ) ) ) );
    }

    return 0;
}

QVariant PlaylistMenuModel::data( const QModelIndex& index, int role ) const
{
    switch( role )
    {
    case Qt::DisplayRole:
        return m_playlist->data( m_playlist->index( index.row() ), Playlist::Title );
    case Qt::DecorationRole:
        if( m_playlist->index( index.row() ) == m_playlist->playing() ) {
            return QIcon( ":icon/mediaplayer/black/nowplaying" );
        }
        break;
    case NAVIGATION_HINT_ROLE:
        return QVariant( LeafHint );
    case PLAYLIST_ITEM_FLAG:
        return QVariant( true );
    case VISUAL_HINT_ROLE:
        if( m_ismyshuffle ) {
            int hint = 0;
            if( index.row() == 3 ) {
                hint |= ITEM_DIVIDER;
            }

            if( index.row() >= 3 ) {
                hint |= GHOST_TEXT;
            }

            return hint;
        }
        break;
    default:
        // Ignore
        break;
    }

    return QVariant();
}

void PlaylistMenuModel::emitDataChanged()
{
    // FIXME Limit to actual change in data
    // For change of playing
    emit dataChanged( index( 0 ), index( rowCount() -1 ) );
}

void PlaylistMenuModel::doBeginInsertRows( const QModelIndex& parent, int start, int end )
{
    beginInsertRows( parent, start, end );
}

void PlaylistMenuModel::doEndInsertRows()
{
    endInsertRows();
}

void PlaylistMenuModel::doBeginRemoveRows( const QModelIndex& parent, int start, int end )
{
    beginRemoveRows( parent, start, end );
}

void PlaylistMenuModel::doEndRemoveRows()
{
    endRemoveRows();
}

class PlaylistBrowser : public ContentBrowser
{
public:
    PlaylistBrowser( const QContentFilter& filter = QContentFilter(), QObject* parent = 0 );

    ServiceRequest* action( const QModelIndex& index, ActionContext context ) const;

    // QAbstractListModel
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

private:
    PlaylistMenuModel *m_playlistmenu;
};

PlaylistBrowser::PlaylistBrowser( const QContentFilter& filter, QObject* parent )
    : ContentBrowser( filter, parent )
{
    m_playlistmenu = new PlaylistMenuModel( this );
}

ServiceRequest* PlaylistBrowser::action( const QModelIndex& index, ActionContext context ) const
{
    QString file = content( index ).fileName();

    switch( context )
    {
    case Select:
        {
        m_playlistmenu->setPlaylist( file );

        PushMenuRequest pushmenu( m_playlistmenu );
        PushTitleRequest pushtitle( data( index ).toString() );

        return new CompoundRequest( QList<ServiceRequest*>() << &pushmenu << &pushtitle );
        }
    case Hold:
        return new CuePlaylistRequest( Playlist::construct_playlist( file ) );
    case LongHold:
        return new PlayNowRequest( Playlist::construct_playlist( file ) );
    }

    return 0;
}

QVariant PlaylistBrowser::data( const QModelIndex& index, int role ) const
{
    if( role == NAVIGATION_HINT_ROLE ) {
        return QVariant( NodeHint );
    }

    if( role == PLAYLIST_ITEM_FLAG ) {
        return QVariant( true );
    }

    return ContentBrowser::data( index, role );
}

class IconCache
{
public:
    const QPixmap& pixmap( int iconid, const QString& filename, const QSize& size ) const;

private:
    mutable QHash<int,QPixmap> m_cache;
};

const QPixmap& IconCache::pixmap( int iconid, const QString& filename, const QSize& size ) const
{
    QPixmap &pixmap = m_cache[iconid];

    if( pixmap.isNull() || pixmap.size() != size ) {
        pixmap = QIcon( filename ).pixmap( size );
    }

    return pixmap;
}

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    TitleBar( PlayerControl* control, QWidget* parent = 0 );

    // Push title onto title stack
    void push( const QString& title );
    // Pop title off of title stack
    void pop();

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void activate();
    void deactivate();

    void setMute( bool mute );

protected:
    void paintEvent( QPaintEvent* e );

private:
    ElidedLabel *m_titlelabel;
    QStack<QString> m_titlestack;

    QMediaContent *m_content;
    QMediaControlNotifier *m_notifier;
    QMediaControl *m_control;
    StateWidget *m_statewidget;

    QMediaVolumeLabel *m_muteicon;
    bool m_ismute;

    IconCache m_iconcache;
};

void TitleBar::push( const QString& title )
{
    m_titlestack.push( title );
    m_titlelabel->setText( title );
}

void TitleBar::pop()
{
    m_titlestack.pop();
    m_titlelabel->setText( m_titlestack.top() );
}

TitleBar::TitleBar( PlayerControl* control, QWidget* parent )
    : QWidget( parent ), m_control( 0 ), m_ismute( false )
{
    static const int STRETCH_MAX = 1;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin( 6 );

    // Use custom palette
    QPalette pal = palette();
    pal.setBrush( QPalette::WindowText, Qt::black );

    m_statewidget = new StateWidget( control );
    m_statewidget->setPalette( pal );
    layout->addWidget( m_statewidget );

    m_muteicon = new QMediaVolumeLabel( QMediaVolumeLabel::MuteVolume );
    m_muteicon->setPalette( pal );
    layout->addWidget( m_muteicon );

    m_titlelabel = new ElidedLabel;
    m_titlelabel->setAlignment( Qt::AlignRight );
    m_titlelabel->setPalette( pal );
    layout->addWidget( m_titlelabel, STRETCH_MAX );

    QFont titlefont = font();
    titlefont.setItalic( true );
    m_titlelabel->setFont( titlefont );

    m_muteicon->hide();

    setLayout( layout );

    m_notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( m_notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );
}

void TitleBar::setMediaContent( QMediaContent* content )
{
    m_content = content;
    m_notifier->setMediaContent( content );
}

void TitleBar::activate()
{
    m_control = new QMediaControl( m_content );
    connect( m_control, SIGNAL(volumeMuted(bool)),
        this, SLOT(setMute(bool)) );

    setMute( m_control->isMuted() );
}

void TitleBar::deactivate()
{
    delete m_control;
    m_control = 0;
}

void TitleBar::setMute( bool mute )
{
    if( m_ismute != mute ) {
        m_ismute = !m_ismute;

        if( m_ismute ) {
            m_muteicon->show();
        } else {
            m_muteicon->hide();
        }
    }
}

void TitleBar::paintEvent( QPaintEvent* )
{
    static const int TITLEBAR_ICON_ID = 1;

    // Paint background
    QPainter painter( this );
    painter.drawPixmap( QPoint( 0, 0 ), m_iconcache.pixmap( TITLEBAR_ICON_ID,
        ":icon/mediaplayer/black/titlebar", size() ) );
}

class ActionContext
{
public:
    virtual ~ActionContext() { }

    virtual bool isVisible() const = 0;
};

class ActionGroup
{
public:
    ActionGroup( const QList<QAction*>& group )
        : m_group( group )
    { }

    void setContext( const ActionContext& context );

private:
    QList<QAction*> m_group;
};

void ActionGroup::setContext( const ActionContext& context )
{
    bool visible = context.isVisible();

    foreach( QAction* action, m_group ) {
        action->setVisible( visible );
    }
}

class PlaylistRemoveContext : public ActionContext
{
public:
    PlaylistRemoveContext( MenuModel* model )
        : m_model( model )
    { }

    // ActionContext
    bool isVisible() const;

private:
    MenuModel *m_model;
};

bool PlaylistRemoveContext::isVisible() const
{
    PlaylistModel *playlistmodel = qobject_cast<PlaylistModel*>(m_model);
    if( playlistmodel ) {
        QExplicitlySharedDataPointer<Playlist> playlist = playlistmodel->playlist();
        PlaylistRemove *remove = qobject_cast<PlaylistRemove*>(playlist.data());
        if( remove && playlist->rowCount() > 0 ) {
            return true;
        }
    }

    return false;
}

class PlaylistSaveContext : public ActionContext
{
public:
    PlaylistSaveContext( MenuModel* model )
        : m_model( model )
    { }

    // ActionContext
    bool isVisible() const;

private:
    MenuModel *m_model;
};

bool PlaylistSaveContext::isVisible() const
{
    PlaylistModel *playlistmodel = qobject_cast<PlaylistModel*>(m_model);
    if( playlistmodel ) {
        QExplicitlySharedDataPointer<Playlist> playlist = playlistmodel->playlist();
        PlaylistSave *save = qobject_cast<PlaylistSave*>(playlist.data());
        if( save && playlist->rowCount() > 0 ) {
            return true;
        }
    }

    return false;
}

class PlaylistResetContext : public ActionContext
{
public:
    PlaylistResetContext( MenuModel* model )
        : m_model( model )
    { }

    // ActionContext
    bool isVisible() const;

private:
    MenuModel *m_model;
};

bool PlaylistResetContext::isVisible() const
{
    PlaylistModel *playlistmodel = qobject_cast<PlaylistModel*>(m_model);
    if( playlistmodel ) {
        QExplicitlySharedDataPointer<Playlist> playlist = playlistmodel->playlist();
        PlaylistMyShuffle *myshuffle = qobject_cast<PlaylistMyShuffle*>(playlist.data());
        if( myshuffle ) {
            return true;
        }
    }

    return false;
}

class IndexHistory
{
public:
    // Move back one index in history, return index
    int back();

    // Move forward one index in history, return index
    int forward();

    // Set current index in history
    void setIndex( int index );

private:
    QStack<int> m_back, m_forward;
};

int IndexHistory::back()
{
    int index = -1;

    if( m_back.size() ) {
        index = m_back.pop();
        m_forward.push( index );
    }

    return index;
}

int IndexHistory::forward()
{
    int index = -1;

    if( m_forward.size() ) {
        m_back.push( m_forward.pop() );
    }

    if( m_forward.size() ) {
        index = m_forward.top();
    }

    return index;
}

void IndexHistory::setIndex( int index )
{
     // If index not equal to current index, clear forward history
    if( m_forward.isEmpty() || index != m_forward.top() ) {
        m_forward.clear();
        m_forward.push( index );
    }
}

class MenuServiceRequestHandler : public RequestHandler
{
public:
    struct Context
    {
        MenuStack *stack;
        TitleBar *titlebar;

        MenuView *view;
        IndexHistory *history;

        ActionGroup *removegroup;
        ActionGroup *savegroup;
        ActionGroup *resetgroup;
    };

    MenuServiceRequestHandler( const Context& context, RequestHandler* successor = 0 )
        : RequestHandler( successor ), m_context( context )
    { }

    // RequestHandler
    void execute( ServiceRequest *request );

private:
    Context m_context;
};

void MenuServiceRequestHandler::execute( ServiceRequest* request )
{
    switch( request->type() )
    {
    case ServiceRequest::PushTitle:
        {
        PushTitleRequest *req = (PushTitleRequest*)request;

        QString title = req->title();
        m_context.titlebar->push( title );

        delete request;
        }
        break;
    case ServiceRequest::PushMenu:
        {
        PushMenuRequest *req = (PushMenuRequest*)request;

        m_context.history->setIndex( m_context.view->currentIndex().row() );

        MenuModel *model = req->menu();
        m_context.stack->push( model );

        int index = m_context.history->forward();
        if( index != -1 ) {
            m_context.view->setCurrentIndex( model->index( index ) );
        } else {
            m_context.view->setCurrentIndex( model->index( 0 ) );
        }

#ifdef QTOPIA_KEYPAD_NAVIGATION
        m_context.removegroup->setContext( PlaylistRemoveContext( model ) );
        m_context.savegroup->setContext( PlaylistSaveContext( model ) );
        m_context.resetgroup->setContext( PlaylistResetContext( model ) );
#endif

        delete request;
        }
        break;
    default:
        // Pass on request
        RequestHandler::execute( request );
        break;
    }
}

class SavePlaylistDialog : public QDialog
{
    Q_OBJECT
public:
    SavePlaylistDialog( QWidget* parent = 0 );

    QString text() const { return m_edit->text(); }
    void setText( const QString& text ) { m_edit->setText( text ); }

private:
    QLineEdit *m_edit;
};

SavePlaylistDialog::SavePlaylistDialog( QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( tr( "Save As" ) );

    m_edit = new QLineEdit;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget( m_edit );
    setLayout( layout );
}

class Animation
{
public:
    virtual ~Animation() { }

    virtual Animation* clone() const = 0;

    virtual bool isDone() const = 0;
    virtual void animate() = 0;
};

class NullFrame : public Animation
{
public:
    NullFrame( int frames = 1 )
        : m_frames( frames ), m_done( false )
    { }

    // Animation
    Animation* clone() const { return new NullFrame( m_frames ); }

    bool isDone() const { return m_done; }
    void animate();

private:
    int m_frames;
    bool m_done;
};

void NullFrame::animate()
{
    if( --m_frames < 0 ) {
        m_done = true;
    }
}

class AnimationSequence : public Animation
{
public:
    AnimationSequence( const QList<Animation*>& sequence );
    ~AnimationSequence();

    // Animation
    Animation* clone() const { return new AnimationSequence( m_sequence ); }

    bool isDone() const { return m_done; }
    void animate();

private:
    QList<Animation*> m_sequence;
    bool m_done;

    Animation *m_active;
};

AnimationSequence::AnimationSequence( const QList<Animation*>& sequence )
    : m_done( false ), m_active( 0 )
{
    for( int i = 0; i < sequence.count(); ++i ) {
        m_sequence.append( sequence[i]->clone() );
    }
}

AnimationSequence::~AnimationSequence()
{
    for( int i = 0; i < m_sequence.count(); ++i ) {
        delete m_sequence[i];
    }
}

void AnimationSequence::animate()
{
    if( !m_active && m_sequence.count() ) {
        m_active = m_sequence.takeFirst();
    }

    if( m_active ) {
        m_active->animate();

        if( m_active->isDone() ) {
            delete m_active;
            m_active = 0;
        }
    } else {
        m_done = true;
    }
}

class AnimationEngine : public QObject
{
    Q_OBJECT
public:
    AnimationEngine( CustomView *view, QObject* parent = 0 );
    ~AnimationEngine();

    void play( const Animation& animation );

private slots:
    void execute();

private:
    CustomView *m_view;
    Animation *m_animation;

    QTimer *m_timer;
};

class AnimationProxy;

class AnimationDirector
{
    friend class AnimationProxy;
public:
    AnimationDirector( AnimationEngine* engine )
        : m_engine( engine )
    { }

    ~AnimationDirector();

    void animate( int timelineid, const Animation& animation );

    bool isDone() const;

private:
    void animate();
    void executeAnimation();

    AnimationEngine *m_engine;

    QHash<int,int> m_idmap;
    QList<Animation*> m_timelines;
};

AnimationDirector::~AnimationDirector()
{
    for( int i = 0; i < m_timelines.count(); ++i ) {
        delete m_timelines[i];
    }
}

void AnimationDirector::animate( int timelineid, const Animation& animation )
{
    if( m_idmap.contains( timelineid ) ) {
        int mapping = m_idmap[timelineid];

        delete m_timelines[mapping];
        m_timelines[mapping] = animation.clone();
    } else {
        m_idmap.insert( timelineid, m_timelines.count() );
        m_timelines.append( animation.clone() );
    }

    executeAnimation();
}

bool AnimationDirector::isDone() const
{
    bool done = true;

    for( int i = 0; i < m_timelines.count(); ++i ) {
        done &= m_timelines[i]->isDone();
    }

    return done;
}

void AnimationDirector::animate()
{
    for( int i = 0; i < m_timelines.count(); ++i ) {
        Animation *animation = m_timelines[i];
        if( !animation->isDone() ) {
            animation->animate();
        }
    }
}

class AnimationProxy : public Animation
{
public:
    AnimationProxy( AnimationDirector* director )
        : m_director( director )
    { }

    // Animation
    Animation* clone() const { return new AnimationProxy( m_director ); }

    bool isDone() const { return m_director->isDone(); }

    void animate() { m_director->animate(); }

private:
    AnimationDirector *m_director;
};

void AnimationDirector::executeAnimation()
{
    m_engine->play( AnimationProxy( this ) );
}

class CustomPalette
{
public:
    enum Role { Text, HighlightText, HighlightBegin, HighlightEnd, EndRole };

    QColor color( int role ) const;
    void setColor( int role, const QColor& color ) { m_palette[role] = color; }

private:
    QHash<int,QColor> m_palette;
};

QColor CustomPalette::color( int role ) const
{
    if( m_palette.contains( role ) ) {
        return m_palette[role];
    }

    return QColor();
}

class ItemDelegate : public QAbstractItemDelegate
{
public:
    ItemDelegate( QObject* parent = 0 )
        : QAbstractItemDelegate( parent ), m_helpicon( None ), m_helpiconopacity( 1.0 ), m_hintopacity( 1.0 ), m_spacing( 0 )
    { }

    enum HelpIcon { None, Cue, PlayNow };

    void setHelpIcon( HelpIcon icon ) { m_helpicon = icon; }
    HelpIcon helpIcon() const { return m_helpicon; }

    void setHelpIconOpacity( qreal opacity ) { m_helpiconopacity = opacity; }
    qreal helpIconOpacity() const { return m_helpiconopacity; }

    void setNavigationHintOpacity( qreal opacity ) { m_hintopacity = opacity; }
    qreal navigationHintOpacity() const { return m_hintopacity; }

    void setCustomPalette( const CustomPalette& palette ) { m_palette = palette; }

    void setSpacing( int spacing ) { m_spacing = spacing; }

    void setFixedWidth( int width ) { m_fixedwidth = width; }

    // QAbstractItemDelegate
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

private:
    HelpIcon m_helpicon;
    qreal m_helpiconopacity;
    qreal m_hintopacity;
    int m_spacing;
    int m_fixedwidth;
    CustomPalette m_palette;
    IconCache m_iconcache;
};

static const int DECORATION_MARGIN = 3;

void ItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    static const int GHOST_TEXT_OPACITY = 128;
    static const int HIGHLIGHT_ICON_ID = 1;
    static const int DIVIDER_ICON_ID = 2;
    static const int NODE_ICON_ID = 3;
    static const int LEAF_ICON_ID = 4;
    static const int ADD_ICON_ID = 5;
    static const int PLAYNOW_ICON_ID = 6;

    painter->save();

    QRect rect = option.rect;

    bool selected = option.showDecorationSelected && (option.state & QStyle::State_Selected);

    // Draw background
    if( selected ) {
        painter->drawPixmap( rect, m_iconcache.pixmap( HIGHLIGHT_ICON_ID,
            ":icon/mediaplayer/black/highlight", QSize( m_fixedwidth, rect.height() ) ) );
    }

    // Draw divider
    QVariant visualhint = index.sibling( index.row() + 1, index.column() ).data( VISUAL_HINT_ROLE );
    if( visualhint.isValid() && visualhint.toInt() & ITEM_DIVIDER ) {
        painter->drawPixmap( rect.adjusted( 0, rect.height()/2, 0, rect.height()/2 ), m_iconcache.pixmap( DIVIDER_ICON_ID,
            ":icon/mediaplayer/black/divider", QSize( m_fixedwidth, rect.height() ) ) );
    }

    visualhint = index.data( VISUAL_HINT_ROLE );
    if( visualhint.isValid() && visualhint.toInt() & ITEM_DIVIDER ) {
        painter->drawPixmap( rect.adjusted( 0, -rect.height()/2, 0, -rect.height()/2 ), m_iconcache.pixmap( DIVIDER_ICON_ID,
            ":icon/mediaplayer/black/divider", QSize( m_fixedwidth, rect.height() ) ) );
    }

    // Subtract left and right spacing
    rect.adjust( m_spacing, 0, -m_spacing, 0 );

    // Draw decoration
    QVariant decoration = index.data( Qt::DecorationRole );
    if( decoration.isValid() ) {
        // Calculate decoration rect
        QRect r = QRect( rect.topLeft(), option.decorationSize + QSize( DECORATION_MARGIN, DECORATION_MARGIN * 2 ) );

        QIcon icon = qvariant_cast<QIcon>( decoration );

        if ( option.direction == Qt::LeftToRight ) {
            // Subtract decoration from item rect
            rect.adjust( r.width(), 0, 0, 0 );
            icon.paint( painter, r.adjusted( 0, DECORATION_MARGIN, -DECORATION_MARGIN*2, -DECORATION_MARGIN ) );
        } else {
            // Subtract decoration from item rect
            rect.adjust( 0, 0, -r.width(), 0 );
            r.translate( rect.width(), 0 );
            icon.paint( painter, r.adjusted( DECORATION_MARGIN*2, DECORATION_MARGIN, 0, -DECORATION_MARGIN ) );
        }
    }

    // Subtract top and bottom spacing
    rect.adjust( 0, m_spacing, 0, -m_spacing );

    // Determine help icon height
    QFontMetrics metrics( option.font );
    int iconwidth = metrics.height() - 4;

    // If selected, make space for navigation and help icons
    if( selected ) {
        if ( option.direction == Qt::LeftToRight ) {
            rect.adjust( 0, 0, -iconwidth - m_spacing, 0 );
        } else {
            rect.adjust( iconwidth + m_spacing, 0, 0, 0 );
        }
    }

    // Draw text
    QString text = metrics.elidedText( index.data().toString(), option.textElideMode, rect.width() );

    if( selected ) {
        painter->setPen( m_palette.color( CustomPalette::HighlightText ) );
    } else {
        QColor textcolor = m_palette.color( CustomPalette::Text );
        if( visualhint.isValid() && (visualhint.toInt() & GHOST_TEXT) ) {
            textcolor.setAlpha( GHOST_TEXT_OPACITY );
        }
        painter->setPen( textcolor );
    }
    painter->drawText( rect, Qt::AlignVCenter, text );

    // Draw navigation and help icons
    if( selected ) {
        if ( option.direction == Qt::LeftToRight ) {
            rect.adjust( rect.width() + m_spacing, 0, iconwidth, 0 );
        } else {
            rect.adjust( -iconwidth, 0, -rect.width() - m_spacing, 0 );
        }

        QPoint topleft = QPoint( rect.x() + (rect.width() - iconwidth) / 2,
            rect.y() + (rect.height() - iconwidth) / 2 );

        // Draw navigation hint
        if( m_hintopacity > 0 ) {
            int navicon = index.data( NAVIGATION_HINT_ROLE ).toInt();

            painter->setOpacity( m_hintopacity );

            switch( navicon )
            {
            case NodeHint:
                if ( option.direction == Qt::LeftToRight ) {
                    painter->drawPixmap( topleft, m_iconcache.pixmap( NODE_ICON_ID,
                        ":icon/mediaplayer/black/node", QSize( iconwidth, iconwidth ) ) );
                } else {
                    painter->drawPixmap( topleft, m_iconcache.pixmap( NODE_ICON_ID,
                        ":icon/mediaplayer/black/node_mirrored", QSize( iconwidth, iconwidth ) ) );
                }
                break;
            case LeafHint:
                painter->drawPixmap( topleft, m_iconcache.pixmap( LEAF_ICON_ID,
                    ":icon/mediaplayer/black/leaf", QSize( iconwidth, iconwidth ) ) );
                break;
            }
        }

        // Draw help icon
        if( m_helpiconopacity > 0 ) {
            painter->setOpacity( m_helpiconopacity );

            switch( m_helpicon )
            {
            case Cue:
                painter->drawPixmap( topleft, m_iconcache.pixmap( ADD_ICON_ID, 
                    ":icon/mediaplayer/black/add", QSize( iconwidth, iconwidth ) ) );
                break;
            case PlayNow:
                painter->drawPixmap( topleft, m_iconcache.pixmap( PLAYNOW_ICON_ID,
                    ":icon/mediaplayer/black/playnow", QSize( iconwidth, iconwidth ) ) );
                break;
            default:
                // Ignore
                break;
            }
        }
    }

    painter->restore();
}

QSize ItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& ) const
{
    return option.decorationSize + QSize( DECORATION_MARGIN, DECORATION_MARGIN * 2 );
}

class IconHold : public Animation
{
public:
    IconHold( ItemDelegate::HelpIcon hold, ItemDelegate* delegate, int frames = 1, qreal opacity = 0.9 )
        : m_hold( hold ), m_delegate( delegate ), m_frames( frames ), m_opacity( opacity )
    { }

    // Animation
    Animation* clone() const { return new IconHold( m_hold, m_delegate, m_frames, m_opacity ); }

    bool isDone() const { return m_frames <= 0; }
    void animate();

private:
    ItemDelegate::HelpIcon m_hold;
    ItemDelegate *m_delegate;
    int m_frames;
    qreal m_opacity;
};

void IconHold::animate()
{
    m_delegate->setHelpIcon( m_hold );
    m_delegate->setHelpIconOpacity( m_opacity );

    --m_frames;
}

class IconFlash : public Animation
{
public:
    IconFlash( ItemDelegate* delegate, qreal opacity = 0.9 )
        : m_delegate( delegate ), m_opacity( opacity ), m_done( false ), m_count( 0 ), m_flipflop( 0 )
    { }

    // Animation
    Animation* clone() const { return new IconFlash( m_delegate, m_opacity ); }

    bool isDone() const { return m_done; }
    void animate();

private:
    ItemDelegate *m_delegate;
    qreal m_opacity;
    bool m_done;
    int m_count;
    int m_flipflop;
};

void IconFlash::animate()
{
    static const int TOTAL_FRAMES = 12;

    if( !m_flipflop ) {
        if( m_delegate->helpIconOpacity() == 0.0 ) {
            m_delegate->setHelpIconOpacity( m_opacity );
        } else {
            m_delegate->setHelpIconOpacity( 0.0 );
        }
    }

    ++m_count;
    m_flipflop = m_count % 4;
    if( m_count >= TOTAL_FRAMES ) {
        m_done = true;
    }
}

class IconFade : public Animation
{
public:
    enum Type { FadeIn, FadeOut };

    IconFade( Type type, ItemDelegate* delegate )
        : m_type( type ), m_delegate( delegate ), m_done( false )
    { }

    // Animation
    Animation* clone() const { return new IconFade( m_type, m_delegate ); }

    bool isDone() const { return m_done; }
    void animate();

private:
    Type m_type;
    ItemDelegate *m_delegate;
    bool m_done;
};

void IconFade::animate()
{
    static const qreal OPACITY_MAX = 0.9;
    static const qreal OPACITY_MIN = 0.0;
    static const qreal FADE_DELTA = 0.05;

    qreal opacity = m_delegate->helpIconOpacity();

    switch( m_type )
    {
    case FadeIn:
        opacity += FADE_DELTA;
        if( opacity > OPACITY_MAX ) {
            opacity = OPACITY_MAX;
            m_done = true;
        }
        break;
    case FadeOut:
        opacity -= FADE_DELTA;
        if( opacity < OPACITY_MIN ) {
            opacity = OPACITY_MIN;
            m_done = true;
        }
        break;
    }

    m_delegate->setHelpIconOpacity( opacity );
}

class HintFade : public Animation
{
public:
    enum Type { FadeIn, FadeOut };

    HintFade( Type type, ItemDelegate* delegate )
        : m_type( type ), m_delegate( delegate ), m_done( false )
    { }

    // Animation
    Animation *clone() const { return new HintFade( m_type, m_delegate ); }

    bool isDone() const { return m_done; }
    void animate();

private:
    Type m_type;
    ItemDelegate *m_delegate;
    bool m_done;
};

void HintFade::animate()
{
    static const qreal OPACITY_MAX = 1.0;
    static const qreal OPACITY_MIN = 0.0;
    static const qreal OPACITY_DELTA = 0.2;

    qreal opacity = m_delegate->navigationHintOpacity();

    switch( m_type )
    {
    case FadeIn:
        opacity += OPACITY_DELTA;
        if( opacity > OPACITY_MAX ) {
            opacity = OPACITY_MAX;
            m_done = true;
        }
        break;
    case FadeOut:
        opacity -= OPACITY_DELTA;
        if( opacity < OPACITY_MIN ) {
            opacity = OPACITY_MIN;
            m_done = true;
        }
        break;
    }

    m_delegate->setNavigationHintOpacity( opacity );
}

static const int KEY_SELECT_HOLD = Qt::Key_unknown + Qt::Key_Select;

class CustomView : public MenuView
{
    Q_OBJECT
public:
    CustomView( QWidget* parent = 0 );

    void setHelpText( const QString& text ) { m_helptext = text; }

    void setHelpOpacity( qreal opacity ) { m_helpopacity = opacity; }
    qreal helpOpacity() const { return m_helpopacity; }

signals:
    void heldReleased();
    void heldLong( const QModelIndex& index );

    void currentChanged();

protected slots:
    // QAbstractItemView
    void currentChanged( const QModelIndex& current, const QModelIndex& previous );

private slots:
    void emitHeld();
    void emitHeldLong();

protected:
    // MenuView
    void keyPressEvent( QKeyEvent* e );
    void keyReleaseEvent( QKeyEvent* e );

    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

    void paintEvent( QPaintEvent* e );

private:
    QString m_helptext;
    qreal m_helpopacity;

    QMouseEvent m_eventcache;
    QTimer *m_holdtimer, *m_longholdtimer;

    QFont m_smallfont;
    IconCache m_iconcache;
};

CustomView::CustomView( QWidget* parent )
    : MenuView( parent ), m_helpopacity( 0.0 ), m_eventcache( QEvent::None, QPoint(), Qt::NoButton, Qt::NoButton, Qt::NoModifier )
{
    m_holdtimer = new QTimer( this );
    connect( m_holdtimer, SIGNAL(timeout()),
        this, SLOT(emitHeld()) );
    m_holdtimer->setInterval( 500 );
    m_holdtimer->setSingleShot( true );

    m_longholdtimer = new QTimer( this );
    connect( m_longholdtimer, SIGNAL(timeout()),
        this, SLOT(emitHeldLong()) );
    m_longholdtimer->setInterval( 3000 );
    m_longholdtimer->setSingleShot( true );

    m_smallfont = font();
    m_smallfont.setPointSize( m_smallfont.pointSize() - 2 );
}

void CustomView::currentChanged( const QModelIndex& current, const QModelIndex& previous )
{
    MenuView::currentChanged( current, previous );

    emit currentChanged();
}

void CustomView::keyPressEvent( QKeyEvent* e )
{
    if( e->key() == KEY_SELECT_HOLD ) {
        e->accept();
        emitHeld();
        m_longholdtimer->start();
    }
    else
        MenuView::keyPressEvent( e );
}

void CustomView::keyReleaseEvent( QKeyEvent* e )
{
    if( e->key() == KEY_SELECT_HOLD ) {
        e->accept();
        if( m_longholdtimer->isActive() ) {
            emit heldReleased();
        }

        m_longholdtimer->stop();
    }
    else
        MenuView::keyReleaseEvent( e );
}

void CustomView::emitHeld()
{
    QModelIndex current = currentIndex();
    if( current.isValid() ) {
        emit held( current );
    }
}

void CustomView::emitHeldLong()
{
    QModelIndex current = currentIndex();
    if( current.isValid() ) {
        emit heldLong( current );
    }
}

void CustomView::mousePressEvent( QMouseEvent* e )
{
    m_eventcache = *e;

    QModelIndex current = indexAt( e->pos() );
    if( current.isValid() ) {
        setCurrentIndex( current );
    }

    m_holdtimer->start();
    m_longholdtimer->start();
}

void CustomView::mouseReleaseEvent( QMouseEvent* e )
{
    if( m_holdtimer->isActive() ) {
        QListView::mousePressEvent( &m_eventcache );
        QListView::mouseReleaseEvent( e );
    }

    if( !m_holdtimer->isActive() && m_longholdtimer->isActive() ) {
        emit heldReleased();
    }

    m_holdtimer->stop();
    m_longholdtimer->stop();
}

void CustomView::paintEvent( QPaintEvent* e )
{
    static const QColor TROLLTECH_YELLOW = QColor( 242, 234, 54 );
    static const int TIP_UP_ICON_ID = 1;
    static const int TIP_DOWN_ICON_ID = 2;

    MenuView::paintEvent( e );

    if( m_helpopacity > 0 ) {
        QPainter painter( viewport() );
        painter.setOpacity( m_helpopacity );
        painter.setPen( Qt::NoPen );
        painter.setBrush( TROLLTECH_YELLOW );

        painter.setFont( m_smallfont );

        QRect rect;
        QRect vrect = visualRect( currentIndex() );
        if( vrect.bottom() < height()/2 ) {
            int top = vrect.bottom() + 16;
            rect = QRect( QPoint( 0, top ), QPoint( width(), top + fontMetrics().height() + 8 ) );
        } else {
            int bottom = vrect.top() - 16;
            rect = QRect( QPoint( 0, bottom - fontMetrics().height() - 8 ), QPoint( width(), bottom ) );
        }
        painter.drawRect( rect );

        int iconwidth = fontMetrics().height() - 4;
        int scrollbar = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

        if( vrect.bottom() < height()/2 ) {
            painter.drawPixmap( QPoint( rect.right() - iconwidth - 8 - scrollbar, rect.top() - iconwidth ),
                m_iconcache.pixmap( TIP_UP_ICON_ID, ":icon/mediaplayer/black/tip-up", QSize( iconwidth, iconwidth ) ) );
        } else {
            painter.drawPixmap( QPoint( rect.right() - iconwidth - 8 - scrollbar, rect.bottom() + 1 ),
                m_iconcache.pixmap( TIP_DOWN_ICON_ID, ":icon/mediaplayer/black/tip-down", QSize( iconwidth, iconwidth ) ) );
        }

        painter.setPen( Qt::black );
        painter.drawText( rect.adjusted( 4, 4, -4, -4 ), Qt::AlignCenter|Qt::TextWordWrap, m_helptext );
    }
}

class HelpFade : public Animation
{
public:
    enum Type { FadeIn, FadeOut };

    HelpFade( Type type, CustomView* view )
        : m_type( type ), m_view( view ), m_done( false )
    { }

    // Animation
    Animation* clone() const { return new HelpFade( m_type, m_view ); }

    bool isDone() const { return m_done; }
    void animate();

private:
    Type m_type;
    CustomView *m_view;

    bool m_done;
};

void HelpFade::animate()
{
    static const qreal OPACITY_MAX = 0.9;
    static const qreal OPACITY_MIN = 0.0;
    static const qreal FADE_DELTA = 0.05;

    qreal opacity = m_view->helpOpacity();

    switch( m_type )
    {
    case FadeIn:
        opacity += FADE_DELTA;
        if( opacity > OPACITY_MAX ) {
            opacity = OPACITY_MAX;
            m_done = true;
        }
        break;
    case FadeOut:
        opacity -= FADE_DELTA;
        if( opacity < OPACITY_MIN ) {
            opacity = OPACITY_MIN;
            m_done = true;
        }
        break;
    }

    m_view->setHelpOpacity( opacity );
}

AnimationEngine::AnimationEngine( CustomView* view, QObject* parent )
    : QObject( parent ), m_view( view ), m_animation( 0 )
{
    static const int EXECUTION_INTERVAL = 60; // Approx. 15 fps

    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()), this, SLOT(execute()) );
    m_timer->setInterval( EXECUTION_INTERVAL );
}

AnimationEngine::~AnimationEngine()
{
    delete m_animation;
}

void AnimationEngine::play( const Animation& animation )
{
    delete m_animation;

    m_animation = animation.clone();

    m_timer->start();
}

void AnimationEngine::execute()
{
    if( m_animation->isDone() ) {
        m_timer->stop();
    } else {
        m_animation->animate();
        m_view->update();
    }
}

static const int HELP_MESSAGE_TIMELINE_ID = 1;
static const int HELP_ICON_TIMELINE_ID = 2;
static const int NAVIGATION_HINT_TIMELINE_ID =3;

class HelpDirector : public QObject
{
    Q_OBJECT
public:
    struct Context {
        CustomView *view;
        ItemDelegate *delegate;
        AnimationDirector *director;
    };

    HelpDirector( const Context& context, QObject* parent = 0 );

    bool isEnabled() const { return m_enabled; }
    void setEnabled( bool enabled );

    enum Event {
        GenericEvent,
        IdleTimeout,
        FocusIn,
        FocusOut,
        SelectRelease,
        SelectHold,
        SelectLongHold
    };

    void directEvent( Event event );

private slots:
    void directIdleTimeout();

private:
    enum State { Disabled, Idle, Active, Cue, PlayNow, Done };

    void transition( State end );

    Context m_context;
    bool m_enabled;

    State m_state;
    QTimer *m_idletimer;
};

HelpDirector::HelpDirector( const Context& context, QObject* parent )
    : QObject( parent ), m_context( context ), m_enabled( true ), m_state( Idle )
{
    static const int IDLE_INTERVAL = 3000;

    m_idletimer = new QTimer( this );
    m_idletimer->setInterval( IDLE_INTERVAL );
    connect( m_idletimer, SIGNAL(timeout()), this, SLOT(directIdleTimeout()) );
}

void HelpDirector::setEnabled( bool enabled )
{
    if( m_enabled == enabled )
        return;

    m_enabled = enabled;

    if( m_enabled ) {
        // Reset initial state
        m_state = Idle;
        m_idletimer->start();
    } else {
        m_idletimer->stop();
    }
}

void HelpDirector::directEvent( Event event )
{
    if( !m_enabled )
        return;

    switch( m_state )
    {
    case Disabled:
        if( event == FocusIn ) {
            transition( Idle );
        }
        break;
    case Idle:
        switch( event )
        {
        case IdleTimeout:
            transition( Cue );
            break;
        case FocusOut:
            transition( Disabled );
            break;
        case SelectHold:
            transition( PlayNow );
            break;
        default:
            transition( Active );
            break;
        }
        break;
    case Active:
        switch( event )
        {
        case IdleTimeout:
            transition( Idle );
            break;
        case FocusOut:
            transition( Disabled );
            break;
        case SelectHold:
            transition( PlayNow );
            break;
        default:
            transition( Active );
            break;
        }
        break;
    case Cue:
        if( event == SelectHold ) {
            transition( PlayNow );
        } else {
            transition( Idle );
        }
        break;
    case PlayNow:
        switch( event )
        {
        case SelectRelease:
            // Fade out play now icon
            m_context.director->animate( HELP_ICON_TIMELINE_ID, IconFade( IconFade::FadeOut, m_context.delegate ) );
            // Fade in navigation hint
            {
            NullFrame pause( 15 );
            HintFade fade( HintFade::FadeIn, m_context.delegate );

            m_context.director->animate( NAVIGATION_HINT_TIMELINE_ID, AnimationSequence( QList<Animation*>() << &pause << &fade ) );
            }

            transition( Done );
            break;
        case SelectLongHold:
            // Flash and fade out play now icon
            {
            IconFlash flash( m_context.delegate );
            IconHold hold( ItemDelegate::PlayNow, m_context.delegate );
            IconFade fade( IconFade::FadeOut, m_context.delegate );
            IconHold reset( ItemDelegate::None, m_context.delegate );

            m_context.director->animate( HELP_ICON_TIMELINE_ID, AnimationSequence( QList<Animation*>() << &flash << &hold << &fade << &reset ) );
            }
            // Fade in navigation hint
            {
            NullFrame pause( 25 );
            HintFade fade( HintFade::FadeIn, m_context.delegate );

            m_context.director->animate( NAVIGATION_HINT_TIMELINE_ID, AnimationSequence( QList<Animation*>() << &pause << &fade ) );
            }
            transition( Done );
            break;
        default:
            transition( Idle );
            break;
        }
        break;
    case Done:
        switch( event )
        {
        case FocusIn:
        case FocusOut:
        case SelectRelease:
            // Null
            break;
        case SelectHold:
            transition( PlayNow );
            break;
        default:
            transition( Idle );
            break;
        }
        break;
    }
}

void HelpDirector::directIdleTimeout()
{
    directEvent( IdleTimeout );
}

void HelpDirector::transition( State end )
{
    bool illegal = false;

    switch( end )
    {
    case Disabled:
        switch( m_state )
        {
        case Idle:
        case Active:
             m_idletimer->stop();
            break;
        default:
            illegal = true;
            break;
        }
        break;
    case Idle:
        switch( m_state )
        {
        case Cue:
        case PlayNow:
        case Done:
            // Remove help icon and message
            m_context.view->setHelpOpacity( 0.0 );
            m_context.delegate->setHelpIcon( ItemDelegate::None );
            m_context.delegate->setHelpIconOpacity( 0.0 );
            // Reset navigation hint opacity
            m_context.delegate->setNavigationHintOpacity( 1.0 );
            m_context.view->update();
            // Flush animation
            m_context.director->animate( HELP_MESSAGE_TIMELINE_ID, NullFrame() );
            m_context.director->animate( HELP_ICON_TIMELINE_ID, NullFrame() );
            m_context.director->animate( NAVIGATION_HINT_TIMELINE_ID, NullFrame() );
            // Fall
        case Disabled:
            m_idletimer->start();
            break;
        case Active:
            // Null
            break;
        default:
            illegal = true;
            break;
        }
        break;
    case Active:
        switch( m_state )
        {
        case Idle:
        case Active:
            // Null
            break;
        default:
            illegal = true;
            break;
        }
        break;
    case Cue:
        switch( m_state )
        {
        case Idle:
            m_idletimer->stop();
            // Set help message
            m_context.view->setHelpText( tr( "Press and hold to add to playlist" ) );
            // Fade up messgae and cue icon
            m_context.director->animate( HELP_MESSAGE_TIMELINE_ID, HelpFade( HelpFade::FadeIn, m_context.view ) );
            m_context.delegate->setHelpIcon( ItemDelegate::Cue );
            m_context.delegate->setHelpIconOpacity( 0.0 );
            m_context.director->animate( HELP_ICON_TIMELINE_ID, IconFade( IconFade::FadeIn, m_context.delegate ) );
            // Fade out navigation hint
            m_context.director->animate( NAVIGATION_HINT_TIMELINE_ID, HintFade( HintFade::FadeOut, m_context.delegate ) );
            break;
        default:
            illegal = true;
            break;
        }
        break;
    case PlayNow:
        switch( m_state )
        {
        case Idle:
        case Active:
            m_idletimer->stop();
            // Fall
        case Done:
            // Hide navigation hint
            m_context.delegate->setNavigationHintOpacity( 0.0 );
            // Display message
            m_context.view->setHelpOpacity( 0.9 );
            m_context.director->animate( HELP_MESSAGE_TIMELINE_ID, NullFrame() );
            // Set help message
            m_context.view->setHelpText( tr( "Continue holding to play now" ) );
            // Flash cue icon and hold on play now
            {
            IconHold init( ItemDelegate::Cue, m_context.delegate, 4 );
            IconFlash flash( m_context.delegate );
            IconHold hold( ItemDelegate::PlayNow, m_context.delegate );

            m_context.director->animate( HELP_ICON_TIMELINE_ID, AnimationSequence( QList<Animation*>() << &init << &flash << &hold ) );
            }
            break;
        case Cue:
            // Set help message
            m_context.view->setHelpText( tr( "Continue holding to play now" ) );
            // Flash cue icon and hold on play now
            {
            IconFlash flash( m_context.delegate );
            IconHold hold( ItemDelegate::PlayNow, m_context.delegate );

            m_context.director->animate( HELP_ICON_TIMELINE_ID, AnimationSequence( QList<Animation*>() << &flash << &hold ) );
            }
            break;
        default:
            illegal = true;
            break;
        }
        break;
    case Done:
        switch( m_state )
        {
        case PlayNow:
            // Fade out message
            m_context.director->animate( HELP_MESSAGE_TIMELINE_ID, HelpFade( HelpFade::FadeOut, m_context.view ) );
            break;
        default:
            illegal = true;
            break;
        }
        break;
    }

    if( illegal ) {
        qLog(Media) << "HelpDirector: Illegal transition from" << m_state << "to" << end;
    } else {
        m_state = end;
    }
}

static const QColor TROLLTECH_BLACK = QColor::fromCmykF( 0, 0, 0, 0.9 );
static const QColor TROLLTECH_BLACK_LIGHT = QColor::fromCmykF( 0, 0, 0, 0.7 );

MediaBrowser::MediaBrowser( PlayerControl* control, RequestHandler* handler, QWidget* parent )
    : QWidget( parent ),
      m_requesthandler( NULL ),
      m_requesthandlerparent(handler),
      m_removegroup( NULL ),
      m_savegroup( NULL ),
      m_resetgroup( NULL ),
      m_hasnowplaying( false ),
      m_focused( false )
{
    static const int BROWSER_IDLE_THRESHOLD = 12000;

    CustomView *view = new CustomView;
    connect( view, SIGNAL(heldLong(QModelIndex)),
        this, SLOT(executeHeldLongAction(QModelIndex)) );
    connect( view, SIGNAL(heldReleased()),
        this, SLOT(directSelectRelease()) );
    connect( view, SIGNAL(currentChanged()),
        this, SLOT(directCurrentChange()) );

    view->setHelpText( "Press and hold to add to playlist" );
    view->setHelpOpacity( 0.0 );

    m_view = view;
    connect( m_view, SIGNAL(selected(QModelIndex)),
        this, SLOT(executeSelectedAction(QModelIndex)) );
    connect( m_view, SIGNAL(held(QModelIndex)),
        this, SLOT(executeHeldAction(QModelIndex)) );

    // Process focus events
    m_view->installEventFilter( this );

    m_delegate = new ItemDelegate( this );
    m_delegate->setSpacing( 6 );

#ifdef Q_WS_QWS
    // ### Hack
    m_delegate->setFixedWidth( QScreen::instance()->width() );
#endif

    m_delegate->setHelpIcon( ItemDelegate::Cue );
    m_delegate->setHelpIconOpacity( 0.0 );

    AnimationEngine *engine = new AnimationEngine( m_view, this );

    m_director = new AnimationDirector( engine );

    m_view->setItemDelegate( m_delegate );

    HelpDirector::Context helpcontext = { m_view, m_delegate, m_director };

    m_helpdirector = new HelpDirector( helpcontext, this );
    m_helpdirector->setEnabled( true );

    // Initialize delegate palette
    CustomPalette black;
    black.setColor( CustomPalette::Text, palette().windowText().color() );
    black.setColor( CustomPalette::HighlightText, Qt::white );
    black.setColor( CustomPalette::HighlightBegin, TROLLTECH_BLACK );
    black.setColor( CustomPalette::HighlightEnd, TROLLTECH_BLACK_LIGHT );

    m_delegate->setCustomPalette( black );

    m_stack = new MenuStack( m_view );

    m_history = new IndexHistory;

    m_titlebar = new TitleBar( control );

    // Construct menu
    m_mainmenu = new SimpleMenuModel( this );

    QStringList musicMimeTypes = QMediaContent::supportedMimeTypes().filter( QRegExp( QLatin1String( "audio/\\S+" ) ) );

    if( !musicMimeTypes.isEmpty() )
    {
        if( musicMimeTypes.contains( QLatin1String( "audio/mpeg" ) ) )
        {
            if( !musicMimeTypes.contains( QLatin1String( "audio/mpeg3" ) ) )
                musicMimeTypes.append( QLatin1String( "audio/mpeg3" ) );
            if( !musicMimeTypes.contains( QLatin1String( "audio/mp3" ) ) )
                musicMimeTypes.append( QLatin1String( "audio/mp3" ) );
            if( !musicMimeTypes.contains( QLatin1String( "audio/x-mp3" ) ) )
                musicMimeTypes.append( QLatin1String( "audio/x-mp3" ) );
        }
        musicMimeTypes.removeAll( QLatin1String( "audio/mpegurl" ) );
        musicMimeTypes.removeAll( QLatin1String( "audio/x-mpegurl" ) );
        musicMimeTypes.removeAll( QLatin1String( "audio/x-scpls" ) );

        // Construct music menu
        QContentFilter musicfilter;

        foreach( const QString &mimeType, musicMimeTypes )
            musicfilter |= QContentFilter::mimeType( mimeType );

        musicfilter &= QContentFilter( QContent::Document );

        // Construct music menu
        SimpleMenuModel *musicmenu = new SimpleMenuModel( this );
        m_mainmenu->addItem( tr( "Music" ),
            QIcon( ":icon/mediaplayer/black/music" ), NodeHint, musicmenu );

        SongBrowser *songbrowser = new SongBrowser( musicfilter, this );
        songbrowser->setSortOrder( QStringList() << "synthetic/none/Track" );

        // Construct artist menu
        CustomPropertyBrowser *browser = new CustomPropertyBrowser(
            PropertyBrowser::Artist, musicfilter, this );
        musicmenu->addItem( tr( "Artists" ), browser );

        CustomPropertyBrowser *next = new CustomPropertyBrowser(
            PropertyBrowser::Album, musicfilter, this );
        browser->setNextMenu( next );
        next->setNextMenu( songbrowser );

        // Construct album menu
        browser = new CustomPropertyBrowser(
            PropertyBrowser::Album, musicfilter, this );
        browser->setNextMenu( songbrowser );
        musicmenu->addItem( tr( "Albums" ), browser );

        // Construct genre menu
        browser = new CustomPropertyBrowser(
            PropertyBrowser::Genre, musicfilter, this );
        musicmenu->addItem( tr( "Genres" ), browser );

        next = new CustomPropertyBrowser(
            PropertyBrowser::Artist, musicfilter, this );
        browser->setNextMenu( next );

        browser = next;
        next = new CustomPropertyBrowser(
            PropertyBrowser::Album, musicfilter, this );
        browser->setNextMenu( next );
        next->setNextMenu( songbrowser );

        // Construct song menu
        songbrowser = new SongBrowser( musicfilter, this );
        musicmenu->addItem( tr( "Songs" ), songbrowser );
    }

    QStringList videoMimeTypes = QMediaContent::supportedMimeTypes().filter( QRegExp( QLatin1String( "video/\\S+" ) ) );

    if( !videoMimeTypes.isEmpty() )
    {
        if( videoMimeTypes.contains( QLatin1String( "video/3gpp" ) ) && !videoMimeTypes.contains( "audio/3gpp" ) )
            videoMimeTypes.append( "audio/3gpp" );

        // Construct video menu
        QContentFilter videofilter;

        foreach( const QString &mimeType, videoMimeTypes )
            videofilter |= QContentFilter::mimeType( mimeType );

        videofilter &= QContentFilter( QContent::Document );

        SongBrowser *videobrowser = new SongBrowser( videofilter, this );
        m_mainmenu->addItem( tr( "Videos" ),
            QIcon( ":icon/mediaplayer/black/videos" ), NodeHint, videobrowser );
    }
    // Construct playlist menu
    SimpleMenuModel *playlistmenu = new SimpleMenuModel( this );

    m_currentplaylistmenu = new PlaylistMenuModel( this );
    playlistmenu->addItem( tr( "Current Playlist" ), m_currentplaylistmenu );

    m_myshufflemenu = new PlaylistMenuModel( this );

    QContentFilter playlistfilter = QContentFilter( QContent::Document ) &
            ( QContentFilter( QContentFilter::MimeType, "audio/mpegurl" )
            | QContentFilter::mimeType( QLatin1String( "audio/x-mpegurl" ) )
            | QContentFilter::mimeType( QLatin1String( "audio/x-scpls" ) ));

    PlaylistBrowser *savedbrowser = new PlaylistBrowser( playlistfilter, this );
    playlistmenu->addItem( tr( "Saved Playlists" ), savedbrowser );

    m_mainmenu->addItem( tr( "Playlists" ),
        QIcon( ":icon/mediaplayer/black/playlists" ), NodeHint, playlistmenu );

    m_mainmenu->addItem( tr( "My Shuffle" ),
        QIcon( ":icon/mediaplayer/black/myshuffle" ), NodeHint,
    // ### FIXME should be able to give SLOT(generateMyShuffle)
        this, "generateMyShuffle" );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( m_titlebar );
    layout->addWidget( m_view );
    setLayout( layout );

    QTimer::singleShot(1, this, SLOT(delayMenuCreation()));

    m_browsermonitor = new ActivityMonitor( BROWSER_IDLE_THRESHOLD, this );
    connect( m_browsermonitor, SIGNAL(inactive()), this, SLOT(executeShowPlayerRequest()) );

    // Filter back key events in menu view
    mediaplayer::KeyFilter *filter = new mediaplayer::KeyFilter( m_view, this, this );
    filter->addKey( Qt::Key_Back );

    setFocusProxy( m_view );
}

MediaBrowser::~MediaBrowser()
{
    delete m_mainmenu;
    delete m_stack;

    delete m_history;

    delete m_removegroup;
    delete m_savegroup;

    delete m_director;
}

void MediaBrowser::setCurrentPlaylist( QExplicitlySharedDataPointer<Playlist> playlist )
{
    if( m_playlist != playlist ) {

        if( m_playlist.data() != NULL ) {
            m_playlist->disconnect( this );
        }

        m_playlist = playlist;

        if ( m_playlist == NULL)
            return;

        // Connect to new playlist
        connect( m_playlist, SIGNAL(playingChanged(QModelIndex)),
            this, SLOT(enableNowPlaying()) );
    }
    m_currentplaylistmenu->setPlaylist( playlist );

    enableNowPlaying();
}

bool MediaBrowser::hasBack() const
{
    return m_stack->top() != m_mainmenu;
}

void MediaBrowser::goBack()
{
    m_history->setIndex( m_view->currentIndex().row() ); 

    m_stack->pop();

    MenuModel *top = m_stack->top();

    m_view->setCurrentIndex( top->index( m_history->back() ) );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    m_removegroup->setContext( PlaylistRemoveContext( top ) );
    m_savegroup->setContext( PlaylistSaveContext( top ) );
#endif

    m_titlebar->pop();

    m_helpdirector->directEvent( HelpDirector::GenericEvent );
    m_browsermonitor->update();
}

bool MediaBrowser::eventFilter( QObject* o, QEvent* e )
{
    if( o == m_view ) {
        switch( e->type() )
        {
        case QEvent::FocusIn:
            m_helpdirector->directEvent( HelpDirector::FocusIn );
            m_browsermonitor->update();
            m_focused = true;
            break;
        case QEvent::FocusOut:
            m_helpdirector->directEvent( HelpDirector::FocusOut );
            m_focused = false;
            break;
        default:
            // Ignore
            break;
        }
    }

    return QWidget::eventFilter( o, e );
}

void MediaBrowser::setMediaContent( QMediaContent* content )
{
    m_titlebar->setMediaContent( content );
}

void MediaBrowser::executeSelectedAction( const QModelIndex& index )
{
    ServiceRequest *request = m_stack->top()->action( index, MenuModel::Select );

    if( request ) {
        m_requesthandler->execute( request );
    }

    m_helpdirector->directEvent( HelpDirector::GenericEvent );
    m_browsermonitor->update();
}

void MediaBrowser::executeHeldAction( const QModelIndex& )
{
    // Postpone held action exection until release
    m_helpdirector->directEvent( HelpDirector::SelectHold );
    m_browsermonitor->update();
}

void MediaBrowser::executeHeldLongAction( const QModelIndex& index )
{
    ServiceRequest *request = m_stack->top()->action( index, MenuModel::LongHold );

    if( request ) {
        m_requesthandler->execute( request );
    }

    m_helpdirector->directEvent( HelpDirector::SelectLongHold );
    m_browsermonitor->update();
}

void MediaBrowser::removePlaylistItem()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        QExplicitlySharedDataPointer<Playlist> playlist = model->playlist();
        PlaylistRemove *remove = qobject_cast<PlaylistRemove*>(playlist.data());
        if( remove ) {
            remove->remove( playlist->index( m_view->currentIndex().row() ) );
#ifdef QTOPIA_KEYPAD_NAVIGATION
            MenuModel *top = m_stack->top();
            m_removegroup->setContext( PlaylistRemoveContext( top ) );
            m_savegroup->setContext( PlaylistSaveContext( top ) );
#endif
        }
    }
}

void MediaBrowser::clearPlaylist()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        PlaylistRemove *remove = qobject_cast<PlaylistRemove*>(model->playlist().data());
        if( remove ) {
            if( QMessageBox::question( this, tr( "Clear Playlist" ),
                tr( "Are you sure you want to clear the playlist?" ),
                QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok ) {
                remove->clear();
#ifdef QTOPIA_KEYPAD_NAVIGATION
                MenuModel *top = m_stack->top();
                m_removegroup->setContext( PlaylistRemoveContext( top ) );
                m_savegroup->setContext( PlaylistSaveContext( top ) );
#endif
            }
        }
    }
}

void MediaBrowser::savePlaylist()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        PlaylistSave *save = qobject_cast<PlaylistSave*>(model->playlist().data());
        if( save ) {
            SavePlaylistDialog dialog( this );
            dialog.setText( save->suggestedName() );
            if( QtopiaApplication::execDialog( &dialog, false ) ) {
                save->save( dialog.text() );
            }
        }
    }
}

void MediaBrowser::generateMyShuffle()
{
    // If current playlist isn't already, generate My Shuffle playlist
    if( !qobject_cast<PlaylistMyShuffle*>(m_currentplaylistmenu->playlist().data()) ) {
        if ( !m_myshufflemenu->playlist() ) {
            QContentFilter musicfilter = QContentFilter( QContent::Document ) &
                (QContentFilter( QContentFilter::MimeType, "audio/mpeg" ) |
                QContentFilter::mimeType( "audio/mpeg3" ) |
                QContentFilter::mimeType( "audio/mp3" ) );

            m_myshufflemenu->setPlaylist( QExplicitlySharedDataPointer<Playlist>( new MyShufflePlaylist( musicfilter ) ) );
        }
        m_requesthandler->execute( new PushMenuRequest( m_myshufflemenu ) );
    } else {
        m_requesthandler->execute( new PushMenuRequest( m_currentplaylistmenu ) );
    }
    m_requesthandler->execute( new PushTitleRequest( tr( "Shuffle Playlist" ) ) );
}

void MediaBrowser::resetMyShuffle()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        PlaylistMyShuffle *myshuffle = qobject_cast<PlaylistMyShuffle*>(model->playlist().data());
        if( myshuffle ) {
            if( QMessageBox::question( this, tr( "My Shuffle Reset" ),
                tr( "Are you sure you want to reset all track ratings for My Shuffle?" ),
                QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok ) {
                // Reset track ratings
                myshuffle->reset();
                m_myshufflemenu->setPlaylist( 0 );
                // Generate new shuffle
                goBack();
                generateMyShuffle();
            }
        }
    }
}

void MediaBrowser::enableNowPlaying()
{
    if( m_playlist->playing().isValid() ) {
        if( !m_hasnowplaying ) {
            m_mainmenu->addItem( tr( "Now Playing" ),
                QIcon( ":icon/mediaplayer/black/nowplaying" ), LeafHint, ShowPlayerRequest() );

            m_hasnowplaying = true;
        }
    } else {
        if( m_hasnowplaying ) {
            m_mainmenu->removeRow( m_mainmenu->rowCount( QModelIndex() ) - 1 );

            m_hasnowplaying = false;
        }
    }
}

void MediaBrowser::directCurrentChange()
{
    m_helpdirector->directEvent( HelpDirector::GenericEvent );
    m_browsermonitor->update();

    // If current item is a playlist item, enable contextual help
    // Otherwise, disable contextual help
    QVariant flag = m_view->currentIndex().data( PLAYLIST_ITEM_FLAG );
    if( flag.isValid() ) {
        m_helpdirector->setEnabled( true );
    } else {
        m_helpdirector->setEnabled( false );
    }
}

void MediaBrowser::directSelectRelease()
{
    ServiceRequest *request = m_stack->top()->action( m_view->currentIndex(), MenuModel::Hold );

    if( request ) {
        m_requesthandler->execute( request );
    }

    m_helpdirector->directEvent( HelpDirector::SelectRelease );
}

void MediaBrowser::executeShowPlayerRequest()
{
    // If browser has focus and player active, show player
    if( m_focused && m_playlist->playing().isValid() ) {
        m_requesthandler->execute( new ShowPlayerRequest() );
    }
}

void MediaBrowser::delayMenuCreation()
{
    // Construct soft menu bar
    QAction *removeaction, *separator, *clearaction;
    QAction *saveaction;
    QAction *reset;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *menu = QSoftMenuBar::menuFor( this );

    removeaction = new QAction( tr( "Remove Item" ), this );
    connect( removeaction, SIGNAL(triggered()), this, SLOT(removePlaylistItem()) );
    menu->addAction( removeaction );

    separator = menu->addSeparator();

    clearaction = new QAction( tr( "Clear Playlist..." ), this );
    connect( clearaction, SIGNAL(triggered()), this, SLOT(clearPlaylist()) );
    menu->addAction( clearaction );

    saveaction = new QAction( tr( "Save Playlist..." ), this );
    connect( saveaction, SIGNAL(triggered()), this, SLOT(savePlaylist()) );
    menu->addAction( saveaction );

    reset = new QAction( QIcon( ":icon/reset" ), tr( "Reset My Shuffle..." ) , this );
    connect( reset, SIGNAL(triggered()), this, SLOT(resetMyShuffle()) );
    menu->addAction( reset );

#ifndef NO_HELIX
    QAction* settingsAction = new QAction( QIcon( ":icon/settings" ), tr( "Settings..." ), this );
    connect( settingsAction, SIGNAL(triggered()), this, SLOT(execSettings()) );
    menu->addAction( settingsAction );
#endif

#endif

    m_removegroup = new ActionGroup( QList<QAction*>() << removeaction << separator << clearaction );
    m_savegroup = new ActionGroup( QList<QAction*>() << saveaction );
    m_resetgroup = new ActionGroup( QList<QAction*>() << reset );

    MenuServiceRequestHandler::Context context = { m_stack, m_titlebar, m_view, m_history,
        m_removegroup, m_savegroup, m_resetgroup };
    m_requesthandler = new MenuServiceRequestHandler( context, m_requesthandlerparent );

    // Initialize view
    m_requesthandler->execute( new PushMenuRequest( m_mainmenu ) );
    m_requesthandler->execute( new PushTitleRequest( QString() ) );

}

void MediaBrowser::execSettings()
{
#ifndef NO_HELIX
    MediaPlayerSettingsDialog settingsdialog( this );
    QtopiaApplication::execDialog( &settingsdialog );
#endif
}

#include "mediabrowser.moc"
