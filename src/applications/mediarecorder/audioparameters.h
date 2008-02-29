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

#ifndef AUDIO_PARAMETERS_H
#define AUDIO_PARAMETERS_H

// Qt includes
#include <QString>

// Qtopia includes
#include <QMimeType>

// Forward class declarations
class QDataStream;

// ============================================================================
//
// AudioParameters class
//
// ============================================================================

class AudioParameters
{
public:
    AudioParameters();
    AudioParameters( const QMimeType& mimeType,
                     const QString&   subFormat,
                     const int        frequency,
                     const int        channels );

    const QMimeType& mimeType() const;
    QString          subFormat() const;
    int              frequency() const;
    int              channels() const;

    void setMimeType( const QMimeType& mimeType );
    void setSubFormat( const QString& subFormat );
    void setFrequency( int frequency );
    void setChannels( int channels );

private:
    QMimeType mMimeType;
    QString   mSubFormat;
    int       mFrequency;
    int       mChannels;
};

QDataStream& operator>>( QDataStream& stream, AudioParameters& parameters );
QDataStream& operator<<( QDataStream& stream, const AudioParameters& parameters );

#endif //AUDIO_PARAMETERS_H
