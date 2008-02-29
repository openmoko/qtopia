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

#ifndef MEDIARECORDER_H
#define MEDIARECORDER_H

#include <qmainwindow.h>
#include <qlist.h>
#include <qmap.h>
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qsound.h>
#include <qtopiaabstractservice.h>

#include "ui_mediarecorderbase.h"

class QToolBar;
class QAction;
class MediaRecorderPluginList;
#ifdef QTOPIA4_TODO
class MediaPlayerPluginList;
#endif
#ifdef QTOPIA4_TODO
class AudioDevice;
#endif
class SampleBuffer;
class ConfigureRecorder;
class Waveform;
class MediaRecorderEncoder;
#ifdef QTOPIA4_TODO
class MediaPlayerDecoder;
#endif
class QtopiaChannel;
class QDocumentSelector;
class QContent;
class QAudioInput;
class QDSActionRequest;

// Define this to record to memory before saving to disk.
//#define RECORD_THEN_SAVE


struct QualitySetting
{
    int         frequency;
    int         channels;
    QString     mimeType;
    QString     formatTag;
};


const int VoiceQuality = 0;
const int MusicQuality = 1;
const int CDQuality = 2;
const int CustomQuality = 3;
const int MaxQualities = 4;


class MediaRecorder : public QMainWindow
{
    Q_OBJECT

public:
    MediaRecorder( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~MediaRecorder();

private:
    void initializeContents();
    void setQualityDisplay( const QualitySetting& quality );
    void recomputeMaxTime();

    bool startSave();

private slots:
    void qualityChanged( int id );
    void newLocation();
    void endSave();
    void startRecording();
    void stopRecording();
    void recordClicked();
    void startPlaying();
    void stopPlaying();
    void replayClicked();
    void deleteClicked();
    void clearData();
    void processAudioData();
    void configure();
    void noPluginError();
    void setRecordLight( bool enable );
    void recordLightBlink();
    void audioOutputDone();
    void audioDeviceReady();
    void audioDeviceError();
    void traySocket( const QString&, const QByteArray& );
    void documentSelected(const QContent&);
    void newSelected();

protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );

public slots:
    void toggleRecording();
    void recordAudio( const QDSActionRequest& request );

private:
    QDocumentSelector *selector;
    QWidget *contentsWidget;
    Ui::MediaRecorderBase *contents;
    ConfigureRecorder *config;
    QToolBar *menu;
    QAction *configureAction;
    QStackedWidget *stack;
    MediaRecorderPluginList *recorderPlugins;
#ifdef QTOPIA4_TODO
    MediaPlayerPluginList *playerPlugins;
#endif
    QAudioInput *m_audioInput;
#ifdef QTOPIA4_TODO
    AudioDevice *audioOutput;
#endif
    bool audioDeviceIsReady;
    bool startWhenAudioDeviceReady;
#ifdef RECORD_THEN_SAVE
    SampleBuffer *samples;
#endif
    short *sampleBuffer;
    QIODevice *io;
    MediaRecorderEncoder *encoder;
#ifdef QTOPIA4_TODO
    MediaPlayerDecoder *decoder;
#endif
    QSound *m_sound;
    QualitySetting qualities[MaxQualities];
    long recordTime;
    long maxRecordTime;
    long samplesPlayed;
    bool recording;
    bool playing;
    QString lastSaved;
    QTimer *lightTimer;
    bool recordLightState;
    QtopiaChannel *trayChannel;
    QString recordingsCategory;
    QDSActionRequest* mRecordAudioRequest;
    QualitySetting recordQuality;
    bool smallScreen;

    void setContextKey( bool record );
    void setReplayEnabled( bool flag );
    void setDeleteEnabled( bool flag );

    void switchToFileSelector();
    void switchToOther();
    void switchToRecorder();
    void switchToPlayback();

    void stopRecordingNoSwitch();
    void stopPlayingNoSwitch();
    void stopEverythingNoSwitch();

    QWidget* getContentsWidget();

    int     m_position;
};

class VoiceRecordingService : public QtopiaAbstractService
{
    Q_OBJECT

    friend class MediaRecorder;

public:

    ~VoiceRecordingService();

public slots:

    void toggleRecording();
    void recordAudio( const QDSActionRequest& request );

private:

    VoiceRecordingService(MediaRecorder *parent):
            QtopiaAbstractService("VoiceRecording", parent)
        {
        this->parent = parent;

        publishAll();
    }

    MediaRecorder *parent;
};

#endif
