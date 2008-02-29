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

#include "confrecorder.h"
#include "mediarecorder.h"
#include "pluginlist.h"

#include <qcombobox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>


// Default quality settings, if not yet set in the configuration.
static QualitySetting DefaultQualities[] = {
    {11025, 1, "audio/x-wav", "pcm"},
    {22050, 2, "audio/x-wav", "pcm"},
    {44100, 2, "audio/x-wav", "pcm"},
    {11025, 1, "audio/x-wav", "gsm"},
};

// Section names within the configuration file.
static const char * const ConfigSections[MaxQualities] = {
    "VoiceQuality", "MusicQuality", "CDQuality", "CustomQuality"
};


ConfigureRecorder::ConfigureRecorder( QualitySetting *_qualities, MediaRecorderPluginList *_plugins, QWidget *parent, const char *name, WFlags f )
    : ConfigureRecorderBase( parent, name, TRUE, f )
{
    qualities = _qualities;
    plugins = _plugins;

    // Load the default quality settings.
    int qual;
    for ( qual = 0; qual < MaxQualities; ++qual ) {
	qualities[qual].frequency = DefaultQualities[qual].frequency;
	qualities[qual].channels = DefaultQualities[qual].channels;
	qualities[qual].mimeType = DefaultQualities[qual].mimeType;
	qualities[qual].formatTag = DefaultQualities[qual].formatTag;
    }
#ifdef QTOPIA_PHONE
    quality = CustomQuality;
#else
    quality = VoiceQuality;
#endif

    // Load configuration overrides.
    loadConfig();

    // Populate the list of sample rates.
    sampleRate->insertItem( tr("8 kHz") );
    sampleRate->insertItem( tr("11 kHz") );
    sampleRate->insertItem( tr("22 kHz") );
    sampleRate->insertItem( tr("44 kHz") );

    // Populate the list of formats.
    uint numPlugins;
    uint plugin;
    MediaRecorderEncoder *encoder;
    QString formatName;
    numPlugins = plugins->count();
    for ( plugin = 0; plugin < numPlugins; ++plugin ) {
	encoder = plugins->at( plugin );
	formatName = plugins->formatNameAt( plugin );
	format->insertItem( formatName );
    }

#ifdef QTOPIA_PHONE
    // Don't display the quality group for the Phone Edition.
    // It only uses the "Custom" quality setting, for simplicity.
    qualityGroup->hide();
#endif

    // Hook up interesting signals.
    connect( qualityGroup, SIGNAL( clicked(int) ),
	     this, SLOT( setQuality(int) ) );
    connect( channelsGroup, SIGNAL( clicked(int) ),
	     this, SLOT( setChannels(int) ) );
    connect( sampleRate, SIGNAL( activated(int) ),
	     this, SLOT( setSampleRate(int) ) );
    connect( format, SIGNAL( activated(int) ),
	     this, SLOT( setFormat(int) ) );
}


ConfigureRecorder::~ConfigureRecorder()
{
}


static void copyQualities( QualitySetting *dest, QualitySetting *src, int num )
{
    while ( num > 0 ) {
	dest->frequency = src->frequency;
	dest->channels = src->channels;
	dest->mimeType = src->mimeType;
	dest->formatTag = src->formatTag;
	++dest;
	++src;
	--num;
    }
}

void ConfigureRecorder::processPopup()
{
    QualitySetting savedQualities[MaxQualities];
    int savedQuality;

    // Save the current quality settings, in case we have to cancel.
    copyQualities( savedQualities, qualities, MaxQualities );
    savedQuality = quality;

    // Update the configuration dialog's display with the current state.
    setQuality( quality );

    // Process the dialog.
    if ( QPEApplication::execDialog( this ) != QDialog::Accepted) {

	// Copy the saved configuration back.
	copyQualities( qualities, savedQualities, MaxQualities );
	quality = savedQuality;
    }
}


void ConfigureRecorder::setQuality( int index )
{
    // Set the quality value.
    quality = index;
    switch ( quality ) {

	case VoiceQuality:
	    voiceQuality->setChecked( TRUE );
	    break;

	case MusicQuality:
	    musicQuality->setChecked( TRUE );
	    break;

	case CDQuality:
	    cdQuality->setChecked( TRUE );
	    break;

	case CustomQuality:
	    customQuality->setChecked( TRUE );
	    break;
    }

    // Set the number of channels.
    if ( qualities[quality].channels == 1) {
	monoChannels->setChecked( TRUE );
    } else {
	stereoChannels->setChecked( TRUE );
    }

    // Set the sample rate frequency.
    switch ( qualities[quality].frequency ) {

	case 8000:
	    sampleRate->setCurrentItem( 0 );
	    break;

	case 11025:
	    sampleRate->setCurrentItem( 1 );
	    break;

	case 22050:
	    sampleRate->setCurrentItem( 2 );
	    break;

	case 44100:
	    sampleRate->setCurrentItem( 3 );
	    break;
    }

    // Set the format.
    int formatIndex = plugins->indexFromType
	    ( qualities[quality].mimeType, qualities[quality].formatTag );
    if( formatIndex >= 0 ) {
	format->setCurrentItem( formatIndex );
    }
}


void ConfigureRecorder::setChannels( int index )
{
    updateConfig( index + 1, qualities[quality].frequency,
		  qualities[quality].mimeType, qualities[quality].formatTag );
}


void ConfigureRecorder::setSampleRate( int index )
{
    int frequency;

    switch ( index ) {

	case 0:	    frequency = 8000; break;
	case 1:	    frequency = 11025; break;
	case 2:	    frequency = 22050; break;
	default:    frequency = 44100; break;

    }

    updateConfig( qualities[quality].channels, frequency,
		  qualities[quality].mimeType, qualities[quality].formatTag );
}


void ConfigureRecorder::setFormat( int index )
{
    updateConfig( qualities[quality].channels,
		  qualities[quality].frequency,
		  plugins->at( (uint)index )->pluginMimeType(),
		  plugins->formatAt( (uint)index ) );
}


void ConfigureRecorder::resetQuality()
{
    updateConfig( DefaultQualities[quality].channels,
		  DefaultQualities[quality].frequency,
		  DefaultQualities[quality].mimeType,
		  DefaultQualities[quality].formatTag );
    setQuality( quality );
}


void ConfigureRecorder::updateConfig( int channels, int frequency, const QString& mimeType, const QString& formatTag )
{
    if ( channels != qualities[quality].channels ) {
	qualities[quality].channels = channels;
    }

    if ( frequency != qualities[quality].frequency ) {
	qualities[quality].frequency = frequency;
    }

    if ( mimeType != qualities[quality].mimeType ) {
	qualities[quality].mimeType = mimeType;
    }

    if ( formatTag != qualities[quality].formatTag ) {
	qualities[quality].formatTag = formatTag;
    }
}


void ConfigureRecorder::loadConfig()
{
    int qual, value;
    QString qvalue;
    QString svalue;
    QString fvalue;
    int index;

    Config cfg( "MediaRecorder" );

    cfg.setGroup( "Options" );
    qvalue = cfg.readEntry( "Quality" );

    for ( qual = 0; qual < MaxQualities; ++qual ) {

	if ( qvalue == ConfigSections[qual] ) {
	    quality = qual;
	}

	cfg.setGroup( ConfigSections[qual] );

	value = cfg.readNumEntry( "Channels" );
	if ( value == 1 || value == 2 ) {
	    qualities[qual].channels = value;
	}

	value = cfg.readNumEntry( "Frequency" );
	if ( value == 8000 || value == 11025 ||
	     value == 22050 || value == 44100 ) {
	    qualities[qual].frequency = value;
	}

	svalue = cfg.readEntry( "Type" );
	fvalue = cfg.readEntry( "Format" );
	if ( svalue == QString::null ) {
	    svalue = qualities[qual].mimeType;
	}
	if ( fvalue == QString::null ) {
	    fvalue = qualities[qual].formatTag;
	}

	index = plugins->indexFromType( svalue, fvalue );
	if( index >= 0 ) {
	    qualities[qual].mimeType = plugins->at( index )->pluginMimeType();
	    qualities[qual].formatTag = plugins->formatAt( index );
	} else {
	    qualities[qual].mimeType = svalue;
	    qualities[qual].formatTag = fvalue;
	}
    }
}


void ConfigureRecorder::saveConfig()
{
    int qual;

    // Write out only what is different from the defaults, which
    // makes it easier to migrate to new versions of the app
    // that change the defaults to something better.

    Config cfg( "MediaRecorder" );

    cfg.setGroup( "Options" );
    if ( quality != VoiceQuality ) {
	cfg.writeEntry( "Quality", ConfigSections[quality] );
    } else {
	cfg.removeEntry( "Quality" );
    }

    for ( qual = 0; qual < MaxQualities; ++qual ) {

	cfg.setGroup( ConfigSections[qual] );
	cfg.clearGroup();

	if ( qualities[qual].channels != DefaultQualities[qual].channels ) {
	    cfg.writeEntry( "Channels", qualities[qual].channels );
	}

	if ( qualities[qual].frequency != DefaultQualities[qual].frequency ) {
	    cfg.writeEntry( "Frequency", qualities[qual].frequency );
	}

	if ( qualities[qual].mimeType != DefaultQualities[qual].mimeType ) {
	    cfg.writeEntry( "Type", qualities[qual].mimeType );
	}

	if ( qualities[qual].formatTag != DefaultQualities[qual].formatTag ) {
	    cfg.writeEntry( "Format", qualities[qual].formatTag );
	}
    }
}

