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
#include "qpstream.h"
#include <QTextStream>
#include <QDataStream>
#include <QDebug>

QPStream::QPStream()
    :
_state(None),
_lastState(None),
_decodeInStream(0),
_decodeOutStream(0),
_completeDecode(false),
_rfc2047(false)
{
}

bool QPStream::decode(QTextStream* instream, QDataStream* outstream, bool rfc2047, bool complete)
{
    _decodeInStream = instream;
    _decodeOutStream = outstream;
    _completeDecode = complete;
    _lastState = _state = None;
    _rfc2047 = rfc2047;

    return decodeContinue();
}

bool QPStream::decodeContinue()
{
    Q_ASSERT(_decodeInStream);
    Q_ASSERT(_decodeOutStream);
    char ch;
    if(_completeDecode)
        _lastState = _state = None;

    while(!_decodeInStream->atEnd() ) {
        (*_decodeInStream) >> ch;
        switch ( _state ) {
            case None:
                if ( ch == '=' )
                    setState(First);
                else if ( _rfc2047 ) {
                    if ( ch == '_' )
                        (*_decodeOutStream) << 0x20;    // _ is always hex 20 (q encoding rfc2047)
                    else
                        (*_decodeOutStream) << ch;
                } else
                    (*_decodeOutStream) << (quint8)ch;
                break;
            case First:
                if(_lastState != None)
                    return false;
                if ( ch == '\n' || ch == '\r' ) {
                    // soft line break
                    setState(None);
                } else {
                    _buffer += ch;
                    setState(Second);
                }
                break;
            case Second:
                if(_lastState != First)
                    return false;
                _buffer += ch;
                setState(None);
                bool ok;
                unsigned char b = _buffer.toInt( &ok, 16 );
                if ( ok )
                    (*_decodeOutStream) << (quint8)b;
                else
                    return false; //error
                _buffer = "";
                break;
        }
    }

    return true;
}

void QPStream::encode(QDataStream& instream, QTextStream& outstream, bool rfc2047)
{
    uint lineLen = 0;
    quint8 ch;

    QString result;
    bool lastWasSpace = false;
    static char const hexchars[] = "0123456789ABCDEF";

    if ( !rfc2047 ) {
        while ( !instream.atEnd() ) {
                instream >> ch;
            if ( ch < 32 || ch >= 127 || ch == '=' ) {
                if ((lineLen+3) > (LINE_LENGTH-1) ) {
                    outstream << '=';
                    outstream << '\n';
                    lineLen = 0;
                }
                outstream << '=';
                outstream <<  hexchars[(ch >> 4) & 0x0F];
                outstream <<  hexchars[ch & 0x0F];
                lineLen += 3;
                lastWasSpace = false;
            } else if ( ch == ' ' ) {
                if ( (lineLen+1) > (LINE_LENGTH-1) ) {
                    outstream << '=';
                    outstream << '\n';
                    lineLen = 0;
                }
                outstream << (char)ch;
                ++lineLen;
                lastWasSpace = true;
            } else {
                if ( (lineLen+1) > (LINE_LENGTH-1) ) {
                    outstream << '=';
                    outstream << '\n';
                    lineLen = 0;
                }
                outstream << (char)ch;
                ++lineLen;
                lastWasSpace = false;
            }
        }
    } else {
        while ( !instream.atEnd() ) {
            instream >> ch;
            if ( ch < 0x20 || ch >= 0x7F || ch == '=' || ch == '?' ) {
                outstream << '=';
                outstream <<  hexchars[(ch >> 4) & 0x0F];
                outstream <<  hexchars[ch & 0x0F];
            } else if ( ch == ' ' ) {
                outstream <<  '_';
            } else {
                outstream << (char)ch;
            }
        }
    }

    outstream << flush;
}
