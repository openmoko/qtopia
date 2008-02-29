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

#include "mediarecorder.h"
#include "audioparameters.h"

#include "samplebuffer.h"
#include "pluginlist.h"
#include "timeprogressbar.h"
#include "confrecorder.h"
#include "waveform.h"


#include <qdocumentselector.h>
#include <qcontent.h>
#include <qsettings.h>
#include <qstorage.h>
#include <qstoragedeviceselector.h>
#include <qtopiaapplication.h>
#include <qmimetype.h>
#include <qdocumentselector.h>
#include <qcategorymanager.h>
#include <qaudioinput.h>
#include <qtopiaipcenvelope.h>

#include <qaction.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qevent.h>

#include <QDSData>
#include <QDSActionRequest>
#include <QDSServiceInfo>

#ifdef QTOPIA_KEYPAD_NAVIGATION
# include <qtopia/qsoftmenubar.h>
#endif

#include <stdlib.h>


#define MR_BUFSIZE  1024


MediaRecorder::MediaRecorder(QWidget *parent, Qt::WFlags f):
    QMainWindow( parent, f ),
    contentsWidget( NULL ),
    config( 0 ),
    recorderPlugins( NULL ),
    m_audioInput( new QAudioInput ),
    audioDeviceIsReady( false ),
    startWhenAudioDeviceReady( false ),
    m_sound( NULL ),
    recordingsCategory( "Recordings" ),
    mRecordAudioRequest( 0 ),
    m_position( 0 )
{
    // We remove some of the UI if the screen is too small to hold it.
    smallScreen = height() < 120;

    // Adjust window decorations
    setWindowTitle(tr("Voice Notes"));
    setWindowIcon(QIcon( ":image/SoundPlayer"));

    // We don't need an input method with this application.
    QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);

    // Make sure that the "Recordings" category is registered.
    QCategoryManager catman("Documents");
    // For new code a more unique id should be used instead of using the untranslated text
    // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
    catman.ensureSystemCategory(recordingsCategory, recordingsCategory);

    // Create stack widget
    stack = new QStackedWidget(this);
    stack->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setCentralWidget(stack);

    // Add the Document selector
    selector = new QDocumentSelector( stack );
    selector->enableOptions( QDocumentSelector::NewDocument );
    selector->setSortMode(QDocumentSelector::ReverseChronological);
    selector->setFilter(
            QContentFilter( QContent::Document ) &
            QContentFilter( QContentFilter::MimeType, "audio/*" ) &
            QContentFilter( QContentFilter::Category, recordingsCategory ) );
    selector->setFocus( Qt::OtherFocusReason );
    stack->addWidget(selector);

    connect(selector,
            SIGNAL(documentSelected(const QContent&)),
                this,
            SLOT(documentSelected(const QContent&)));

    connect(selector,
            SIGNAL(newSelected()),
                this,
            SLOT(newSelected()));

    // Listen for "VoiceRecording" service messages.
    new VoiceRecordingService(this);

    // extra config
    sampleBuffer = 0;
    recordTime = 0;
    recording = false;
    playing = false;
    io = 0;
    recordLightState = false;

#ifdef RECORD_THEN_SAVE
    samples = 0;
#endif

#ifndef QTOPIA_PHONE
    // Make a timer to flash the light
    lightTimer = new QTimer(this);
    connect(lightTimer,
            SIGNAL(timeout()),
                this,
            SLOT(recordLightBlink()));
#endif

    // Listen on the system tray for clicks on the record light so that
    // we can raise the application when the user clicks on it.
    trayChannel = new QtopiaChannel("Qt/Tray", this);
    connect(trayChannel,
            SIGNAL(received(const QString&,const QByteArray&)),
                this,
            SLOT(traySocket(const QString&,const QByteArray&)));
}


MediaRecorder::~MediaRecorder()
{
    delete m_audioInput;
    delete recorderPlugins;

#ifdef RECORD_THEN_SAVE
    if ( samples )
        delete samples;
#endif

    if (sampleBuffer)
        delete[] sampleBuffer;

    if (io )
        delete io;

#ifndef QTOPIA_PHONE
    delete lightTimer;
#endif

    delete m_sound;

    delete mRecordAudioRequest;
}


void MediaRecorder::initializeContents()
{
    // The progress bar initially has no time display.  This will be fixed
    // up when we start recording.
    contents->progress->setMaximum( 10 );
    contents->progress->setValue( -1 );

    // Cannot replay yet, because there is no recorded sound.
    setReplayEnabled(false);
    setDeleteEnabled(false);

    if (recorderPlugins == NULL)
        recorderPlugins = new MediaRecorderPluginList();

    // Load the initial quality settings.
    if ( recorderPlugins == 0 )
        recorderPlugins = new MediaRecorderPluginList();

    config = new ConfigureRecorder(qualities, recorderPlugins, this);
    contents->qualityCombo->setCurrentIndex(config->currentQuality());
    setQualityDisplay(qualities[config->currentQuality()]);
    connect( contents->qualityCombo, SIGNAL( activated(int) ),
             this, SLOT( qualityChanged(int) ) );

    connect( contents->storageLocation, SIGNAL(newPath()),
             this, SLOT(newLocation()) );
    recomputeMaxTime();

#ifdef QTOPIA_KEYPAD_NAVIGATION

    // Create a menu with "Help" on the dialog.
    QSoftMenuBar::menuFor( config );

    // Disable the settings boxes in phone mode.
    contents->GroupBox1->hide();
    contents->GroupBox2->hide();

#else
    if ( smallScreen )
        contents->GroupBox1->hide();
#endif
}


void MediaRecorder::setQualityDisplay( const QualitySetting& quality )
{
    QString str;
    QString format;
    int index;

    // Map the MIME type to a format name.  Do the best we can with
    // the MIME type and tag if that is all we have.
    index = recorderPlugins->indexFromType
                ( quality.mimeType, quality.formatTag );
    if( index >= 0 ) {
        format = recorderPlugins->formatNameAt( (uint)index );
    } else if( quality.mimeType.startsWith( "audio/" ) ) {
        format = quality.mimeType.mid(6) + " [" + quality.formatTag + "]";
    } else {
        format = quality.mimeType + " [" + quality.formatTag + "]";
    }

    // Format the details and display them.
    int khz = (quality.frequency / 1000);
    if ( quality.channels == 1 ) {
        str = tr("%1 kHz Mono - %2").arg(khz).arg(format);
    } else {
        str = tr("%1 kHz Stereo - %2").arg(khz).arg(format);
    }
    contents->details->setText( str );
}


void MediaRecorder::recomputeMaxTime()
{
    // Determine the maximum available space on the device.
    const QFileSystem *fs = contents->storageLocation->fileSystem();

    long availBlocks;
    long blockSize;
    if ( fs ) {
        availBlocks = fs->availBlocks();
        blockSize = fs->blockSize();
    } else {
        availBlocks = 0;
        blockSize = 512;
    }

    // Calculate the number of bytes per second for the current quality,
    // by asking the plugin for an estimate.
    MediaRecorderEncoder *encoder = recorderPlugins->fromType
            ( qualities[config->currentQuality()].mimeType,
              qualities[config->currentQuality()].formatTag );
    long bytesPerSec;
    if( encoder ) {
        bytesPerSec = encoder->estimateAudioBps
            ( qualities[config->currentQuality()].frequency,
              qualities[config->currentQuality()].channels,
              qualities[config->currentQuality()].formatTag );
        if ( bytesPerSec <= 0)
            bytesPerSec = 1;
    } else {
        // We don't have an encoder, so make an estimate based on
        // assuming that the format is wav.
        bytesPerSec = qualities[config->currentQuality()].frequency *
                      qualities[config->currentQuality()].channels * 2;
    }

    // Get an estimate of the maximum number of seconds that we can record.
    // Use "double" to avoid truncation errors with 32-bit arithmetic.
    long maxSecs = (long)(((double)availBlocks) * ((double)blockSize) /
                                (double)bytesPerSec);

    // Truncate the maximum to a reasonable human-grokkable time boundary,
    // as there is no point displaying things like "5 hrs 23 mins 6 secs".
    if ( maxSecs >= (60 * 60 * 24) ) {
        // Truncate to a 1 hour boundary.
        maxSecs -= (maxSecs % (60 * 60));
    } else if ( maxSecs >= (60 * 60 * 10) ) {
        // Truncate to a 15 minute boundary.
        maxSecs -= (maxSecs % (15 * 60));
    } else if ( maxSecs >= (60 * 10) ) {
        // Tuncate to a 1 minute boundary.
        maxSecs -= (maxSecs % 60);
    } else if ( maxSecs > 60 ) {
        // Truncate to a 15 second boundary.
        maxSecs -= (maxSecs % 15);
    }

#ifndef QTOPIA_PHONE        // will never be seen
    // Format the string for the max time field.
    QString str;
    if ( maxSecs >= (60 * 60 * 24) ) {
        if ( (maxSecs % (60 * 60 * 24)) == 0 ) {
            str = tr("%1 days").arg((int)(maxSecs / (60 * 60 * 24)));
        } else {
            str = tr("%1 days %2 hrs")
                .arg((int)(maxSecs / (60 * 60 * 24)))
                .arg((int)((maxSecs / (60 * 60)) % 24));
        }
    } else if ( maxSecs >= (60 * 60) ) {
        if ( (maxSecs % (60 * 60)) == 0 ) {
            str = tr("%1 hrs").arg((int)(maxSecs / (60 * 60)));
        } else {
            str = tr("%1 hrs %2 mins")
                .arg((int)(maxSecs / (60 * 60)))
                .arg((int)((maxSecs / 60) % 60));
        }
    } else if ( maxSecs >= 60 ) {
        if ( (maxSecs % 60) == 0 ) {
            str = tr("%1 mins").arg((int)(maxSecs / 60));
        } else {
            str = tr("%1 mins %2 secs").arg((int)(maxSecs / 60)).arg((int)(maxSecs % 60));
        }
    } else {
        str = tr("%1 secs").arg((int)maxSecs);
    }

    // Update the max time field.
    contents->maxTime->setText( str );
#endif

    maxRecordTime = maxSecs;
}

void MediaRecorder::qualityChanged( int id )
{
    config->setQuality( id );
    config->saveConfig();
    setQualityDisplay( qualities[id] );
    recomputeMaxTime();
}

void MediaRecorder::newLocation()
{
    recomputeMaxTime();
}


bool MediaRecorder::startSave()
{
    // Find the plugin to use to save the data.
    encoder = recorderPlugins->fromType(recordQuality.mimeType,
                                        recordQuality.formatTag);

    // Open the document.
    QContent    doc;
    QString     name = tr("%1 %2","date,time")
            .arg(QTimeString::localYMD(QDate::currentDate(),QTimeString::Short))
            .arg(QTimeString::localHM(QTime::currentTime()));

    doc.setName(name);
    doc.setType( encoder->pluginMimeType() );

    QList<QString>  cats;

    cats.append(recordingsCategory);
    doc.setCategories(cats);

        io = doc.open(QIODevice::WriteOnly);

        // Write the sample data using the encoder.
        encoder->begin(io, recordQuality.formatTag);
        encoder->setAudioChannels(m_audioInput->channels());
        encoder->setAudioFrequency(m_audioInput->frequency());

        // Record the location of the file that we are saving.
        lastSaved = doc.file();

    doc.commit();

        return true;
}

void MediaRecorder::endSave()
{
    // Flush the samples if we recorded to memory.
#ifdef RECORD_THEN_SAVE
    samples->rewind();
    short *buf;
    unsigned int length;
    while ( samples->nextReadBuffer( buf, length ) ) {
        if ( !encoder->writeAudioSamples( buf, (long)length ) )
            break;
    }
#endif

    // Terminate the encode process.
    encoder->end();

    // Close the document.
    io->close();
    delete io;
    io = 0;

    // Clear the data for another recording.
    clearData();
}


void MediaRecorder::startRecording()
{
    if ( config == 0 )
        switchToRecorder();

    if ( mRecordAudioRequest == 0)
        recordQuality = qualities[config->currentQuality()];

    // Bail out if we don't have a plugin for the selected format.
    if (recorderPlugins->fromType(recordQuality.mimeType,
                                    recordQuality.formatTag) == 0) {
        noPluginError();
        return;
    }

    // Disable power save while recording so that the device
    // doesn't suspend while a long-term recording session
    // is in progress.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);

    // Configure and open device
    m_audioInput->setFrequency(recordQuality.frequency);
    m_audioInput->setChannels(recordQuality.channels);

    m_audioInput->open(QIODevice::ReadOnly);

    // TODO: move to ctor
    connect(m_audioInput,
            SIGNAL(readyRead()),
                this,
            SLOT(processAudioData()));

    // Create the sample buffer, for recording the data temporarily.
#ifdef RECORD_THEN_SAVE
    if (samples)
        delete samples;

    samples = new SampleBuffer(audioInput->bufferSize());
#else
    if (sampleBuffer)
        delete[] sampleBuffer;

    sampleBuffer = new short[MR_BUFSIZE];
#endif

    // Reset the position, which is used to calculated the sample progress
    m_position = 0;

    // Start the save process.
    if (startSave()) {

        // Create the waveform display.
        contents->waveform->changeSettings( m_audioInput->frequency(),
                                            m_audioInput->channels());

        if (configureAction)
            configureAction->setEnabled(false);

        contents->qualityCombo->setEnabled( false );
        contents->storageLocation->setEnabled( false );
        recordTime = 0;
        contents->progress->setMaximum( 120 );
        contents->progress->setValue( 0 );
        contents->progress->setRecording();
        recording = true;
        setContextKey( false );
        contents->recordButton->setText( tr("Stop") );
        contents->recordButton->setEnabled( true );
        setReplayEnabled( false );
        setDeleteEnabled( false );

        // Turn on the recording light.
        setRecordLight(true);

        // Some audio devices may start sending us data immediately, but
        // others may need an initial "read" to start the ball rolling.
        // Processing at least one data block will prime the device.
        processAudioData();
    }
}


void MediaRecorder::stopRecordingNoSwitch()
{
    m_audioInput->close();
    contents->waveform->reset();

    if (configureAction)
        configureAction->setEnabled( true );

    contents->qualityCombo->setEnabled( true );
    contents->storageLocation->setEnabled( true );
    contents->recordButton->setEnabled( false );
    recording = false;
    setContextKey( true );
    contents->recordButton->setText( tr("Record") );
    setReplayEnabled( true );
    setDeleteEnabled( true );

    // Turn off the recording light.
    setRecordLight( false );

    // Terminate the data save.
    endSave();

    // Re-enable power save.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    // If we were in request mode, then send the response and quit the app.
    if ( mRecordAudioRequest != 0 ) {
        QByteArray contentArray;
        {
            QDataStream stream( &contentArray, QIODevice::WriteOnly );
            stream << lastSaved;
        }

        QDSData contentData( contentArray, QMimeType( "audio/x-qstring" ) );
        mRecordAudioRequest->respond( contentData );

        delete mRecordAudioRequest;
        mRecordAudioRequest = 0;
        qApp->quit();
    }
}


void MediaRecorder::stopRecording()
{
    stopRecordingNoSwitch();
    switchToFileSelector();
}


void MediaRecorder::recordClicked()
{
    if (recording)
    {
        stopRecording();
    }
    else
    {
        startRecording();
    }
}


void MediaRecorder::startPlaying()
{
    // Reconfigure the UI to reflect the current mode.
    if (configureAction)
        configureAction->setEnabled( false );

    contents->qualityCombo->setEnabled( false );
    contents->storageLocation->setEnabled( false );
    recordTime = 0;
    samplesPlayed = 0;

    // Disable power save while playing so that the device
    // doesn't suspend before the file finishes.
#ifdef Q_WS_QWS
        QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);
#endif

    contents->progress->setValue( 0 );
    contents->progress->setPlaying();
    playing = true;
    setContextKey( false );
    contents->recordButton->setEnabled( false );
    contents->replayButton->setText( tr("Stop") );
    setReplayEnabled( true );
    setReplayEnabled( true );

#ifdef QTOPIA4_TODO
    // Create the waveform display.
    contents->waveform->changeSettings( decoder->audioFrequency( 0 ),
                                        decoder->audioChannels( 0 ) );
#endif

    startWhenAudioDeviceReady = true;
}


void MediaRecorder::audioDeviceReady()
{
    audioDeviceIsReady = true;

    if (startWhenAudioDeviceReady)  // Force the first block to be played, to prime the device.
        QTimer::singleShot(1, this, SLOT(audioOutputDone())); // ready signal generated by audiodevice, it's no longer reentrant
}


void MediaRecorder::audioDeviceError()
{
    audioDeviceIsReady = false;
//    QMessageBox::critical(0, tr("Audio Error"), tr("<qt>Error initialising audio.</qt>"));
    QTimer::singleShot(1, this, SLOT(stopPlaying())); // error signal generated by audiodevice, it's no longer reentrant
}


void MediaRecorder::stopPlayingNoSwitch()
{
    // Stop playing back the recorded sound
    if (m_sound)
    {
        delete m_sound;
        m_sound = 0;
    }

    if ( sampleBuffer ) {
        delete[] sampleBuffer;
        sampleBuffer = 0;
    }

    // Re-enable power save (that was disabled in startPlaying()).
#ifdef Q_WS_QWS
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
#endif

    // Ensure UI initialized
    getContentsWidget();

    // Return the UI to the default state.
    contents->waveform->reset();
    if ( configureAction )
        configureAction->setEnabled( true );

    contents->qualityCombo->setEnabled( true );
    contents->storageLocation->setEnabled( true );
    contents->recordButton->setEnabled( true );
    playing = false;
    setReplayEnabled( true );
    setContextKey( true );
    contents->replayButton->setText( tr("Play") );
    setReplayEnabled( true );
    contents->progress->setValue( 0 );
}


void MediaRecorder::stopPlaying()
{
    stopPlayingNoSwitch();
    switchToFileSelector();
}


void MediaRecorder::replayClicked()
{
    if ( playing ) {
        stopPlaying();
    } else {
        startPlaying();
    }
}


void MediaRecorder::deleteClicked()
{
    if (playing) {
        stopPlaying();
    }

    QContent        doc(lastSaved);

    doc.removeFiles();

    setReplayEnabled(false);
    setDeleteEnabled(false);
}


void MediaRecorder::clearData()
{
#ifdef RECORD_THEN_SAVE
    samples->clear();
#endif
    contents->waveform->reset();
    if ( configureAction )
        configureAction->setEnabled( true );
    contents->recordButton->setEnabled( true );
    recordTime = 0;
    contents->progress->setMaximum( 120 );
    contents->progress->setValue( 0 );
}


void MediaRecorder::processAudioData()
{
    int     result;
    long    newTime;
    bool    stopped = false;

#ifdef RECORD_THEN_SAVE
    short *buf;
    unsigned int length;

    if ( samples->nextWriteBuffer( buf, length ) ) {

        // Read the next block of samples into the write buffer.
        result = m_audioInput->read(buf, length);
        samples->commitWriteBuffer( (unsigned int)result );

        // Update the waveform display.
        contents->waveform->newSamples( buf, result );
    }
    else {

        // The sample buffer is out of space, so stop recording.
        stopRecording();
        stopped = true;
    }
#else

    result = m_audioInput->read(reinterpret_cast<char*>(sampleBuffer), MR_BUFSIZE);

    result /= sizeof(short) * m_audioInput->channels();

    contents->waveform->newSamples(sampleBuffer, result);

    encoder->writeAudioSamples(sampleBuffer, (long)result);

    m_position += result;

#endif

    // Update the record time if another second has elapsed.
    newTime = m_position / (long)(m_audioInput->frequency());
    if (newTime != recordTime) {
        recordTime = newTime;

        if (recordTime >= contents->progress->maximum()) {
            // Change the resolution on the progress bar as we've
            // max'ed out the current limit.
            contents->progress->setMaximum(contents->progress->maximum() * 4);
        }

        contents->progress->setValue((int) recordTime);
    }

    // Stop recording if we have hit the maximum record time.
    if (recordTime >= maxRecordTime && !stopped) {
        stopRecording();
    }
}


void MediaRecorder::configure()
{
    config->processPopup();
    contents->qualityCombo->setCurrentIndex( config->currentQuality() );
    qualityChanged( config->currentQuality() );
}


void MediaRecorder::noPluginError()
{
    QMessageBox::critical( this, tr( "No plugin found" ),
                           tr( "<qt>Voice Recorder was unable to "
                               "locate a suitable plugin to "
                               "record in the selected format.</qt>" ) );
}


#define RECORD_LIGHT_ID     ((int)0x56526563)   // "VRec"

void MediaRecorder::setRecordLight( bool enable )
{
    if (enable)
    {
        recordLightState = true;
#ifndef QTOPIA_PHONE
        QtopiaIpcEnvelope( "Qt/Tray", "setIcon(int,QPixmap)" ) << RECORD_LIGHT_ID << QPixmap( ":image/record-light" );
        lightTimer->start( 500 );
#endif
    }
    else
    {
        recordLightState = false;
#ifndef QTOPIA_PHONE
        QtopiaIpcEnvelope( "Qt/Tray", "remove(int)" ) << RECORD_LIGHT_ID;
        lightTimer->stop();
#endif
    }
}


void MediaRecorder::recordLightBlink()
{
    recordLightState = !recordLightState;
#ifndef QTOPIA_PHONE
    if ( recordLightState ) {
        QtopiaIpcEnvelope( "Qt/Tray", "setIcon(int,QPixmap)" )
                << RECORD_LIGHT_ID
                << QPixmap( ":image/record-light" );
    } else {
        QtopiaIpcEnvelope( "Qt/Tray", "setIcon(int,QPixmap)" )
                << RECORD_LIGHT_ID
                << QPixmap( ":image/record-blank" );
    }
#endif
}


void MediaRecorder::closeEvent(QCloseEvent *e)
{
    // Shut down recording or playback.
    if (contentsWidget != NULL)
    {
        stopEverythingNoSwitch();

        // Disable the "Play" and "Delete" buttons so that if we
        // are restarted in "fast load" mode, we will return to
        // the initial "nothing is recorded" state in the UI.
        setReplayEnabled(false);
        setReplayEnabled(false);
    }

    // Determine if we should return to the file selector screen,
    // or exit from the application.
    if (stack->currentWidget() == selector)
    {
        e->accept();
    }
    else if ( mRecordAudioRequest != 0)
    {
        QByteArray contentArray;
        {
            QDataStream stream( &contentArray, QIODevice::WriteOnly );
            stream << lastSaved;
        }

        QDSData contentData( contentArray, QMimeType( "audio/x-qstring" ) );
        mRecordAudioRequest->respond( contentData );

        delete mRecordAudioRequest;
        mRecordAudioRequest = 0;

        e->accept();
    }
    else
    {
        switchToFileSelector();
        e->ignore();
    }
}


void MediaRecorder::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (e->key() == Qt::Key_Select)
    {
        if (playing)
            stopPlaying();
        else if (recording)
            stopRecording();
        else
            startRecording();

        e->accept();

        return;
    }
#endif

    QMainWindow::keyPressEvent(e);
}


void MediaRecorder::audioOutputDone()
{
    // Read the next block of samples.
    long samplesRead = 0;

    if ( samplesRead <= 0 ) {
        stopPlaying();
        return;
    }

    // Update the waveform display.
    contents->waveform->newSamples( sampleBuffer, samplesRead );

    // Update the playback time if another second has elapsed.
    samplesPlayed += samplesRead;
#ifdef QTOPIA4_TODO
    long newTime = samplesPlayed / (long)(decoder->audioFrequency( 0 ));
    if ( newTime != recordTime ) {
        recordTime = newTime;
        if ( recordTime > contents->progress->maximum() ) {
            recordTime = contents->progress->maximum();
        }
        contents->progress->setValue( (int)recordTime );
    }
#endif
}


void MediaRecorder::traySocket( const QString& msg, const QByteArray &data )
{
    QDataStream stream( data );
    int         id = 0;
    QPoint      p;

    if (msg == "popup(int,QPoint)" )
    {
        stream >> id >> p;
    }
    else if ( msg == "clicked(int,QPoint)" || msg == "doubleClicked(int,QPoint)" )
    {
        stream >> id >> p;
    }

    if (id == RECORD_LIGHT_ID )
    {
        if (this->isVisible())
            this->raise();
        else
            this->showMaximized();
    }
}


void MediaRecorder::setContextKey( bool record )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if ( record )
        QSoftMenuBar::setLabel( contents->recordButton, Qt::Key_Select, "mediarecorder/record", tr("Record") );
    else
        QSoftMenuBar::setLabel( contents->recordButton, Qt::Key_Select, "stop", tr("Stop") );
#else
    Q_UNUSED(record);
#endif
}


void MediaRecorder::setReplayEnabled( bool flag )
{
    contents->replayButton->setEnabled( flag );
}


void MediaRecorder::setDeleteEnabled( bool flag )
{
    contents->deleteButton->setEnabled( flag );
}

void MediaRecorder::documentSelected(const QContent& doc)
{
    stopEverythingNoSwitch();

    lastSaved = doc.file();

    m_sound = new QSound( lastSaved );
    m_sound->play();
    playing = true;
}


void MediaRecorder::newSelected()
{
    if (recorderPlugins == NULL)
        recorderPlugins = new MediaRecorderPluginList();

    if (m_sound != NULL)        // stop playing
    {
        delete m_sound;
        m_sound = NULL;
    }

    switchToRecorder();
}

void MediaRecorder::toggleRecording()
{
    if ( playing )
        stopPlayingNoSwitch();

    switchToRecorder();
    recordClicked();
}

void MediaRecorder::recordAudio( const QDSActionRequest& request )
{
    if ( mRecordAudioRequest != 0 ) {
        QDSActionRequest( request ).respond( tr( "Busy serving previous request" ) );
    }

    // Stop existing recording or playback sessions.
    stopEverythingNoSwitch();

    // Unpack the audio parameters and setup the recorder
    QDataStream stream( request.requestData().toIODevice() );
    AudioParameters parameters;
    stream >> parameters;

    recordQuality.frequency = parameters.frequency();
    recordQuality.channels = parameters.channels();
    recordQuality.mimeType = parameters.mimeType().id();
    recordQuality.formatTag = parameters.subFormat();

    // Save the request and switch to the recording mode.
    mRecordAudioRequest = new QDSActionRequest( request );
    showMaximized();
    switchToRecorder();
}

void MediaRecorder::switchToFileSelector()
{
#ifndef QTOPIA_PHONE
    menu->hide();
#endif
    stack->setCurrentWidget( selector );
    selector->setFocus();

    configureAction->setEnabled( false );
}


void MediaRecorder::switchToOther()
{
    stack->setCurrentIndex( stack->indexOf( getContentsWidget() ) );
}

void MediaRecorder::switchToRecorder()
{
    switchToOther();

    setContextKey(true);
    setReplayEnabled(false);
    setDeleteEnabled(false);
    configureAction->setEnabled(true);

#ifndef QTOPIA_PHONE
    if ( mRecordAudioRequest != 0 )
    {
        contents->GroupBox1->hide();
        contents->GroupBox2->hide();
        menu->hide();
        configureAction->setEnabled( false );
    }
    else
    {
        if (smallScreen)
            contents->GroupBox1->hide();
        else
            contents->GroupBox1->show();
        contents->GroupBox2->show();
        menu->show();
    }
#endif

    contents->recordButton->show();
    contents->replayButton->hide();
    contents->deleteButton->hide();
    contents->waveform->reset();
    contents->recordButton->setFocus();
}


void MediaRecorder::switchToPlayback()
{
    switchToOther();
    setContextKey( false );
    setReplayEnabled( true );
    setDeleteEnabled( false );
    configureAction->setEnabled( false );
#ifndef QTOPIA_PHONE
    contents->GroupBox1->hide();
    contents->GroupBox2->hide();
    menu->hide();
#endif
    contents->replayButton->show();
    contents->recordButton->hide();
    contents->deleteButton->hide();
    contents->waveform->reset();
    contents->recordButton->setFocus();
}

void MediaRecorder::stopEverythingNoSwitch()
{
    if (recording)
    {
        stopRecordingNoSwitch();
    }
    else if (playing)
    {
        stopPlayingNoSwitch();
    }
}

QWidget* MediaRecorder::getContentsWidget()
{
    if (contentsWidget == NULL)
    {
        // contents (buttons & graph)
        contentsWidget = new QWidget(stack);
        contents = new Ui::MediaRecorderBase();
        contents->setupUi(contentsWidget);

        QFileSystemFilter *fsf = new QFileSystemFilter;
        fsf->documents = QFileSystemFilter::Set;
        contents->storageLocation->setFilter(fsf);
        stack->addWidget( contentsWidget );

        // other init
        initializeContents();

        // Hook up interesting signals.
        connect(contents->recordButton,
                SIGNAL(clicked()),
                this,
                SLOT(recordClicked()));

        // menu
        QMenu* options;

#ifdef QTOPIA_KEYPAD_NAVIGATION
        // Create the context menu for the record/playback screen.
        options = QSoftMenuBar::menuFor(contentsWidget);

#else
        QToolBar *bar = new QToolBar( this );
        addToolBar( bar );
        bar->setMovable( false );
        menu = bar;

        QMenuBar *mb = new QMenuBar( bar );
        options = mb->addMenu( tr( "Options" ) );
#endif
        configureAction = new QAction(QIcon(":icon/settings"), tr( "Settings..."), this);
        connect(configureAction, SIGNAL(triggered()), this, SLOT(configure()));
        configureAction->setWhatsThis(tr("Configure the recording quality settings."));
        configureAction->setEnabled(true);
        options->addAction(configureAction);

        // GUI opts
#ifdef QTOPIA_KEYPAD_NAVIGATION
        // Don't display the buttons in Qtopia Phone Edition, because
        // they take up too much screen real estate and have confusing
        // key navigation behaviours.
        //contents->recordButton->hide();
        contents->replayButton->hide();
        contents->deleteButton->hide();

        // Make the context key say "Record".
        setContextKey(true);
#else
        connect(contents->replayButton,
                SIGNAL(clicked()),
                this,
                SLOT(replayClicked()));

        connect(contents->deleteButton,
                SIGNAL(clicked()),
                this,
                SLOT(deleteClicked()));
#endif
    }

    return contentsWidget;
}



/*!
    \service VoiceRecordingService VoiceRecording
    \brief Provides the Qtopia VoiceRecording service.

    The \i VoiceRecording service enables applications to toggle
    audio recording on or off.
*/
/*!
    \internal
*/
VoiceRecordingService::~VoiceRecordingService()
{
}

/*!
    Toggle audio recording on or off.

    This slot corresponds to the QCop service message
    \c{VoiceRecording::toggleRecording()}.
*/
void VoiceRecordingService::toggleRecording()
{
    parent->toggleRecording();
}

/*!
    Prepares for audio recording using the parameters specified in \a request.

    This slot corresponds to a QDS service with a request data type of
    "x-parameters/x-audioparameters" and a response data type of "audio/x-qstring".

    This slot corresponds to the QCop service message
    \c{VoiceRecording::recordAudio(QDSActionRequest)}.
*/
void VoiceRecordingService::recordAudio( const QDSActionRequest& request )
{
    parent->recordAudio( request );
}
