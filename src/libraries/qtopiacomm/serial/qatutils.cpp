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

#include <qatutils.h>
#include <qatresultparser.h>
#include <qgsmcodec.h>
#include <qtextcodec.h>

/*!
    \class QAtUtils
    \brief The QAtUtils class provides utility functions for use with AT modems
    \ingroup communication

    The QAtUtils provides several utility functions to assist with
    using AT-based modem commands.
*/

static const char hexchars[] = "0123456789ABCDEF";
static bool octalEscapesFlag = false;

/*!
    Quote \a str so that it is suitable to be sent as an AT
    command argument.  The caller will need to add double
    quotes to the start and end of the return value to complete
    the AT command argument.
*/
QString QAtUtils::quote( const QString& str )
{
    // Bail out if the string does not need to be quoted.
    if ( str.indexOf( QChar('"') ) == -1 && str.indexOf( QChar('\\') ) == -1 &&
         str.indexOf( QChar('\r') ) == -1 && str.indexOf( QChar('\n') ) == -1 ) {
        return str;
    }

    // Build the quoted result.
    QString result = "";
    int posn = 0;
    uint ch;
    while ( posn < str.length() ) {
        ch = str[posn++].unicode();
        if ( ch == '"' || ch == '\\' || ch == '\r' || ch == '\n' ) {
            result += (QChar)'\\';
            result += (QChar)(hexchars[(ch >> 4) & 0x0F]);
            result += (QChar)(hexchars[ch & 0x0F]);
        } else {
            result += (QChar)ch;
        }
    }
    return result;
}

/*!
    Convert the byte array, \a binary, into a hexadecimal string.
*/
QString QAtUtils::toHex( const QByteArray& binary )
{
    QString str = "";
    static char const hexchars[] = "0123456789ABCDEF";

    for ( int i = 0; i < binary.size(); i++ ) {
        str += (QChar)(hexchars[ (binary[i] >> 4) & 0x0F ]);
        str += (QChar)(hexchars[ binary[i] & 0x0F ]);
    }

    return str;
}

/*!
    Convert a hexadecimal string, \a hex, into a byte array.
*/
QByteArray QAtUtils::fromHex( const QString& hex )
{
    QByteArray bytes;
    uint ch;
    int posn;
    int nibble, value, flag, size;

    flag = 0;
    value = 0;
    size = 0;
    for ( posn = 0; posn < hex.length(); ++posn ) {
        ch = (uint)( hex[posn].unicode() );
        if ( ch >= '0' && ch <= '9' ) {
            nibble = ch - '0';
        } else if ( ch >= 'A' && ch <= 'F' ) {
            nibble = ch - 'A' + 10;
        } else if ( ch >= 'a' && ch <= 'f' ) {
            nibble = ch - 'a' + 10;
        } else {
            continue;
        }
        value = (value << 4) | nibble;
        flag = !flag;
        if ( !flag ) {
            bytes.resize( size + 1 );
            bytes[size++] = (char)value;
            value = 0;
        }
    }

    return bytes;
}

/*!
    Decode a phone number from \a value and \a type.  The \a type is
    a type of address octet, usually 145 for international numbers
    and 129 for local numbers.  The return will normalize the
    value to include the \c{+} prefix for international numbers.
*/
QString QAtUtils::decodeNumber( const QString& value, uint type )
{
    if ( type == 145 && value.length() != 0 && value[0] != '+' )
        return "+" + value;
    else
        return value;
}

/*!
    Read a string field and a numeric field from \a parser and
    then decode them into a properly normalized phone number.
*/
QString QAtUtils::decodeNumber( QAtResultParser& parser )
{
    QString value = parser.readString();
    uint type = parser.readNumeric();
    return decodeNumber( value, type );
}

/*!
    Encode the phone number in \a value into a string plus a
    type of address octet.  International numbers that start
    with \c{+} become \c{"number",145}, and local numbers
    become \c{"number",129}.  If \a keepPlus is true,
    then the \c{+} will be left on the resulting number,
    even if the type is 145.
*/
QString QAtUtils::encodeNumber( const QString& value, bool keepPlus )
{
    if ( value.length() > 0 && value[0] == '+' ) {
        if ( keepPlus )
            return "\"" + quote( value ) + "\",145";
        else
            return "\"" + quote( value.mid(1) ) + "\",145";
    } else {
        return "\"" + quote( value ) + "\",129";
    }
}

static int FromHexDigit( uint ch )
{
    if ( ch >= '0' && ch <= '9' ) {
        return (int)( ch - '0' );
    } else if ( ch >= 'A' && ch <= 'F' ) {
        return (int)( ch - 'A' + 10 );
    } else if ( ch >= 'a' && ch <= 'f' ) {
        return (int)( ch - 'a' + 10 );
    } else {
        return -1;
    }
}

static int FromOctalDigit( uint ch )
{
    if ( ch >= '0' && ch <= '7' ) {
        return (int)( ch - '0' );
    } else {
        return -1;
    }
}

/*!
    Extract the next quoted string from \a buf, starting at
    \a posn.
*/
QString QAtUtils::nextString( const QString& buf, uint& posn )
{
    QString result = "";
    uint ch;
    int digit, digit2, digit3;
    while ( posn < (uint)(buf.length()) && buf[posn] != '"' ) {
        ++posn;
    }
    if ( posn >= (uint)(buf.length()) ) {
        return result;
    }
    ++posn;
    while ( posn < (uint)(buf.length()) && ( ch = buf[posn].unicode() ) != '"' ) {
        ++posn;
        if ( ch == '\\' ) {
            if ( !octalEscapesFlag ) {
                // Hex-quoted character.
                if ( posn >= (uint)buf.length() )
                    break;
                digit = FromHexDigit( buf[posn].unicode() );
                if ( digit == -1 ) {
                    result += (QChar)'\\';
                    continue;
                }
                if ( ( posn + 1 ) >= (uint)buf.length() ) {
                    ch = (uint)digit;
                    ++posn;
                } else {
                    digit2 = FromHexDigit( buf[posn + 1].unicode() );
                    if ( digit2 == -1 ) {
                        ch = (uint)digit;
                        ++posn;
                    } else {
                        ch = (uint)(digit * 16 + digit2);
                        posn += 2;
                    }
                }
            } else {
                // Octal-quoted character.
                if ( posn >= (uint)buf.length() )
                    break;
                digit = FromOctalDigit( buf[posn].unicode() );
                if ( digit == -1 ) {
                    result += (QChar)'\\';
                    continue;
                }
                if ( ( posn + 1 ) >= (uint)buf.length() ) {
                    ch = (uint)digit;
                    ++posn;
                } else {
                    digit2 = FromOctalDigit( buf[posn + 1].unicode() );
                    if ( digit2 == -1 ) {
                        ch = (uint)digit;
                        ++posn;
                    } else {
                        if ( ( posn + 2 ) >= (uint)buf.length() ) {
                            ch = (uint)(digit * 8 + digit2);
                            posn += 2;
                        } else {
                            digit3 = FromOctalDigit( buf[posn + 2].unicode() );
                            if ( digit3 == -1 ) {
                                ch = (uint)(digit * 8 + digit2);
                                posn += 2;
                            } else {
                                ch = (uint)(digit * 64 + digit2 * 8 + digit3);
                                posn += 3;
                            }
                        }
                    }
                }
            }
        }
        result += (QChar)ch;
    }
    if ( posn < (uint)buf.length() ) {
        ++posn;
    }
    return result;
}

/*!
    Utility function for parsing a number from position \a posn in \a str.
*/
uint QAtUtils::parseNumber( const QString& str, uint& posn )
{
    uint num = 0;
    while ( posn < (uint)str.length() && ( str[posn] == ' ' || str[posn] == ',' ) ) {
        ++posn;
    }
    while ( posn < (uint)str.length() && str[posn] >= '0' && str[posn] <= '9' ) {
        num = num * 10 + (uint)(str[posn].unicode() - '0');
        ++posn;
    }
    return num;
}

/*!
    Utility function for skipping a comma-delimited field starting
    at \a posn within \a str.
*/
void QAtUtils::skipField( const QString& str, uint& posn )
{
    if ( posn < (uint)str.length() && str[posn] == ',' ) {
        ++posn;
    }
    while ( posn < (uint)str.length() && str[posn] != ',' ) {
        ++posn;
    }
}

class QGsmHexCodec : public QTextCodec
{
public:
    QGsmHexCodec();
    ~QGsmHexCodec();

    QByteArray name() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

QGsmHexCodec::QGsmHexCodec()
{
}

QGsmHexCodec::~QGsmHexCodec()
{
}

QByteArray QGsmHexCodec::name() const
{
    return QByteArray( "gsm-hex" );
}

int QGsmHexCodec::mibEnum() const
{
    return 61239;
}

QString QGsmHexCodec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    int nibble = 0;
    int value = 0;
    int digit;
    while ( length-- > 0 ) {
        char ch = *in++;
        if ( ch >= '0' && ch <= '9' )
            digit = ch - '0';
        else if ( ch >= 'A' && ch <= 'F' )
            digit = ch - 'A' + 10;
        else if ( ch >= 'a' && ch <= 'f' )
            digit = ch - 'a' + 10;
        else
            continue;
        if ( !nibble ) {
            value = digit * 16;
            nibble = 1;
        } else {
            str += QGsmCodec::singleToUnicode( (char)( value + digit ) );
            nibble = 0;
        }
    }
    return str;
}

QByteArray QGsmHexCodec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray buf;
    while ( length-- > 0 ) {
        char ch = QGsmCodec::singleFromUnicode( *in++ );
        buf += hexchars[ (ch >> 4) & 0x0F ];
        buf += hexchars[ ch & 0x0F ];
    }
    return buf;
}

class QUcs2HexCodec : public QTextCodec
{
public:
    QUcs2HexCodec();
    ~QUcs2HexCodec();

    QByteArray name() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

QUcs2HexCodec::QUcs2HexCodec()
{
}

QUcs2HexCodec::~QUcs2HexCodec()
{
}

QByteArray QUcs2HexCodec::name() const
{
    return "ucs2-hex";
}

int QUcs2HexCodec::mibEnum() const
{
    return 61240;
}

QString QUcs2HexCodec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    int nibble = 0;
    int value = 0;
    int digit;
    while ( length-- > 0 ) {
        char ch = *in++;
        if ( ch >= '0' && ch <= '9' )
            digit = ch - '0';
        else if ( ch >= 'A' && ch <= 'F' )
            digit = ch - 'A' + 10;
        else if ( ch >= 'a' && ch <= 'f' )
            digit = ch - 'a' + 10;
        else
            continue;
        value = value * 16 + digit;
        ++nibble;
        if ( nibble >= 4 ) {
            str += QChar( (ushort)value );
            nibble = 0;
            value = 0;
        }
    }
    return str;
}

QByteArray QUcs2HexCodec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray buf;
    while ( length-- > 0 ) {
        uint ch = in->unicode();
        ++in;
        buf += hexchars[ (ch >> 12) & 0x0F ];
        buf += hexchars[ (ch >> 8) & 0x0F ];
        buf += hexchars[ (ch >> 4) & 0x0F ];
        buf += hexchars[ ch & 0x0F ];
    }
    return buf;
}

class QCodePage437Codec : public QTextCodec
{
public:
    QCodePage437Codec();
    ~QCodePage437Codec();

    QByteArray name() const;
    QList<QByteArray> aliases() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

// Convert IBM437 character codes 0x80 - 0xFF into Unicode.
static ushort const cp437ToUnicode[128] =
   {0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
    0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
    0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
    0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
    0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
    0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
    0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
    0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
    0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
    0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0};

// Convert Unicode 0x0080 - 0x00FF into IBM437.
static unsigned char const cp437FromUnicode[128] =
   {'?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    0xff, 0xad, 0x9b, 0x9c, '?' , 0x9d, '?' , '?' ,
    '?' , '?' , 0xa6, 0xae, 0xaa, '?' , '?' , '?' ,
    0xf8, 0xf1, 0xfd, '?' , '?' , 0xe6, '?' , 0xfa,
    '?' , '?' , 0xa7, 0xaf, 0xac, 0xab, '?' , 0xa8,
    '?' , '?' , '?' , '?' , 0x8e, 0x8f, 0x92, 0x80,
    '?' , 0x90, '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , 0xa5, '?' , '?' , '?' , '?' , 0x99, '?' ,
    '?' , '?' , '?' , '?' , 0x9a, '?' , '?' , 0xe1,
    0x85, 0xa0, 0x83, '?' , 0x84, 0x86, 0x91, 0x87,
    0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
    '?' , 0xa4, 0x95, 0xa2, 0x93, '?' , 0x94, 0xf6,
    '?' , 0x97, 0xa3, 0x96, 0x81, '?' , '?' , 0x98};

QCodePage437Codec::QCodePage437Codec()
{
}

QCodePage437Codec::~QCodePage437Codec()
{
}

QByteArray QCodePage437Codec::name() const
{
    return "IBM437";
}

QList<QByteArray> QCodePage437Codec::aliases() const
{
    QList<QByteArray> list;
    list << "CP437";
    return list;
}

int QCodePage437Codec::mibEnum() const
{
    return 437;
}

QString QCodePage437Codec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    if ( length >= 6 && in[0] == '8' && in[1] == '0' &&
         in[length - 4] == 'F' && in[length - 3] == 'F' &&
         in[length - 2] == 'F' && in[length - 1] == 'F') {

        // UCS-2 string embedded within a 437-encoded string.
        int nibble = 0;
        int value = 0;
        int digit;
        in += 2;
        length -= 6;
        while ( length-- > 0 ) {
            char ch = *in++;
            if ( ch >= '0' && ch <= '9' )
                digit = ch - '0';
            else if ( ch >= 'A' && ch <= 'F' )
                digit = ch - 'A' + 10;
            else if ( ch >= 'a' && ch <= 'f' )
                digit = ch - 'a' + 10;
            else
                continue;
            value = value * 16 + digit;
            ++nibble;
            if ( nibble >= 4 ) {
                str += QChar( (ushort)value );
                nibble = 0;
                value = 0;
            }
        }

    } else {

        // Regular 437-encoded string.
        unsigned char ch;
        while ( length-- > 0 ) {
            ch = (unsigned char)(*in++);
            if ( ch < 0x80 )
                str += QChar((unsigned int)ch);
            else {
                str += QChar((unsigned int)cp437ToUnicode[ch - 0x80]);
            }
        }

    }
    return str;
}

QByteArray QCodePage437Codec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray result;
    unsigned int ch;
    char *out;

    // Determine if the string should be encoded using the UCS-2 hack.
    bool non437 = false;
    for ( int posn = 0; !non437 && posn < length; ++posn ) {
        ch = in[posn].unicode();
        if ( ch >= 0x0100 )
            non437 = true;
        else if ( ch >= 0x0080 && cp437FromUnicode[ch - 0x80] == '?' )
            non437 = true;
    }
    if ( non437 ) {
        // There is a non CP437 character in this string, so use UCS-2.
        result.resize( length * 4 + 6 );
        out = result.data();
        *out++ = '8';
        *out++ = '0';
        while ( length-- > 0 ) {
            uint ch = in->unicode();
            ++in;
            *out++ += hexchars[ (ch >> 12) & 0x0F ];
            *out++ += hexchars[ (ch >> 8) & 0x0F ];
            *out++ += hexchars[ (ch >> 4) & 0x0F ];
            *out++ += hexchars[ ch & 0x0F ];
        }
        *out++ = 'F';
        *out++ = 'F';
        *out++ = 'F';
        *out   = 'F';
        return result;
    }

    // If we get here, we can guarantee that the string only contains
    // valid CP437 code points between 0x0000 and 0x00FF.
    result.resize( length );
    out = result.data();
    while ( length-- > 0 ) {
        ch = in->unicode();
        if ( ch < 0x0080 )
            *out++ = (char)ch;
        else
            *out++ = (char)cp437FromUnicode[ch - 0x80];
        ++in;
    }
    return result;
}

/*!
    Get the text codec for the GSM character set identifier \a gsmCharset.
*/
QTextCodec *QAtUtils::codec( const QString& gsmCharset )
{
    QString cs = gsmCharset.toLower();
    QTextCodec *codec = 0;

    // Convert the name into an appropriate codec.
    if ( cs == "gsm" ) {
        // 7-bit GSM character set.
        static QTextCodec *gsm = 0;
        if ( !gsm )
            gsm = new QGsmCodec();
        codec = gsm;
    } else if ( cs == "gsm-noloss" ) {
        // 7-bit GSM character set, with no loss of quality.
        static QTextCodec *gsmNoLoss = 0;
        if ( !gsmNoLoss )
            gsmNoLoss = new QGsmCodec( true );
        codec = gsmNoLoss;
    } else if ( cs == "hex" ) {
        // Direct hex character set.  The underlying character set could
        // be anything according to the specification, but we need to pick
        // something.  We assume that it is 7-bit GSM, as that is the most
        // likely value.
        static QTextCodec *hex = 0;
        if ( !hex )
            hex = new QGsmHexCodec();
        codec = hex;
    } else if ( cs == "ucs2" ) {
        // Hex-encoded UCS2 character set.
        static QTextCodec *ucs2 = 0;
        if ( !ucs2 )
            ucs2 = new QUcs2HexCodec();
        codec = ucs2;
    } else if ( cs == "ira" ) {
        // International Reference Alphabet (i.e. ASCII).  Use Latin-1 codec.
        codec = QTextCodec::codecForName( "ISO-8859-1" );
    } else if ( cs == "pccp437" ) {
        // PC DOS code page 437, which isn't standard in Qt,
        // but which is the default charset for Wavecom modems.
        //
        // Wavecom also has a horrible hack in its PCCP437 implementation
        // to handle embedded UCS-2 character strings.  A hex UCS-2 string
        // will start with "80" and end with "FFFF".  If the string does
        // not have this format, it is interpreted as code page 437.
        static QTextCodec *cp437 = 0;
        if ( !cp437 )
            cp437 = new QCodePage437Codec();
        codec = cp437;
    } else if ( cs == "pcdn" ) {
        // PC Danish/Norwegian character set.  Map to PC code page 850.
        codec = QTextCodec::codecForName( "CP850" );
    } else if ( cs.startsWith( "pccp" ) ) {
        // Some other PC DOS code page.
        codec = QTextCodec::codecForName( "CP" + cs.mid(4).toLatin1() );
    } else if ( cs == "8859-c" ) {
        // ISO 8859 Latin/Cyrillic - same as ISO-8859-5.
        codec = QTextCodec::codecForName( "ISO-8859-5" );
    } else if ( cs == "8859-a" ) {
        // ISO 8859 Latin/Arabic - same as ISO-8859-6.
        codec = QTextCodec::codecForName( "ISO-8859-6" );
    } else if ( cs == "8859-g" ) {
        // ISO 8859 Latin/Greek - same as ISO-8859-7.
        codec = QTextCodec::codecForName( "ISO-8859-7" );
    } else if ( cs == "8859-h" ) {
        // ISO 8859 Latin/Hebrew - same as ISO-8859-8.
        codec = QTextCodec::codecForName( "ISO-8859-8" );
    } else if ( cs.startsWith( "8859-" ) ) {
        // ISO-8859-n character set.
        codec = QTextCodec::codecForName( "ISO-" + cs.toLatin1() );
    } else {
        // Not one of the standard GSM charset specifiers.  Look it up as-is.
        codec = QTextCodec::codecForName( gsmCharset.toLatin1() );
    }

    // Return the codec that we found, or bail out with Latin1 if unknown.
    if ( codec )
        return codec;
    else
        return QTextCodec::codecForName( "ISO-8859-1" );
}

/*!
    Quote \a str so that it is suitable to be sent as an AT
    command argument and use \a codec to encode the string.
    The caller will need to add double quotes to the start and
    end of the return value to complete the AT command argument.
*/
QString QAtUtils::quote( const QString& str, QTextCodec *codec )
{
    // Convert the string into raw bytes.
    QByteArray bytes;
    if ( codec )
        bytes = codec->fromUnicode( str );
    else
        bytes = str.toLatin1();

    // Determine if we need to apply quoting to the string.
    if ( bytes.indexOf( '"' ) == -1 && bytes.indexOf( '\\' ) == -1 &&
         bytes.indexOf( '\r' ) == -1 && bytes.indexOf( '\n' ) == -1 ) {
        return QString::fromLatin1( bytes.data(), bytes.length() );
    }

    // Create a new string for the quoted form.  The result is suitable
    // for converting to bytes again with toLatin1 in QAtChat::writeLine().
    QString result = "";
    int posn = 0;
    int ch;
    while ( posn < bytes.length() ) {
        ch = bytes[posn++] & 0xFF;
        if ( ch == '"' || ch == '\\' || ch == '\r' || ch == '\n' ) {
            result += (QChar)'\\';
            result += (QChar)(hexchars[(ch >> 4) & 0x0F]);
            result += (QChar)(hexchars[ch & 0x0F]);
        } else {
            result += (QChar)ch;
        }
    }
    return result;
}

/*!
    Decode \a str according to \a codec.  The string is assumed to have
    been retrieved from an AT modem using the facilities in QAtChat
    and QAtResultParser.
*/
QString QAtUtils::decode( const QString& str, QTextCodec *codec )
{
    if ( codec )
        return codec->toUnicode( str.toLatin1() );
    else
        return str;
}

/*!
    Strip non-digit characters from \a number and normalize special
    characters.
*/
QString QAtUtils::stripNumber( const QString& number )
{
    QString n = "";
    int posn;
    uint ch;
    for ( posn = 0; posn < number.length(); ++posn ) {
        ch = number[posn].unicode();
        if ( ch >= '0' && ch <= '9' ) {
            n += (QChar)ch;
        } else if ( ch == '+' || ch == '#' || ch == '*' ) {
            n += (QChar)ch;
        } else if ( ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' ) {
            // ABCD can actually be digits!
            n += (QChar)ch;
        } else if ( ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' ) {
            n += (QChar)( ch - 'a' + 'A' );
        } else if ( ch == ',' || ch == 'p' || ch == 'P' || ch == 'X' || ch == 'x' ) {
            // Comma and 'p' mean a short pause.
            // 'x' means an extension, which for now is the same as a pause.
            n += (QChar)',';
        } else if ( ch == 'w' || ch == 'W' ) {
            // 'w' means wait for dial tone.
            n += (QChar)'W';
        } else if ( ch == '!' || ch == '@' ) {
            // '!' = hook flash, '@' = wait for silence.
            n += (QChar)ch;
        }
    }
    return n;
}

/*!
    Determine if nextString() should parse backslash escape
    sequences in octal rather than the default of hexadecimal.
*/
bool QAtUtils::octalEscapes()
{
    return octalEscapesFlag;
}

/*!
    Indicate that nextString() should parse backslash escape
    sequences in octal if \a value is true, rather than the
    default of hexadecimal (\value is false).
*/
void QAtUtils::setOctalEscapes( bool value )
{
    octalEscapesFlag = value;
}
