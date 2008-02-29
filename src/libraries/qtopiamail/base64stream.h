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
#ifndef __BASE64STREAM_H
#define __BASE64STREAM_H

#include <QByteArray>
#include <qtopiaglobal.h>

class QTextStream;
class QDataStream;

class QTOPIAMAIL_EXPORT Base64Stream
{
public:
    static const int LINE_LENGTH = 76;
public:
    Base64Stream();
    ~Base64Stream(){};
    bool decode(QTextStream* ts, QDataStream* ds, bool complete = true);
    bool decodeContinue();
    void encode(QDataStream* instream, QTextStream* outstream, bool complete = true);
    void encodeContinue(bool endofdata = false);
private:
    int parse64base(char *src, char *bufOut);
    quint8 convertChar(char c)
    {
        if ( (int) c >= 'A' && (int) c <= 'Z')
            return (int) c - (int) 'A';
        else if ( (int) c >= 'a' && (int) c <= 'z')
            return (int) c - (int) 'a' + 26;
        else if ( (int) c >= '0' && (int) c <= '9')
            return (int) c - (int) '0' + 52;
        else if (c == '+')
            return 62;
        else if (c == '/')
            return 63;
        else if (c == '=')
            return 64;
        else return 65;
    }
private:
    QTextStream* _decodeInStream;
    QDataStream* _decodeOutStream;
    QDataStream* _encodeInStream;
    QTextStream* _encodeOutStream;
    QByteArray _inBuffer;
    QByteArray _outBuffer;
    bool _completeDecode, _needData, _completeEncode;
    int _in,_out;
};

#endif
