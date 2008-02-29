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
#include "base64stream.h"
#include <QTextStream>
#include <QDataStream>

Base64Stream::Base64Stream()
:
_decodeInStream(0),
_decodeOutStream(0),
_encodeInStream(0),
_encodeOutStream(0),
_completeDecode(true),
_needData(false),
_completeEncode(true)
{
    _inBuffer.resize(4);
    _outBuffer.resize(3);
}

bool Base64Stream::decode(QTextStream* instream, QDataStream* outstream, bool complete)
{
    _decodeInStream = instream;
    _decodeOutStream = outstream;
    _completeDecode = complete;
    _needData = false;

    if(!decodeContinue())
        return false;
    else if(_completeDecode && _needData)
        return false;
    else return true;

}

bool Base64Stream::decodeContinue()
{
    Q_ASSERT(_decodeInStream);
    Q_ASSERT(_decodeOutStream);

    while(!_decodeInStream->atEnd())
    {
        if(!_needData)
            _in = _out = 0;
        do
        {
            char c;
            (*_decodeInStream) >> c;

            if(c != '\r' && c != '\n')
            {
                quint8 v = convertChar(c);
                if(v > 64)
                    return false;
                _inBuffer[_in] = v;
                ++_in;
            }
        }
        while(_in < 4 && !_decodeInStream->atEnd());

        if(_in == 0)
            break;
        if(_in < 4)
        {
                _needData = true;
                return true;
        }
        else
            _needData = false;

        //decode

        _out = parse64base(_inBuffer.data(),_outBuffer.data());

        //write out

        for(int x = 0; x < _out; ++x)
            (*_decodeOutStream) << (quint8)_outBuffer[x];
    }

    if(_needData)
        return false;
    else
        return true;
}

int Base64Stream::parse64base(char *src, char *bufOut)
{
    char z;
    int processed;

    processed = 1;
    bufOut[0] = (char) src[0] & (32+16+8+4+2+1); //mask out top 2 bits
    bufOut[0] <<= 2;
    z = src[1] >> 4;
    bufOut[0] = bufOut[0] | z;          //first byte retrived

    if (src[2] != 64) {
        bufOut[1] = (char) src[1] & (8+4+2+1);   //mask out top 4 bits
        bufOut[1] <<= 4;
        z = src[2] >> 2;
        bufOut[1] = bufOut[1] | z;              //second byte retrived
        processed++;

        if (src[3] != 64) {
            bufOut[2] = (char) src[2] & (2+1);   //mask out top 6 bits
            bufOut[2] <<= 6;
            z = src[3];
            bufOut[2] = bufOut[2] | z;  //third byte retrieved
            processed++;
        }
    }

    return processed;
}

void Base64Stream::encode(QDataStream* instream, QTextStream* outstream, bool complete)
{
    _completeEncode = complete;
    _encodeInStream = instream;
    _encodeOutStream = outstream;
    encodeContinue(_completeEncode);
}

void Base64Stream::encodeContinue(bool endOfData)
{
    Q_ASSERT(_encodeInStream);
    Q_ASSERT(_encodeOutStream);

    int count = 0;
    int totalCount = 0;
    unsigned char inBuffer[3];

    while(!_encodeInStream->atEnd())
    {
        count = 0;

        do
        {
            quint8 b;
            (*_encodeInStream) >> b;
            inBuffer[count] = b;
            ++count;
        }
        while(count < 3 && !_encodeInStream->atEnd());

        bool atEnd = _encodeInStream->atEnd() && endOfData;

        static const char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        for(int i = 0; i < 4; ++i)
        {
            if(totalCount && (totalCount % LINE_LENGTH) == 0)
                (*_encodeOutStream) << '\n';

            int byte = (i * 6) / 8;
            int bits = (i * 6) % 8;
            int index = 0;
            if(byte >= count)
                break;

            if (bits < 3)
            {
                index = (inBuffer[byte] >> (2 - bits)) & 0x3F;
                (*_encodeOutStream) << b64[index];
            }
            else
            {
                if (((byte + 1) == count) && atEnd ) //last in byte
                {
                    index = ((inBuffer[byte] << (bits - 2)) & 0x3F);
                    (*_encodeOutStream) << b64[index];
                } else
                {
                    index = ((inBuffer[byte] << (bits - 2)) | (inBuffer[byte + 1] >> (10 - bits))) & 0x3F;
                    (*_encodeOutStream) << b64[index];
                }
            }
            ++totalCount;
        }

        if(atEnd)
        {
            for(int i = 3 - count; i > 0; --i)
                (*_encodeOutStream) << '=';
        }
    }

    (*_encodeOutStream) << flush;
}
