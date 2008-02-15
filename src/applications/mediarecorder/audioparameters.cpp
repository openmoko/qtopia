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

// Local includes
#include "audioparameters_p.h"

// ============================================================================
//
// AudioParameters class
//
// ============================================================================

AudioParameters::AudioParameters()
:   mMimeType( QString() ),
    mSubFormat(),
    mFrequency( 0 ),
    mChannels( 0 )
{
}

AudioParameters::AudioParameters( const QMimeType& mimeType,
                                  const QString&   subFormat,
                                  const int        frequency,
                                  const int        channels )
:   mMimeType( mimeType ),
    mSubFormat( subFormat),
    mFrequency( frequency ),
    mChannels( channels )
{
}

const QMimeType& AudioParameters::mimeType() const
{
    return mMimeType;
}

QString AudioParameters::subFormat() const
{
    return mSubFormat;
}

int AudioParameters::frequency() const
{
    return mFrequency;
}

int AudioParameters::channels() const
{
    return mChannels;
}

void AudioParameters::setMimeType( const QMimeType& mimeType )
{
    mMimeType = mimeType;
}

void AudioParameters::setSubFormat( const QString& subFormat )
{
    mSubFormat = subFormat;
}

void AudioParameters::setFrequency( int frequency )
{
    mFrequency = frequency;
}

void AudioParameters::setChannels( int channels )
{
    mChannels = channels;
}

QDataStream& operator>>( QDataStream& stream, AudioParameters& parameters )
{
    QString mimeType;
    QString subFormat;
    int frequency;
    int channels;

    stream >> mimeType >> subFormat >> frequency >> channels;

    parameters.setMimeType( QMimeType( mimeType ) );
    parameters.setSubFormat( subFormat );
    parameters.setFrequency( frequency );
    parameters.setChannels( channels );

    return stream;
}

QDataStream& operator<<( QDataStream& stream, const AudioParameters& parameters )
{
    stream << parameters.mimeType().id()
           << parameters.subFormat()
           << parameters.frequency()
           << parameters.channels();

    return stream;
}

