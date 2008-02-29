#include "composeim.h"

//if the last 4 characters of s form a hex digit; replace them
// with the unicode character they describe.
static void compose( QString &s )
{
    if ( s.length() < 4 )
	return;
    QString hex = s.right(4);
    for ( int i=0; i < 4; i++ ) {
	if ( hex[i].isSpace() )
	    return;
    }
    bool ok;
    unsigned short int unicode = hex.toUShort( &ok, 16 );
    if ( ok && unicode >= ' ' && unicode < 0xffff ) {
	s.truncate( s.length() - 4 );
	s += QChar( unicode );
    }
}

ComposeIM::ComposeIM()
{
    state = Off;
}

void ComposeIM::reset()
{
    if ( state == On ) {
	state = Off;
	composed = "";
	sendIMEvent( QWSServer::IMEnd, composed, 0 );
    }
}

bool ComposeIM::filter(int unicode, int keycode, int modifiers, 
  bool isPress, bool autoRepeat)
{
    if ( isPress && keycode == Qt::Key_Space && 
	 modifiers & Qt::ShiftButton ) {
	//switch to opposite state
	if ( state == On ) {
	    sendIMEvent( QWSServer::IMEnd, QString::null, 0 );
	    composed = "";
	    state = Off; //reset and remove text
	} else {
	    state = On;
	}
	return TRUE; //block event
    } else if ( state == On ) {
	if ( isPress ) {
	    if ( keycode == Qt::Key_Return ) {
		//accept text and remain active
		sendIMEvent( QWSServer::IMEnd, composed, composed.length() );
		composed = "";
	    } else if ( keycode == Qt::Key_Backspace ) {
		if ( composed.length() > 0 )
		    composed = composed.left( composed.length() - 1 );
		sendIMEvent( QWSServer::IMCompose, composed, composed.length(), 0 );	} else if ( unicode > 0 && unicode < 0xffff) {
		    composed += QChar( unicode );
		    compose( composed );
		    sendIMEvent( QWSServer::IMCompose, composed, composed.length(), 0 );
		}
	}
	return TRUE; //block event
    }  
    return FALSE; //pass keystroke normally.
}
