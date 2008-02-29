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

#include <qatchatscript.h>
#include <qtopialog.h>
#include <qstringlist.h>
#include <qfile.h>

/*!
    \class QAtChatScript
    \brief The QAtChatScript class provides a mechanism to run pppd chat scripts.
    \ingroup communication

    The QAtChatScript class provides a mechanism to run pppd chat scripts,
    such as those described in the chat(1) Unix man page.

    This class is intended for use by Qtopia's network component.  Other
    components should use QAtChat.

    \sa QAtChat
*/

class QAtChatScriptPrivate
{
public:
    QAtChatScriptPrivate( QAtChat *chat )
    {
        atchat = chat;
        posn = 0;
        state = 0;
        saveState = 0;
    }

    QAtChat *atchat;
    QStringList commands;
    int posn;
    int state;
    int saveState;
};

/*!
    Construct a chat script object that sends AT commands from
    the script on \a atchat.  The new object will be attached to \a parent.
*/
QAtChatScript::QAtChatScript( QAtChat *atchat, QObject *parent )
    : QObject( parent )
{
    d = new QAtChatScriptPrivate( atchat );
}

/*!
    Destruct this chat script object.
*/
QAtChatScript::~QAtChatScript()
{
    delete d;
}

/*!
    Get the total number of commands to be executed in the chat script.
*/
int QAtChatScript::totalCommands() const
{
    return d->commands.size();
}

/*!
    Get the number of commands that were successful.  If this is less
    than totalCommands() when the done() signal is emitted, then the
    script stopped with an error before the whole script could be
    executed.
*/
int QAtChatScript::successfulCommands() const
{
    return d->posn;
}

/*!
    Run the chat script within \a filename.
*/
void QAtChatScript::runChatFile( const QString& filename )
{
    QFile file( filename );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QString contents( file.readAll() );
        file.close();
        runChat( contents );
    } else {
        qLog(AtChat) << "QAtChatScript::runChatFile: could not open" << filename;
        QAtResult result;
        result.setResultCode( QAtResult::Error );
        emit done( false, result );
    }
}

/*!
    Run a literal \a chatScript.
*/
void QAtChatScript::runChat( const QString& chatScript )
{
    // Reset the parse state.
    d->commands.clear();
    d->posn = 0;
    d->state = 0;
    d->saveState = 0;

    // Parse the chat script.
    bool prevWasNL = true;
    int posn = 0;
    uint ch, quote;
    QString value;
    while ( posn < chatScript.length() ) {
        ch = chatScript[posn++].unicode();
        if ( ch == '#' && prevWasNL ) {
            // Comment line.
            while ( posn < chatScript.length() ) {
                ch = chatScript[posn++].unicode();
                if ( ch == '\n' )
                    break;
            }
        } else if ( ch == '\'' || ch == '"' ) {
            // Start of a quoted value.
            quote = ch;
            value = "";
            while ( posn < chatScript.length() ) {
                ch = chatScript[posn++].unicode();
                if ( ch == quote )
                    break;
                value += QChar( ch );
            }
            word( expandEscapes( value ) );
        } else if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' ) {
            // Start of an unquoted value.
            value = "";
            value += QChar( ch );
            while ( posn < chatScript.length() ) {
                ch = chatScript[posn++].unicode();
                if ( ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' ) {
                    break;
                }
                value += QChar( ch );
            }
            word( expandEscapes( value ) );
        } else {
            // Whitespace: ignore it.
        }
        prevWasNL = ( ch == '\n' );
    }

    // Send the first command.
    sendNext();
}

/*!
    Run a list of \a commands which have already been parsed from a
    chat script.
*/
void QAtChatScript::runChat( const QStringList& commands )
{
    d->commands = commands;
    d->posn = 0;
    d->state = 0;
    d->saveState = 0;
    sendNext();
}

/*!
    Stop the chat script.
*/
void QAtChatScript::stop()
{
    d->posn = d->commands.size();
}

/*!
    \fn void QAtChatScript::done( bool ok, const QAtResult& result )

    Signal that is emitted when the chat script is done.  The \a ok flag
    indicates whether the script succeeded or failed, and \a result
    indicates more information about the failure (busy, no carrier, etc).
*/

void QAtChatScript::commandDone( bool ok, const QAtResult& result )
{
    if ( !ok || d->posn >= d->commands.size() ) {
        emit done( ok, result );
    } else {
        sendNext();
    }
}

void QAtChatScript::sendNext()
{
    if ( d->posn < d->commands.size() ) {
        d->atchat->chat( d->commands[(d->posn)++], this,
                         SLOT(commandDone(bool,QAtResult)) );
    } else {
        // There were no commands in the script, so just succeed.
        QAtResult result;
        emit done( true, result );
    }
}

QString QAtChatScript::expandEscapes( const QString& value )
{
    // Bail out if there are no escape sequences in the string.
    if ( value.indexOf( QChar( '\\' ) ) == -1 &&
         value.indexOf( QChar( '^' ) ) == -1 )
        return value;

    // Process the escapes and construct a new string.
    QString val = "";
    int posn = 0;
    uint ch, byte;
    while ( posn < value.length() ) {
        ch = value[posn++].unicode();
        if ( ch == '\\' && posn < value.length() ) {
            ch = value[posn++].unicode();
            switch ( ch ) {
                case 'b':   val += QChar( 0x08 ); break;
                case 'n':   val += QChar( 0x0A ); break;
                case 'N':   val += QChar( 0x00 ); break;
                case 'r':   val += QChar( 0x0D ); break;
                case 's':   val += QChar( 0x20 ); break;
                case 't':   val += QChar( 0x09 ); break;
                case '\\':  val += QChar( 0x5C ); break;
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    byte = ch - '0';
                    if ( posn < value.length() &&
                         value[posn] >= '0' && value[posn] <= '7' ) {
                        ch = value[posn++].unicode();
                        byte = ( byte * 8 ) + ( ch - '0' );
                        if ( posn < value.length() &&
                             value[posn] >= '0' && value[posn] <= '7' ) {
                            ch = value[posn++].unicode();
                            byte = ( byte * 8 ) + ( ch - '0' );
                        }
                    }
                    val += QChar( byte );
                    break;
                default: break;
            }
        } else if ( ch == '^' && posn < value.length() ) {
            ch = value[posn++].unicode();
            if ( ch >= 0x40 && ch <= 0x7F ) {
                val += QChar( ch & 0x1F );
            } else {
                val += QChar( ch );
            }
        } else {
            val += QChar( ch );
        }
    }
    return val;
}

void QAtChatScript::word( const QString& value )
{
    if ( value == "ABORT" || value == "SAY" || value == "ECHO" ||
         value == "TIMEOUT" || value == "HANGUP" ) {
        // Compatibility with Unix chat - just ignore these commands for now.
        d->saveState = d->state;
        d->state = 2;
        return;
    }
    switch ( d->state ) {

        case 0:
        {
            // Looking for an expect string.  The value will typically
            // be something like '', OK, or CONNECT, which we ignore
            // because QAtChat is better at detecting succeed/fail codes.
            if ( !value.isEmpty() && value != "OK" &&
                 !value.startsWith( "CONNECT" ) ) {
                qLog(AtChat) << "QAtChatScript::word: odd expect string" << value;
            }
            d->state = 1;
        }
        break;

        case 1:
        {
            // Looking for a send string.
            if ( value == "+++" ) {
                qLog(AtChat) << "QAtChatScript::word: ignoring +++ hangup";
            } else {
                d->commands += value;
            }
            d->state = 0;
        }
        break;

        case 2:
        {
            // Skip the argument to 'ABORT', 'SAY', etc.
            d->state = d->saveState;
        }
        break;
    }
}
