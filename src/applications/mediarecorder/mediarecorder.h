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

#ifndef MEDIARECORDER_H
#define MEDIARECORDER_H

#include <qmainwindow.h>
#include <qlist.h>
#include <qmap.h>
#include <qstackedwidget.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qsound.h>
#include <qtopiaabstractservice.h>

#include "ui_mediarecorderbase.h"

class QAction;
class MediaRecorderPluginList;
class SampleBuffer;
class ConfigureRecorder;
class Waveform;
class MediaRecorderEncoder;
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
    void startPlaying();
    void stopPlaying();
    void clearData();
    void processAudioData();
    void configure();
    void noPluginError();
    void documentSelected(const QContent&);
    void newSelected();
    void currentDocumentChanged();

protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );

public slots:
    void toggleRecording();
    void recordAudio( const QDSActionRequest& request );
    void recordClicked();

private:
    QDocumentSelector *selector;
    QWidget *contentsWidget;
    Ui::MediaRecorderBase *contents;
    ConfigureRecorder *config;
    QAction *configureAction;
    QStackedWidget *stack;
    MediaRecorderPluginList *recorderPlugins;
    QAudioInput *m_audioInput;
    bool audioDeviceIsReady;
#ifdef RECORD_THEN_SAVE
    SampleBuffer *samples;
#endif
    short *sampleBuffer;
    QIODevice *io;
    MediaRecorderEncoder *encoder;
    QSound *m_sound;
    QualitySetting qualities[MaxQualities];
    long recordTime;
    long maxRecordTime;
    bool recording;
    bool playing;
    QString lastSaved;
    QString recordingsCategory;
    QDSActionRequest* mRecordAudioRequest;
    QualitySetting recordQuality;

    enum ContextKey { Select, Record, Stop, Play };
    void setContextKey( ContextKey key );

    void switchToFileSelector();
    void switchToOther();
    void switchToRecorder();
    void switchToPlayback();

    void stopRecordingNoSwitch();
    void stopPlayingNoSwitch();
    void stopEverythingNoSwitch();

    QWidget* getContentsWidget();

    int     m_position;
    bool    m_mousePref;
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
    VoiceRecordingService(MediaRecorder *parent)
        : QtopiaAbstractService("VoiceRecording", parent)
    {
        this->parent = parent;
        publishAll();
    }

    MediaRecorder *parent;
};

#endif
