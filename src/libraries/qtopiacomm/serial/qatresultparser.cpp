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

#include <qatresultparser.h>
#include <qatresult.h>

/*!
    \class QAtResultParser
    \brief The QAtResultParser class provides support for parsing the response to AT modem commands
    \ingroup communication

    The QAtResultParser provides functions that support parsing the response
    from AT modem commands and unsolicited notifications.

    The following example demonstrates how to parse the response to a
    \c{AT+CPOL} (preferred operator list) query:

    \code
        QAtResultParser parser( result );
        while ( parser.next( "+CPOL:" ) ) {
            uint index = parser.readNumeric();
            uint format = parser.readNumeric();
            QString name;
            if ( format == 2 )  // Numeric format.
                name = QString::number( parser.readNumeric() );
            else                // String format.
                name = parser.readString();
        }
    \endcode

    \sa QAtResult
*/

class QAtResultParserPrivate
{
public:
    QAtResultParserPrivate( const QString& content )
    {
        response = content;
        posn = 0;
        linePosn = 0;
        notification = false;
    }

    QString response;
    QString line;
    int posn;
    int linePosn;
    bool notification;
};

/*!
    Construct an AT modem result parser to parse the content of \a result.
    The caller will typically follow this with a call to next() to position
    the parser on the first line of relevant result data.
*/
QAtResultParser::QAtResultParser( const QAtResult& result )
{
    d = new QAtResultParserPrivate( result.content() );
}

/*!
    Construct an AT modem result parser and initialize it to parse
    the specified unsolicited \a notification.  Notifications are
    expected to have the format \c{NAME: VALUE}.  The next() function
    will be called internally to position the parser at \c{VALUE}.
*/
QAtResultParser::QAtResultParser( const QString& notification )
{
    d = new QAtResultParserPrivate( notification );
    int posn = 0;
    while ( posn < notification.length() && notification[posn] != ':' ) {
        ++posn;
    }
    if ( posn < notification.length() )
        ++posn;     // Account for the colon.
    next( notification.left( posn ) );
    d->notification = true;
}

/*!
    Destruct this AT modem result parser.
*/
QAtResultParser::~QAtResultParser()
{
    delete d;
}

/*!
    Reset this AT modem result parser to the beginning of the content.
*/
void QAtResultParser::reset()
{
    if ( d->notification ) {
        d->linePosn = 0;
    } else {
        d->line = QString();
        d->posn = 0;
        d->linePosn = 0;
    }
}

/*!
    Position this AT modem result parser on the next line that begins
    with \a prefix.
*/
bool QAtResultParser::next( const QString& prefix )
{
    while ( d->posn < d->response.length() ) {

        // Extract the next line.
        d->line = "";
        d->linePosn = 0;
        while ( d->posn < d->response.length() &&
                d->response[d->posn] != '\n' ) {
            d->line += d->response[(d->posn)++];
        }
        if ( d->posn < d->response.length() ) {
            ++(d->posn);
        }

        // Bail out if the line starts with the expected prefix.
        if ( d->line.startsWith( prefix ) ) {
            d->linePosn = prefix.length();
            while ( d->linePosn < d->line.length() &&
                    d->line[d->linePosn] == ' ' ) {
                ++(d->linePosn);
            }
            d->line = d->line.mid( d->linePosn );
            d->linePosn = 0;
            return true;
        }

    }
    return false;
}

/*!
    Get the full content of the line that next() positioned us on.
    The line's prefix is not included in the return value.
*/
QString QAtResultParser::line()
{
    return d->line;
}

/*!
    Read a numeric value from the current line.
*/
uint QAtResultParser::readNumeric()
{
    uint value = 0;
    while ( d->linePosn < d->line.length() &&
            d->line[d->linePosn] >= '0' && d->line[d->linePosn] <= '9' ) {
        value = value * 10 + (uint)(d->line[d->linePosn].unicode() - '0');
        ++(d->linePosn);
    }
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == ',' ) {
        ++(d->linePosn);
    }
    while ( d->linePosn < d->line.length() && d->line[d->linePosn] == ' ' ) {
        ++(d->linePosn);
    }
    return value;
}

static int fromHexDigit( uint ch )
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

static QString nextString( const QString& buf, int& posn )
{
    QString result = "";
    uint ch;
    int digit, digit2;
    while ( posn < buf.length() && buf[posn] != '"' ) {
        ++posn;
    }
    if ( posn >= buf.length() ) {
        return result;
    }
    ++posn;
    while ( posn < buf.length() && ( ch = buf[posn].unicode() ) != '"' ) {
        ++posn;
        if ( ch == '\\' ) {
            // Hex-quoted character.
            if ( posn >= buf.length() )
                break;
            digit = fromHexDigit( buf[posn].unicode() );
            if ( digit == -1 ) {
                result += (QChar)'\\';
                continue;
            }
            if ( ( posn + 1 ) >= buf.length() ) {
                ch = (uint)digit;
                ++posn;
            } else {
                digit2 = fromHexDigit( buf[posn + 1].unicode() );
                if ( digit2 == -1 ) {
                    ch = (uint)digit;
                    ++posn;
                } else {
                    ch = (uint)(digit * 16 + digit2);
                    posn += 2;
                }
            }
        }
        result += (QChar)ch;
    }
    if ( posn < buf.length() ) {
        ++posn;
    }
    return result;
}

/*!
    Read a string from the current line.
*/
QString QAtResultParser::readString()
{
    QString value = nextString( d->line, d->linePosn );
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == ',' ) {
        ++(d->linePosn);
    }
    while ( d->linePosn < d->line.length() && d->line[d->linePosn] == ' ' ) {
        ++(d->linePosn);
    }
    return value;
}

/*!
    Skip the contents of a comma-separated field in the current line.
*/
void QAtResultParser::skip()
{
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == ',' ) {
        ++(d->linePosn);
    }
    while ( d->linePosn < d->line.length() && d->line[d->linePosn] != ',' ) {
        ++(d->linePosn);
    }
}

/*!
    Read the next line of input as literal text, without looking for a prefix.
    This is for results from commands such as \c{AT+CMGL} which place the
    PDU on a line of its own.
*/
QString QAtResultParser::readNextLine()
{
    QString line = "";

    while ( d->posn < d->response.length() &&
            d->response[d->posn] != '\n' ) {
        line += d->response[(d->posn)++];
    }
    if ( d->posn < d->response.length() ) {
        ++(d->posn);
    }

    return line;
}

/*!
    Get the content of all lines that begin with \a prefix starting
    at the current position.
*/
QStringList QAtResultParser::lines( const QString& prefix )
{
    QStringList result;
    while ( next( prefix ) ) {
        result << d->line;
    }
    return result;
}

/*!
    Read a list of values surrounded by parentheses.  This is for
    complex command results that cannot be parsed with readNumeric()
    and readString().
*/
QList<QAtResultParser::Node> QAtResultParser::readList()
{
    QList<QAtResultParser::Node> list;
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == '(' ) {
        ++(d->linePosn);
        while ( d->linePosn < d->line.length() &&
                d->line[d->linePosn] != ')' ) {
            uint ch = d->line[d->linePosn].unicode();
            if ( ch >= '0' && ch <= '9' ) {
                // Parse a number or range.
                uint number = readNumeric();
                if ( d->linePosn < d->line.length() &&
                     d->line[d->linePosn] == '-' ) {
                    ++(d->linePosn);
                    uint last = readNumeric();
                    list.append( QAtResultParser::Node( number, last ) );
                } else {
                    list.append( QAtResultParser::Node( number ) );
                }
            } else if ( ch == '"' ) {
                // Parse a string.
                list.append( QAtResultParser::Node( readString() ) );
            } else {
                // Encountered something unknown - bail out at this point.
                d->linePosn = d->line.length();
                return list;
            }
        }
        if ( d->linePosn < d->line.length() ) {
            // Skip the ')' at the end of the list.
            ++(d->linePosn);
            if ( d->linePosn < d->line.length() &&
                 d->line[d->linePosn] == ',' ) {
                // Skip a trailing comma.
                ++(d->linePosn);
            }
            while ( d->linePosn < d->line.length() &&
                 d->line[d->linePosn] == ' ' ) {
                // Skip trailing white space.
                ++(d->linePosn);
            }
        }
    }
    return list;
}

/*!
    \class QAtResultParser::Node
    \brief Provides access to a generic value parsed from an AT command list

    The QAtResultParser::Node class provides access to a generic value
    that was parsed from an AT command list.  It is returned from
    the QAtResultParser::readList() class.

    \sa QAtResultParser
*/

QAtResultParser::Node::Node( uint number )
{
    _kind = Number;
    _number = number;
    _list = 0;
}

QAtResultParser::Node::Node( uint first, uint last )
{
    _kind = Range;
    _number = first;
    _last = last;
}

QAtResultParser::Node::Node( const QString& str )
{
    _kind = String;
    _str = str;
    _list = 0;
}

QAtResultParser::Node::Node( QList<Node> *list )
{
    _kind = List;
    _list = list;
}

/*!
    Create a new result list node from \a other.
*/
QAtResultParser::Node::Node( const Node& other )
{
    _kind = other._kind;
    _number = other._number;
    _str = other._str;
    _list = ( other._list ? new QList<Node>( *other._list ) : 0 );
}

/*!
    Destruct this node.
*/
QAtResultParser::Node::~Node()
{
    if ( _list )
        delete _list;
}

/*!
    \fn bool QAtResultParser::Node::isNumber() const

    Determine if this node contains a number.

    \sa asNumber()
*/

/*!
    \fn bool QAtResultParser::Node::isRange() const

    Determine if this node contains a range.

    \sa asFirst(), asLast()
*/

/*!
    \fn bool QAtResultParser::Node::isString() const

    Determine if this node contains a string.

    \sa asString()
*/

/*!
    \fn bool QAtResultParser::Node::isList() const

    Determine if this node contains a list.

    \sa asList()
*/

/*!
    \fn uint QAtResultParser::Node::asNumber() const

    Get the number contained within this node, or zero if not a number.

    \sa isNumber()
*/

/*!
    \fn uint QAtResultParser::Node::asFirst() const

    Get the first number in a range that is contained within this node,
    or zero if not a range.

    \sa isRange(), asLast()
*/

/*!
    \fn uint QAtResultParser::Node::asLast() const

    Get the last number in a range that is contained within this node,
    or zero if not a range.

    \sa isRange(), asFirst()
*/

/*!
    \fn QString QAtResultParser::Node::asString() const

    Get the string contained within this node, or null if not a string.

    \sa isString()
*/

/*!
    \fn QList<QAtResultParser::Node> QAtResultParser::Node::asList() const

    Get the list contained within this node, or an empty list if this
    node does not contain a list.

    \sa isList()
*/
