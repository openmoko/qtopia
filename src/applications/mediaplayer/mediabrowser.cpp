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

#include "mediabrowser.h"

#include "playlist.h"
#include "statewidget.h"
#include "elidedlabel.h"

#include <qmediacontrol.h>
#include <qmediatools.h>
#include <qmediawidgets.h>
#include <browser.h>
#include <servicerequest.h>

#include <qtopiaapplication.h>
#include <qcontentset.h>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#include <custom.h>
#endif

class HighlightMenuItemRequest : public ServiceRequest
{
public:
    // ServiceRequest
    Type type() const { return ServiceRequest::User; }
    ServiceRequest* clone() const { return new HighlightMenuItemRequest; }
};

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
        QContentList items = QContentSet( filter() & QContentFilter( QContentFilter::Synthetic, filterString( index ) ) ).items();

        QStringList urls;
        foreach( QContent content, items ) {
            urls.append( content.file() );
        }

        CuePlaylistRequest cue( new BasicPlaylist( urls ) );
        HighlightMenuItemRequest highlight;

        return new CompoundRequest( QList<ServiceRequest*>() << &cue << &highlight );
        }
    default:
        // Ignore
        break;
    }

    return 0;
}

class SongBrowser : public ContentBrowser
{
public:
    SongBrowser( const QContentFilter& filter = QContentFilter(), QObject* parent = 0 )
        : ContentBrowser( filter, parent )
    { }

    ServiceRequest* action( const QModelIndex& index, ActionContext context ) const;
};

ServiceRequest* SongBrowser::action( const QModelIndex& index, ActionContext context ) const
{
    switch( context )
    {
    case Select:
        {
        QStringList urls;
        for( int i = 0; i < rowCount(); ++i ) {
            urls.append( content( SongBrowser::index( i ) ).file() );
        }

        Playlist *playlist = new BasicPlaylist( urls );
        playlist->setPlaying( playlist->index( index.row() ) );

        return new OpenPlaylistRequest( playlist );
        }
    case Hold:
        {
        CuePlaylistRequest cue( new BasicPlaylist( QStringList( content( index ).file() ) ) );
        HighlightMenuItemRequest highlight;

        return new CompoundRequest( QList<ServiceRequest*>() << &cue << &highlight );
        }
    default:
        // Ignore
        break;
    }

    return 0;
}

class PlaylistBrowser : public ContentBrowser
{
public:
    PlaylistBrowser( const QContentFilter& filter = QContentFilter(), QObject* parent = 0 )
        : ContentBrowser( filter, parent )
    { }

    ServiceRequest* action( const QModelIndex& index, ActionContext context ) const;
};

ServiceRequest* PlaylistBrowser::action( const QModelIndex& index, ActionContext context ) const
{
    static const int SUFFIX_LENGTH = 4;

    Playlist *playlist = 0;

    QString file = content( index ).file();
    if( file.right( SUFFIX_LENGTH ) == ".m3u" ) {
        playlist = new M3UPlaylist( file );
    }

    switch( context )
    {
    case Select:
        if( playlist ) {
            playlist->setPlaying( playlist->index( 0 ) );
            return new OpenPlaylistRequest( playlist );
        }
    case Hold:
        if( playlist ) {
            CuePlaylistRequest cue( playlist );
            HighlightMenuItemRequest highlight;

            return new CompoundRequest( QList<ServiceRequest*>() << &cue << &highlight );
        }
    default:
        // Ignore
        break;
    }

    return 0;
}

class PlaylistMenuModel : public MenuModel,
    public PlaylistModel
{
    Q_OBJECT
    Q_INTERFACES(PlaylistModel)
public:
    PlaylistMenuModel( QObject* parent = 0 )
        : MenuModel( parent ), m_playlist( 0 )
    { }

    void setPlaylist( Playlist* playlist );

    // PlaylistModel
    Playlist* playlist() const { return m_playlist; }

    // MenuModel
    ServiceRequest* action( const QModelIndex& index, ActionContext context = Select ) const;

    // AbstractListModel
    int rowCount( const QModelIndex& = QModelIndex() ) const { return m_playlist->rowCount(); }
    QVariant data( const QModelIndex& index, int role ) const;

private:
    Playlist *m_playlist;
};

void PlaylistMenuModel::setPlaylist( Playlist* playlist )
{
    // Disconnect from old playlist
    if( m_playlist ) {
        m_playlist->disconnect( this );
    }

    m_playlist = playlist;

    // Connect to new playlist
    connect( m_playlist, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
        this, SIGNAL(rowsInserted(const QModelIndex&,int,int)) );
    connect( m_playlist, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
        this, SIGNAL(rowsRemoved(const QModelIndex&,int,int)) );
}

ServiceRequest* PlaylistMenuModel::action( const QModelIndex& index, ActionContext context ) const
{
    switch( context )
    {
    case MenuModel::Select:
        m_playlist->setPlaying( m_playlist->index( index.row() ) );
        return new OpenPlaylistRequest( m_playlist );
    case MenuModel::Hold:
        {
        CuePlaylistRequest cue( new BasicPlaylist( QStringList( m_playlist->data( m_playlist->index( index.row() ), Playlist::Url ).toString() ) ) );
        HighlightMenuItemRequest highlight;

        return new CompoundRequest( QList<ServiceRequest*>() << &cue << &highlight );
        }
    }

    return 0;
}

QVariant PlaylistMenuModel::data( const QModelIndex& index, int role ) const
{
    if( role == Qt::DisplayRole ) {
        return m_playlist->data( m_playlist->index( index.row() ), Playlist::Title );
    }

    return QVariant();
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

    m_statewidget = new StateWidget( control );
    layout->addWidget( m_statewidget );

    m_muteicon = new QMediaVolumeLabel( QMediaVolumeLabel::MuteVolume );
    layout->addWidget( m_muteicon );

    m_titlelabel = new ElidedLabel;
    m_titlelabel->setAlignment( Qt::AlignRight );
    layout->addWidget( m_titlelabel, STRETCH_MAX );

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
    m_control = new QMediaControl( m_content->handle() );
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
    static const QColor TROLLTECH_WHITE = Qt::white;
    static const QColor TROLLTECH_WHITE_DARK = QColor::fromCmykF( 0, 0, 0, 0.1 );

    QPainter painter( this );

    // Paint gradient background
    QLinearGradient gradient( 0, 0, 0, height() );
    gradient.setColorAt( 0.3, TROLLTECH_WHITE );
    gradient.setColorAt( 1.0, TROLLTECH_WHITE_DARK );

    painter.setPen( Qt::NoPen );
    painter.setBrush( gradient );
    painter.drawRect( rect() );
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
        Playlist *playlist = playlistmodel->playlist();
        PlaylistRemove *remove = qobject_cast<PlaylistRemove*>(playlist);
        if( remove ) {
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
        Playlist *playlist = playlistmodel->playlist();
        PlaylistSave *save = qobject_cast<PlaylistSave*>(playlist);
        if( save ) {
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

        QTimeLine *timeline;
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
#endif

        delete request;
        }
        break;
    case ServiceRequest::User:
        m_context.timeline->start();
        delete request;
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

// Blend y into x
static QColor blend( const QColor& x, const QColor& y )
{
    int r = 255 * x.red() + (y.red() - x.red()) * y.alpha();
    int g = 255 * x.green() + (y.green() - x.green()) * y.alpha();
    int b = 255 * x.blue() + (y.blue() - x.blue()) * y.alpha();

    return QColor(  r/255, g/255, b/255 );
}

static CustomPalette blend( const CustomPalette& x, const CustomPalette& y )
{
    CustomPalette palette;
    for( int role = CustomPalette::Text; role < CustomPalette::EndRole; ++role ) {
        palette.setColor( role, blend( x.color( role ), y.color( role ) ) );
    }

    return palette;
}

class ItemDelegate : public QAbstractItemDelegate
{
public:
    ItemDelegate( QObject* parent = 0 )
        : QAbstractItemDelegate( parent )
    { }

    void setCustomPalette( const CustomPalette& palette ) { m_palette = palette; }

    void setSpacing( int spacing ) { m_spacing = spacing; }

    // QAbstractItemDelegate
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

private:
    int m_spacing;
    CustomPalette m_palette;
};

void ItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    painter->save();

    QRect rect = option.rect;

    bool selected = option.showDecorationSelected && (option.state & QStyle::State_Selected);

    // Draw background
    if( selected ) {
        QLinearGradient gradient( rect.topLeft(), rect.bottomLeft() );
        gradient.setColorAt( 0.3, m_palette.color( CustomPalette::HighlightBegin ) );
        gradient.setColorAt( 0.9, m_palette.color( CustomPalette::HighlightEnd ) );

        painter->setPen( Qt::NoPen );
        painter->setBrush( gradient );
        painter->drawRect( rect );
    }

    // Subtract spacing from item rect
    rect.adjust( m_spacing, m_spacing, -m_spacing, -m_spacing );

    // Draw text
    QFontMetrics metrics( option.font );
    QString text = metrics.elidedText( index.data().toString(), option.textElideMode, rect.width() );

    if( selected ) {
        painter->setPen( m_palette.color( CustomPalette::HighlightText ) );
    } else {
        painter->setPen( m_palette.color( CustomPalette::Text ) );
    }
    painter->drawText( rect, Qt::AlignVCenter, text );

    painter->restore();
}

QSize ItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QFontMetrics metrics( option.font );

    int size = metrics.height() + m_spacing*2;

    return QSize( size, size );
}

class PaletteBlend : public QObject
{
    Q_OBJECT
public:
    struct Context
    {
        MenuView *view;
        ItemDelegate *delegate;

        CustomPalette begin, end;
    };

    PaletteBlend( const Context& context, QObject* parent = 0 )
        : QObject( parent ), m_context( context )
    { }

public slots:
    void reset();
    void blend( int frame );

private:
    Context m_context;
};

void PaletteBlend::reset()
{
    m_context.delegate->setCustomPalette( m_context.begin );
}

void PaletteBlend::blend( int frame )
{
    CustomPalette palette = m_context.end;

    for( int role = CustomPalette::Text; role < CustomPalette::EndRole; ++role ) {
        QColor color = palette.color( role );
        color.setAlpha( frame );
        palette.setColor( role, color );
    }

    m_context.delegate->setCustomPalette( ::blend( m_context.begin, palette ) );
    m_context.view->repaint();
}

static const QColor TROLLTECH_GREEN = QColor( 166, 206, 57 );
static const QColor TROLLTECH_GREEN_DARK = QColor( 146, 181, 51 );

static const QColor TROLLTECH_BLACK = QColor::fromCmykF( 0, 0, 0, 0.9 );
static const QColor TROLLTECH_BLACK_LIGHT = QColor::fromCmykF( 0, 0, 0, 0.7 );

MediaBrowser::MediaBrowser( PlayerControl* control, RequestHandler* handler, QWidget* parent )
    : QWidget( parent )
{
    m_view = new MenuView;
    connect( m_view, SIGNAL(selected(const QModelIndex&)),
        this, SLOT(executeSelectedAction(const QModelIndex&)) );
    connect( m_view, SIGNAL(held(const QModelIndex&)),
        this, SLOT(executeHeldAction(const QModelIndex&)) );

    m_delegate = new ItemDelegate( this );
    m_delegate->setSpacing( 6 );

    m_view->setItemDelegate( m_delegate );

    CustomPalette black;
    black.setColor( CustomPalette::Text, Qt::black );
    black.setColor( CustomPalette::HighlightText, Qt::white );
    black.setColor( CustomPalette::HighlightBegin, TROLLTECH_BLACK );
    black.setColor( CustomPalette::HighlightEnd, TROLLTECH_BLACK_LIGHT );

    CustomPalette green;
    green.setColor( CustomPalette::Text, Qt::black );
    green.setColor( CustomPalette::HighlightText, Qt::black );
    green.setColor( CustomPalette::HighlightBegin, TROLLTECH_GREEN );
    green.setColor( CustomPalette::HighlightEnd, TROLLTECH_GREEN_DARK );

    PaletteBlend::Context blendcontext = { m_view, m_delegate, black, green };
    m_paletteblend = new PaletteBlend( blendcontext );

    m_timeline = new QTimeLine( 300, this );
    connect( m_timeline, SIGNAL(frameChanged(int)), m_paletteblend, SLOT(blend(int)) );
    connect( m_timeline, SIGNAL(finished()), m_paletteblend, SLOT(reset()) );
    m_timeline->setFrameRange( 0, 255 );

    m_stack = new MenuStack( m_view );

    m_history = new IndexHistory;

    m_titlebar = new TitleBar( control );

    // Construct menu
    m_mainmenu = new SimpleMenuModel( this );

    // Construct music menu
    SimpleMenuModel *musicmenu = new SimpleMenuModel( this );
    m_mainmenu->addItem( tr( "Music" ), musicmenu );

    QContentFilter musicfilter = QContentFilter( QContent::Document ) &
        QContentFilter( QContentFilter::MimeType, "audio/mpeg" );

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

    // Construct video menu
    QContentFilter videofilter = QContentFilter( QContent::Document ) &
        QContentFilter( QContentFilter::MimeType, "video/3gpp" );

    SongBrowser *videobrowser = new SongBrowser( videofilter, this );
    m_mainmenu->addItem( tr( "Videos" ), videobrowser );

    // Construct playlist menu
    SimpleMenuModel *playlistmenu = new SimpleMenuModel( this );

    m_currentplaylistmenu = new PlaylistMenuModel( this );
    playlistmenu->addItem( tr( "Current Playlist" ), m_currentplaylistmenu );

    QContentFilter playlistfilter = QContentFilter( QContent::Document ) &
        QContentFilter( QContentFilter::MimeType, "audio/mpegurl" );

    PlaylistBrowser *savedbrowser = new PlaylistBrowser( playlistfilter, this );
    playlistmenu->addItem( tr( "Saved Playlists" ), savedbrowser );

    m_mainmenu->addItem( tr( "Playlists" ), playlistmenu );

    m_mainmenu->addItem( tr( "Now Playing" ), ShowPlayerRequest() );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( m_titlebar );
    layout->addWidget( m_view );
    setLayout( layout );

    // Construct soft menu bar
    QAction *removeaction, *clearaction;
    QAction *saveaction;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *menu = QSoftMenuBar::menuFor( this );

    removeaction = new QAction( tr( "Remove Item" ), this );
    connect( removeaction, SIGNAL(triggered()), this, SLOT(removePlaylistItem()) );
    menu->addAction( removeaction );

    menu->addSeparator();

    clearaction = new QAction( tr( "Clear Playlist..." ), this );
    connect( clearaction, SIGNAL(triggered()), this, SLOT(clearPlaylist()) );
    menu->addAction( clearaction );

    saveaction = new QAction( tr( "Save Playlist..." ), this );
    connect( saveaction, SIGNAL(triggered()), this, SLOT(savePlaylist()) );
    menu->addAction( saveaction );
#endif

    m_removegroup = new ActionGroup( QList<QAction*>() << removeaction << clearaction );
    m_savegroup = new ActionGroup( QList<QAction*>() << saveaction );

    MenuServiceRequestHandler::Context context = { m_stack, m_titlebar, m_view, m_history, m_removegroup, m_savegroup, m_timeline };
    m_requesthandler = new MenuServiceRequestHandler( context, handler );

    // Initialize view
    m_requesthandler->execute( new PushMenuRequest( m_mainmenu ) );
    m_requesthandler->execute( new PushTitleRequest( QString() ) );

    m_paletteblend->reset();

    // Filter back key events in menu view
    KeyFilter *filter = new KeyFilter( m_view, this, this );
    filter->addKey( Qt::Key_Back );

    setFocusProxy( m_view );
}

MediaBrowser::~MediaBrowser()
{
    delete m_mainmenu;
    delete m_stack;

    delete m_paletteblend;

    delete m_history;

    delete m_removegroup;
    delete m_savegroup;
}

void MediaBrowser::setCurrentPlaylist( Playlist* playlist )
{
    m_currentplaylistmenu->setPlaylist( playlist );
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
}

void MediaBrowser::executeHeldAction( const QModelIndex& index )
{
    ServiceRequest *request = m_stack->top()->action( index, MenuModel::Hold );

    if( request ) {
        m_requesthandler->execute( request );
    }
}

void MediaBrowser::removePlaylistItem()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        Playlist *playlist = model->playlist();
        PlaylistRemove *remove = qobject_cast<PlaylistRemove*>(playlist);
        if( remove ) {
            remove->remove( playlist->index( m_view->currentIndex().row() ) );
        }
    }
}

void MediaBrowser::clearPlaylist()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        PlaylistRemove *remove = qobject_cast<PlaylistRemove*>(model->playlist());
        if( remove ) {
            remove->clear();
        }
    }
}

void MediaBrowser::savePlaylist()
{
    PlaylistModel *model = qobject_cast<PlaylistModel*>(m_view->model());
    if( model ) {
        PlaylistSave *save = qobject_cast<PlaylistSave*>(model->playlist());
        if( save ) {
            SavePlaylistDialog dialog( this );
            dialog.setText( save->suggestedName() );
            if( QtopiaApplication::execDialog( &dialog, false ) ) {
                save->save( dialog.text() );
            }
        }
    }
}

#include "mediabrowser.moc"
