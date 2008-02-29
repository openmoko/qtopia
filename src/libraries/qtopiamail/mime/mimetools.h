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

#ifndef MIME_TOOLS_H
#define MIME_TOOLS_H

#include "pstring.h"

class MimeAddressField;
class MimeBodyPart;

class MimeTools
{
public:
    static QByteArray decodeBase64( const PString &encoded );
    static PString decodeQuotedPrintable( const PString &s, bool RFC2047 = false );

    static PString encodeBase64(QByteArray fileData);

    static PString decodeLocale(const PString &str, PString &charset);
    static PString encodeLocale(const PString &str, PString &charset);

    static QDateTime parseDate(PString in);
    static PString buildDateString(QDateTime d);
    static PString formattedDateTimeString(QDateTime d);

    static PString defaultCharset();
    static void setDefaultCharset(const PString &);

    static PString join(QValueList<MimeAddressField> list);
    static QValueList<MimeAddressField> split(const PString &s);

    static bool isAttachment(MimeBodyPart *bp);

private:
    static int parse64base(char *src, char *bufOut);
    static PString decodeEncodedWord(const PString &str, PString &charset);
};

#endif
