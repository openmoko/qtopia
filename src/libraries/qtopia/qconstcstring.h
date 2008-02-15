/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef QTOPIA_CONSTCSTRING_H
#define QTOPIA_CONSTCSTRING_H

class QConstCString
{
public:
    QConstCString()
    : m_str(0), m_length(0)
    {
    }

    QConstCString(const QConstCString &other)
    : m_str(other.m_str), m_length(other.m_length)
    {
    }

    QConstCString(const char *str)
    : m_str(str), m_length(::strlen(str))
    {
    }

    QConstCString(const char *str, int len)
    : m_str(str), m_length(len)
    {
    }

    char operator[](int pos) const
    {
        Q_ASSERT(pos < m_length);
        return m_str[pos];
    }

    bool operator==(const QConstCString &other) const
    {
        return other.m_length == m_length &&
               (0 == m_length ||
                0 == ::memcmp(other.m_str, m_str, m_length));
    }

    bool operator<(const QConstCString &other) const
    {
        if(other.m_length != m_length)
            return m_length < other.m_length;

        return ::memcmp(m_str, other.m_str, m_length) < 0;
    }

    QConstCString &operator=(const QConstCString &other)
    {
        m_str = other.m_str;
        m_length = other.m_length;
        return *this;
    }

    QByteArray toByteArray() const
    {
        return QByteArray(m_str, m_length);
    }

    int length() const
    {
        return m_length;
    }
    const char * data() const
    {
        return m_str;
    }

    QConstCString left(int len) const
    {
        return QConstCString(m_str, qMin(len, m_length));
    }

    QConstCString mid(int pos, int len=-1) const
    {
        if(pos >= m_length)
            return QConstCString(0, 0);

        return QConstCString(m_str + pos,
                             (len == -1)?
                             (m_length - pos):qMin(len, m_length - pos));
    }

private:
    const char * m_str;
    int m_length;
};

#endif
