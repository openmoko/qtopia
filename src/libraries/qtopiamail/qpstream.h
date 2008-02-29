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
#ifndef __QPSTREAM_H
#define __QPSTREAM_H

#include <QString>
#include <qtopiaglobal.h>

class QTextStream;
class QDataStream;

class QTOPIAMAIL_EXPORT QPStream
{
public:
    static const int LINE_LENGTH = 76;
private:
    enum state { First, Second, None };
public:
    QPStream();
    ~QPStream(){};
    bool decode(QTextStream* instream, QDataStream* outstream, bool rfc2047 = false, bool complete = true);
    bool decodeContinue();
    static void encode(QDataStream& instream, QTextStream& outstream, bool rfc2047 = false);
private:
    void setState(state s)
    {
        _lastState = _state;
        _state = s;
    }
private:
    state _state;
    state _lastState;
    QTextStream* _decodeInStream;
    QDataStream* _decodeOutStream;
    QString _buffer;
    bool _completeDecode, _rfc2047;
};

#endif
