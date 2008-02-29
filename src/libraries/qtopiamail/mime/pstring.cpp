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


#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "pstring.h"

/*  PString, as in Pascal string. (well, sort of.  We keep track of the lenght, but we don't store it in
    front of the string, as Pascal did)

    Basically a hybrid QString/QCString.  Uses single bytes as QCString, implicit sharing as QString, and
    remembers the length, so a lot of methods don't need to call strlen.

    Word of warning:
    The const char *data() method gives you access to the interals withouth detaching.  This method is
    meant for fast read access.  If you use the ptr returned to manipulate the internals, you will change
    any implicit copies as well (use at own risk)
*/
PString::PString()
{
    d = new PStringData();
}

//  Deep copy of 0 terminated string
PString::PString(const char *str)
{
    d = new PStringData();

    append( str );
}

// Deep copy of len bytes
PString::PString(const char *str, uint len)
{
    char *ptr = (char *) malloc(len + 1);
    ::memcpy(ptr, str, len);

    d = new PStringData(len, len+1, ptr);
}

// Implicit copy
PString::PString(const PString &str)
{
    d = str.d;
    d->ref();
}

/*  ByteArrays are normally not being expanded, so let's
    only allocate the size we need */
PString::PString(const QByteArray &b)
{
    uint len = b.size();
    char *ptr = (char *) malloc(len + 1);       //allow for 0 as end marker in cstrings
    ::memcpy(ptr, b.data(), len);

    d = new PStringData(len, len+1, ptr);
}

PString::~PString()
{
    if ( d->deref() ) {
        delete d;
    }
}

PString& PString::operator+=(const char *str)
{
    append(str);
    return *this;
}

PString& PString::operator+=(const PString &str)
{
    append( str );

    return *this;
}

// simple impl. for now
PString& PString::operator+=(char c)
{
    char p[2];
    p[0] = c;
    p[1] = 0;

    append( p );
    return *this;
}

PString& PString::operator=(const PString& str)
{
    str.d->ref();
    deref();
    d = str.d;

    return *this;
}

//  Change this one if you add substring referencing
bool PString::operator==(const PString &s) const
{
    if ( d == s.d ) return true;
    return ( (d->len == s.d->len) && ( ::memcmp(d->data, s.d->data, d->len) == 0) );
}

bool PString::operator==(const char *s) const
{
    return ( ::strcmp(data(), s) == 0 );
}

bool PString::operator!=(const PString &s) const
{
    return !(*this == s);
}

bool PString::operator!=(const char *s) const
{
    return !(*this == s);
}

bool PString::operator<(const PString &s) const
{
    return (::strcmp(data(), s.data()) < 0);  //need to ensure 0 terminated
}

char PString::operator[](uint i) const
{
    if ( i >= d->len )
        return 0;

    return *(d->data + i);
}

void PString::deref()
{
    if ( d->deref() ) {
        delete d;
    }
}

void PString::detach()
{
    if ( d->count != 1 ) {
        uint len = d->len;
        uint max = d->size;
        char *ptr = (char *) malloc(max);
        ::memcpy(ptr, d->data, len);

        deref();
        d = new PStringData(len, max, ptr);
    }
}

PString PString::mid(uint index, uint len) const
{
    uint myLen = d->len;
    if ( index >= myLen || myLen == 0 ) {
        return PString();
    } else {
        if ( len > myLen-index )
            len = myLen - index;

        return PString( d->data + index, len );
    }
}

PString PString::lower() const
{
    PString s(d->data, d->len );    //deep copy

    uint len = d->len;
    if ( len ) {
        char *ptr = s.d->data;
        while ( len-- ) {
            *ptr = ::tolower( *ptr );
            ptr++;
        }
    }

    return s;
}

PString PString::upper() const
{
    PString s(d->data, d->len );    //deep copy

    uint len = d->len;
    if ( len ) {
        char *ptr = s.d->data;
        while ( len-- ) {
            *ptr = ::toupper( *ptr );
            ptr++;
        }
    }

    return s;
}

int PString::find(const PString &str, int index, bool cs) const
{
    if ( index < 0 )
        index += d->len;
    if ( (index < 0) || ((uint) index >= d->len) || (str.length() == 0) )   //out of bounds
        return -1;

    const char *p = data() + index;     //ensure trailing 0
    const char *substr = str.data();
    if ( cs ) {
        char *pos = ::strstr(p, substr);
        if ( pos )
            return (pos - d->data);
    } else {
        uint len = str.length();
        while ( *p ) {
            if ( ::strncasecmp(p, substr, len) == 0 )
                break;
            p++;
        }
        if ( *p )
            return (p - d->data);
    }

    return -1;
}

PString& PString::replace(uint index, uint len, const PString &s)
{
    remove(index, len);
    insert(index, s);
    return *this;
}

PString& PString::insert(uint index, const PString &s)
{
    if ( s.length() == 0 )
        return *this;

    if ( index >= d->len ) {
        append( s );
    } else {
        resize( d->len + s.length() );
        ::memmove( d->data + index + s.length(), d->data + index, d->len - index);
        ::memcpy( d->data + index, s.data(), s.length() );
        d->len += s.length();
    }

    return *this;
}

PString& PString::remove(uint index, uint len)
{
    if ( index + len >= d->len ) {
        if ( index < d->len ) {
            resize(index);
        }
    } else if ( len ) {
        resize(d->len);
        ::memmove( d->data + index, d->data + index + len, d->len - index - len);
        resize(d->len -len);
    }

    return *this;
}

int PString::toInt(bool *ok, int base)
{
    if ( !d->len ) {
        if ( ok )
            *ok = false;
        return 0;
    }

    char *c;
    long l = ::strtol( data(), &c, base);
    if ( *c == 0 )
        if ( ok )
            *ok = true;
    else
        if ( ok )
            *ok = false;

    return (int) l;
}

PString PString::stripWhiteSpace() const
{
    uint len = d->len;
    if ( len == 0 )
        return *this;

    if ( !isSpace( *d->data ) && !isSpace( *(d->data+len-1) ) )
        return *this;

    uint start = 0;
    const char *p = d->data;
    while ( len -- ) {
        if ( isSpace( *p ) )
            start++;
        else break;

        p++;
    }

    len = d->len;
    uint stop = len;
    p = d->data + len-1;
    while ( len -- ) {
        if ( isSpace( *p ) )
            stop--;
        else break;
        p--;
    }

//   if ( !start && !stop )  //no whitespace stripped
//      return *this;

    if ( start >= stop )
        return PString();

    return PString(d->data + start, stop - start );
}

PString PString::simplifyWhiteSpace() const
{
    uint len = d->len;
    if ( len == 0 )
        return *this;

    PString out(d->data, d->len);   //this one copies as well, a bit redundant
    char *from = const_cast<char*>( data() );   //ensure 0 terminated
    char *to = const_cast<char*>( out.data() );

    while( true ) {
        while ( isSpace(*from) )
            from++;
        while( *from && !isSpace(*from) )
            *to++ = *from++;
        if ( *from )
            *to++ = 0x20;
        else break;
    }
    if ( (to > out.d->data) && ( *(to-1) == 0x20 ) )
        to--;

    //  Should probably truncate here
    out.d->len = (to - out.d->data);

    return out;
}

bool PString::isSpace(char c) const
{
    return ( (c >= 9 && c <= 13 ) || ( c == 32 ) );
}

// Should be safe.  Tries with size bytes first, and if that would fail
// it resizes appropriately
PString& PString::sprintf(const char *format, ...)
{
    detach();
    va_list ap;
    va_start(ap, format);

    data(); //ensure trailing 0
    int res = vsnprintf( d->data, d->size-1, format, ap);   //need room for trailing 0
    if ( res >= (int) d->size - 1 ) {
        resize( res + 2 );
        vsnprintf( d->data, d->size-1, format, ap);
    }
    d->len = strlen( d->data );

    va_end(ap);

    return *this;
}

void PString::append(const PString &str)
{
    int len = str.length();

    if ( len ) {
        resize( d->len + len);
        ::memcpy(d->offset() , str.data(), len);
        d->len += len;
    }
}

void PString::append(const char *str)
{
    int len = strlen(str);

    if ( len ) {
        resize( d->len + len);
        ::memcpy(d->offset() , str, len);
        d->len += len;
    }
}

//  Resize expands or shrinks the string.  If we are shared
//  we always detach, otherwise we delete/allocate only when
//  needed (faster)
//  TODO: The size calclation (copied from QString) makes sense on
//  small strings, but not really on larger strings.
void PString::resize(uint newMin)
{
    uint newMax = 8;

    // TODO: smart shrinking
    if ( newMin >= d->size ) {
        while ( newMax <= newMin )
            newMax *= 2;

    } else
        newMax = d->size;       //keep old max

    uint len = qMin(d->len, newMin);
    if ( d->count > 1 || newMax > d->size) {
        char *ptr = (char *) malloc(newMax);
        ::memcpy(ptr, d->data, len);

        deref();
        d = new PStringData(len, newMax, ptr);
    } else {
        d->len = len;
        d->size = newMax;
    }
}

