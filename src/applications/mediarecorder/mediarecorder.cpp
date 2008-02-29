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

#define	QTOPIA_INTERNAL_MIMEEXT

#include "mediarecorder.h"
#include "mediarecorderbase.h"
#include "audioinput.h"
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
#include <qtopia/qpeapplication.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/mimetype.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qaction.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qfile.h>

#include <stdlib.h>


MediaRecorder::MediaRecorder( QWidget *parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f )
{
    // Load the media plugins.
    recorderPlugins = new MediaRecorderPluginList();
    playerPlugins = new MediaPlayerPluginList();

    // Create a storage handler.
    storage = new StorageInfo( this );

    // Find the primary document location.
    homeDocuments = QString(getenv("HOME")) + "/Documents";
    currentDocPath = homeDocuments;

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );
    setCaption( tr("Voice Recorder") );

    setIcon( Resource::loadPixmap( "SoundPlayer" ) );

    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    menu = bar;

    QPEMenuBar *mb = new QPEMenuBar( bar );
    QPopupMenu *options = new QPopupMenu( this );

    mb->insertItem( tr( "Options" ), options );

    configureAction = new QAction( tr( "Configure Qualities ..." ), QString::null, 0, this, 0 );
    connect( configureAction, SIGNAL( activated() ), this, SLOT( configure() ) );
    configureAction->setWhatsThis( tr("Configure the recording quality settings.") );
    configureAction->addTo( options );
    configureAction->setEnabled( TRUE );

    stack = new QWidgetStack( this );
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setCentralWidget( stack );

    contents = new MediaRecorderBase( stack );
    stack->addWidget( contents, 0 );

    config = new ConfigureRecorder( qualities, recorderPlugins, stack );
    stack->addWidget( config, 1 );

    initializeContents();
    stack->raiseWidget( contents );

    audio = 0;
#ifdef RECORD_THEN_SAVE
    samples = 0;
#else
    sampleBuffer = 0;
#endif
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
    connect( config->done, SIGNAL( clicked() ),
	     this, SLOT( configureDone() ) );
    connect( lightTimer, SIGNAL( timeout() ),
	     this, SLOT( recordLightBlink() ) );
}


MediaRecorder::~MediaRecorder()
{
    delete recorderPlugins;
    delete playerPlugins;
    if ( audio )
	delete audio;
#ifdef RECORD_THEN_SAVE
    if ( samples )
	delete samples;
#else
    if ( sampleBuffer )
	delete[] sampleBuffer;
#endif
    if ( io )
	delete io;
    if ( lightTimer )
	delete lightTimer;
}


void MediaRecorder::initializeContents()
{
    // Set the waveform background to black.
    contents->waveform->setBackgroundColor ( black );

    // The progress bar initially has no time display.  This will be fixed
    // up when we start recording.
    contents->progress->setTotalSteps( 10 );
    contents->progress->setProgress( -1 );

    // Cannot replay yet, because there is no recorded sound.
    contents->replayButton->setEnabled( FALSE );
    // TODO - unhide the button once we have real replay functionality
    contents->replayButton->hide();

    // Load the initial quality settings.
    contents->qualityGroup->setButton( config->currentQuality() );
    setQualityDisplay( qualities[config->currentQuality()] );
    connect( contents->qualityGroup, SIGNAL( clicked(int) ),
	     this, SLOT( qualityChanged(int) ) );

    // Populate the combo box with the list of storage locations.
    loadDocPaths();
    connect( storage, SIGNAL( disksChanged() ), this, SLOT( disksChanged() ) );
    connect( contents->storage, SIGNAL( activated(int) ),
	     this, SLOT( storageChanged(int) ) );
    recomputeMaxTime();
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
    const FileSystem *fs = storage->fileSystemOf(currentDocPath);
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


void MediaRecorder::loadDocPaths()
{
    // Clear the list of document paths.
    docPaths.clear();
    contents->storage->clear();

    // Add the primary home document store.
    const FileSystem *filesys = storage->fileSystemOf( homeDocuments );
    if ( filesys )
	contents->storage->insertItem( filesys->name() );
    else
	contents->storage->insertItem( tr("Documents") );
    docPaths += homeDocuments;

    // Add all removeable filesystems.
    const QList<FileSystem> &fs = storage->fileSystems();
    QListIterator<FileSystem> it( fs );
    for ( ; it.current(); ++it ) {
	if ( (*it)->isRemovable() ) {
	    contents->storage->insertItem( (*it)->name() );
	    docPaths += (*it)->path() + "/Documents";
	}
    }
}


void MediaRecorder::qualityChanged( int id )
{
    config->setQuality( id );
    config->saveConfig();
    setQualityDisplay( qualities[id] );
    recomputeMaxTime();
}


void MediaRecorder::disksChanged()
{
    storage->update();
    currentDocPath = homeDocuments;
    loadDocPaths();
    recomputeMaxTime();
}


void MediaRecorder::storageChanged( int index )
{
    storage->update();
    currentDocPath = docPaths[index];
    recomputeMaxTime();
}


void MediaRecorder::startSave()
{
    // Find the plugin to use to save the data.
    encoder = recorderPlugins->fromType
	( qualities[config->currentQuality()].mimeType,
	  qualities[config->currentQuality()].formatTag );

    // Open the document.
    DocLnk doc;
    FileManager fm;
    QString name = tr("Recorded Sound");
    name = name + " " + TimeString::timeString( QTime::currentTime() );
    name = name + " " + TimeString::numberDateString( QDate::currentDate() );
    doc.setName( name );
    doc.setType( encoder->pluginMimeType() );
    doc.setLocation( currentDocPath );

    io = fm.saveFile( doc );

    // Write the sample data using the encoder.
    encoder->begin( io, qualities[config->currentQuality()].formatTag );
    encoder->setAudioChannels( audio->channels() );
    encoder->setAudioFrequency( audio->frequency() );

    // Record the location of the file that we are saving.
    lastSaved = doc.file();
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
    QualitySetting *qual = &(qualities[config->currentQuality()]);

    // Bail out if we don't have a plugin for the selected format.
    if( recorderPlugins->fromType( qual->mimeType, qual->formatTag ) == 0) {
    	noPluginError();
	return;
    }

    // Disable power save while recording so that the device
    // doesn't suspend while a long-term recording session
    // is in progress.
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopEnvelope( "QPE/System", "setScreenSaverMode(int)" ) 
	    << QPEApplication::DisableSuspend;
#endif

    // Open the audio input device and normalize the parameters.
    if ( audio )
	delete audio;
    audio = new AudioInput( qual->frequency, qual->channels );
    qDebug( "channels: %d", audio->channels() );
    qDebug( "frequency: %d", audio->frequency() );
    qDebug( "bufferSize: %d", audio->bufferSize() );
    connect( audio, SIGNAL( dataAvailable() ), this, SLOT( processAudioData() ) );

    // Create the sample buffer, for recording the data temporarily.
#ifdef RECORD_THEN_SAVE
    if ( samples )
	delete samples;
    samples = new SampleBuffer( audio->bufferSize() );
#else
    if ( sampleBuffer )
	delete[] sampleBuffer;
    sampleBuffer = new short [ audio->bufferSize() ];
#endif

    // Start the save process.
    startSave();

    // Create the waveform display.
    contents->waveform->changeSettings( audio->frequency(), audio->channels() );

    configureAction->setEnabled( FALSE );
    contents->qualityGroup->setEnabled( FALSE );
    contents->storage->setEnabled( FALSE );
    recordTime = 0;
    contents->progress->setTotalSteps( 120 );
    contents->progress->setProgress( 0 );
    recording = TRUE;
    contents->recordButton->setText( tr("Stop") );
    contents->recordButton->setEnabled( TRUE );
    contents->replayButton->setEnabled( FALSE );

    // Turn on the recording light.
    setRecordLight( TRUE );

    // Some audio devices may start sending us data immediately, but
    // others may need an initial "read" to start the ball rolling.
    // Processing at least one data block will prime the device.
    audio->start();
    processAudioData();
}


void MediaRecorder::stopRecording()
{
    audio->stop();
    contents->waveform->reset();
    configureAction->setEnabled( TRUE );
    contents->qualityGroup->setEnabled( TRUE );
    contents->storage->setEnabled( TRUE );
    contents->recordButton->setEnabled( FALSE );
    recording = FALSE;
    contents->recordButton->setText( tr("Record") );
    contents->replayButton->setEnabled( TRUE );

    // Turn off the recording light.
    setRecordLight( FALSE );

    // Terminate the data save.
    endSave();

    // Re-enable power save.
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopEnvelope( "QPE/System", "setScreenSaverMode(int)" )
    	<< QPEApplication::Enable;
#endif
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
    MediaPlayerDecoder *decoder;

    // Do we have a decoder for this type of file?
    decoder = playerPlugins->fromFile( lastSaved );
    if ( decoder == 0 ) {
        QMessageBox::critical( this, tr( "No playback plugin found" ),
			       tr( "Voice Recorder was unable to\n"
			           "locate a suitable plugin to\n"
			           "play the selected format." ) );
	return;
    }

    // Reconfigure the UI to reflect the current mode.
    configureAction->setEnabled( FALSE );
    contents->qualityGroup->setEnabled( FALSE );
    contents->storage->setEnabled( FALSE );
    recordTime = 0;
    contents->progress->setTotalSteps( 120 );
    contents->progress->setProgress( 0 );
    playing = TRUE;
    contents->recordButton->setEnabled( FALSE );
    contents->replayButton->setText( tr("Stop") );
    contents->replayButton->setEnabled( TRUE );

    // TODO: start playing back the recorded sound in "lastSave".
}


void MediaRecorder::stopPlaying()
{
    // TODO: stop playing back the recorded sound

    contents->waveform->reset();
    configureAction->setEnabled( TRUE );
    contents->qualityGroup->setEnabled( TRUE );
    contents->storage->setEnabled( TRUE );
    contents->recordButton->setEnabled( TRUE );
    playing = FALSE;
    contents->replayButton->setEnabled( TRUE );
    contents->replayButton->setText( tr("Play") );
}


void MediaRecorder::replayClicked()
{
    if ( playing ) {
	stopPlaying();
    } else {
	startPlaying();
    }
}


void MediaRecorder::clearData()
{
#ifdef RECORD_THEN_SAVE
    samples->clear();
#endif
    contents->waveform->reset();
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
	result = audio->read( buf, length );
	samples->commitWriteBuffer( (unsigned int)result );

	// Update the waveform display.
	contents->waveform->newSamples( buf, result );
    } else {
	// The sample buffer is out of space, so stop recording.
	stopRecording();
	stopped = TRUE;
    }
#else
    result = audio->read( sampleBuffer, audio->bufferSize() );
    while ( result > 0 ) {
	contents->waveform->newSamples( sampleBuffer, result );
	encoder->writeAudioSamples( sampleBuffer, (long)result );
	result = audio->read( sampleBuffer, audio->bufferSize() );
    }
#endif

    // Update the record time if another second has elapsed.
    newTime = audio->position() / (long)(audio->frequency());
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
    config->setQuality( config->currentQuality() );
    stack->raiseWidget( config );
    configureAction->setEnabled( FALSE );
}


void MediaRecorder::configureDone()
{
    setQualityDisplay( qualities[config->currentQuality()] );
    recomputeMaxTime();
    stack->raiseWidget( contents );
    configureAction->setEnabled( TRUE );
}


void MediaRecorder::noPluginError()
{
    QMessageBox::critical( this, tr( "No plugin found" ),
			   tr( "Voice Recorder was unable to\n"
			       "locate a suitable plugin to\n"
			       "record in the selected format." ) );
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
        QCopEnvelope( "Qt/Tray", "remove(int)" )
		<< RECORD_LIGHT_ID;
    }
}


void MediaRecorder::closeEvent( QCloseEvent *e )
{
    // Shut down recording or playback.
    if ( recording ) {
	stopRecording();
    } else if ( playing ) {
	stopPlaying();
    }

    // Should we exit the app, or just the configuration pane.
    if ( configureAction->isEnabled() ) {
	QMainWindow::closeEvent(e);
    } else {
	configureDone();
    }
}

