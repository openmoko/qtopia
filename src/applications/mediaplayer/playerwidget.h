/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include "playercontrol.h"
#include "statewidget.h"

#include <media.h>
#include <qmediacontent.h>
#include <qmediacontrol.h>
#include <qmediatools.h>
#include <private/activitymonitor_p.h>

#include <QtGui>

class Playlist;

class VisualizationWidget;

class PlayerControl;

class ProgressView;
class VolumeView;
class SeekView;

class ThrottleKeyMapper;

class TrackInfoWidget;
class TrackInfoDialog;

class VoteDialog;

class RepeatState;
class RepeatDialog;

class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    PlayerWidget( PlayerControl* control, QWidget* parent = 0 );
    ~PlayerWidget();

   Playlist* playlist() const { return m_playlist; }
    // Set playlist and begin playback
    void setPlaylist( Playlist* playlist );

    bool eventFilter( QObject* o, QEvent* e );

#ifdef QTOPIA_KEYPAD_NAVIGATION
#ifndef NO_HELIX
    QAction *settingsAction(){ return m_settingsaction; }
#endif
#endif

signals:
    void contentChanged( QMediaContent* content );

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void activate();
    void deactivate();

    void activateVideo();
    void deactivateVideo();

    void displayErrorMessage( const QString& message );

    void changeState( QtopiaMedia::State state );

    void setMuteDisplay( bool mute );

    void playingChanged( const QModelIndex& index );

    void pingMonitor();

    void showProgress();

    void cycleView();

    void continuePlaying();

    void toggleMute();

    void execSettings();

    void execTrackInfoDialog();

    void execVoteDialog();

    void execRepeatDialog();

protected:
    void keyPressEvent( QKeyEvent* e );
    void keyReleaseEvent( QKeyEvent* e );

    void showEvent( QShowEvent* e );
    void hideEvent( QHideEvent* e );

private:
    enum View { Progress, Volume, Seek };

    View view() const { return m_currentview; }
    void setView( View view );

    void setVideo( QWidget* widget );
    void removeVideo();

    void setCurrentTrack( const QModelIndex& index );

    void openCurrentTrack();
    void playCurrentTrack();

    PlayerControl *m_playercontrol;
    StateWidget *m_statewidget;

    QMediaContent *m_content;
    QMediaControl *m_mediacontrol;
    QMediaContentContext *m_context;

    QDialog *m_settingsdialog;

#ifdef QTOPIA_KEYPAD_NAVIGATION
#ifndef NO_HELIX
    QAction *m_settingsaction;
#endif
#endif

    QVBoxLayout *m_videolayout;
    QWidget *m_videowidget;
    VisualizationWidget *m_visualization;

#ifndef NO_HELIX
    QWidget *m_helixlogoaudio;
    QWidget *m_helixlogovideo;
#endif

    ProgressView *m_progressview;
    VolumeView *m_volumeview;
    SeekView *m_seekview;

    View m_currentview;

    ActivityMonitor *m_monitor;
    QTimer *m_pingtimer;

    ThrottleKeyMapper *m_mapper;

    Playlist *m_playlist;
    QPersistentModelIndex m_currenttrack;

    bool m_continue;

    bool m_ismute;
    QAction *m_muteaction;
    QWidget *m_muteicon;

    TrackInfoWidget *m_trackinfo;
    TrackInfoDialog *m_trackinfodialog;

    QAction *m_voteaction;
    VoteDialog *m_votedialog;

    QAction *m_repeataction;
    RepeatState *m_repeatstate;
    RepeatDialog *m_repeatdialog;
};

#endif // PLAYERWIDGET_H
