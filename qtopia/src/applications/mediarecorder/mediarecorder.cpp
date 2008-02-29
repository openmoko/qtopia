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

#define	QTOPIA_INTERNAL_MIMEEXT

#include "mediarecorder.h"
#include "mediarecorderbase.h"
#include "audioinput.h"
#include "audiodevice.h"
#include "samplebuffer.h"
#include "pluginlist.h"
#include "timeprogressbar.h"
#include "confrecorder.h"
#include "waveform.h"

#include <qtopia/global.h>
#include <qtopia/fileselector.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/locationcombo.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/mimetype.h>
#include <qtopia/fileselector.h>
#include <qtopia/categories.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qaction.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qfile.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qtopia/contextbar.h>
#endif

#include <stdlib.h>


MediaRecorder::MediaRecorder( QWidget *parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f )
{
    audioDeviceIsReady = false;
    startWhenAudioDeviceReady = false;
    requestMode = false;

    // We remove some of the UI if the screen is too small to hold it.
    smallScreen = ( height() < 120 );

    // Load the media plugins.
    recorderPlugins = new MediaRecorderPluginList();
    playerPlugins = new MediaPlayerPluginList();

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );
    setCaption( tr("Voice Notes") );

    setIcon( Resource::loadPixmap( "SoundPlayer" ) );

    // We don't need an input method with this application.
    QPEApplication::setInputMethodHint( this, QPEApplication::AlwaysOff );

    // Make sure that the "Recordings" category is registered.
    Categories cats;
    cats.load(categoryFileName());
    recordingsCategory = cats.id("Document View","_Recordings"); // No tr
    if ( !recordingsCategory ) {
	recordingsCategory =
	    cats.addCategory("Document View","_Recordings"); // No tr
	cats.save(categoryFileName());
    }

    stack = new QWidgetStack( this );
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setCentralWidget( stack );

    contents = new MediaRecorderBase( stack );
    stack->addWidget( contents, 0 );

    selector = new FileSelector( "audio/*", stack, "fileselector", TRUE, FALSE );
    selector->setCurrentCategory( recordingsCategory );
    selector->setSortMode( FileSelector::ReverseChronological );
    stack->addWidget( selector, 1 );
    connect( selector, SIGNAL(fileSelected(const DocLnk&)),
	     this, SLOT(fileSelected(const DocLnk&)) );
    connect( selector, SIGNAL(newSelected(const DocLnk&)),
	     this, SLOT(newSelected(const DocLnk&)) );

#ifdef QTOPIA_PHONE

    // Create the context menu for the file selector screen.
    ContextMenu *options = new ContextMenu( selector );
    selector->addOptions( options );

    // Create the context menu for the record/playback screen.
    options = new ContextMenu( contents );

#else
    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    menu = bar;

    QPEMenuBar *mb = new QPEMenuBar( bar );
    QPopupMenu *options = new QPopupMenu( this );

    mb->insertItem( tr( "Options" ), options );
#endif

    configureAction = new QAction(tr( "Settings..."), Resource::loadIconSet("settings"), QString::null, 0, this, 0 );
    connect( configureAction, SIGNAL( activated() ), this, SLOT( configure() ) );
    configureAction->setWhatsThis( tr("Configure the recording quality settings.") );
    configureAction->addTo( options );
    configureAction->setEnabled( TRUE );

    initializeContents();
    switchToFileSelector();

    audioInput = 0;
    audioOutput = 0;
#ifdef RECORD_THEN_SAVE
    samples = 0;
#endif
    sampleBuffer = 0;
    recordTime = 0;
    recording = FALSE;
    playing = FALSE;
    io = 0;
    recordLightState = FALSE;
    lightTimer = new QTimer( this );

    // Hook up interesting signals.
    connect( contents->recordButton, SIGNAL( clicked() ),
	     this, SLOT( recordClicked() ) );
    connect( contents->replayButton, SIGNAL( clicked() ),
	     this, SLOT( replayClicked() ) );
    connect( contents->deleteButton, SIGNAL( clicked() ),
	     this, SLOT( deleteClicked() ) );
    connect( lightTimer, SIGNAL( timeout() ),
	     this, SLOT( recordLightBlink() ) );

#ifdef QTOPIA_PHONE

    // Don't display the buttons in Qtopia Phone Edition, because
    // they take up too much screen real estate and have confusing
    // key navigation behaviours.
    //contents->recordButton->hide();
    contents->replayButton->hide();
    contents->deleteButton->hide();

    // Make the context key say "Record".
    setContextKey( TRUE );

#endif

    // Listen on the system tray for clicks on the record light so that
    // we can raise the application when the user clicks on it.
    trayChannel = new QCopChannel( "Qt/Tray", this );
    connect( trayChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	     this, SLOT(traySocket(const QCString&,const QByteArray&)) );

    // Listen for app messages, particularly the "recordSound" request.
    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
             this, SLOT(appMessage(const QCString&,const QByteArray&)) );

    // Force the MIME type handling system to preload the list
    // of extensions.  If we don't do this, then "startRecording"
    // ends up being _very_ slow.
    MimeType type("audio/x-wav");
}


MediaRecorder::~MediaRecorder()
{
    delete recorderPlugins;
    delete playerPlugins;
    if ( audioInput )
	delete audioInput;
    if ( audioOutput )
	delete audioOutput;
#ifdef RECORD_THEN_SAVE
    if ( samples )
	delete samples;
#endif
    if ( sampleBuffer )
	delete[] sampleBuffer;
    if ( io )
	delete io;
    if ( lightTimer )
	delete lightTimer;
}


void MediaRecorder::initializeContents()
{
    // The progress bar initially has no time display.  This will be fixed
    // up when we start recording.
    contents->progress->setTotalSteps( 10 );
    contents->progress->setProgress( -1 );

    // Cannot replay yet, because there is no recorded sound.
    setReplayEnabled( FALSE );
    setDeleteEnabled( FALSE );

    // Load the initial quality settings.
    config = new ConfigureRecorder( qualities, recorderPlugins, this );
    contents->qualityCombo->setCurrentItem(config->currentQuality());
    setQualityDisplay( qualities[config->currentQuality()] );
    connect( contents->qualityCombo, SIGNAL( activated(int) ),
	     this, SLOT( qualityChanged(int) ) );

    connect( contents->storageLocation, SIGNAL(newPath()), 
	     this, SLOT(newLocation()) );
    recomputeMaxTime();

#ifdef QTOPIA_PHONE

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
    const FileSystem *fs = contents->storageLocation->fileSystem();

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
    encoder = recorderPlugins->fromType
	( recordQuality.mimeType, recordQuality.formatTag );

    // Open the document.
    DocLnk doc;
    FileManager fm;
    QString name =
	tr("%1 %2","date,time")
	    .arg(TimeString::localYMD(QDate::currentDate(),TimeString::Short))
	    .arg(TimeString::localHM(QTime::currentTime()));
    doc.setName( name );
    doc.setType( encoder->pluginMimeType() );
    QArray<int> cats( 1 );
    cats[0] = recordingsCategory;
    doc.setCategories( cats );

    if (doc.setLocation( contents->storageLocation->documentPath() )) {
	io = fm.saveFile( doc );

	// Write the sample data using the encoder.
	encoder->begin( io, recordQuality.formatTag );
	encoder->setAudioChannels( audioInput->channels() );
	encoder->setAudioFrequency( audioInput->frequency() );

	// Record the location of the file that we are saving.
	lastSaved = doc.file();
	lastSavedLink = doc.linkFile();
	return true;
    }
    QMessageBox::information(this, tr("Voice Notes"), tr("<P>Could not save file at location %1").arg(contents->storageLocation->currentText()), QMessageBox::Ok);
    return false;
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
    if ( !requestMode )
	recordQuality = qualities[config->currentQuality()];

    // Bail out if we don't have a plugin for the selected format.
    if( recorderPlugins->fromType
	    ( recordQuality.mimeType, recordQuality.formatTag ) == 0) {
    	noPluginError();
	return;
    }

    // Disable power save while recording so that the device
    // doesn't suspend while a long-term recording session
    // is in progress.
#ifdef Q_WS_QWS
	QPEApplication::setTempScreenSaverMode( QPEApplication::DisableSuspend );
#endif

    // Open the audio input device and normalize the parameters.
    if ( audioInput )
	delete audioInput;
    audioInput = new AudioInput
	( recordQuality.frequency, recordQuality.channels );
    qDebug( "channels: %d", audioInput->channels() );
    qDebug( "frequency: %d", audioInput->frequency() );
    qDebug( "bufferSize: %d", audioInput->bufferSize() );
    connect( audioInput, SIGNAL( dataAvailable() ),
	     this, SLOT( processAudioData() ) );

    // Create the sample buffer, for recording the data temporarily.
#ifdef RECORD_THEN_SAVE
    if ( samples )
	delete samples;
    samples = new SampleBuffer( audioInput->bufferSize() );
#else
    if ( sampleBuffer )
	delete[] sampleBuffer;
    sampleBuffer = new short [ audioInput->bufferSize() ];
#endif

    // Start the save process.
    if (startSave()) {

	// Create the waveform display.
	contents->waveform->changeSettings( audioInput->frequency(),
		audioInput->channels() );

	if ( configureAction )
	    configureAction->setEnabled( FALSE );
	contents->qualityCombo->setEnabled( FALSE );
	contents->storageLocation->setEnabled( FALSE );
	recordTime = 0;
	contents->progress->setTotalSteps( 120 );
	contents->progress->setProgress( 0 );
	contents->progress->setRecording();
	recording = TRUE;
	setContextKey( FALSE );
	contents->recordButton->setText( tr("Stop") );
	contents->recordButton->setEnabled( TRUE );
	setReplayEnabled( FALSE );
	setDeleteEnabled( FALSE );

	// Turn on the recording light.
	setRecordLight( TRUE );

	// Some audio devices may start sending us data immediately, but
	// others may need an initial "read" to start the ball rolling.
	// Processing at least one data block will prime the device.
	audioInput->start();
	processAudioData();
    }
}


void MediaRecorder::stopRecordingNoSwitch()
{
    audioInput->stop();
    contents->waveform->reset();
    if ( configureAction )
	configureAction->setEnabled( TRUE );
    contents->qualityCombo->setEnabled( TRUE );
    contents->storageLocation->setEnabled( TRUE );
    contents->recordButton->setEnabled( FALSE );
    recording = FALSE;
    setContextKey( TRUE );
    contents->recordButton->setText( tr("Record") );
    setReplayEnabled( TRUE );
    setDeleteEnabled( TRUE );

    // Turn off the recording light.
    setRecordLight( FALSE );

    // Terminate the data save.
    endSave();

    // Re-enable power save.
#ifdef Q_WS_QWS
    QPEApplication::setTempScreenSaverMode( QPEApplication::Enable );
#endif

    // If we were in request mode, then send the response and quit the app.
    if ( requestMode ) {
	qDebug("Respond to id: %s", responseId.latin1());
        QCopEnvelope e( responseChannel, "valueSupplied(QString,QString)" );
	e << responseId << lastSavedLink;
	requestMode = FALSE;
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
    if ( recording ) {
	stopRecording();
    } else {
	startRecording();
    }
}


void MediaRecorder::startPlaying()
{
    // Do we have a decoder for this type of file?
    decoder = playerPlugins->fromFile( lastSaved );
    if ( decoder == 0 || !( decoder->open( lastSaved ) ) ) {
        QMessageBox::critical( this, tr( "No playback plugin found" ),
			       tr( "<qt>Voice Recorder was unable to "
			           "locate a suitable plugin to "
			           "play the selected format.</qt>" ) );
	return;
    }

    // Reconfigure the UI to reflect the current mode.
    if ( configureAction )
	configureAction->setEnabled( FALSE );
    contents->qualityCombo->setEnabled( FALSE );
    contents->storageLocation->setEnabled( FALSE );
    recordTime = 0;
    samplesPlayed = 0;
    long samples = decoder->audioSamples( 0 );
    long freq = decoder->audioFrequency( 0 );
    samples = (samples + freq - 1) / freq;
    contents->progress->setTotalSteps( samples );
    contents->progress->setProgress( 0 );
    contents->progress->setPlaying();
    playing = TRUE;
    setContextKey( FALSE );
    contents->recordButton->setEnabled( FALSE );
    contents->replayButton->setText( tr("Stop") );
    setReplayEnabled( TRUE );
    setReplayEnabled( TRUE );

    // Create the waveform display.
    contents->waveform->changeSettings( decoder->audioFrequency( 0 ),
					decoder->audioChannels( 0 ) );

    // Open the audio output device.
    audioOutput = new AudioDevice( this, "voiceNotesAudioDevice" );
    audioOutput->open( decoder->audioFrequency( 0 ),
				   decoder->audioChannels( 0 ), 2 );
    sampleBuffer = new short [ audioOutput->bufferSize() ];
    QObject::connect( audioOutput, SIGNAL(completedIO()),
		      this, SLOT(audioOutputDone()) );
    connect( audioOutput, SIGNAL(deviceReady()), this, SLOT(audioDeviceReady()) );
    connect( audioOutput, SIGNAL(deviceOpenError()), this, SLOT(audioDeviceError()) );
    audioDeviceIsReady = false;
    startWhenAudioDeviceReady = true;
}


void MediaRecorder::audioDeviceReady()
{
    audioDeviceIsReady = true;
    if (startWhenAudioDeviceReady)
	// Force the first block to be played, to prime the device.
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
    if ( audioOutput ) {
	delete audioOutput;
	audioOutput = 0;
    }
    if ( decoder ) {
	decoder->close();
	decoder = 0;
    }
    if ( sampleBuffer ) {
	delete[] sampleBuffer;
	sampleBuffer = 0;
    }

    // Return the UI to the default state.
    contents->waveform->reset();
    if ( configureAction )
	configureAction->setEnabled( TRUE );
    contents->qualityCombo->setEnabled( TRUE );
    contents->storageLocation->setEnabled( TRUE );
    contents->recordButton->setEnabled( TRUE );
    playing = FALSE;
    setReplayEnabled( TRUE );
    setContextKey( TRUE );
    contents->replayButton->setText( tr("Play") );
    setReplayEnabled( TRUE );
    contents->progress->setProgress( 0 );
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
    if ( playing ) {
	stopPlaying();
    }
    DocLnk doc( lastSaved );
    doc.setLinkFile( lastSavedLink );
    doc.removeFiles();
    setReplayEnabled( FALSE );
    setDeleteEnabled( FALSE );
}


void MediaRecorder::clearData()
{
#ifdef RECORD_THEN_SAVE
    samples->clear();
#endif
    contents->waveform->reset();
    if ( configureAction )
	configureAction->setEnabled( TRUE );
    contents->recordButton->setEnabled( TRUE );
    recordTime = 0;
    contents->progress->setTotalSteps( 120 );
    contents->progress->setProgress( 0 );
}


void MediaRecorder::processAudioData()
{
    int result;
    long newTime;
    bool stopped = FALSE;

#ifdef RECORD_THEN_SAVE
    short *buf;
    unsigned int length;

    if ( samples->nextWriteBuffer( buf, length ) ) {
	// Read the next block of samples into the write buffer.
	result = audioInput->read( buf, length );
	samples->commitWriteBuffer( (unsigned int)result );

	// Update the waveform display.
	contents->waveform->newSamples( buf, result );
    } else {
	// The sample buffer is out of space, so stop recording.
	stopRecording();
	stopped = TRUE;
    }
#else
    result = audioInput->read( sampleBuffer, audioInput->bufferSize() );
    while ( result > 0 ) {
	contents->waveform->newSamples( sampleBuffer, result );
	encoder->writeAudioSamples( sampleBuffer, (long)result );
	result = audioInput->read( sampleBuffer, audioInput->bufferSize() );
    }
#endif

    // Update the record time if another second has elapsed.
    newTime = audioInput->position() / (long)(audioInput->frequency());
    if ( newTime != recordTime ) {
	recordTime = newTime;
	if ( recordTime >= contents->progress->totalSteps() ) {
	    // Change the resolution on the progress bar as we've
	    // max'ed out the current limit.
	    contents->progress->setTotalSteps
		(contents->progress->totalSteps() * 4);
	}
	contents->progress->setProgress( (int)recordTime );
    }

    // Stop recording if we have hit the maximum record time.
    if ( recordTime >= maxRecordTime && !stopped ) {
	stopRecording();
    }
}


void MediaRecorder::configure()
{
    config->processPopup();
    contents->qualityCombo->setCurrentItem( config->currentQuality() );
    qualityChanged( config->currentQuality() );
}


void MediaRecorder::noPluginError()
{
    QMessageBox::critical( this, tr( "No plugin found" ),
			   tr( "<qt>Voice Recorder was unable to "
			       "locate a suitable plugin to "
			       "record in the selected format.</qt>" ) );
}


#define	RECORD_LIGHT_ID	    ((int)0x56526563)	// "VRec"

void MediaRecorder::setRecordLight( bool enable )
{
    if ( enable ) {
        QCopEnvelope( "Qt/Tray", "setIcon(int,QPixmap)" )
		<< RECORD_LIGHT_ID
		<< Resource::loadPixmap( "record-light" );
	recordLightState = TRUE;
	lightTimer->start( 500 );
    } else {
        QCopEnvelope( "Qt/Tray", "remove(int)" )
		<< RECORD_LIGHT_ID;
	recordLightState = FALSE;
	lightTimer->stop();
    }
}


void MediaRecorder::recordLightBlink()
{
    recordLightState = !recordLightState;
    if ( recordLightState ) {
        QCopEnvelope( "Qt/Tray", "setIcon(int,QPixmap)" )
		<< RECORD_LIGHT_ID
		<< Resource::loadPixmap( "record-light" );
    } else {
        QCopEnvelope( "Qt/Tray", "setIcon(int,QPixmap)" )
		<< RECORD_LIGHT_ID
		<< Resource::loadPixmap( "record-blank" );
    }
}


void MediaRecorder::closeEvent( QCloseEvent *e )
{
    // Shut down recording or playback.
    stopEverythingNoSwitch();

    // Disable the "Play" and "Delete" buttons so that if we
    // are restarted in "fast load" mode, we will return to
    // the initial "nothing is recorded" state in the UI.
    setReplayEnabled( FALSE );
    setReplayEnabled( FALSE );

    // Determine if we should return to the file selector screen,
    // or exit from the application.
    if ( stack->visibleWidget() == selector ) {
	e->accept();
    } else if ( requestMode ) {
	// The user did not record any sounds during the request.
        QCopEnvelope qcop( responseChannel, "valueSupplied(QString,QString)" );
	qcop << responseId << QString::null;
	requestMode = FALSE;
	e->accept();
    } else {
	switchToFileSelector();
	e->ignore();
    }
}


void MediaRecorder::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    if ( e->key() == Qt::Key_Select ) {
	if ( playing )
	    stopPlaying();
	else if ( recording )
	    stopRecording();
	else
	    startRecording();
	e->accept();
	return;
    }
#endif
    QMainWindow::keyPressEvent( e );
}


void MediaRecorder::audioOutputDone()
{
    // Read the next block of samples.
    long samplesRead = 0;
    if ( !decoder->audioReadSamples( sampleBuffer, 2, 1024, samplesRead, 0 ) ) {
	stopPlaying();
	return;
    }
    if ( samplesRead <= 0 ) {
	stopPlaying();
	return;
    }
    // Write the samples to the audio output device.
    audioOutput->write( (char *)sampleBuffer, (int)( samplesRead * 4 ) );

    // Update the waveform display.
    contents->waveform->newSamples( sampleBuffer, samplesRead );

    // Update the playback time if another second has elapsed.
    samplesPlayed += samplesRead;
    long newTime = samplesPlayed / (long)(decoder->audioFrequency( 0 ));
    if ( newTime != recordTime ) {
	recordTime = newTime;
	if ( recordTime > contents->progress->totalSteps() ) {
	    recordTime = contents->progress->totalSteps();
	}
	contents->progress->setProgress( (int)recordTime );
    }
}


void MediaRecorder::traySocket( const QCString& msg, const QByteArray &data )
{
    QDataStream stream( data, IO_ReadOnly );
    int id = 0;
    QPoint p;
    
    if ( msg == "popup(int,QPoint)" ) {
	stream >> id >> p;
    } else if ( msg == "clicked(int,QPoint)" || msg == "doubleClicked(int,QPoint)" ) {
	stream >> id >> p;
    }

    if ( id == RECORD_LIGHT_ID ) {
	if ( this->isVisible() )
	    this->raise();
	else
	    this->showMaximized();
    }
}


void MediaRecorder::setContextKey( bool record )
{
#ifdef QTOPIA_PHONE
    if ( record )
	ContextBar::setLabel( contents->recordButton, Qt::Key_Select, "mediarecorder/record", tr("Record") );
    else
	ContextBar::setLabel( contents->recordButton, Qt::Key_Select, "mediarecorder/stop", tr("Stop") );
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


void MediaRecorder::fileSelected( const DocLnk& doc )
{
    stopEverythingNoSwitch();
    lastSaved = doc.file();
    lastSavedLink = doc.linkFile();
    switchToPlayback();
    startPlaying();
}


void MediaRecorder::newSelected( const DocLnk& )
{
    stopEverythingNoSwitch();
    switchToRecorder();
}


void MediaRecorder::appMessage( const QCString& msg, const QByteArray& data )
{
    QCString chan;
    QString id, mimeType, formatTag;
    int frequency, channels;
    
    // Parameters to "getAudio" message:
    //
    //	    QCString	Name of the QCop channel to send the reply on.
    //	    QString	Unique id.
    //	    QString	MIME type of the requested format.
    //	    QString	Requested sub-format (e.g. "pcm", "amr", etc).
    //	    int		Frequency to record with.
    //	    int		Number of channels to record with.
    //
    // The mediarecorder will eventually respond with the message
    // "valueSupplied(QString,QString)", indicating the ID and the
    // name of the file
    // containing the recording.  If the user quit the recorder without
    // recording a sound, then the filename will be empty.
    //
    // Note: it is conceivable that something will go wrong and the
    // response will never be sent (e.g. app crash).  Applications that
    // request recording functionality should not rely upon getting
    // an answer back.
    //
    if ( msg == "getAudio(QCString,QString,QString,QString,int,int)" ) {

	// Decode the parameters to the message.
	QDataStream stream( data, IO_ReadOnly );
	stream >> chan;
	stream >> id;
	stream >> mimeType;
	stream >> formatTag;
	stream >> frequency;
	stream >> channels;

	// Stop existing recording or playback sessions.
	stopEverythingNoSwitch();

	// Copy the parameters into place.
	responseChannel = chan;
	responseId = id;
	recordQuality.frequency = frequency;
	recordQuality.channels = channels;
	recordQuality.mimeType = mimeType;
	recordQuality.formatTag = formatTag;
	requestMode = TRUE;

	// Switch to the recording mode.
	switchToRecorder();

	// Make sure that the app keeps running.
	QPEApplication::setKeepRunning();
    }
}


void MediaRecorder::switchToFileSelector()
{
#ifndef QTOPIA_PHONE
    menu->hide();
#endif
    stack->raiseWidget( selector );
    configureAction->setEnabled( FALSE );
}


void MediaRecorder::switchToOther()
{
    stack->raiseWidget( contents );
}

void MediaRecorder::switchToRecorder()
{
    switchToOther();
    setContextKey( TRUE );
    setReplayEnabled( FALSE );
    setDeleteEnabled( FALSE );
    configureAction->setEnabled( TRUE );
#ifndef QTOPIA_PHONE
    if ( requestMode ) {
	contents->GroupBox1->hide();
	contents->GroupBox2->hide();
	menu->hide();
	configureAction->setEnabled( FALSE );
    } else {
	if ( smallScreen )
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
    setContextKey( FALSE );
    setReplayEnabled( TRUE );
    setDeleteEnabled( FALSE );
    configureAction->setEnabled( FALSE );
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
    if ( recording ) {
	stopRecordingNoSwitch();
    } else if ( playing ) {
	stopPlayingNoSwitch();
    }
}
