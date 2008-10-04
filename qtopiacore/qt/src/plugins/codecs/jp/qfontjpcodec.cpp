/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qfontjpcodec.h"

#include "qjpunicode.h"

QT_BEGIN_NAMESPACE

#ifdef Q_WS_X11
// JIS X 0201

QFontJis0201Codec::QFontJis0201Codec()
{
}

QByteArray QFontJis0201Codec::_name()
{
    return "jisx0201*-0";
}

int QFontJis0201Codec::_mibEnum()
{
    return 15;
}

QByteArray QFontJis0201Codec::convertFromUnicode(const QChar *uc, int len,  ConverterState *) const
{
    QByteArray rstring;
    rstring.resize(len);
    uchar *rdata = (uchar *) rstring.data();
    const QChar *sdata = uc;
    int i = 0;
    for (; i < len; ++i, ++sdata, ++rdata) {
        if (sdata->unicode() < 0x80) {
            *rdata = (uchar) sdata->unicode();
        } else if (sdata->unicode() >= 0xff61 && sdata->unicode() <= 0xff9f) {
            *rdata = (uchar) (sdata->unicode() - 0xff61 + 0xa1);
        } else {
            *rdata = 0;
        }
    }
    return rstring;
}

QString QFontJis0201Codec::convertToUnicode(const char *, int,  ConverterState *) const
{
    return QString();
}

// JIS X 0208

QFontJis0208Codec::QFontJis0208Codec()
{
    convJP = QJpUnicodeConv::newConverter(QJpUnicodeConv::Default);
}


QFontJis0208Codec::~QFontJis0208Codec()
{
    delete convJP;
    convJP = 0;
}


QByteArray QFontJis0208Codec::_name()
{
    return "jisx0208*-0";
}


int QFontJis0208Codec::_mibEnum()
{
    return 63;
}


QString QFontJis0208Codec::convertToUnicode(const char* /*chars*/, int /*len*/, ConverterState *) const
{
    return QString();
}

QByteArray QFontJis0208Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *) const
{
    QByteArray result;
    result.resize(len * 2);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc;

    for (int i = 0; i < len; i++) {
        QChar ch(*ucp++);
        ch = convJP->unicodeToJisx0208(ch.unicode());

        if (!ch.isNull()) {
            *rdata++ = ch.row();
            *rdata++ = ch.cell();
        } else {
            *rdata++ = 0;
            *rdata++ = 0;
        }
    }
    return result;
}

#endif

QT_END_NAMESPACE
