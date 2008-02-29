#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

#include "headerparser.h"

HeaderParser::HeaderParser( void )
{
}

HeaderParser::~HeaderParser( void )
{
}

bool HeaderParser::parse( const QString &buf, size_t bufEnd, int *headerend, int *bodystart, int *bodylength )
{
	size_t pos   = 0;
	size_t start = 0;
	size_t len   = 0;

	// Get field name
	while( pos < bufEnd ) {
		if ( buf[ pos ] == ':' ) {
			break;
		}
		pos++;
	}
	len = pos;

	// Remove any white space at end of field-name
	while( len > 0 ) {
		if( ! buf[ len - 1 ].isSpace() ) break;
		len--;
	}

	*headerend = len;

	if( pos < bufEnd && buf[ pos ] == ':') {
		pos++;
	}

	// Skip spaces and tabs (but not newline!)
	while( pos < bufEnd ) {
		if( buf[ pos ] != ' ' && buf[ pos ] != '\t' ) break;
		pos++;
	}

	start = pos;
	len = 0;
	// Get field body
	while( pos < bufEnd ) {
		if( buf[ pos ] == '\n' ) {
			// Are we at the end of the string?
			if( pos == bufEnd - 1 ) {
				pos++;
				break;
			}

			// Is this really the end of the field body, and not just
			// the end of a wrapped line?
			else if( buf[ pos + 1 ] != ' ' && buf[ pos + 1 ] != '\t' ) {
				pos++;
				break;
			}
		}
		pos++;
	}

	// Remove white space at end of field-body
	while( pos > start ) {
		if( ! buf[ pos - 1 ].isSpace() ) break;
			pos--;
	}
	len = pos - start;
	*bodystart = start;
	*bodylength = len;
	return true;
}

