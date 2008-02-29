/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef MEDIARECORDER_H
#define MEDIARECORDER_H

#include <qmainwindow.h>
#include <qlist.h>
#include <qmap.h>
#include <qvbox.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>

class QToolBar;
class QAction;
class MediaRecorderPluginList;
class MediaPlayerPluginList;
class AudioInput;
class AudioDevice;
class SampleBuffer;
class MediaRecorderBase;
class ConfigureRecorder;
class Waveform;
class MediaRecorderEncoder;
class MediaPlayerDecoder;
class QCopChannel;


// Define this to record to memory before saving to disk.
//#define RECORD_THEN_SAVE


struct QualitySetting
{
    int		frequency;
    int		channels;
    QString	mimeType;
    QString	formatTag;
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
    MediaRecorder( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~MediaRecorder();

private:
    void initializeContents();
    void setQualityDisplay( const QualitySetting& quality );
    void recomputeMaxTime();

private slots:
    void qualityChanged( int id );
    void newLocation();
    void startSave();
    void endSave();
    void startRecording();
    void stopRecording();
    void recordClicked();
    void startPlaying();
    void stopPlaying();
    void replayClicked();
    void clearData();
    void processAudioData();
    void configure();
    void noPluginError();
    void setRecordLight( bool enable );
    void recordLightBlink();
    void audioOutputDone();
    void traySocket( const QCString&, const QByteArray& );

protected:
    void closeEvent( QCloseEvent *e );

private:
    MediaRecorderBase *contents;
    ConfigureRecorder *config;
    QToolBar *menu;
    QAction *configureAction;
    QWidgetStack *stack;
    MediaRecorderPluginList *recorderPlugins;
    MediaPlayerPluginList *playerPlugins;
    AudioInput *audioInput;
    AudioDevice *audioOutput;
#ifdef RECORD_THEN_SAVE
    SampleBuffer *samples;
#endif
    short *sampleBuffer;
    QIODevice *io;
    MediaRecorderEncoder *encoder;
    MediaPlayerDecoder *decoder;
    QualitySetting qualities[MaxQualities];
    long recordTime;
    long maxRecordTime;
    bool recording;
    bool playing;
    QString lastSaved;
    QTimer *lightTimer;
    bool recordLightState;
    QCopChannel *trayChannel;

};

#endif
