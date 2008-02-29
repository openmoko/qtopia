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

#include <qgsmcodec.h>

const unsigned char GUC = 0x10;     // GSM Undefined character

// Table from GSM 07.05, Annex A, combined with the extension table
// from GSM 03.38, Section 6.2.1.1.
static const unsigned short latin1GSMTable[256]=
{
//     0      1     2     3     4     5     6     7
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,    // 0x07
      GUC,  GUC, 0x0a,  GUC,  GUC, 0x0d,  GUC,  GUC,    // 0x0f
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     0x20, 0x21, 0x22, 0x23, 0x02, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
     0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5a, 0x1b3c, 0x1b2f, 0x1b3e, 0x1b14, 0x11,
      GUC, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
     0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
     0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
     0x78, 0x79, 0x7a, 0x1b28, 0x1b40, 0x1b29, 0x1b3d,  GUC,  // 0x7f

      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  // 0x8f
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  // 0x9f
      GUC, 0x40,  GUC, 0x01, 0x24, 0x03,  GUC, 0x5f,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC, 0x60,
     0x41, 0x41, 0x41, 0x41, 0x5b, 0x0e, 0x1c, 0x09,
     0x45, 0x1f, 0x45, 0x45, 0x49, 0x49, 0x49, 0x49, // 0xcf
      GUC, 0x5d, 0x4f, 0x4f, 0x4f, 0x4f, 0x5c,  GUC,
     0x0b, 0x55, 0x55, 0x55, 0x5e, 0x59,  GUC, 0x1e,
     0x7f, 0x61, 0x61, 0x61, 0x7b, 0x0f, 0x1d, 0x09,
     0x04, 0x05, 0x65, 0x65, 0x07, 0x69, 0x69, 0x69,
      GUC, 0x7d, 0x08, 0x6f, 0x6f, 0x6f, 0x7c,  GUC,
     0x0c, 0x06, 0x75, 0x75, 0x7e, 0x79,  GUC, 0x79
};

// Same as above, but with no loss of information due to two
// Latin1 characters mapping to the same thing.
static const unsigned short latin1GSMNoLossTable[256]=
{
//     0      1     2     3     4     5     6     7
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,    // 0x07
      GUC,  GUC, 0x0a,  GUC,  GUC, 0x0d,  GUC,  GUC,    // 0x0f
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     0x20, 0x21, 0x22, 0x23, 0x02, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
     0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5a, 0x1b3c, 0x1b2f, 0x1b3e, 0x1b14, 0x11,
      GUC, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
     0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
     0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
     0x78, 0x79, 0x7a, 0x1b28, 0x1b40, 0x1b29, 0x1b3d,  GUC,  // 0x7f

      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  // 0x8f
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  // 0x9f
      GUC, 0x40,  GUC, 0x01, 0x24, 0x03,  GUC, 0x5f,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
      GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC, 0x60,
      GUC,  GUC,  GUC,  GUC, 0x5b, 0x0e, 0x1c, 0x09,
      GUC, 0x1f,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC, // 0xcf
      GUC, 0x5d,  GUC,  GUC,  GUC,  GUC, 0x5c,  GUC,
     0x0b,  GUC,  GUC,  GUC, 0x5e,  GUC,  GUC, 0x1e,
     0x7f,  GUC,  GUC,  GUC, 0x7b, 0x0f, 0x1d,  GUC,
     0x04, 0x05,  GUC,  GUC, 0x07,  GUC,  GUC,  GUC,
      GUC, 0x7d, 0x08,  GUC,  GUC,  GUC, 0x7c,  GUC,
     0x0c, 0x06,  GUC,  GUC, 0x7e,  GUC,  GUC,  GUC
};

// Reversed version of latin1GSMTable.
static unsigned short gsmLatin1Table[256] =
{
    0x40, 0xa3, 0x24, 0xa5, 0xe8, 0xe9, 0xf9, 0xec,
    0xf2, 0xc7, 0x0a, 0xd8, 0xf8, 0x0d, 0xc5, 0xe5,
    0x00, 0x5f,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC, 0xc6, 0xe6, 0xdf, 0xc9,
    0x20, 0x21, 0x22, 0x23, 0xa4, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0xa1, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0xc4, 0xd6, 0xd1, 0xdc, 0xa7,
    0xbf, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0xe4, 0xf6, 0xf1, 0xfc, 0xe0,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC
};
static unsigned short extensionLatin1Table[256] =
{
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC, 0x5e,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
    0x7b, 0x7d,  GUC,  GUC,  GUC,  GUC,  GUC, 0x5c,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC, 0x5b, 0x7e, 0x5d,  GUC,
    0x7c,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC, 0x20ac,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,
     GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC,  GUC
};

/*!
    \class QGsmCodec qgsmcodec.h
    \brief The QGsmCodec class represents the text codec for the GSM 7-bit encoding of Latin-1
    \ingroup communication

    The GSM specifications for SMS use a compact 7-bit encoding to represent
    Latin-1 characters, compared to the more usual 8-bit ISO-8859-1 encoding
    used on many computer systems.

    The QGsmCodec class enables conversion back and forth between the GSM
    encoding and the normal Unicode encoding used by Qtopia.

    Application programs will rarely need to use this class, because
    the QSMSMessage class automatically converts between 7-bit and Unicode
    encodings as necessary.
*/

/*!
    Construct a new GSM text codec.  If \a noLoss is true, then the
    codec should not encode characters that may result in an
    ambiguous decoding.
*/
QGsmCodec::QGsmCodec( bool noLoss )
{
    this->noLoss = noLoss;
}

/*!
    Destruct a GSM text codec.
*/
QGsmCodec::~QGsmCodec()
{
}

/*!
    Get the name of this codec.
*/
QByteArray QGsmCodec::name() const
{
    if ( noLoss )
        return QByteArray( "gsm-noloss" );
    else
        return QByteArray( "gsm" );
}

/*!
    Get the MIB value associated with this codec.
*/
int QGsmCodec::mibEnum() const
{
    if ( noLoss )
        return 61237;
    else
        return 61238;
}

/*!
    Convert a single Unicode character \a c into GSM 7-bit.
    Returns 0x10 if the character cannot be mapped.  Use of this
    function is discouraged.

    Note: this will not work for two-byte GSM encodings.  Use
    twoByteFromUnicode() instead.
*/
char QGsmCodec::singleFromUnicode(QChar c)
{
    unsigned int ch = c.unicode();
    if ( ch < 256 )
        return (char)(latin1GSMTable[ch]);
    else
        return (char)GUC;
}

/*!
    Convert a single GSM 7-bit character \a ch into Unicode.  Use of this
    function is discouraged.

    Note: this will not work for two-byte GSM encodings.  Use
    twoByteToUnicode() instead.
*/
QChar QGsmCodec::singleToUnicode(char ch)
{
    return QChar((unsigned int)(gsmLatin1Table[((int)ch) & 0xFF]));
}

/*!
    Convert a Unicode character \a ch into its GSM-encoded counterpart.
    The return value will be greater than 256 if the Unicode character
    should be encoded as two bytes.
*/
unsigned short QGsmCodec::twoByteFromUnicode(QChar ch)
{
    unsigned short c = ch.unicode();
    if ( c == 0x20AC )  // Euro
        return 0x1b65;
    else if ( c < 256 )
        return latin1GSMTable[c];
    else
        return GUC;
}

/*!
    Convert a single GSM-encoded character into its Unicode counterpart.
    If \a ch is greater than 256, then it represents a two-byte sequence.
*/
QChar QGsmCodec::twoByteToUnicode(unsigned short ch)
{
    if ( ch < 256 )
        return QChar( gsmLatin1Table[ch] );
    else if ( ( ch & 0xFF00 ) != 0x1B00 )
        return QChar( 0 );
    else
        return QChar( extensionLatin1Table[ch & 0xFF] );
}

/*!
    Convert the \a length bytes at \a in into Unicode.  The \a state
    parameter is unused by this class.
*/
QString QGsmCodec::convertToUnicode(const char *in, int length, ConverterState *state) const
{
    QString str;
    unsigned short ch;
    while ( length > 0 ) {
        if ( *in == 0x1B ) {
            // Two-byte GSM sequence.
            ++in;
            --length;
            if ( length <= 0 ) {
                if ( state )
                    (state->invalidChars)++;
                break;
            }
            ch = extensionLatin1Table[((int)(*in)) & 0xFF];
            if ( ch != GUC )
                str += QChar((unsigned int)ch);
            else if ( state )
                (state->invalidChars)++;
        } else {
            ch = gsmLatin1Table[((int)(*in)) & 0xFF];
            if ( ch != GUC )
                str += QChar((unsigned int)ch);
            else if ( state )
                (state->invalidChars)++;
        }
        ++in;
        --length;
    }
    return str;
}

/*!
    Convert the \a length characters at \a in into 7-bit GSM.  The
    \a state parameter is unused by this class.
*/
QByteArray QGsmCodec::convertFromUnicode(const QChar *in, int length, ConverterState *state) const
{
    QByteArray result;
    unsigned int unicode;
    if ( noLoss ) {
        while ( length > 0 ) {
            unicode = (*in).unicode();
            if ( unicode == 0x20AC ) {    // Euro
                result += (char)0x1B;
                result += (char)0x65;
            } else if ( unicode < 256 ) {
                unsigned short code = latin1GSMNoLossTable[unicode];
                if ( code < 256 ) {
                    if(((char)code == GUC) && state)
                        (state->invalidChars)++;
                    result += (char)code;
                } else {
                    result += (char)(code >> 8);
                    result += (char)code;
                }
            } else {
                result += (char)GUC;
                if ( state )
                    (state->invalidChars)++;
            }
            ++in;
            --length;
        }
    } else {
        while ( length > 0 ) {
            unicode = (*in).unicode();
            if ( unicode == 0x20AC ) {    // Euro
                result += (char)0x1B;
                result += (char)0x65;
            } else if ( unicode < 256 ) {
                unsigned short code = latin1GSMTable[unicode];
                if ( code < 256 ) {
                    result += (char)code;
                } else {
                    result += (char)(code >> 8);
                    result += (char)code;
                }
            } else {
                result += (char)GUC;
                if ( state )
                    (state->invalidChars)++;
            }
            ++in;
            --length;
        }
    }
    return result;
}

