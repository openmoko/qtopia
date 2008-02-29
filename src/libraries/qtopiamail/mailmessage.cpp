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
#include <qtopia/mail/mailmessage.h>
#include <qtopia/mail/qpstream.h>
#include <qtopia/mail/base64stream.h>

#include <qtopiaapplication.h> // needed for setenv
#include <qmimetype.h>

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <stdlib.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <QTextCodec>
#include <QtDebug>
#include <longstream.h> // for freeSpace

//changing timezones are slow, cache info in this var
static int utcTimeZoneDiff = -1;
static QString decodeCharset( const QString &encoded, const QString& _charset );

QString MailMessage::decodeQuotedPrintable( const QString &s, bool RFC2047)
{
    QByteArray out;
    QDataStream outstream(&out,QIODevice::WriteOnly);
    QTextStream instream(s.toAscii(),QIODevice::ReadOnly);
    QPStream ps;
    ps.decode(&instream,&outstream,RFC2047,true);
    return QString().fromAscii(out.data(),out.length());
}

/*
FIXME
it's dumb to provide an encoding function for QByteArray to QString
but no decoding function for QString to QByteArray. Makes it impossible to
encode binary data, set it as a part, then access it again.
*/
QString MailMessage::decodeBase64( const QString &encoded )
{
    QByteArray result = QByteArray::fromBase64(encoded.toAscii());
    return QString().fromAscii(result.data(),result.length());
}

QString MailMessage::encodeBase64(QByteArray fileData)
{
    //QByteArray::toBase64 does not limit line lengths
    QDataStream instream(fileData);
    QString out;
    QTextStream outstream(&out);
    Base64Stream s;
    s.encode(&instream,&outstream);
    return out;
}

QString MailMessage::encodeQuotedPrintable( const QByteArray &fileData, bool RFC2047 )
{
    QPStream s;
    QDataStream instream(fileData);
    QString out;
    QTextStream outstream(&out);
    s.encode(instream,outstream,RFC2047);
    return out;
}

/*  Needs an encoded word of the form =?charset?q?word?=        */
QString MailMessage::decodeEncodedWord(const QString &str)
{
    QString out( str.mid(2, str.length() - 4) );    // strip ?=  =?
    QByteArray charSet, encoding, word;
    QString result;

    int i = out.indexOf('?', 0 );
    if ( i != -1 ) {
        int i2 = out.indexOf('?', i+1);
        if ( i2 != -1 ) {
            charSet = out.mid(0, i).toUpper().toLocal8Bit();
            encoding = out.mid(i + 1, 1).toUpper().toLocal8Bit();
            word = out.mid(i2 + 1).toLocal8Bit();

            if ( encoding == "Q") {
                 result = decodeQuotedPrintable( word, true );
            } else if ( encoding =="B") {
                 result = decodeBase64( word );
            } else
                return str;     //incorrectly formed, do nothing
        result = decodeCharset(result,charSet);
        return result;
        }
    }

    // If we get here, we could not parse the string, do nothing
    return str;
}


QString MailMessage::encodeEncodedWord(const QString &str)
{
    int posn;
    ushort ch;
    bool toLatin1;
    bool toUtf8;

    // Determine if we should use iso-8859-1 or utf-8 to encode.
    toLatin1 = false;
    toUtf8 = false;
    for ( posn = 0; posn < str.length(); ++posn ) {
        ch = str[posn].unicode();
        if ( ch >= 0x7F && ch <= 0xFF ) {
            toLatin1 = true;
        } else if ( ch >= 0x0100 ) {
            toUtf8 = true;
        }
    }

    // Bail out if no encoding is necessary.
    if ( !toLatin1 && !toUtf8 ) {
        return str;
    }

    // Encode the string in either Base64 or Quoted-Printable.
    if ( toUtf8 ) {
        QByteArray utf = str.toUtf8();
        return "=?UTF-8?B?" + encodeBase64( utf ) + "?=";
    } else {
        QByteArray lat = str.toLatin1();
        return "=?ISO-8859-1?Q?" + encodeQuotedPrintable( lat, true ) + "?=";
    }
}


QString MailMessage::localeExpansion(const QString &str)
{
    QString out( str );

    int pos = 0;
    while ( pos != -1 ) {
        pos = out.indexOf("=?", pos);
        if (pos != -1) {
            int endPos = out.indexOf("?=", pos);

            // This is a hack to handle encoded words which uses q-encoding and the first char is
            // the escape character =
            if ( endPos != -1 ) {
                if ( out[endPos-2] == '?' )
                    endPos = out.indexOf("?=", endPos+1);
            }

            if ( endPos != -1) {
                QString encoded = decodeEncodedWord( out.mid(pos,  (endPos+2) - pos) );

                out = out.replace(pos, (endPos+2) - pos, encoded );
                pos += encoded.length();
            } else
                pos = -1;       //no point continuing search
        }
    }

    return out;
}


QString MailMessage::localeEncode(const QString& str)
{
    // We may want to split this up so that only words that need the extra
    // encoding will actually be encoded, and to deal with long strings.
    return encodeEncodedWord( str );
}


static TransferEncoding transferEncodingId( const QString &str )
{
    TransferEncoding te = EightBit;
    if (str.toLower() == "base64") // No tr
        te = Base64;
    else if ( str.toLower() == "quoted-printable" )
        te = QuotedPrintable;
    return te;
}

static LongString decode( const LongString &encoded, TransferEncoding te )
{
    LongString str;
    switch( te ) {
        case EightBit:
            str = encoded;
            break;
        case Base64:
            // Todo - do this efficiently by streaming to a file and creating file longstring
            str = LongString( MailMessage::decodeBase64( encoded.toQString() ).toLocal8Bit() );
            break;
        case QuotedPrintable:
            // Todo - do this efficiently by streaming to a file and creating file longstring
            str = LongString( MailMessage::decodeQuotedPrintable( encoded.toQString() ).toLocal8Bit() );
            break;
    }
    return str;
}

static QString decodeCharset( const QString &encoded, const QString& _charset )
{
    // Remove quotes from the charset, if necessary.
    QByteArray charset = MailMessage::unquoteString( _charset ).toLocal8Bit();

    // Get the codec that is associated with the character set.
    if ( charset.length() == 0 )
    {
        qWarning("No codec found for charset %s",qPrintable(_charset));
        return encoded;
    }
    QTextCodec *codec = QTextCodec::codecForName( charset );
    if ( !codec )
        return encoded;

    // Convert the input body into a byte array.
    QByteArray bytes;
    bytes.resize( encoded.length() );
    int posn = encoded.length();
    while ( posn > 0 ) {
        --posn;
        bytes[posn] = encoded[posn].toLatin1();
    }

    // Convert the byte array into Unicode and return it.
    return codec->toUnicode( bytes );
}

QString MailMessage::quoteString(const QString &str)
{
    if  ( str.length() == 0 || str[0] != '\"' )
        return '\"' + str + '\"';

    return str;
}

QString MailMessage::unquoteString(const QString &str)
{
    if(str.isEmpty())
      return QString();
    QString tmp(str);

    if ( tmp[0] == '\"' )
        tmp = tmp.right( tmp.length() - 1 );
    tmp = tmp.left( tmp.indexOf( '\"' ) );

    return tmp;
}

static QString token(QString str, QChar c1, QChar c2, int *index)
{
    int start, stop;
    start = str.indexOf(c1, *index, Qt::CaseInsensitive);
    if (start == -1)
        return QString();
    stop = str.indexOf(c2, ++start, Qt::CaseInsensitive);
    if (stop == -1)
        return QString();

    *index = stop + 1;
    return str.mid(start, stop - start);
}


/*  Returns the difference between local time and GMT time in seconds   */
static int timeZoneDiff()
{
    if ( utcTimeZoneDiff == -1 ) {
        QDateTime gmt, current = QDateTime::currentDateTime();
        QString tz = getenv("TZ");
        if ( !tz.isEmpty() && (setenv("TZ", "GMT", true) == 0) ) {
            gmt = QDateTime::currentDateTime();
        } else return 0;

        setenv( "TZ", tz.toLocal8Bit().constData(), true );

        utcTimeZoneDiff = gmt.secsTo( current );
    }

    return utcTimeZoneDiff;
}

static QString secsToUTC(int seconds)
{
    QString str;

    int h = seconds / 3600;
    int m = ( abs(seconds) - abs(h * 3600) ) / 60;
    str.sprintf("%+.2d%.2d", h, m );

    return str;
}

static int UTCToSecs(QString utc)
{
    if ( utc.length() != 5 )
        return 0;

    bool result;
    int h = utc.left(3).toInt(&result);
    if ( !result)
        return 0;

    int m = utc.right(2).toInt(&result);
    if ( !result)
        return 0;

    return (h*3600 + m*60);
}

static QString buildDateString(QDateTime d)
{
    static const char * const dayNames[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    static const char * const monthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    QDate date = d.date();
    QTime time = d.time();
    QString str;
    str.sprintf("%s, %.2d %s %d %.2d:%.2d:%.2d %s",
            (const char *)dayNames[ date.dayOfWeek() - 1 ],
            date.day(),
            (const char *)monthNames[ date.month() - 1 ],
            date.year(),
            time.hour(),
            time.minute(),
            time.second(),
            (const char *)secsToUTC( timeZoneDiff() ).toUtf8() );

    return str;
}


/*  MailMessage */
MailMessage::MailMessage()
{
    init();
}

MailMessage::MailMessage(const QString &s)
{
    init();

    fromRFC822(s);
}

MailMessage::~MailMessage()
{
    _messageParts.clear();
}

void MailMessage::init()
{
   _size = 0;
   _transferEncoding = EightBit;
   _status = 0;
   _multipartRelated = false;
}


void MailMessage::setType( int t )
{
    _status &= ~(EFlag_TypeMms | EFlag_TypeEms | EFlag_TypeSms | EFlag_TypeEmail);
    setExtraHeader( "X-Sms-Type", QString() );
    setExtraHeader( "X-Mms-Message-Type", QString() );
    setExtraHeader( "X-Mms-Read-Reply", QString() );
    if( t & MailMessage::MMS ) {
        setExtraHeader( "X-Mms-Message-Type", "m-send-req" );
        _status |= EFlag_TypeMms;
    }
    if( t & MailMessage::EMS ) {
        qWarning("MailMessage::setType( EMS ) - Not implemented."); // TODO
        _status |= EFlag_TypeEms;
    }
    if( t & MailMessage::Email ) {
        _status |= EFlag_TypeEmail;
    }
    if( t & MailMessage::SMS) {
        // determined from recipients
        setExtraHeader( "X-Sms-Type", "");
        _status |= EFlag_TypeSms;
    }
    if( t & MailMessage::System ){
        _status = EFlag_TypeSystem;
    }
}

int MailMessage::type() const
{
    int t = 0;
    if( header("X-Mms-Message-Type").trimmed().length() ) {
        t = MailMessage::MMS;
    } else if( false /* TODO : EMS */ ) {
        t = MailMessage::EMS;
    } else {
        QStringList allRecipients = to() + cc() + bcc();
        if( header("X-Sms-Type").length() || phoneRecipients().count() ) {
     if (_status & EFlag_TypeMms)
         t = MailMessage::MMS;
     else
            t |= MailMessage::SMS;
        }
        if( phoneRecipients().count() < allRecipients.count() ) {
            t |= MailMessage::Email;
        }
    }
    if( t == 0 ) {
        if (_status & EFlag_TypeMms)
            t |= MailMessage::MMS;
        if (_status & EFlag_TypeEms)
            t |= MailMessage::EMS;
        if (_status & EFlag_TypeEmail)
            t |= MailMessage::Email;
        if (_status & EFlag_TypeSms)
            t |= MailMessage::SMS;
        if (_status & EFlag_TypeSystem)
            t |= MailMessage::System;
        if (t == 0)
            t = MailMessage::Default;
    }
    return t;
}

QStringList MailMessage::phoneRecipients() const
{
    return _phoneRecipients;
}

void MailMessage::setPhoneRecipients()
{
    _phoneRecipients.clear();
    QRegExp re( "^[\\+\\*#\\d\\-\\(\\)\\s]*$" );
    QStringList allRecipients = to() + cc() + bcc();
    for( QStringList::Iterator it = allRecipients.begin() ;
                            it != allRecipients.end() ; ++it )
        if( re.exactMatch( *it ) )
            _phoneRecipients.append( *it );
}

QStringList MailMessage::mailRecipients() const
{
    //TODO do a real search on valid email formats

    QStringList mailRecipients;

    QRegExp re( "^[\\+\\*#\\d\\-\\(\\)\\s]*$" );
    QStringList allRecipients = to() + cc() + bcc();
    for( QStringList::Iterator it = allRecipients.begin() ;
        it != allRecipients.end() ; ++it )
        if( !re.exactMatch( *it ) )
            mailRecipients.append( *it );
    return mailRecipients;
}

QString MailMessage::toRFC822() const
{
    return QString(messageHeaders() + "\n\n" + _rawMessageBody.toQString());
}

QString MailMessage::fileName() const
{
    return _rawMessageBody.fileName();
}

void MailMessage::fromRFC822(const QString &s)
{
    fromRFC822( s.toLocal8Bit() );
}

void MailMessage::fromRFC822(const QByteArray &ba)
{
    fromRFC822( LongString( ba ) );
}

void MailMessage::fromRFC822(const LongString &ls)
{
    int pos = ls.indexOf("\n\n", 0);

    if (pos == -1) {
        _rawMessageBody = LongString();
        parseHeader( ls.toQString() );
    } else {
        _rawMessageBody = ls.mid( pos + 2);

        parseHeader( ls.left(pos).toQString() );
        parseBody();
    }
}

QUuid MailMessage::uuid() const
{
    return _uuid;
}

void MailMessage::setUuid(const QUuid id)
{
    _uuid = id;
}

QString MailMessage::from() const
{
    return _from;
}

void MailMessage::setFrom(const QString &s)
{
    _from = s;
    parseEmailAddress(_from, _fromName, _fromEmail);
}

QString MailMessage::fromName() const
{
    return _fromName;
}

QString MailMessage::fromEmail() const
{
    return _fromEmail;
}

QString MailMessage::subject() const
{
    return _subject;
}

void MailMessage::setSubject(const QString &s)
{
    _subject = s;
}

QString MailMessage::dateString() const
{
    return _dateString;
}

QDateTime MailMessage::dateTime() const
{
    return _dateTime;
}

void MailMessage::setDateTime(QDateTime d)
{
    _dateTime = d;
}

void MailMessage::setDateString(const QString &s)
{
    _dateString = s;
    _dateTime = parseDate(_dateString);
}

QStringList MailMessage::to() const
{
    return _to;
}

void MailMessage::setTo(QStringList s)
{
    _to = s;
    setPhoneRecipients();
}

QStringList MailMessage::cc() const
{
    return _cc;
}

void MailMessage::setCc(QStringList s)
{
    _cc = s;
    setPhoneRecipients();
}

QStringList MailMessage::bcc() const
{
    return _bcc;
}

void MailMessage::setBcc(QStringList s)
{
    _bcc = s;
    setPhoneRecipients();
}

QString MailMessage::replyTo() const
{
    return _replyTo;
}

void MailMessage::setReplyTo(const QString &s)
{
    _replyTo = s;
}


QString MailMessage::messageId() const
{
    return _messageId;
}

void MailMessage::setMessageId(const QString &s)
{
    _messageId = s;
}

QString MailMessage::inReplyTo() const
{
    return _inReplyTo;
}

void MailMessage::setInReplyTo(const QString &s)
{
    _inReplyTo = s;
}

EmailStatus MailMessage::status(EmailStatus es) const
{
    return (es & _status);
}

void MailMessage::setStatus(EmailStatus es, bool b)
{
    if ( b ) {
        _status |= es;
    } else {
        _status &= ~es;
    }
}

uint MailMessage::allStatusFields() const
{
    return _status;
}

void MailMessage::setAllStatusFields(uint s)
{
    _status = s;
}

bool MailMessage::unfinished()
{
    bool toEmpty = _to.join("").isEmpty();
    bool ccEmpty = _cc.join("").isEmpty();
    bool bccEmpty = _bcc.join("").isEmpty();
    return toEmpty && ccEmpty && bccEmpty;
}

QString MailMessage::fromAccount() const
{
    return _fromAccount;
}

void MailMessage::setFromAccount(const QString &s)
{
    _fromAccount = s;
}

QString MailMessage::fromMailbox() const
{
    return _fromMailbox;
}

void MailMessage::setFromMailbox(const QString &s)
{
    _fromMailbox = s;
}

QString MailMessage::serverUid() const
{
    return _serverUid;
}

void MailMessage::setServerUid(const QString &s)
{
    _serverUid = s;
}

uint MailMessage::size() const
{
    return _size;
}

void MailMessage::setSize(uint i)
{
    _size = i;
}

QString MailMessage::plainTextBody() const
{
    if ( _plainTextBody.indexOf("\n.\n", -3) != -1)
        return _plainTextBody.left( _plainTextBody.length() - 3 );

    return _plainTextBody;
}

void MailMessage::setPlainTextBody(const QString &s)
{
    _plainTextBody = s;
}

QString MailMessage::htmlBody() const
{
    return _htmlBody;
}

void MailMessage::setHtmlBody(const QString &s)
{
    _htmlBody = s;
}

uint MailMessage::messagePartCount() const
{
    return _messageParts.count();
}

void MailMessage::addMessagePart(const MailMessagePart part)
{
    _messageParts.append( part );
}

void MailMessage::prependMessagePart(const MailMessagePart part)
{
    _messageParts.prepend( part );
}

/*  Const reference returned    */
const MailMessagePart& MailMessage::messagePartAt(uint pos) const
{
//    if ( pos >= _messageParts.count() )
//      return MailMessagePart();

    return _messageParts[pos];
}

/*  Reference returned  */
MailMessagePart& MailMessage::messagePartAt(uint pos)
{
//    if ( pos >= _messageParts.count() )
//      return MailMessagePart();

    return _messageParts[pos];
}

void MailMessage::removeAllMessageParts()
{
    _messageParts.clear();
}

bool MailMessage::multipartRelated() const
{
    return _multipartRelated;
}

void MailMessage::setMultipartRelated(bool value)
{
    _multipartRelated = value;
}

void MailMessage::parseHeader(const QString &_rawMessageHeader)
{
    int pos = 0, lineEnd = 0, endPos = 0;
    bool continued;

    _headerFields.clear();
    _extraFields.clear();

    while (endPos < (int) _rawMessageHeader.length() ) {

        do {
            continued = false;
            if ( ( endPos = _rawMessageHeader.indexOf("\n", lineEnd) ) == -1) {
                endPos = _rawMessageHeader.length();
            }
            lineEnd = endPos + 1;

            if ( lineEnd < _rawMessageHeader.length() ) {
                if ( _rawMessageHeader[lineEnd] == ' ' || _rawMessageHeader[lineEnd] == '\t') {
                    continued = true;
                }
            }
        } while( continued );

        QString field( _rawMessageHeader.mid(pos, lineEnd - pos) );
        if (!parseHeaderField( field ))
            _extraFields.append(field);
        else
            _headerFields.append(field);

        pos = lineEnd;
    }
}

bool MailMessage::parseHeaderField(const QString &field)
{
    QString str;

    if ( ( str = contents("from:", field) ) != QString() ) { // No tr
        _from = localeExpansion( str );
        parseEmailAddress(_from, _fromName, _fromEmail);
        return true;
    }

    if ( ( str = contents("subject:", field) ) != QString() ) { // No tr
        _subject = localeExpansion( str );
        return true;
    }

    if ( ( str = contents("date:", field) ) != QString() ) { // No tr
        _dateString = str;
        _dateTime = parseDate(str);
        return true;
    }

    if ( ( str = contents("to:", field) ) != QString() ) { // No tr
        _to = localeExpansion(str).split(", ");
        setPhoneRecipients();
        return true;
    }

    if ( ( str = contents("cc:", field) ) != QString() ) {
        _cc = localeExpansion(str).split(", ");
        setPhoneRecipients();
        return true;
    }

    if ( ( str = contents("bcc:", field) ) != QString() ) {
        _bcc = localeExpansion(str).split(", ");
        setPhoneRecipients();
        return true;
    }

    if ( ( str = contents("X-SMS:", field) ) != QString() ) {
        //kept for compatibility with previous qtmail mails.
        _to = localeExpansion(str).split(", ");
        setPhoneRecipients();
        return true;
    }

    _replyTo = "";
    if ( ( str = contents("reply-to:", field) ) != QString() ) {
        _replyTo = localeExpansion(str);
        return true;
    }

    if ( ( str = contents("message-id:", field) ) != QString() ) {
        _messageId = str;
        return true;
    }

    if ( ( str = contents("in-reply-to:", field) ) != QString() ) {
        _inReplyTo = str;
        return true;
    }

    bool rv = false;
    if ( ( str = contents("mime-version:", field) ) != QString() ) {
        _mimeType = str;
        rv = true;
        //boundary could be on the same line, don't exit yet
    }

    if ( ( str = contents("content-type:", field) ) != QString() ) {    // No tr
        if ( str.startsWith( "multipart/related" ) ) {  // No tr
            _multipartRelated = true;
        }
        rv = true;
        // fall through to get the boundary marker.
    }

    if ( ( str = headerFieldContent("boundary=", field) ) != QString() ) { // No tr
        _boundary = "--" + unquoteString( str );        //create boundary field
        return true;
    }

    if ( ( str = contents("content-transfer-encoding:", field) ) != QString() ) {
        setTransferEncoding( transferEncodingId( str ) );
        return true;
    }

    return rv;
}

QString MailMessage::contents(const QString &str, const QString &target)
{
    if ( target.indexOf(str, 0, Qt::CaseInsensitive) == 0 ) {
        return target.mid( str.length() ).simplified();
    }

    return QString();
}

QString MailMessage::headerFieldContent(QString str, QString target)
{
    int pos, endPos;

    pos = target.indexOf(str, 0, Qt::CaseInsensitive);
    if ( pos == -1 || pos > target.length())
        return QString();

    pos += str.length();
    int lineEnd = pos;
    bool continued;
    do {
        continued = false;
        if ( ( endPos = target.indexOf("\n", lineEnd) ) == -1) {
            endPos = target.length();
        }
        lineEnd = endPos + 1;

        if ( endPos < target.length() ) {
            if ( target[lineEnd] == ' ' || target[lineEnd] == '\t') {
                continued = true;
            }
        }
    } while( continued );

    return target.mid(pos, lineEnd - pos).simplified();
}

QString MailMessage::formattedDateTimeString(QDateTime d)
{
    QDate date = d.date();
    QTime time = d.time();
    QString str;
    str.sprintf("%s, %.2d %s %d %.2d:%.2d",
            (const char *)date.shortDayName( date.dayOfWeek() ).toUtf8(),
            date.day(),
            (const char *)date.shortMonthName( date.month() ).toUtf8(),
            date.year(),
            time.hour(),
            time.minute() );
    return str;
}

void MailMessage::parseEmailAddress(QString fromStr, QString &nameStr, QString &emailStr)
{
    int pos;

    nameStr = fromStr;
    if ( (pos = fromStr.indexOf('<', 0, Qt::CaseInsensitive)) != -1) {
        nameStr = fromStr.mid(0, pos).trimmed();

        nameStr = unquoteString(nameStr);

        pos++;
        int endPos = fromStr.indexOf('>', pos, Qt::CaseInsensitive);
        emailStr = fromStr.mid(pos, endPos - pos).trimmed();
        if ( nameStr.isEmpty() )
            nameStr = emailStr;
    } else {
        emailStr = nameStr;
    }
}


/*  Parses the email date format(s).  It also adjusts the senders time
    to the local time:  eg. 14:34:40 +5000 will be adjusted to 19:34
    if your timezone setting is Brisbane.  We don't adjust for daylight savings
    as this would be too expensive (it's only about an hour diff either way)
    Be careful making modifications to this method as it needs to be very fast, and
    a lot of email clients uses different standards of writing out the date.
*/
QDateTime MailMessage::parseDate(QString in)
{
    const QString Months("janfebmaraprmayjunjulaugsepoctnovdec");
    QDateTime dateTime;
    QString str, org;
    int month = -1, day = -1, year = -1;
    bool ok;
    int x, index;
    uint len;
    QString time, timeDiff;

    for (int z = 0; z < in.length(); z++) {
        if (in[z] != ',') {
            org += in[z];
        } else {
            org += " ";
        }
    }

    org = org.simplified();
    org += " ";

    index = org.indexOf(' ');
    str = org.left((uint) index);
    while ( str != QString() ) {
        len = str.length();
        index--;
        if ( (day == -1) && (len <= 2) ) {
            x = str.toInt(&ok);
            if ( ok )
                day = x;
        } else if ( (month == -1) && (len == 3) ) {
            x = Months.indexOf( str.toLower() );
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

        str = token(org, ' ', ' ', &index);
    }

    if ( (day != -1) && (month != -1) && (year != -1) ) {
        dateTime.setDate( QDate(year, month, day) );

        if ( !time.isEmpty() ) {
            int h = time.left(2).toInt();
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

void MailMessage::parseBody()
{
    LongString body( decode( _rawMessageBody, _transferEncoding) );
    _messageParts.clear();

    // If the boundary is empty, we cannot distinguish body parts in the message
    if ( _mimeType == "1.0" && !_boundary.isEmpty() ) {
        int _bodyId = 0;

        parseMimePart(body, _boundary, _bodyId);
    } else {
        QString content;

        if ( body.indexOf("\n.\n", -3) != -1)
            _plainTextBody = body.left( body.length() - 3 ).toQString();
        else
            _plainTextBody = body.toQString();

        content = headerFieldContent( "charset=", header( "Content-Type" ) );
        _plainTextBody = decodeCharset( _plainTextBody, content );
    }
}

void MailMessage::parseMimePart(LongString body, QString boundary, int &bodyId)
{
    LongString mimeBody;
    QString mimeHeader, str;
    QString fileName, name, doctype, id, location;
    int pos, endPos;
    TransferEncoding encoding;
    bool havePartsAlready = false;

    QString lineShift = "\n";
    QString delimiter = lineShift + lineShift;

    pos = body.indexOf(QString(boundary+"--").toLocal8Bit(), 0);

    while (body.length() > 0) {
        pos = body.indexOf(boundary.toLocal8Bit(), 0);

    //check if we are at the final boundary position.

    endPos = body.indexOf(QString(boundary+"--").toLocal8Bit(),0);
    if(pos == endPos)
        break; //we are done.

        pos = body.indexOf(delimiter.toLocal8Bit(), pos);

        if (pos < 0) // run out of parts
            break;

        mimeHeader = body.left(pos).toQString();
        mimeBody = body.right(body.length() - pos - delimiter.length());

        fileName = "";
        name = "";
        doctype = "";
        encoding = EightBit;
        id = "";
        location = "";

        if ((pos = mimeHeader.indexOf("name=", 0, Qt::CaseInsensitive)) != -1) { // No tr
            pos += 5;
            endPos = mimeHeader.indexOf(lineShift, pos);
            name = mimeHeader.mid(pos, endPos - pos).trimmed();

            name = unquoteString(name);
        }

        if ( (pos = mimeHeader.indexOf("filename=", 0, Qt::CaseInsensitive)) != -1) { // No tr
            pos += 9;
            endPos = mimeHeader.indexOf(lineShift, pos);
            fileName = mimeHeader.mid(pos, endPos - pos).trimmed();

            fileName = unquoteString(fileName);
        } else fileName = name;

        if ( (pos = mimeHeader.indexOf("content-type:", 0, Qt::CaseInsensitive)) != -1) {
            pos += 13;
            int semi = mimeHeader.indexOf(';', pos);

            int semiMissing = mimeHeader.indexOf('\n', pos);
            if ( semiMissing != -1 ) {
                if ( semi == -1 ) {
                    semi = semiMissing;
                } else if ( semiMissing < semi ) {
                    semi = semiMissing;
                }
            }

            doctype = mimeHeader.mid(pos, semi-pos)
                .trimmed().toLower();
            if ( doctype == "application/octet-stream" ) {
                // Sender's client might not know what type,
                // but maybe we do. Try...
                doctype = QMimeType(fileName).id();
            }
        }

        if ( (pos = mimeHeader.indexOf("encoding:", 0, Qt::CaseInsensitive)) != -1) { // No tr
            pos += 9;
            endPos = mimeHeader.indexOf(lineShift, pos);
            encoding = transferEncodingId ( mimeHeader.mid(pos, endPos - pos).trimmed() );
        }

        if ( (pos = mimeHeader.indexOf("content-id:", 0, Qt::CaseInsensitive)) != -1) {
            pos += 11;
            endPos = mimeHeader.indexOf(lineShift, pos);
            id = mimeHeader.mid(pos, endPos - pos).trimmed();
        }

        if ( (pos = mimeHeader.indexOf("content-location:", 0, Qt::CaseInsensitive)) != -1) {
            pos += 17;
            endPos = mimeHeader.indexOf(lineShift, pos);
            location = mimeHeader.mid(pos, endPos - pos).trimmed();
        }

        pos = mimeBody.indexOf( QString("\n" + boundary).toLocal8Bit(), 0);
        if (pos == -1)                  //should not occur, malformed mail
            pos = mimeBody.length();

        body = LongString( mimeBody.right(mimeBody.length() - pos) );
        mimeBody = mimeBody.left(pos);

        if ( !fileName.isEmpty() ) {
            // This is an actual attached file, rather than something
            // inline like an image or forwarded message.
            MailMessagePart part;
            part.setFilename( fileName );
            part.setName( name );
            part.setContentType( doctype );
            part.setContentID( id );
            part.setContentLocation( location );
            part.setTransferEncoding( encoding );
            part.setLongEncodedBody( mimeBody, encoding );

            addMessagePart( part );
            bodyId++;
        } else if ( doctype.startsWith("text") ) { // No tr
            QString text = decode( mimeBody, encoding ).toQString();
            QString content = headerFieldContent( "charset=", mimeHeader );
            text = decodeCharset( text, content );
            if ( doctype == "text/html" ) {
                _htmlBody += text;
            } else if ( doctype == "text/plain" &&
                        _plainTextBody.length() == 0 && !havePartsAlready ) {
                _plainTextBody += text;
            } else {
                // Add extra text bodies as attachments, so that we
                // can display them in the correct order.
                MailMessagePart part;
                part.setFilename( fileName );
                part.setName( name );
                part.setContentType( doctype );
                part.setContentID( id );
                part.setContentLocation( location );
                part.setTransferEncoding( encoding );
                part.setLongEncodedBody( mimeBody, encoding );
                addMessagePart( part );
                bodyId++;
            }
        } else if ( doctype.startsWith("message") ) { // No tr
            QString text = decode( mimeBody, encoding ).toQString();
            _plainTextBody += text;
        } else if ( doctype.startsWith("multipart") ) {

            if ( doctype.startsWith("multipart/related") ) {
                _multipartRelated = true;
            }

            if ( ( str = headerFieldContent("boundary=", mimeHeader ) ) != QString() ) { // No tr
                int i;
                str = "--" + unquoteString( str );
                if ((i = str.indexOf('\"',0) > 0)) // br:39066
                    str = str.mid( 0, i );
            } else
                str = boundary;     //no new boundary defined, use old

            parseMimePart( mimeBody, str, bodyId );    //encapsulated mime-part (multipart)
        } else {
            // Everything else is added as an inline attachment part.
            MailMessagePart part;
            part.setFilename( fileName );
            part.setName( name );
            part.setContentType( doctype );
            part.setContentID( id );
            part.setContentLocation( location );
            part.setTransferEncoding( encoding );
            part.setLongEncodedBody( mimeBody, encoding );

            addMessagePart( part );
            bodyId++;
        }
        havePartsAlready = true;
    }
}

QString MailMessage::messageHeaders(bool withbcc) const
{
    QString str;

    if(withbcc)
        str = _headerFields.join("");
    else
    {
        int bccindex = _headerFields.indexOf(QRegExp("^BCC:.*$"));
        if(bccindex >= 0)
        {
            for(int i = 0; i < _headerFields.count(); ++i)
            {
                if(i != bccindex)
                    str += _headerFields[i];
            }
        }
        else
            str = _headerFields.join("");

    }

    while ( str[(int) str.length() - 1]  == '\n' )
        str.truncate( str.length() - 1 );
    str += '\n';

    str += _extraFields.join("");

    while ( str[(int) str.length() - 1]  == '\n' )
        str.truncate( str.length() - 1 );

    return str;
}

QString MailMessage::header( const QString &name ) const
{
    QString nameSuf = name + ":";
    QString result;
    QStringList::ConstIterator it = _headerFields.begin();
    for ( ; it != _headerFields.end(); ++it ) {
        if ( (*it).startsWith( nameSuf ) ) {
            result = (*it).mid( nameSuf.length() );
            break;
        }
    }
    if (result.isNull()) {
        for (it = _extraFields.begin() ; it != _extraFields.end(); ++it ) {
            if ( (*it).startsWith( nameSuf ) ) {
                result = (*it).mid( nameSuf.length() );
                break;
            }
        }
    }

    if (!result.isNull()) {
        if (result[0].isSpace())
            return result.mid( 1 );
        else
            return result;
    }

    return QString();
}

void MailMessage::setExtraHeader(const QString &name, const QString &value)
{
    QString header = name + ": " + value;
    if ( header[(int)header.length() - 1]  != '\n' )
        header += "\n";

    QStringList::Iterator it = _extraFields.begin();
    for ( ; it != _extraFields.end(); ++it ) {
        if ( (*it).startsWith( name+':' ) ) {
            if (value.isNull())
                _extraFields.erase(it);
            else
                *it = header;
            return;
        }
    }

    // new header, add it
    if (!value.isNull())
        _extraFields.append( header );
}

void MailMessage::updateHeader(QString header, QString content)
{
    QStringList::Iterator it = _headerFields.begin();
    QString newHeader = header + " " + content;
    if ( newHeader[(int)newHeader.length() - 1]  != '\n' )
        newHeader += "\n";

    for ( ; it != _headerFields.end(); ++it ) {
        if ( (*it).startsWith( header ) ) {
            *it = newHeader;
            return;
        }
    }

    // new header, add it
    _headerFields.append( newHeader );
}

// Encode a plaintext body in UTF-8 and then quoted-printable.
// We need the odd copy logic below to avoid encoding the NUL
// at the end of the return value from "QString::toUtf8()".
static QString encodeUtf8QP( const QString& body )
{
    QByteArray array = body.toUtf8();
    return MailMessage::encodeQuotedPrintable( array );
}

bool MailMessage::saveMail(QTextStream& out, const QString& checkPath)
{
    //if the mail is incoming or a sms/mms message
    //dump the mail to stream as is,
    //otherwise encode the mail to the stream without attachments

    bool isPhoneMessage = status(EFlag_TypeMms) || status(EFlag_TypeSms);
    bool isIncoming = !(status(EFlag_Outgoing) || status(EFlag_Sent));

    if(isPhoneMessage || isIncoming)
    {
        if(!isIncoming) //encode outgoing
            encodeMail();
        if(!LongStream::freeSpace( checkPath ))
            return false;
        out << messageHeaders(true) << "\n\n";
        if(!LongStream::freeSpace( checkPath ))
            return false;
        out << _rawMessageBody.toQString();
        return true;
    }
    else
        return encodeMail(out,true,false,checkPath);
}

bool MailMessage::encodeMail(QTextStream& out, bool privateHeaders, bool attachments,
                             const QString &checkPath)
{
    encodeHeaders();

    if (!checkPath.isEmpty() && !LongStream::freeSpace( checkPath ))
            return false;
    out << messageHeaders(privateHeaders) + "\n\n";

    if ( _messageParts.count() == 0) {          //just a simple mail

        out << encodeUtf8QP( _plainTextBody ); //TODO stream this
        if (!checkPath.isEmpty() && !LongStream::freeSpace( checkPath ))
            return false;

    } else {
        out << "This is a multipart message in Mime 1.0 format\n\n"; // No tr

        if ( !_plainTextBody.isEmpty() ) {
            out << "--" << _boundary << "\nContent-Type: text/plain; charset=utf-8\n"; // No tr
            out << "Content-Transfer-Encoding: quoted-printable\n\n"; // No tr
            out << encodeUtf8QP( _plainTextBody ); //TODO stream this
            out << "\n";
        }

        MailMessagePart part;
        for ( int i = 0; i < _messageParts.count(); i++ ) {
            part = messagePartAt( i );

            out << "--" << _boundary << "\n";
            out << "Content-Type: " << part.contentType();
            if ( part.name().isEmpty() )
                out << "\n";
            else
                out << "; name=\"" << part.name() << "\"\n"; // No tr
            if ( ! part.contentID().isEmpty() )
                out << "Content-ID: " << part.contentID() << "\n";

            if (!part.contentLocation().isEmpty())
                out << "Content-Location: " << part.contentLocation() << "\n";

            out << "Content-Transfer-Encoding: "; // No tr
            switch ( part.transferEncoding() ) {
                case QuotedPrintable:
                    out << "quoted-printable\n"; // No tr
                    break;

                case Base64:
                    out << "base64\n"; // No tr
                    break;

                case EightBit:
                default:
                    out << "8bit\n"; // No tr
                    break;
            }

            if ( !part.filename().isEmpty() ) {
                QFileInfo fi( part.filename() );
                out << "Content-Disposition: inline; filename=\""  << fi.fileName() << "\"\n"; //no tr
            } else {
                out << "Content-Disposition: inline\n"; //no tr
            }
            out << "\n";
            if(attachments)
            {
                part.encodedBody(out); //stream the encoded body out
                out << "\n";
            }
            if (!checkPath.isEmpty() && !LongStream::freeSpace( checkPath ))
                return false;
        }
        out << "--" + _boundary << "--";
        if (!checkPath.isEmpty() && !LongStream::freeSpace( checkPath ))
            return false;
    }
    return true;
}

void MailMessage::encodeHeaders()
{
    QString userName, newBody;

    _headerFields.clear();

    //add standard headers

    updateHeader("From:", _from); // No tr
    if ( _to.count() > 0) {
        updateHeader("To:", _to.join(", ") ); // No tr
    }

    if ( _cc.count() > 0) {
        updateHeader("CC:", _cc.join(", ") );
    }

    if(_bcc.count() > 0)
        updateHeader("BCC:", _bcc.join(", ") );


    QDateTime _dateTime = QDateTime::currentDateTime();
    _dateString = buildDateString( _dateTime );

    updateHeader("Date:", _dateString); // No tr
    updateHeader("Subject:", localeEncode( _subject ) ); // No tr

    if ( !replyTo().isEmpty() )
        updateHeader("Reply-To:", _replyTo);

    if ( !_inReplyTo.isEmpty() )
        updateHeader("In-Reply-To:", _inReplyTo);

    if ( !_messageId.isEmpty() )
        updateHeader("Message-ID:", _messageId);

    if ( _messageParts.count() == 0) {          //just a simple mail

        updateHeader("Mime-Version:", "1.0");   // No tr
        updateHeader("Content-Type:", "text/plain; charset=utf-8"); // No tr
        updateHeader("Content-Transfer-Encoding:", "quoted-printable"); // No tr

    } else {
        //Build mime encoded mail
        if ( _boundary.isEmpty() )
            _boundary = "-----4345=next_bound=0495----";

        updateHeader("Mime-Version:", "1.0");   // No tr
        if ( _multipartRelated )
            updateHeader("Content-Type:", "multipart/related; type=\"text/html\"; boundary=\"" + _boundary + "\""); // No tr
        else
            updateHeader("Content-Type:", "multipart/mixed; boundary=\"" + _boundary + "\""); // No tr
    }
}


bool MailMessage::encodeMail()
{
    encodeHeaders();

    QString userName, newBody;

    if ( _messageParts.count() == 0) {          //just a simple mail
        newBody = encodeUtf8QP( _plainTextBody );
    } else {
        newBody = "This is a multipart message in Mime 1.0 format\n\n"; // No tr

        if ( !_plainTextBody.isEmpty() ) {
            newBody += "--" + _boundary + "\nContent-Type: text/plain; charset=utf-8\n"; // No tr
            newBody += "Content-Transfer-Encoding: quoted-printable\n\n"; // No tr
            newBody += encodeUtf8QP( _plainTextBody );
            newBody += "\n";
        }

        MailMessagePart part;
        for ( int i = 0; i < _messageParts.count(); i++ ) {
            part = messagePartAt( i );

            newBody += "--" + _boundary + "\n";
            newBody += "Content-Type: " + part.contentType();
            if ( part.name().isEmpty() )
                newBody += "\n";
            else
                newBody += "; name=\"" + part.name() + "\"\n"; // No tr
            if ( ! part.contentID().isEmpty() )
                newBody += "Content-ID: " + part.contentID() + "\n";
            if ( !part.contentLocation().isEmpty() )
                newBody += "Content-Location: " + part.contentLocation() + "\n";

            newBody += "Content-Transfer-Encoding: "; // No tr
            switch ( part.transferEncoding() ) {
                case QuotedPrintable:
                    newBody += "quoted-printable\n"; // No tr
                    break;

                case Base64:
                    newBody += "base64\n"; // No tr
                    break;

                case EightBit:
                default:
                    newBody += "8bit\n"; // No tr
                    break;
            }

            if ( !part.filename().isEmpty() ) {
                QFileInfo fi( part.filename() );
                newBody += "Content-Disposition: inline; filename=\"" + // No tr
                    fi.fileName() + "\"\n";
            } else {
                newBody += "Content-Disposition: inline\n"; // No tr
            }

            newBody += "\n";

            newBody += part.encodedBody();

            newBody += "\n";
        }

        newBody += "--" + _boundary + "--";
    }

    _rawMessageBody = newBody.toLocal8Bit();

    return true;
}

/*      Mail Message Part   */
MailMessagePart::MailMessagePart()
{
    _transferEncoding = Base64;
}

QString MailMessagePart::contentType() const
{
    return _contentType;
}

void MailMessagePart::setContentType(const QString &s)
{
    _contentType = s;
}

QString MailMessagePart::contentDisposition() const
{
    return _contentDisposition;
}

void MailMessagePart::setContentDisposition(const QString &s)
{
    _contentDisposition = s;
}

QString MailMessagePart::contentID() const
{
    return _contentID;
}

void MailMessagePart::setContentID(const QString &s)
{
    _contentID = s;
}

QString MailMessagePart::contentLocation() const
{
    return _contentLocation;
}

void MailMessagePart::setContentLocation(const QString &s)
{
    _contentLocation = s;
}

QString MailMessagePart::name() const
{
    return _name;
}

void MailMessagePart::setName(const QString &s)
{
    _name = s;
}

QString MailMessagePart::prettyName() const
{
    if ( _name.isEmpty() )
        return "(No name)";
    return _name;
}

QString MailMessagePart::filename() const
{
    return _filename;
}

void MailMessagePart::setFilename(const QString &s)
{
    _filename = s;
}

QString MailMessagePart::linkFile() const
{
    return _linkFile;
}

void MailMessagePart::setLinkFile(const QString &s)
{
    _linkFile = s;
}

QString MailMessagePart::storedFilename() const
{
    return _storedFilename;
}

void MailMessagePart::setStoredFilename(const QString &s)
{
    _storedFilename = s;
}

void MailMessagePart::setBody(QDataStream& instream, TransferEncoding te)
{
    _transferEncoding = te;
    QByteArray body;
    QTextStream outstream(&body,QIODevice::WriteOnly);

    switch( _transferEncoding ) {
        case EightBit:
        {
            while(!instream.atEnd())
                instream >> body;
        } break;
        case QuotedPrintable:
        {
            QPStream::encode(instream,outstream,false);
        } break;
        case Base64:
        {
            Base64Stream s;
            s.encode(&instream,&outstream);
        } break;
    }
    _body = LongString( body );
}

void MailMessagePart::setRawEncodedBody(const QByteArray b, TransferEncoding te)
{
    _body = b;

    _transferEncoding = te;
}

void MailMessagePart::setLongEncodedBody(const LongString &ls, TransferEncoding te)
{
    _body = ls;

    _transferEncoding = te;
}

void MailMessagePart::setEncodedBody(const QString &s, TransferEncoding te)
{
    _body = s.toLocal8Bit();
    _transferEncoding = te;
}

QByteArray MailMessagePart::rawEncodedBody() const
{
    QByteArray temp = _body.toQByteArray();
    QByteArray ba( temp.constData(), temp.length() ); // deep copy
    return ba;
}

int MailMessagePart::encodedSize() const
{
    return _body.length();
}

QString MailMessagePart::encodedBody() const
{
    return _body.toQString();
}

QString MailMessagePart::decodedBody() const
{
    switch( _transferEncoding ) {
        case EightBit:
        {
            return _body.toQString();
        }
        case QuotedPrintable:
        {
            return MailMessage::decodeQuotedPrintable( _body.toQByteArray() );
        }
        case Base64:
        {
            return  MailMessage::decodeBase64( _body.toQByteArray() );
        }
    }

    return QString();
}

bool MailMessagePart::decodedBody(QDataStream& result) const
{
    switch( _transferEncoding ) {
        case EightBit:
        {
            int i = 0;
            for(i = 0; i < _body.length(); ++i)
                result << (quint8)_body.toQByteArray()[i];
            return (i == _body.length());
        }
        case QuotedPrintable:
        {
            QTextStream instream(_body.toQByteArray());
            QPStream s;
            return s.decode(&instream,&result);
        }
        case Base64:
        {
            QTextStream instream(_body.toQByteArray());
            Base64Stream s;
            return s.decode(&instream,&result);
        }
    }
    return false;
}

void MailMessagePart::encodedBody(QTextStream& out) const
{
    //if the part is reference to an attachment
    //stream from the attachment
    //otherwise if we cannot find the attachment,
    //stream from the part body

    if(!storedFilename().isEmpty())
    {
        QFile infile(storedFilename());
        if(infile.open(QIODevice::ReadOnly))
        {
            QDataStream in(&infile);
            //stream and encode from file
            switch(_transferEncoding)
            {
                case EightBit:
                {
                    quint8 abyte;
                    while(!in.atEnd())
                    {
                        in >> abyte;
                        out << abyte;
                    }
                    break;
                }
                case QuotedPrintable:
                {
                    QPStream::encode(in,out);
                    break;
                }
                case Base64:
                {
                    Base64Stream b64s;
                    b64s.encode(&in,&out);
                    break;
                }
            }
            infile.close();
            return;
        }
    }

    const int kennedyChunk = 65536;
    int i = 0;
    while ( i*kennedyChunk < _body.length() ) {
        out << _body.toQByteArray().mid( i*kennedyChunk, kennedyChunk );
        ++i;
    }
}
