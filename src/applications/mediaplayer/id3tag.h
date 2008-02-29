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
#ifndef ID3TAG_H
#define ID3TAG_H


#include <qobject.h>
#include <qstring.h>


class ID3Tag : public QObject {
    Q_OBJECT
public:
    ID3Tag();

    bool read( int fd );
    bool write( int fd );

    QString toString();

    const QCString &title() const { return idFields[0]; }
    const QCString &artist() const { return idFields[1]; }
    const QCString &album() const { return idFields[2]; }
    unsigned int year() const { return idFields[3].toUInt(); }
    const QCString &comment() const { return idFields[4]; }
    unsigned int track() const { return idFields[5].toUInt(); }

    void setTitle( const QCString& s ) { idFields[0] = s; }
    void setArtist( const QCString& s ) { idFields[1] = s; }
    void setAlbum( const QCString& s ) { idFields[2] = s; }
    void setYear( unsigned int y ) { idFields[3].sprintf("%i", y ); }
    void setComment( const QCString& s ) { idFields[4] = s; }
    void setTrack( unsigned int t ) { idFields[5].sprintf("%i", t ); }

private:
    QCString idFields[6];
};


#endif
