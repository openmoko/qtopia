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

#ifndef SDPPARSER_H
#define SDPPARSER_H

#include <qstring.h>
#include <qstringlist.h>

class SdpParser
{
public:
    SdpParser();
    SdpParser( const QString& msg );
    ~SdpParser();

    // Parse an SDP message.
    void parse( const QString& msg );

    // Get the value associated with a specific tag.
    QString value( QChar tag ) const;

    // Get a specific media name.  e.g. "audio" or "video".
    QString media( const QString& name ) const;

    // Get the attribute value associated with a specific media name.
    QString attribute( const QString& name, const QString& attr ) const;

    // Get the port number for a media name.
    int port( const QString& media ) const;

    // Get the transport type for a media name.
    QString transport( const QString& name ) const;

    // Get the codec formats for a specific media name.
    // Will contain "unknown" for unrecognised codec types.
    QStringList codecs( const QString& name ) const;

    // Get the payload type code for a specific codec in a media description.
    // Returns -1 if the codec type is unknown.
    int payloadType( const QString& name, const QString& codec ) const;

    // Get the connection information to use for a specific media name.
    QString connection( const QString& name ) const;

    // Determine the protocol type of a connection string.
    enum Protocol { IPv4, IPv6, Unknown };
    static Protocol protocol( const QString& conn );

    // Get the address information from a connection string.
    static QString address( const QString& conn );

    // Format an outgoing SDP message.
    static QString format( const QString& sessionName,
                           Protocol addressType,
                           const QString& address,
                           int port,
                           const QStringList& audioCodecs,
                           QList<int>& audioPayloadTypes );

private:
    QStringList lines;
};

#endif /* SDPPARSER_H */
