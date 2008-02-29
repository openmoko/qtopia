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

#include "playerwidget.h"

#include "playlist.h"
#include "elidedlabel.h"
#include "visualization.h"

#include <media.h>
#include <qmediatools.h>
#include <qmediawidgets.h>
#include <qmediahelixsettingscontrol.h>
#include <qmediavideocontrol.h>
#include <private/keyhold_p.h>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#include <custom.h>
#endif

#include <qtranslatablesettings.h>
#include <qtopiaapplication.h>
#include <qthumbnail.h>

#include <QtSvg>

#ifndef NO_HELIX_LOGO
class HelixLogo : public QWidget
{
public:
    HelixLogo( QWidget* parent = 0 );

    // QWidget
    QSize sizeHint() const { return QSize( m_helixlogo.width(), m_helixlogo.height() ); }

protected:
    // QWidget
    void resizeEvent( QResizeEvent* e );
    void paintEvent( QPaintEvent* e );

private:
    QImage m_helixlogo;
    QPoint m_helixlogopos;
};

HelixLogo::HelixLogo( QWidget* parent )
    : QWidget( parent )
{
    static const QString HELIX_LOGO_PATH = ":image/mediaplayer/helixlogo";

    m_helixlogo = QImage( HELIX_LOGO_PATH );
}

void HelixLogo::resizeEvent( QResizeEvent* )
{
    m_helixlogopos = QPoint( (width() - m_helixlogo.width()) / 2,
        (height() - m_helixlogo.height()) / 2 );
}

void HelixLogo::paintEvent( QPaintEvent* )
{
    QPainter painter( this );

    painter.drawImage( m_helixlogopos, m_helixlogo );
}
#endif

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog( QWidget* parent = 0 );

    // QDialog
    void accept();

private:
    void readConfig();
    void writeConfig();

    void applySettings();

    QMediaContent *m_content;

    QComboBox *m_speedcombo;

    QLineEdit *m_connecttimeout;
    QLineEdit *m_servertimeout;
    QValidator *m_validator;
};

SettingsDialog::SettingsDialog( QWidget* parent )
    : QDialog( parent )
{
    static const int TIMEOUT_MIN = 5;
    static const int TIMEOUT_MAX = 99;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 4 );

    QGroupBox *group = new QGroupBox( tr( "Network Settings" ), this );

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget( new QLabel( tr( "Connection Speed" ) ) );

    m_speedcombo = new QComboBox;
    vbox->addWidget( m_speedcombo );

    QFrame *hr = new QFrame;
    hr->setFrameStyle( QFrame::HLine | QFrame::Plain );
    vbox->addWidget( hr );

    QGridLayout *grid = new QGridLayout;
    grid->addWidget( new QLabel( tr( "Timeouts:" )), 0, 0, 1, 2, Qt::AlignCenter );
    grid->addWidget( new QLabel( tr( "Connection" ) ), 1, 0, Qt::AlignLeft );

    m_validator = new QIntValidator( TIMEOUT_MIN, TIMEOUT_MAX, this );

    m_connecttimeout = new QLineEdit;
    m_connecttimeout->setAlignment( Qt::AlignRight );
    m_connecttimeout->setValidator( m_validator );
    grid->addWidget( m_connecttimeout, 1, 1, Qt::AlignRight );

    grid->addWidget( new QLabel( tr( "Server" ) ), 2, 0, Qt::AlignLeft );

    m_servertimeout = new QLineEdit;
    m_servertimeout->setAlignment( Qt::AlignRight );
    m_servertimeout->setValidator( m_validator );
    grid->addWidget( m_servertimeout, 2, 1, Qt::AlignRight );

    vbox->addLayout( grid );
    vbox->addStretch();

    group->setLayout( vbox );

    layout->addWidget( group );
    setLayout( layout );

    // Initialize settings
    readConfig();

    applySettings();
}

void SettingsDialog::accept()
{
    writeConfig();
    applySettings();

    QDialog::accept();
}

void SettingsDialog::readConfig()
{
    QTranslatableSettings config( "Trolltech", "MediaPlayer" );
    config.beginGroup( "Network" );

    int size = config.beginReadArray( "ConnectionSpeed" );
    for( int i = 0; i < size; ++i ) {
        config.setArrayIndex( i );
        m_speedcombo->addItem( config.value( "Type" ).toString(), config.value( "Speed" ) );
    }
    config.endArray();

    if( !m_speedcombo->count() ) {
        m_speedcombo->addItem( tr("GPRS (32 kbps)"), 32000 );
        m_speedcombo->addItem( tr("EGPRS (128 kbps)"), 128000 );
    }

    QVariant value = config.value( "ConnectionSpeedIndex" );
    if( value.isNull() || value.toInt() > m_speedcombo->count() ) {
        value = 0;
    }
    m_speedcombo->setCurrentIndex( value.toInt() );

    value = config.value( "ConnectionTimeout" );
    if( value.isNull() ) {
        value = "5";
    }
    m_connecttimeout->setText( value.toString() );

    value = config.value( "ServerTimeout" );
    if( value.isNull() ) {
        value = "5";
    }
    m_servertimeout->setText( value.toString() );
}

void SettingsDialog::writeConfig()
{
    QTranslatableSettings config( "Trolltech", "MediaPlayer" );
    config.beginGroup( "Network" );

    config.beginWriteArray( "ConnectionSpeed" );
    for( int i = 0; i < m_speedcombo->count(); ++i ) {
        config.setArrayIndex( i );
        config.setValue( "Type", m_speedcombo->itemText( i ) );
        config.setValue( "Speed", m_speedcombo->itemData( i ) );
    }
    config.endArray();

    config.setValue( "ConnectionSpeedIndex", m_speedcombo->currentIndex() );

    config.setValue( "ConnectionTimeout", m_connecttimeout->text() );
    config.setValue( "ServerTimeout", m_servertimeout->text() );
}

void SettingsDialog::applySettings()
{
    QMediaHelixSettingsControl settings;

    settings.setOption( "Bandwidth",
        m_speedcombo->itemData( m_speedcombo->currentIndex() ) );

    settings.setOption( "ConnectionTimeOut", m_connecttimeout->text() );
    settings.setOption( "ServerTimeOut", m_servertimeout->text() );
}

class PlaylistLabel : public QWidget
{
    Q_OBJECT
public:
    PlaylistLabel( QWidget* parent = 0 );

    void setPlaylist( Playlist* playlist );

private slots:
    void updateLabel();

private:
    Playlist *m_playlist;

    QLabel *m_label;
};

PlaylistLabel::PlaylistLabel( QWidget* parent )
    : QWidget( parent ), m_playlist( 0 )
{
    m_label = new QLabel ( tr( "- of -", "song '- of -'") );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    layout->addWidget( m_label );
    setLayout( layout );
}

void PlaylistLabel::setPlaylist( Playlist* playlist )
{
    // Disconnect from old playlist
    if( m_playlist ) {
        m_playlist->disconnect( this );
    }

    m_playlist = playlist;

    // Connect to new playlist
    connect( m_playlist, SIGNAL(playingChanged(const QModelIndex&)),
        this, SLOT(updateLabel()) );
    connect( m_playlist, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
        this, SLOT(updateLabel()) );
    connect( m_playlist, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
        this, SLOT(updateLabel()) );

    updateLabel();
}

void PlaylistLabel::updateLabel()
{
    int count = m_playlist->rowCount();

    if( count ) {
        QModelIndex playing = m_playlist->playing();
        if( playing.isValid() ) {
            m_label->setText( tr( "%1 of %2", " e.g. '4 of 6' as in song 4 of 6" ).arg( playing.row() + 1 ).arg( count ) );
        } else {
            m_label->setText( tr( "- of %2", "- of 8" ).arg( count ) );
        }
    } else {
        m_label->setText( tr( "- of -", "song '- of -'" ) );
    }
}

class ProgressView : public QWidget
{
    Q_OBJECT
public:
    ProgressView( QWidget* parent = 0 );

    void setPlaylist( Playlist* playlist ) { m_playlistlabel->setPlaylist( playlist ); }

    QWidget* keyEventHandler() const { return m_progress; }

public slots:
    void setMediaContent( QMediaContent* content );

private:
    QMediaContentContext *m_context;
    QMediaProgressWidget *m_progress;
    PlaylistLabel *m_playlistlabel;
};

ProgressView::ProgressView( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_progress = new QMediaProgressWidget;
    layout->addWidget( m_progress );

    QHBoxLayout *hbox = new QHBoxLayout;

    m_playlistlabel = new PlaylistLabel;
    hbox->addWidget( m_playlistlabel );
    hbox->addStretch();

    QMediaProgressLabel *progresslabel = new QMediaProgressLabel( QMediaProgressLabel::ElapsedTotalTime );
    hbox->addWidget( progresslabel );

    layout->addLayout( hbox );
    setLayout( layout );

    m_context = new QMediaContentContext( this );
    m_context->addObject( m_progress );
    m_context->addObject( progresslabel );
}

void ProgressView::setMediaContent( QMediaContent* content )
{
    m_context->setMediaContent( content );
}

class VolumeView : public QWidget
{
    Q_OBJECT
public:
    VolumeView( QWidget* parent = 0 );

    QWidget* keyEventHandler() const { return m_volume; }

public slots:
    void setMediaContent( QMediaContent* content );

private:
    QMediaVolumeWidget *m_volume;
};

VolumeView::VolumeView( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_volume = new QMediaVolumeWidget;
    layout->addWidget( m_volume );

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget( new QMediaVolumeLabel( QMediaVolumeLabel::MinimumVolume ) );
    hbox->addStretch();
    hbox->addWidget( new QMediaVolumeLabel( QMediaVolumeLabel::MaximumVolume ) );

    layout->addLayout( hbox );
    setLayout( layout );
}

void VolumeView::setMediaContent( QMediaContent* content )
{
    m_volume->setMediaContent( content );
}

class SeekView : public QWidget
{
    Q_OBJECT
public:
    SeekView( QWidget* parent = 0 );

    QWidget* keyEventHandler() const { return m_seekwidget; }

public slots:
    void setMediaContent( QMediaContent* content );

private:
    QMediaSeekWidget *m_seekwidget;
};

SeekView::SeekView( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_seekwidget = new QMediaSeekWidget;
    layout->addWidget( m_seekwidget );

    QHBoxLayout *hbox = new QHBoxLayout;

    QMediaProgressLabel *elapsed = new QMediaProgressLabel( QMediaProgressLabel::ElapsedTime );
    connect( m_seekwidget, SIGNAL(lengthChanged(quint32)),
        elapsed, SLOT(setTotal(quint32)) );
    connect( m_seekwidget, SIGNAL(positionChanged(quint32)),
        elapsed, SLOT(setElapsed(quint32)) );
    hbox->addWidget( elapsed );
    hbox->addStretch();

    QMediaProgressLabel *remaining = new QMediaProgressLabel( QMediaProgressLabel::RemainingTime );
    connect( m_seekwidget, SIGNAL(lengthChanged(quint32)),
        remaining, SLOT(setTotal(quint32)) );
    connect( m_seekwidget, SIGNAL(positionChanged(quint32)),
        remaining, SLOT(setElapsed(quint32)) );
    hbox->addWidget( remaining );

    layout->addLayout( hbox );
    setLayout( layout );
}

void SeekView::setMediaContent( QMediaContent* content )
{
    m_seekwidget->setMediaContent( content );
}

class ThumbnailWidget : public QWidget
{
public:
    ThumbnailWidget( QWidget* parent = 0 )
        : QWidget( parent )
    { }

    void setFile( const QString& file );

protected:
    void resizeEvent( QResizeEvent* ) { m_thumb = QPixmap(); }

    // Load thumbnail and draw onto widget
    void paintEvent( QPaintEvent* );

private:
    QString m_file;

    QPixmap m_thumb;
    QPoint m_thumbpos;
};

void ThumbnailWidget::setFile( const QString& file )
{
    m_file = file;
    m_thumb = QPixmap();

    update();
}

void ThumbnailWidget::paintEvent( QPaintEvent* )
{
    if( m_thumb.isNull() && !m_file.isNull() ) {
        m_thumb = QThumbnail( m_file ).pixmap( size() );
        m_thumbpos = QPoint( (width() - m_thumb.width())/2, (height() - m_thumb.height())/2 );
    }

    QPainter painter( this );
    painter.drawPixmap( m_thumbpos, m_thumb );
}

class TrackInfoDialog : public QDialog
{
    Q_OBJECT
public:
    TrackInfoDialog( QWidget* parent = 0, Qt::WindowFlags f = 0 );

    void setPlaylist( Playlist* playlist );

private slots:
    void updateInfo();

private:
    Playlist *m_playlist;

    ElidedLabel *m_track, *m_artist, *m_album;
    ThumbnailWidget *m_cover;
};

TrackInfoDialog::TrackInfoDialog( QWidget* parent, Qt::WindowFlags f )
    : QDialog( parent, f ), m_playlist( 0 )
{
    static const int STRETCH_MAX = 1;
    static const QColor TROLLTECH_FROST = QColor( 255, 255, 255, 190 );

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin( 6 );
    layout->setSpacing( 6 );

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin( 0 );
    vbox->setSpacing( 4 );

    m_cover = new ThumbnailWidget;
    int side = fontMetrics().height()*3 + vbox->spacing()*2;
    m_cover->setMinimumSize( QSize( side, side ) );

    layout->addWidget( m_cover );

    m_track = new ElidedLabel;
    vbox->addWidget( m_track );

    m_album = new ElidedLabel;
    vbox->addWidget( m_album );

    m_artist = new ElidedLabel;
    vbox->addWidget( m_artist );

    layout->addLayout( vbox, STRETCH_MAX );

    setLayout( layout );

    // Set custom palette
    QPalette pal = palette();
    pal.setColor( QPalette::Window, TROLLTECH_FROST );
    setPalette( pal );

    // Remove title bar from dialog
    setWindowFlags( windowFlags() | Qt::FramelessWindowHint );
}

void TrackInfoDialog::setPlaylist( Playlist* playlist )
{
    // Disconnect from old playlist
    if( m_playlist ) {
        m_playlist->disconnect( this );
    }

    m_playlist = playlist;

    // Connect to new playlist
    connect( m_playlist, SIGNAL(playingChanged(const QModelIndex&)),
        this, SLOT(updateInfo()) );

    updateInfo();
}

void TrackInfoDialog::updateInfo()
{
    QModelIndex playing = m_playlist->playing();

    QVariant variant = m_playlist->data( playing, Playlist::Title );
    if( variant.isValid() ) {
        m_track->setText( variant.toString() );
    } else {
        m_track->setText( tr( "Unknown Track" ) );
    }

    variant = m_playlist->data( playing, Playlist::Album );
    if( variant.isValid() ) {
        m_album->setText( variant.toString() );
    } else {
        m_album->setText( tr( "Unknown Album" ) );
    }

    variant = m_playlist->data( playing, Playlist::Artist );
    if( variant.isValid() ) {
        m_artist->setText( variant.toString() );
    } else {
        m_artist->setText( tr( "Unknown Artist" ) );
    }

    variant = m_playlist->data( playing, Playlist::AlbumCover );
    if( variant.isValid() ) {
        m_cover->setFile( variant.toString() );
        m_cover->show();
    } else {
        m_cover->hide();
    }
}

class TrackInfoWidget : public QWidget
{
    Q_OBJECT
public:
    TrackInfoWidget( QWidget* parent = 0 );

    void setPlaylist( Playlist* playlist );

private slots:
    void updateInfo();

private:
    Playlist *m_playlist;

    ElidedLabel *m_label;
};

TrackInfoWidget::TrackInfoWidget( QWidget* parent )
    : QWidget( parent ), m_playlist( 0 )
{
    m_label = new ElidedLabel;
    m_label->setAlignment( Qt::AlignRight );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    layout->addWidget( m_label );
    setLayout( layout );
}

void TrackInfoWidget::setPlaylist( Playlist* playlist )
{
    // Disconnect from old playlist
    if( m_playlist ) {
        m_playlist->disconnect( this );
    }

    m_playlist = playlist;

    // Connect to new playlist
    connect( m_playlist, SIGNAL(playingChanged(const QModelIndex&)),
        this, SLOT(updateInfo()) );

    updateInfo();
}

void TrackInfoWidget::updateInfo()
{
    QModelIndex playing = m_playlist->playing();
    m_label->setText( m_playlist->data( playing, Playlist::Title ).toString() );
}

class ThrottleWidget : public QWidget
{
    Q_OBJECT
public:
    ThrottleWidget( QWidget* parent = 0 );

    void setOpacity( qreal opacity );

    // Set resolution between 0.0 and 1.0, default 0.1
    void setResolution( qreal resolution ) { m_resolution = resolution; }

    // Return current intensity between -1.0 and 1.0
    qreal intensity() const { return m_intensity; }

    QSize sizeHint() const { return QSize( fontMetrics().height(), fontMetrics().height() ); }

signals:
    void pressed();
    void released();

    void intensityChanged( qreal intensity );

protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );

    void resizeEvent( QResizeEvent* e );
    void paintEvent( QPaintEvent* e );

private:
    qreal calculateIntensity( const QPoint& point );

    qreal m_intensity;
    qreal m_resolution;
    qreal m_opacity;

    QImage m_control;
    QPoint m_controlpos;
};

ThrottleWidget::ThrottleWidget( QWidget* parent )
    : QWidget( parent ), m_intensity( 0.0 ), m_resolution( 0.1 ), m_opacity( 1.0 )
{ }

void ThrottleWidget::setOpacity( qreal opacity )
{
    m_opacity = opacity;

    update();
}

void ThrottleWidget::mousePressEvent( QMouseEvent* e )
{
    m_intensity = calculateIntensity( e->pos() );

    emit pressed();
}

void ThrottleWidget::mouseReleaseEvent( QMouseEvent* )
{
    m_intensity = 0.0;

    emit released();
}

void ThrottleWidget::mouseMoveEvent( QMouseEvent* e )
{
    qreal intensity = calculateIntensity( e->pos() );
    qreal delta = m_intensity - intensity;
    if( delta < 0 ) {
        delta = -delta;
    }

    if( delta >= m_resolution  ) {
        m_intensity = intensity;

        emit intensityChanged( m_intensity );
    }
}

void ThrottleWidget::resizeEvent( QResizeEvent* )
{
    m_control = QImage();
}

void ThrottleWidget::paintEvent( QPaintEvent* )
{
    static const QString THROTTLE_CONTROL = ":image/mediaplayer/black/throttle";

    if( m_control.isNull() ) {
        QSvgRenderer renderer( THROTTLE_CONTROL );
        QSize scaled = renderer.defaultSize();
        scaled.scale( size(), Qt::KeepAspectRatio );

        QImageReader reader( THROTTLE_CONTROL );
        reader.setScaledSize( scaled );
        m_control = reader.read();
        m_controlpos = QPoint( (width() - m_control.width())/2, (height() - m_control.height())/2 );
    }

    QPainter painter( this );
    painter.setOpacity( m_opacity );

    painter.drawImage( m_controlpos, m_control );
}

qreal ThrottleWidget::calculateIntensity( const QPoint& point )
{
    int center = rect().center().x();

    qreal intensity = (qreal)(point.x() - center) / (qreal)center;

    // Limit intensity between -1..1
    if( intensity < -1.0 ) {
        intensity = -1.0;
    }
    if( intensity > 1.0 ) {
        intensity = 1.0;
    }

    return intensity;
}

class ThrottleControl : public QWidget
{
    Q_OBJECT
public:
    ThrottleControl( QWidget* parent = 0 );

signals:
    void clicked();

    // Intensity changed to either -1, 0 or 1
    void intensityChanged( int intensity );

private slots:
    void processPressed();
    void processReleased();
    void processIntensityChange();
    void processTimeout();

    void activate();
    void deactivate();

    void setOpacity( qreal opacity );

private:
    enum State { Deactivated, PendingActivate, Activated, PendingDeactivate };

    State state() const { return m_state; }
    void setState( State state ) { m_state = state; }

    ThrottleWidget *m_throttle;

    QTimer *m_timer;
    int m_intensity;
    State m_state;
};

ThrottleControl::ThrottleControl( QWidget* parent )
    : QWidget( parent ), m_intensity( 0 ), m_state( Deactivated )
{
    static const int CONTROL_SENSITIVITY = 500; // Sensitivity to user actions in ms

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_throttle = new ThrottleWidget;
    connect( m_throttle, SIGNAL(pressed()),
        this, SLOT(processPressed()) );
    connect( m_throttle, SIGNAL(released()),
        this, SLOT(processReleased()) );
    connect( m_throttle, SIGNAL(intensityChanged(qreal)),
        this, SLOT(processIntensityChange()) );
    m_throttle->setResolution( 0.2 );
    m_throttle->setOpacity( 0.0 );

    layout->addWidget( m_throttle );
    setLayout( layout );

    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()), this, SLOT(processTimeout()) );
    m_timer->setInterval( CONTROL_SENSITIVITY );
    m_timer->setSingleShot( true );
}

void ThrottleControl::processPressed()
{
    switch( state() )
    {
    case Deactivated:
        m_timer->start();
        setState( PendingActivate );
        break;
    case PendingDeactivate:
        m_timer->stop();
        setState( Activated );
        processIntensityChange();
        break;
    default:
        // Ignore
        break;
    }
}

void ThrottleControl::processReleased()
{
    switch( state() )
    {
    case PendingActivate:
        m_timer->stop();
        setState( Deactivated );
        emit clicked();
        break;
    case Activated:
        m_timer->start();
        setState( PendingDeactivate );
        processIntensityChange();
        break;
    default:
        // Ignore
        break;
    }
}

void ThrottleControl::processIntensityChange()
{
    static const qreal NEGATIVE_THRESHOLD = -0.35;
    static const qreal POSITIVE_THRESHOLD = 0.35;

    if( state() == Activated || state() == PendingDeactivate ) {
        int intensity = 0;

        qreal value = m_throttle->intensity();
        if( value <= NEGATIVE_THRESHOLD ) {
            intensity = -1;
        } else if( value >= POSITIVE_THRESHOLD ) {
            intensity = 1;
        }

        if( intensity != m_intensity ) {
            m_intensity = intensity;

            emit intensityChanged( m_intensity );
        }
    }
}

void ThrottleControl::processTimeout()
{
    switch( state() )
    {
    case PendingActivate:
        setState( Activated );
        activate();
        break;
    case PendingDeactivate:
        setState( Deactivated );
        deactivate();
    default:
        // Ignore
        break;
    }
}

void ThrottleControl::activate()
{
    static const int FADEIN_DURATION = 500;

    processIntensityChange();

    // Animate opacity
    QTimeLine *animation = new QTimeLine( FADEIN_DURATION, this );
    connect( animation, SIGNAL(valueChanged(qreal)),
        this, SLOT(setOpacity(qreal)) );
    connect( animation, SIGNAL(finished()),
        animation, SLOT(deleteLater()) );
    animation->start();
}

void ThrottleControl::deactivate()
{
    static const int FADEOUT_DURATION = 500;

    if( m_intensity != 0 ) {
        m_intensity = 0;
        emit intensityChanged( m_intensity );
    }

    // Animate opacity
    QTimeLine *animation = new QTimeLine( FADEOUT_DURATION, this );
    animation->setDirection( QTimeLine::Backward );
    connect( animation, SIGNAL(valueChanged(qreal)),
        this, SLOT(setOpacity(qreal)) );
    connect( animation, SIGNAL(finished()),
        animation, SLOT(deleteLater()) );
    animation->start();
}

void ThrottleControl::setOpacity( qreal opacity )
{
    static const qreal FULL_OPACITY = 0.65;

    m_throttle->setOpacity( opacity * FULL_OPACITY );
}

static const int KEY_LEFT_HOLD = Qt::Key_unknown + Qt::Key_Left;
static const int KEY_RIGHT_HOLD = Qt::Key_unknown + Qt::Key_Right;

class ThrottleKeyMapper : public QObject
{
    Q_OBJECT
public:
    ThrottleKeyMapper( ThrottleControl* control, QObject* parent );

    enum Mapping { LeftRight, UpDown };

    void setMapping( Mapping mapping ) { m_mapping = mapping; }

private slots:
    void processIntensityChange( int intensity );

private:
    Mapping m_mapping;
    int m_lastpressed;
};

ThrottleKeyMapper::ThrottleKeyMapper( ThrottleControl* control, QObject* parent )
    : QObject( parent ), m_mapping( LeftRight )
{
    connect( control, SIGNAL(intensityChanged(int)),
        this, SLOT(processIntensityChange(int)) );
}

void ThrottleKeyMapper::processIntensityChange( int intensity )
{
    switch( intensity )
    {
    case -1:
        {
        if( m_lastpressed ) {
            // Send release event
            processIntensityChange( 0 );
        }
        m_lastpressed = m_mapping == LeftRight ? KEY_LEFT_HOLD : Qt::Key_Down;
        QKeyEvent event = QKeyEvent( QEvent::KeyPress, m_lastpressed, Qt::NoModifier );
        QCoreApplication::sendEvent( parent(), &event );
        }
        break;
    case 0:
        if( m_lastpressed ) {
            QKeyEvent event = QKeyEvent( QEvent::KeyRelease, m_lastpressed, Qt::NoModifier );
            QCoreApplication::sendEvent( parent(), &event );
            m_lastpressed = 0;
        }
        break;
    case 1:
        {
        if( m_lastpressed ) {
            // Send release event
            processIntensityChange( 0 );
        }
        m_lastpressed = m_mapping == LeftRight ? KEY_RIGHT_HOLD : Qt::Key_Up;
        QKeyEvent event = QKeyEvent( QEvent::KeyPress, m_lastpressed, Qt::NoModifier );
        QCoreApplication::sendEvent( parent(), &event );
        }
        break;
    }
}

class PileLayout : public QLayout
{
public:
    PileLayout( QWidget* parent = 0 )
        : QLayout( parent )
    { }
    ~PileLayout();

    void addLayout( QLayout* layout );
    int count() const { return m_pile.count(); }
    void addItem( QLayoutItem* item );
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutItem *itemAt( int index ) const;
    QLayoutItem *takeAt( int index );
    void setGeometry( const QRect& rect );

private:
    QList<QLayoutItem*> m_pile;
};

PileLayout::~PileLayout()
{
    foreach( QLayoutItem* item, m_pile ) {
        delete item;
    }
}

void PileLayout::addLayout( QLayout* layout )
{
    QWidget* widget = new QWidget;
    widget->setLayout( layout );
    addWidget( widget );
}

void PileLayout::addItem( QLayoutItem* item )
{
    m_pile.append( item );
}

QSize PileLayout::sizeHint() const
{
    QSize hint( 0, 0 );

    foreach( QLayoutItem* item, m_pile ) {
        hint = hint.expandedTo( item->sizeHint() );
    }

    return hint;
}

QSize PileLayout::minimumSize() const
{
    QSize min( 0, 0 );

    foreach( QLayoutItem* item, m_pile ) {
        min = min.expandedTo( item->minimumSize() );
    }

    return min;
}


QLayoutItem* PileLayout::itemAt( int index ) const
{
    return m_pile.value( index );
}

QLayoutItem* PileLayout::takeAt( int index )
{
    if( index >= 0 && index < m_pile.count() ) {
        return m_pile.takeAt( index );
    }

    return 0;
}

void PileLayout::setGeometry( const QRect& rect )
{
    QLayout::setGeometry( rect );

    foreach( QLayoutItem* item, m_pile ) {
        item->setGeometry( rect );
    }
}

static const int KEY_SELECT_HOLD = Qt::Key_unknown + Qt::Key_Select;

PlayerWidget::PlayerWidget( PlayerControl* control, QWidget* parent )
    : QWidget( parent ), m_playercontrol( control ), m_content( 0 ), m_mediacontrol( 0 ), m_videowidget( 0 ), m_playlist( 0 )
{
    static const int HOLD_THRESHOLD = 500;
    static const int STRETCH_MAX = 1;

    QVBoxLayout *background = new QVBoxLayout;
    background->setMargin( 0 );

    m_visualization = new VisualizationWidget;
    background->addWidget( m_visualization );
    setLayout( background );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 6 );

    m_statewidget = new StateWidget( control );

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget( m_statewidget );

    m_muteicon = new QMediaVolumeLabel( QMediaVolumeLabel::MuteVolume );
    hbox->addWidget( m_muteicon );

    m_trackinfo = new TrackInfoWidget;
    hbox->addWidget( m_trackinfo, STRETCH_MAX );

    layout->addLayout( hbox );
    layout->addStretch();

    m_videolayout  = new QVBoxLayout;

#ifndef NO_HELIX_LOGO
    m_helixlogoaudio = new HelixLogo;
    m_videolayout->addWidget( m_helixlogoaudio );
#endif

    m_videolayout->setMargin( 0 );

    layout->addLayout( m_videolayout, STRETCH_MAX );

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin( 0 );
    vbox->addStretch();

    m_progressview = new ProgressView;
    vbox->addWidget( m_progressview );

    m_volumeview = new VolumeView;
    vbox->addWidget( m_volumeview );

    m_seekview = new SeekView;
    vbox->addWidget( m_seekview );

    PileLayout *pile = new PileLayout;
    pile->addLayout( vbox );

    ThrottleControl *throttle = new ThrottleControl;
    connect( throttle, SIGNAL(clicked()), this, SLOT(cycleView()) );
    pile->addWidget( throttle );

    m_mapper = new ThrottleKeyMapper( throttle, this );

    hbox = new QHBoxLayout;
    hbox->setMargin( 0 );
    hbox->addLayout( pile );

#ifndef NO_HELIX_LOGO
    m_helixlogovideo = new HelixLogo;
    hbox->addWidget( m_helixlogovideo );
#endif

    layout->addLayout( hbox );

    m_visualization->setLayout( layout );

    m_settingsdialog = new SettingsDialog( this );

    m_trackinfodialog = new TrackInfoDialog( this );

    KeyFilter *filter = new KeyFilter( m_trackinfodialog, this, this );
    filter->addKey( Qt::Key_Left );
    filter->addKey( Qt::Key_Right );

    // Construct soft menu bar
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *menu = QSoftMenuBar::menuFor( this );

    m_muteaction = new QAction( QIcon( ":icon/mute" ), tr( "Mute" ), this );
    connect( m_muteaction, SIGNAL(triggered()), this, SLOT(toggleMute()) );
    menu->addAction( m_muteaction );

    QAction *trackdetails = new QAction( QIcon( ":icon/info" ), tr( "Track Details..." ), this );
    connect( trackdetails, SIGNAL(triggered()), this, SLOT(execTrackInfoDialog()) );
    menu->addAction( trackdetails );

    menu->addSeparator();

    QAction *settings = new QAction( QIcon( ":icon/settings" ), tr( "Settings..." ), this );
    connect( settings, SIGNAL(triggered()), this, SLOT(execSettings()) );
    menu->addAction( settings );
#endif

    // Initialize view
    setView( Progress );

    m_muteaction->setText( tr( "Mute On" ) );

    m_ismute = false;
    m_muteicon->hide();

#ifndef NO_HELIX_LOGO
    m_helixlogovideo->hide();
#endif

    new KeyHold( Qt::Key_Left, KEY_LEFT_HOLD, HOLD_THRESHOLD, this, this );
    new KeyHold( Qt::Key_Right, KEY_RIGHT_HOLD, HOLD_THRESHOLD, this, this );

    // Activity monitor
    m_monitor = new ActivityMonitor( 4000, this );
    connect( m_monitor, SIGNAL(inactive()), this, SLOT(showProgress()) );

    m_pingtimer = new QTimer( this );
    m_pingtimer->setInterval( 1000 );
    connect( m_pingtimer, SIGNAL(timeout()), this, SLOT(pingMonitor()) );

    m_context = new QMediaContentContext( this );
    m_context->addObject( m_progressview );
    m_context->addObject( m_volumeview );
    m_context->addObject( m_seekview );

    QMediaControlNotifier *notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );
    m_context->addObject( notifier );

    notifier = new QMediaControlNotifier( QMediaVideoControl::name(), this );
    connect( notifier, SIGNAL(valid()), this, SLOT(activateVideo()) );
    connect( notifier, SIGNAL(invalid()), this, SLOT(deactivateVideo()) );
    m_context->addObject( notifier );

    setFocusProxy( m_statewidget );

    // Filter application key events for media keys
    qApp->installEventFilter( this );

    // Context sensitive help hint
    setObjectName( "playback" );
}

PlayerWidget::~PlayerWidget()
{
    if( m_videowidget ) {
        delete m_videowidget;
    }
}

void PlayerWidget::setPlaylist( Playlist* playlist )
{
    // Disconnect from old playlist
    if( m_playlist ) {
        m_playlist->disconnect( this );
    }

    m_playlist = playlist;

    // Connect to new playlist
    connect( m_playlist, SIGNAL(playingChanged(const QModelIndex&)),
        this, SLOT(playingChanged(const QModelIndex&)) );

    if( m_playlist ) {
        setCurrentTrack( m_playlist->playing() );
        if( !m_currenttrack.isValid() ) {
            setCurrentTrack( m_playlist->index( 0 ) );
        }

        if( m_currenttrack.isValid() ) {
            openCurrentTrack();
        }
    } else {
        qLog(Media) << "PlayerWidget::setPlaylist playlist is null";
    }

    m_progressview->setPlaylist( m_playlist );
    m_trackinfo->setPlaylist( m_playlist );
    m_trackinfodialog->setPlaylist( m_playlist );
}

bool PlayerWidget::eventFilter( QObject* o, QEvent* e )
{
    // Guard against recursion
    static QEvent* d = 0;

    if( o != this && d != e && (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) ) {
        QKeyEvent *ke = (QKeyEvent*)e;

        if( ke->isAutoRepeat() ) { return false; }

        switch( ke->key() )
        {
        case Qt::Key_VolumeUp:
            {
            QKeyEvent event = QKeyEvent( e->type(), Qt::Key_VolumeUp, Qt::NoModifier );
            QCoreApplication::sendEvent( this, d = &event );
            d = 0;
            }
            return true;
        case Qt::Key_VolumeDown:
            {
            QKeyEvent event = QKeyEvent( e->type(), Qt::Key_VolumeDown, Qt::NoModifier );
            QCoreApplication::sendEvent( this, d = &event );
            d = 0;
            }
            return true;
        default:
            // Ignore
            break;
        }
    }

    return false;
}

void PlayerWidget::setMediaContent( QMediaContent* content )
{
    if( m_content ) {
        m_content->disconnect( this );
    }

    m_content = content;
    connect( content, SIGNAL(mediaError(const QString&)),
        this, SLOT(displayErrorMessage(const QString&)) );

    m_context->setMediaContent( content );
}

void PlayerWidget::activate()
{
    m_mediacontrol = new QMediaControl( m_content->handle() );
    connect( m_mediacontrol, SIGNAL(volumeMuted(bool)),
        this, SLOT(setMuteDisplay(bool)) );
    connect( m_mediacontrol, SIGNAL(playerStateChanged(QtopiaMedia::State)),
        this, SLOT(changeState(QtopiaMedia::State)) );
}

void PlayerWidget::deactivate()
{
    delete m_mediacontrol;
    m_mediacontrol = 0;
}

void PlayerWidget::activateVideo()
{
    QMediaVideoControl control( m_content->handle() );
    setVideo( control.createVideoWidget( this ) );
}

void PlayerWidget::deactivateVideo()
{
    removeVideo();
}

void PlayerWidget::displayErrorMessage( const QString& message )
{
    QMessageBox::warning( this, tr( "Media Player Error" ),
        QString( "<qt>%1</qt>" ).arg( message ) );
}

void PlayerWidget::changeState( QtopiaMedia::State state )
{
    switch( state )
    {
    case QtopiaMedia::Stopped:
        if( m_playercontrol->state() == PlayerControl::Playing ) {
            continuePlaying();
        }
        break;
    case QtopiaMedia::Error:
        m_playercontrol->setState( PlayerControl::Stopped );
        displayErrorMessage( m_mediacontrol->errorString() );
        break;
    default:
        // Ignore
        break;
    }
}

void PlayerWidget::setMuteDisplay( bool mute )
{
    if( m_ismute == mute ) {
        return;
    }

    m_ismute = mute;

    if( m_ismute ) {
        m_muteaction->setText( tr( "Mute Off" ) );
        m_muteicon->show();
    } else {
        m_muteaction->setText( tr( "Mute On" ) );
        m_muteicon->hide();
    }
}

void PlayerWidget::playingChanged( const QModelIndex& index )
{
    if( index.isValid() ) {
        setCurrentTrack( index );
        openCurrentTrack();
    } else {
        m_playercontrol->setState( PlayerControl::Stopped );
    }
}

void PlayerWidget::pingMonitor()
{
    m_monitor->update();
}

void PlayerWidget::showProgress()
{
    setView( Progress );
}

void PlayerWidget::cycleView()
{
    switch( view() )
    {
    case Progress:
        setView( Volume );
        m_monitor->update();
        break;
    case Seek:
        setView( Volume );
        m_monitor->update();
        break;
    case Volume:
        setView( Progress );
        break;
    }
}

void PlayerWidget::continuePlaying()
{
    // Skip forward one playlist item
    QModelIndex index = m_currenttrack.sibling( m_currenttrack.row() + 1, m_currenttrack.column() );
    if( index.isValid() ) {
        m_playlist->setPlaying( index );
    } else {
        m_playlist->setPlaying( QModelIndex() );
    }
}

void PlayerWidget::toggleMute()
{
    if( m_mediacontrol ) {
        if( m_mediacontrol->isMuted() ) {
            m_mediacontrol->setMuted( false );
        } else {
            m_mediacontrol->setMuted( true );
        }
    }
}

void PlayerWidget::execSettings()
{
    QtopiaApplication::execDialog( m_settingsdialog );
}

void PlayerWidget::keyPressEvent( QKeyEvent* e )
{
    if( e->isAutoRepeat() || !m_mediacontrol ) { e->ignore(); return; }

    switch( e->key() )
    {
    case Qt::Key_Up:
    case Qt::Key_VolumeUp:
        {
        setView( Volume );

        QKeyEvent event = QKeyEvent( QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier );
        QCoreApplication::sendEvent( m_volumeview->keyEventHandler(), &event );

        m_pingtimer->start();
        m_monitor->update();
        }
        break;
    case Qt::Key_Down:
    case Qt::Key_VolumeDown:
        {
        setView( Volume );

        QKeyEvent event = QKeyEvent( QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier );
        QCoreApplication::sendEvent( m_volumeview->keyEventHandler(), &event );

        m_pingtimer->start();
        m_monitor->update();
        }
        break;
    case Qt::Key_Left:
        {
        // Skip backward one playlist item
        QModelIndex index = m_currenttrack.sibling( m_currenttrack.row() - 1, m_currenttrack.column() );
        if( index.isValid() ) {
            m_playlist->setPlaying( index );
        }
        }
        break;
    case Qt::Key_Right:
        {
        // Skip forward one playlist item
        QModelIndex index = m_currenttrack.sibling( m_currenttrack.row() + 1, m_currenttrack.column() );
        if( index.isValid() ) {
            m_playlist->setPlaying( index );
        }
        }
        break;
    case KEY_LEFT_HOLD:
        if( m_playercontrol->state() != PlayerControl::Stopped ) {
            setView( Seek);

            QKeyEvent event = QKeyEvent( QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier );
            QCoreApplication::sendEvent( m_seekview->keyEventHandler(), &event );

            m_pingtimer->start();
            m_monitor->update();
        }
        break;
    case KEY_RIGHT_HOLD:
        if( m_playercontrol->state() != PlayerControl::Stopped ) {
            setView( Seek );

            QKeyEvent event = QKeyEvent( QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier );
            QCoreApplication::sendEvent( m_seekview->keyEventHandler(), &event );

            m_pingtimer->start();
            m_monitor->update();
        }
        break;
    default:
        // Ignore
        e->ignore();
        break;
    }
}

void PlayerWidget::keyReleaseEvent( QKeyEvent* e )
{
    if( e->isAutoRepeat() || !m_mediacontrol ) { e->ignore(); return; }

    switch( e->key() )
    {
    case Qt::Key_Up:
    case Qt::Key_VolumeUp:
        {
        QKeyEvent event = QKeyEvent( QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier );
        QCoreApplication::sendEvent( m_volumeview->keyEventHandler(), &event );

        m_pingtimer->stop();
        }
        break;
    case Qt::Key_Down:
    case Qt::Key_VolumeDown:
        {
        QKeyEvent event = QKeyEvent( QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier );
        QCoreApplication::sendEvent( m_volumeview->keyEventHandler(), &event );

        m_pingtimer->stop();
        }
        break;
    case KEY_LEFT_HOLD:
        if( m_playercontrol->state() != PlayerControl::Stopped ) {
        QKeyEvent event = QKeyEvent( QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier );
        QCoreApplication::sendEvent( m_seekview->keyEventHandler(), &event );

        m_pingtimer->stop();
        }
        break;
    case KEY_RIGHT_HOLD:
        if( m_playercontrol->state() != PlayerControl::Stopped ) {
        QKeyEvent event = QKeyEvent( QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier );
        QCoreApplication::sendEvent( m_seekview->keyEventHandler(), &event );

        m_pingtimer->stop();
        }
        break;
    default:
        // Ignore
        e->ignore();
    }
}

void PlayerWidget::showEvent( QShowEvent* )
{
    if( m_videowidget ) {
        QtopiaApplication::setPowerConstraint( QtopiaApplication::Disable );
    }

#ifndef NO_VISUALIZATION
    else {
        m_visualization->setActive( true );
    }
#endif
}

void PlayerWidget::hideEvent( QHideEvent* )
{
    if( m_videowidget ) {
        QtopiaApplication::setPowerConstraint( QtopiaApplication::Enable );
    }

#ifndef NO_VISUALIZATION
    m_visualization->setActive( false );
#endif
}

void PlayerWidget::setView( View view )
{
    m_currentview = view;

    switch( m_currentview )
    {
    case Progress:
        m_volumeview->hide();
        m_seekview->hide();

        m_progressview->show();
        m_mapper->setMapping( ThrottleKeyMapper::LeftRight );
        break;
    case Volume:
        m_progressview->hide();
        m_seekview->hide();

        m_volumeview->show();
        m_mapper->setMapping( ThrottleKeyMapper::UpDown );
        break;
    case Seek:
        m_progressview->hide();
        m_volumeview->hide();

        m_seekview->show();
        m_mapper->setMapping( ThrottleKeyMapper::LeftRight );
        break;
    }
}

void PlayerWidget::setVideo( QWidget* widget )
{
    m_videowidget = widget;
    m_videowidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_videolayout->addWidget( m_videowidget );

#ifndef NO_VISUALIZATION
    m_visualization->setActive( false );
#endif

#ifndef NO_HELIX_LOGO
    m_helixlogoaudio->hide();
    m_helixlogovideo->show();
#endif

    QtopiaApplication::setPowerConstraint( QtopiaApplication::Disable );
}

void PlayerWidget::removeVideo()
{
    delete m_videowidget;
    m_videowidget = 0;

#ifndef NO_VISUALIZATION
    m_visualization->setActive( true );
#endif

#ifndef NO_HELIX_LOGO
    m_helixlogoaudio->show();
    m_helixlogovideo->hide();
#endif

    QtopiaApplication::setPowerConstraint( QtopiaApplication::Enable );
}

void PlayerWidget::setCurrentTrack( const QModelIndex& index )
{
    m_currenttrack = index;
}

void PlayerWidget::openCurrentTrack()
{
    // Open and play current playlist item
    QString url = qvariant_cast<QString>(m_playlist->data( m_currenttrack, Playlist::Url ));

    if( url.contains( "://" ) ) {
        m_playercontrol->open( url );
    } else {
        m_playercontrol->open( QContent( url ) );
    }
}

void PlayerWidget::execTrackInfoDialog()
{
    QtopiaApplication::execDialog( m_trackinfodialog, false );
}

#include "playerwidget.moc"
