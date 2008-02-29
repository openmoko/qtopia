/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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
class FileSelector;
class DocLnk;


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
    void traySocket( const QCString&, const QByteArray& );
    void fileSelected( const DocLnk& );
    void newSelected( const DocLnk& );
    void appMessage( const QCString&, const QByteArray& );

protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );

private:
    FileSelector *selector;
    MediaRecorderBase *contents;
    ConfigureRecorder *config;
    QToolBar *menu;
    QAction *configureAction;
    QWidgetStack *stack;
    MediaRecorderPluginList *recorderPlugins;
    MediaPlayerPluginList *playerPlugins;
    AudioInput *audioInput;
    AudioDevice *audioOutput;
    bool audioDeviceIsReady;
    bool startWhenAudioDeviceReady;
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
    long samplesPlayed;
    bool recording;
    bool playing;
    QString lastSaved;
    QString lastSavedLink;
    QTimer *lightTimer;
    bool recordLightState;
    QCopChannel *trayChannel;
    int recordingsCategory;
    bool requestMode;
    QualitySetting recordQuality;
    QCString responseChannel;
    QString responseId;
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

};

#endif
