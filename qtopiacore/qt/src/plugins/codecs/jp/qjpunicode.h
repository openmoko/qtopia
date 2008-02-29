/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Most of the code here was originally written by Serika Kurusugawa
// a.k.a. Junji Takagi, and is included in Qt with the author's permission,
// and the grateful thanks of the Trolltech team.

/*
 * Copyright (C) 1999 Serika Kurusugawa, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef QJPUNICODE_H
#define QJPUNICODE_H

#include <QtCore/qglobal.h>

class QJpUnicodeConv {
public:
    virtual ~QJpUnicodeConv() {}
    enum Rules {
        // "ASCII" is ANSI X.3.4-1986, a.k.a. US-ASCII here.
        Default                        = 0x0000,

        Unicode                        = 0x0001,
        Unicode_JISX0201                = 0x0001,
        Unicode_ASCII                 = 0x0002,
        JISX0221_JISX0201         = 0x0003,
        JISX0221_ASCII                = 0x0004,
        Sun_JDK117                     = 0x0005,
        Microsoft_CP932                = 0x0006,

        NEC_VDC                = 0x0100,                // NEC Vender Defined Char
        UDC                        = 0x0200,                // User Defined Char
        IBM_VDC                = 0x0400                // IBM Vender Defined Char
    };
    static QJpUnicodeConv *newConverter(int rule);

    virtual uint asciiToUnicode(uint h, uint l) const;
    /*virtual*/ uint jisx0201ToUnicode(uint h, uint l) const;
    virtual uint jisx0201LatinToUnicode(uint h, uint l) const;
    /*virtual*/ uint jisx0201KanaToUnicode(uint h, uint l) const;
    virtual uint jisx0208ToUnicode(uint h, uint l) const;
    virtual uint jisx0212ToUnicode(uint h, uint l) const;

    uint asciiToUnicode(uint ascii) const {
        return asciiToUnicode((ascii & 0xff00) >> 8, (ascii & 0x00ff));
    }
    uint jisx0201ToUnicode(uint jis) const {
        return jisx0201ToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0201LatinToUnicode(uint jis) const {
        return jisx0201LatinToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0201KanaToUnicode(uint jis) const {
        return jisx0201KanaToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0208ToUnicode(uint jis) const {
        return jisx0208ToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0212ToUnicode(uint jis) const {
        return jisx0212ToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }

    virtual uint unicodeToAscii(uint h, uint l) const;
    /*virtual*/ uint unicodeToJisx0201(uint h, uint l) const;
    virtual uint unicodeToJisx0201Latin(uint h, uint l) const;
    /*virtual*/ uint unicodeToJisx0201Kana(uint h, uint l) const;
    virtual uint unicodeToJisx0208(uint h, uint l) const;
    virtual uint unicodeToJisx0212(uint h, uint l) const;

    uint unicodeToAscii(uint unicode) const {
        return unicodeToAscii((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0201(uint unicode) const {
        return unicodeToJisx0201((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0201Latin(uint unicode) const {
        return unicodeToJisx0201Latin((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0201Kana(uint unicode) const {
        return unicodeToJisx0201Kana((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0208(uint unicode) const {
        return unicodeToJisx0208((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0212(uint unicode) const {
        return unicodeToJisx0212((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }

    uint sjisToUnicode(uint h, uint l) const;
    uint unicodeToSjis(uint h, uint l) const;
    uint sjisibmvdcToUnicode(uint h, uint l) const;
    uint unicodeToSjisibmvdc(uint h, uint l) const;

    uint sjisToUnicode(uint sjis) const {
        return sjisToUnicode((sjis & 0xff00) >> 8, (sjis & 0x00ff));
    }
    uint unicodeToSjis(uint unicode) const {
        return unicodeToSjis((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }

protected:
    explicit QJpUnicodeConv(int r) : rule(r) {}

private:
    int rule;
};

#endif // QJPUNICODE_H
