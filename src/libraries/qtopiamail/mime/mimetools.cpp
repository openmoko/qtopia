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

#include "mimetools.h"
#include "mimeaddressfield.h"
#include "mimebodypart.h"

//changing timezones are slow, cache info in this var
static int utcTimeZoneDiff = -1;
static const PString Months("janfebmaraprmayjunjulaugsepoctnovdec");
static PString _defaultCharset("us-ascii");


PString MimeTools::defaultCharset()
{
    return _defaultCharset;
}

void MimeTools::setDefaultCharset(const PString &s)
{
    _defaultCharset = s;
}

PString MimeTools::decodeQuotedPrintable( const PString &s, bool RFC2047)
{
    PString decoded;
    int l = s.length();
//    decoded.resize( l ); // always enough
    int i = 0;
    const char *uc = s.data();
    PString quoted;
    enum { First, Second, None } state;
    state = None;
    while( i < l ) {
        switch ( state ) {
            case None:
                if ( uc[i] == '=' )
                    state = First;
                else if ( RFC2047 ) {
                    if ( uc[i] == '_' )
                        decoded += 0x20;    // _ is always hex 20 (q encoding rfc2047)
                    else
                        decoded += uc[i];
                } else
                    decoded += uc[i];
                break;
            case First:
                if ( uc[i] == '\n' || uc[i] == '\r' ) {
                    // soft line break
                    decoded += uc[i];
                    state = None;
                } else {
                    quoted += uc[i];
                    state = Second;
                }
                break;
            case Second:
                quoted += uc[i];
                state = None;
                bool ok;
                char ch = quoted.toInt( &ok, 16 );
                if ( ok )
                    decoded += ch;
                quoted = "";
                break;
        }
        i++;
    }
    return decoded;
}

int MimeTools::parse64base(char *src, char *bufOut)
{
    char c, z;
    char li[4];
    int processed;

    //conversion table withouth table...
    for (int x = 0; x < 4; x++) {
        c = src[x];

        if ( (int) c >= 'A' && (int) c <= 'Z')
            li[x] = (int) c - (int) 'A';
        if ( (int) c >= 'a' && (int) c <= 'z')
            li[x] = (int) c - (int) 'a' + 26;
        if ( (int) c >= '0' && (int) c <= '9')
            li[x] = (int) c - (int) '0' + 52;
        if (c == '+')
            li[x] = 62;
        if (c == '/')
            li[x] = 63;
    }

    processed = 1;
    bufOut[0] = (char) li[0] & (32+16+8+4+2+1); //mask out top 2 bits
    bufOut[0] <<= 2;
    z = li[1] >> 4;
    bufOut[0] = bufOut[0] | z;          //first byte retrived

    if (src[2] != '=') {
        bufOut[1] = (char) li[1] & (8+4+2+1);   //mask out top 4 bits
        bufOut[1] <<= 4;
        z = li[2] >> 2;
        bufOut[1] = bufOut[1] | z;              //second byte retrived
        processed++;

        if (src[3] != '=') {
            bufOut[2] = (char) li[2] & (2+1);   //mask out top 6 bits
            bufOut[2] <<= 6;
            z = li[3];
            bufOut[2] = bufOut[2] | z;  //third byte retrieved
            processed++;
        }
    }

    return processed;
}

QByteArray MimeTools::decodeBase64( const PString &encoded )
{
    QByteArray buffer;
    uint len = encoded.length();
    buffer.resize( len * 3 / 4 + 2);
    uint bufCount = 0;
    uint pos = 0, decodedCount = 0;
    char src[4];
    char *destPtr = buffer.data();

    while (pos < len ) {
        decodedCount = 4;
        int x = 0;
        while ( (x < 4) && (pos < len ) ) {
            src[x] = encoded[pos];
            pos++;
            if (src[x] == '\r' || src[x] == '\n' || src[x] == ' ')
                x--;
            x++;
        }
        if (x > 1) {
            decodedCount = parse64base(src, destPtr);
            destPtr += decodedCount;
            bufCount += decodedCount;
        }
    }

    if ( bufCount != buffer.size() )
        buffer.resize( bufCount );

    return buffer;
}

PString MimeTools::encodeBase64(QByteArray fileData)
{
    // follows simple algorithm from rsync code
    uchar *in = (uchar*)fileData.data();

    int inbytes = fileData.size();
    int outbytes = ((inbytes * 8) + 5) / 6;
    int spacing = (outbytes-1)/76;
    int padding = 4-outbytes%4; if ( padding == 4 ) padding = 0;

    QByteArray outbuf(outbytes+spacing+padding);
    uchar* out = (uchar*)outbuf.data();

    const char *b64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (int i = 0; i < outbytes; i++) {
        if ( i && i%76==0 )
            *out++ = '\n';

        int byte = (i * 6) / 8;
        int bit = (i * 6) % 8;
        if (bit < 3) {
            if (byte >= inbytes)
                abort();
            *out = (b64[(in[byte] >> (2 - bit)) & 0x3F]);
        } else {
            if (byte + 1 == inbytes) {
                *out = (b64[(in[byte] << (bit - 2)) & 0x3F]);
            } else {
                *out = (b64[(in[byte] << (bit - 2) |
                   in[byte + 1] >> (10 - bit)) & 0x3F]);
            }
        }
        ++out;
    }

    ASSERT(out == (uchar*)outbuf.data() + outbuf.size() - padding);
    while ( padding-- )
        *out++='=';

    return PString(outbuf);
}

PString MimeTools::encodeLocale(const PString &str, PString &charset)
{
    PString s = str;
    qWarning("MimeTools::encodeLocale: not implemented");

    return s;
}

// We can only deal with a single charset.  If there are multiple
// encodings of different charsets within the field the last one
// will be used.
PString MimeTools::decodeLocale(const PString &str, PString &charset)
{
    PString out( str );

    int pos = 0;
    while ( pos != -1 ) {
        pos = out.find("=?", pos, false);
        if (pos != -1) {
            int endPos = out.find("?=", pos, false);

            // This is a hack to handle encoded words which uses q-encoding and the first char is
            // the escape character =
            if ( endPos != -1 ) {
                if ( out[endPos-2] == '?' )
                    endPos = out.find("?=", endPos+1, false);
            }

            if ( endPos != -1) {
                PString encoded = decodeEncodedWord( out.mid(pos,  (endPos+2) - pos), charset );

                out = out.replace(pos, (endPos+2) - pos, encoded );
                pos += encoded.length();
            } else
                pos = -1;       //no point continuing search
        }
    }

    return out;
}


/*  Needs an encoded word of the form =?charset?q?word?=        */
PString MimeTools::decodeEncodedWord(const PString &str, PString &charset)
{
    PString out( str.mid(2, str.length() - 4) );    // strip ?=  =?
    PString encoding, word;

    int i = out.find("?", 0, false );
    if ( i != -1 ) {
        int i2 = out.find("?", i+1, false);
        if ( i2 != -1 ) {
            charset = out.mid(0, i).upper();
            encoding = out.mid(i + 1, 1).upper();
            word = out.mid(i2 + 1);

            if ( encoding == "Q" ) {
                return decodeQuotedPrintable( word, true );
            } else if ( encoding == "B") {
                QByteArray b = decodeBase64(word);
                return PString( b.data(), b.size() );
            }
        }
    }

    // If we get here, we could not parse the string, do nothing
    return str;
}

static PString token(PString str, PString c1, PString c2, int *index)
{
    int start, stop;
    start = str.find(c1, *index, false);
    if (start == -1)
        return "";
    stop = str.find(c2, ++start, false);
    if (stop == -1)
        return "";

    *index = stop + 1;
    return str.mid(start, stop - start);
}


/*  Returns the difference between local time and GMT time in seconds
    TODO: Cache the timezone and listen in on changes
*/
static int timeZoneDiff()
{
    if ( utcTimeZoneDiff == -1 ) {
        QDateTime gmt, current = QDateTime::currentDateTime();
        PString tz = getenv("TZ");
        if ( !tz.isEmpty() && (setenv("TZ", "GMT", true) == 0) ) {
            gmt = QDateTime::currentDateTime();
        } else return 0;

        setenv("TZ", tz.data(), true );

        utcTimeZoneDiff = gmt.secsTo( current );
    }

    return utcTimeZoneDiff;
}

static PString secsToUTC(int seconds)
{
    PString str;

    int h = seconds / 3600;
    int m = ( abs(seconds) - abs(h * 3600) ) / 60;
    str.sprintf("%+.2d%.2d", h, m );

    return str;
}

static int UTCToSecs(PString utc)
{
    if ( utc.length() != 5 )
        return 0;

    bool result;
    int h = utc.mid(0, 3).toInt(&result);
    if ( !result)
        return 0;

    int m = utc.mid(3).toInt(&result);
    if ( !result)
        return 0;

    return (h*3600 + m*60);
}

PString MimeTools::buildDateString(QDateTime d)
{
    static const char * const dayNames[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    static const char * const monthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    QDate date = d.date();
    QTime time = d.time();
    PString str;
    str.sprintf("%s, %.2d %s %d %.2d:%.2d:%.2d %s",
           (const char*)dayNames[ date.dayOfWeek() - 1 ],
            date.day(),
           (const char *)monthNames[ date.month() - 1 ],
            date.year(),
            time.hour(),
            time.minute(),
            time.second(),
           (const char *)(secsToUTC( timeZoneDiff() ).toUtf8()) );

    return str;
}

PString MimeTools::formattedDateTimeString(QDateTime d)
{
    QDate date = d.date();
    QTime time = d.time();
    PString str;
    str.sprintf("%s, %.2d %s %d %.2d:%.2d",
           (const char *)(date.dayName( date.dayOfWeek() ).toUtf8()),
            date.day(),
           (const char *)(date.monthName( date.month() ).toUtf8()),
            date.year(),
            time.hour(),
            time.minute() );

    return str;
}


/*  Parses the email date format(s).  It also adjusts the senders time
    to the local time:  eg. 14:34:40 +5000 will be adjusted to 19:34
    if your timezone setting is Brisbane.  We don't adjust for daylight savings
    as this would be too expensive (it's only about an hour diff either way)
    Be careful making modifications to this method as it needs to be very fast, and
    a lot of email clients uses different standards of writing out the date.
*/
QDateTime MimeTools::parseDate(PString in)
{
    QDateTime dateTime;
    PString str, org;
    int month = -1, day = -1, year = -1;
    bool ok;
    int x, index;
    uint len;
    PString time, timeDiff;

    for (uint z = 0; z < in.length(); z++) {
        if (in[z] != ',') {
            org += in[z];
        } else {
            org += " ";
        }
    }

    org = org.simplifyWhiteSpace();
    org += " ";

    index = org.find(" ", 0, false);
    str = org.mid(0, (uint) index);
    while ( str != "" ) {
        len = str.length();
        index--;
        if ( (day == -1) && (len <= 2) ) {
            x = str.toInt(&ok);
            if ( ok )
                day = x;
        } else if ( (month == -1) && (len == 3) ) {
            x = Months.find( str.lower() );
            if ( x >= 0 )
                month = (x + 3) / 3;
        } else if ( (year == -1) && (len == 4) ) {
            x = str.toInt(&ok);
            if ( ok )
                year = x;
        } else if ( time.isEmpty() && len == 8 ) {      // time part: 14:22:22
            time = str;
        } else if ( timeDiff.isEmpty() && len == 5 ) {  // time localizer: +1000
            timeDiff = str;
        }

        str = token(org, " ", " ", &index);
    }

    if ( (day != -1) && (month != -1) && (year != -1) ) {
        dateTime.setDate( QDate(year, month, day) );

        if ( !time.isEmpty() ) {
            int h = time.mid(0, 2).toInt();
            int m = time.mid(3, 2).toInt();
            int senderLocal = UTCToSecs( timeDiff );

            //adjust sender local time to our local time
            int localDiff = timeZoneDiff() - senderLocal;

            //we add seconds after adding time, as it may change the date
            dateTime.setTime( QTime(h, m) );
            dateTime = dateTime.addSecs( localDiff);
        }
    }

    return dateTime;
}

PString MimeTools::join(QValueList<MimeAddressField> list)
{
    PString str;
    QValueList<MimeAddressField>::Iterator it = list.begin();

    while ( it != list.end() ) {
        if ( str.isEmpty() )
            str = (*it).toString();
        else
            str += ", " + (*it).toString();

        ++it;
    }

    return str;
}

QValueList<MimeAddressField> MimeTools::split(const PString &s)
{
    QValueList<MimeAddressField> f;

    if ( s == "" )
        return f;

    uint pos = 0;
    uint beg = 0;
    uint len = s.length();
    while ( pos < len ) {
        if ( s[pos] != ',' )
            pos++;
        else {
            f.append( MimeAddressField( s.mid(beg, pos - beg) ) );
            beg = ++pos;
        }

        if ( pos >= len && beg < pos) {
            PString str = s.mid(beg, pos - beg);
            f.append( MimeAddressField( str ) );
        }
    }

    return f;
}

/*  MimeMessages can be complex beasts.  This helper function returns whether
    the MimeBodyPart *bp is regarded as an attachment, eg. meaning not a
    part of multipart/alternative, and not body text in a multipart/mixed
    or a message/rfc822 type.
*/
bool MimeTools::isAttachment(MimeBodyPart *bp)
{
    if ( !bp ) {
        qWarning("passing null ptr to MimeTools::isAttachment");
        return false;
    }

    MimeContentType f = bp->parent()->header()->contentType();
    if ( f.contentType() == "multipart/alternative" )
        return false;

    f = bp->header()->contentType();

    if ( f.type() == "message") // No tr
        return false;

    PString str = bp->header()->contentDisposition().filename();
    str += f.name();
    if ( str.isEmpty() )
        return false;

    return true;
}

