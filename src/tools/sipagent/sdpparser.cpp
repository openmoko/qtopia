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

#include "sdpparser.h"

// Based on RFC-2327, "Session Description Protocol".

SdpParser::SdpParser()
{
}

SdpParser::SdpParser( const QString& msg )
{
    parse( msg );
}

SdpParser::~SdpParser()
{
}

void SdpParser::parse( const QString& msg )
{
    lines = msg.split( QChar('\n') );
}

QString SdpParser::value( QChar tag ) const
{
    QStringList::ConstIterator it;
    for ( it = lines.begin(); it != lines.end(); ++it ) {
        if ( (*it).length() >= 2 &&
             (*it)[0] == tag && (*it)[1] == QChar('=') ) {
            return (*it).mid(2);
        }
    }
    return QString();
}

QString SdpParser::media( const QString& name ) const
{
    QStringList::ConstIterator it;
    QString prefix = "m=" + name;
    for ( it = lines.begin(); it != lines.end(); ++it ) {
        if ( (*it).startsWith( prefix ) ) {
            QString rest = (*it).mid( prefix.length() );
            if ( rest.isEmpty() || rest[0] == QChar(' ') )
                return rest.simplified();
        }
    }
    return QString();
}

QString SdpParser::attribute( const QString& name, const QString& attr ) const
{
    // Find the "a=" line within the specified media block.
    QStringList::ConstIterator it;
    QString prefix = "m=" + name;
    QString aprefix = "a=" + attr;
    for ( it = lines.begin(); it != lines.end(); ++it ) {
        if ( (*it).startsWith( prefix ) ) {
            QString rest = (*it).mid( prefix.length() );
            if ( rest.isEmpty() || rest[0] == QChar(' ') ) {
                ++it;
                for ( ; it != lines.end(); ++it ) {
                    if ( (*it).startsWith( "m=" ) || (*it).startsWith( "v=" ) )
                        break;
                    else if ( (*it).startsWith( aprefix ) ) {
                        rest = (*it).mid( aprefix.length() );
                        if ( rest.isEmpty() || rest[0] == QChar(' ') )
                            return rest.simplified();
                    }
                }
                break;
            }
        }
    }
    return QString();
}

int SdpParser::port( const QString& name ) const
{
    QStringList list = media( name ).split( QChar(' ') );
    if ( list.size() >= 1 ) {
        QString p = list[0];
        int index = p.indexOf( QChar('/') );
        if ( index < 0 )
            return p.toInt();
        else
            return p.left(index).toInt();
    } else {
        return -1;
    }
}

QString SdpParser::transport( const QString& name ) const
{
    QStringList list = media( name ).split( QChar(' ') );
    if ( list.size() >= 2 )
        return list[1];
    else
        return QString();
}

QStringList SdpParser::codecs( const QString& name ) const
{
    QStringList list = media( name ).split( QChar(' ') );
    QStringList codecs;
    QString codecName;
    if ( name != "audio" )      // We only do audio codecs at present.
        return QStringList();
    for ( int index = 2; index < list.size(); ++index ) {
        int num = list[index].toInt();
        codecName = "unknown";
        switch ( num ) {
            // These numbers come from RFC-3551.
            case 0:         codecName = "PCMU/8000"; break;
            case 3:         codecName = "GSM/8000"; break;
            case 8:         codecName = "PCMA/8000"; break;
            case 10:        codecName = "L16/44100/2"; break;
            case 11:        codecName = "L16/44100"; break;

            default:
            {
                // Check for dynamically-assigned payload types.
                if ( num >= 96 && num <= 127 ) {
                    codecName = attribute( name, "rtpmap:" + list[index] );
                }
            }
            break;
        }
        codecs += codecName;
    }
    return codecs;
}

int SdpParser::payloadType( const QString& name, const QString& codec ) const
{
    QStringList list = media( name ).split( QChar(' ') );
    QStringList allCodecs = codecs( name );
    int index = allCodecs.indexOf( codec );
    if ( index < 0 || ( index + 2 ) >= list.size() )
        return -1;
    else
        return list[index + 2].toInt();
}

QString SdpParser::connection( const QString& name ) const
{
    // Find the "c=" line within the specified media block,
    // or use the common one within the session block.
    QStringList::ConstIterator it;
    QString prefix = "m=" + name;
    QString common;
    for ( it = lines.begin(); it != lines.end(); ++it ) {
        if ( (*it).startsWith( "c=" ) ) {
            common = (*it).mid(2);
        } else if ( (*it).startsWith( prefix ) ) {
            QString rest = (*it).mid( prefix.length() );
            if ( rest.isEmpty() || rest[0] == QChar(' ') ) {
                ++it;
                for ( ; it != lines.end(); ++it ) {
                    if ( (*it).startsWith( "m=" ) || (*it).startsWith( "v=" ) )
                        break;
                    else if ( (*it).startsWith( "c=" ) )
                        return (*it).mid(2);
                }
                return common;
            }
        }
    }
    return common;
}

SdpParser::Protocol SdpParser::protocol( const QString& conn )
{
    if ( conn.startsWith( "IN IP4 " ) )
        return IPv4;
    else if ( conn.startsWith( "IN IP6 " ) )
        return IPv4;
    else
        return Unknown;
}

QString SdpParser::address( const QString& conn )
{
    QStringList args = conn.split( QChar(' ') );
    if ( args.size() < 3 )
        return QString();
    QString host = args[2];
    int index = host.indexOf( QChar('/') );
    if ( index < 0 )
        return host;
    else
        return host.left( index );
}

QString SdpParser::format( const QString& sessionName,
                           Protocol addressType,
                           const QString& address,
                           int port,
                           const QStringList& audioCodecs,
                           QList<int>& audioPayloadTypes )
{
    QString conn;
    QString media;
    QString attrs;
    QString sdp;

    // Format the connection information.
    if ( addressType == IPv6 )
        conn = "IN IP6 " + address;
    else
        conn = "IN IP4 " + address;

    // Add the SDP version information.
    sdp += "v=0\r\n";

    // Add owner/session information.
    sdp += "o=- 0 0 " + conn + "\r\n";
    sdp += "s=" + sessionName + "\r\n";

    // Add connection details.
    sdp += "c=" + conn + "\r\n";

    // Indicate the time the session should be active (i.e. "now").
    sdp += "t=0 0\r\n";

    // Encode the audio codec information.
    media = "m=audio " + QString::number( port ) + " RTP/AVP";
    QStringList::ConstIterator it;
    int dynamic = 96;
    audioPayloadTypes.clear();
    for ( it = audioCodecs.begin(); it != audioCodecs.end(); ++it ) {
        QString codec = (*it).toUpper();
        if ( codec == "PCMU/8000" ) {
            media += " 0";
            attrs += "a=rtpmap:0 PCMU/8000\r\n";
            audioPayloadTypes += 0;
        } else if ( codec == "GSM/8000" ) {
            media += " 3";
            attrs += "a=rtpmap:3 GSM/8000\r\n";
            audioPayloadTypes += 3;
        } else if ( codec == "PCMA/8000" ) {
            media += " 8";
            attrs += "a=rtpmap:8 PCMA/8000\r\n";
            audioPayloadTypes += 8;
        } else if ( codec == "L16/44100/2" ) {
            media += " 10";
            attrs += "a=rtpmap:10 L16/44100/2\r\n";
            audioPayloadTypes += 10;
        } else if ( codec == "L16/44100" || codec == "L16/44100/1" ) {
            media += " 11";
            attrs += "a=rtpmap:11 L16/44100\r\n";
            audioPayloadTypes += 11;
        } else {
            media += " " + QString::number( dynamic );
            attrs += "a=rtpmap:" + QString::number( dynamic ) + " " + *it + "\r\n";
            audioPayloadTypes += dynamic;
            ++dynamic;
        }
    }
    sdp += media + "\r\n";
    sdp += attrs;

    // Return the completed SDP packet.
    return sdp;
}
