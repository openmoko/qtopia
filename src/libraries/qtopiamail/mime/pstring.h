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


#ifndef PSTRING_H
#define PSTRING_H

#include <qt.h>

#include <stdlib.h>

class PStringData : public QShared
{
public:
    PStringData()
    {
        len = 0;
        size = 4;
        data = (char *) malloc(4);
    }

    PStringData(uint l, uint s, char *d)
    {
        len = l;
        size = s;
        data = d;
    }

    ~PStringData()
    {
        free(data);
    }

    inline char* offset()
    {
        return data + len;
    }

    char *data;
    uint len, size;
};


class PString
{
public:
    PString();
    PString(const char *str);
    PString(const char *str, uint len);
    PString(const PString& str);
    PString(const QByteArray &b);

    ~PString();

    uint length() const;
    bool isEmpty() const { return d->len == 0; };
    const char* data() const { *d->offset() = 0; return d->data; };

    PString mid(uint index, uint len = 0xffffffff) const;

    PString lower() const;
    PString upper() const;

    int find(const PString &str, int index=0, bool cs=true) const;

    PString& replace(uint index, uint len, const PString &);
    PString& insert(uint index, const PString &);
    PString& remove(uint index, uint len);

    int toInt(bool *ok=0, int base = 10);

    PString stripWhiteSpace() const;
    PString simplifyWhiteSpace() const;

    PString& sprintf(const char *format, ...);

    PString& operator+=(const char *str);
    PString& operator+=(const PString &str);
    PString& operator+=(char c);

    PString& operator=(const PString &);

    bool operator==(const PString &) const;
    bool operator==(const char *) const;
    bool operator!=(const PString &) const;
    bool operator!=(const char *) const;
    bool operator<(const PString &) const;
    char operator[](uint i) const;
    operator const char *() const;

private:
    bool isSpace(char c) const;

    void deref();
    void detach();

    void append(const char *str);
    void append(const PString &str);
    void resize(uint newSize);

    PStringData *d;
};


/*  Inline functions    */

inline uint PString::length() const
{
    return d->len;
}

inline PString operator+(const PString &s, const char *s2)
{
    PString tmp( s.data() );
    tmp += s2;
    return tmp;
}

inline PString operator+(const char *s, const PString &s2)
{
    PString tmp( s );
    tmp += s2;
    return tmp;
}

inline PString::operator const char*() const
{
    return data();
}

#endif
