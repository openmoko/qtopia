/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  codechunk.h
*/

#ifndef CODECHUNK_H
#define CODECHUNK_H

#include <qstring.h>

// ### get rid of that class

/*
  The CodeChunk class represents a tiny piece of C++ code.

  The class provides convertion between a list of lexemes and a string.  It adds
  spaces at the right place for consistent style.  The tiny pieces of code it
  represents are data types, enum values, and default parameter values.

  Apart from the piece of code itself, there are two bits of metainformation
  stored in CodeChunk: the base and the hotspot.  The base is the part of the
  piece that may be a hypertext link.  The base of

      QMap<QString, QString>

  is QMap.

  The hotspot is the place the variable name should be inserted in the case of a
  variable (or parameter) declaration.  The base of

      char * []

  is between '*' and '[]'.
*/
class CodeChunk
{
public:
    CodeChunk();
    CodeChunk( const QString& str );
    CodeChunk( const CodeChunk& chk );

    CodeChunk& operator=( const CodeChunk& chk );

    void append( const QString& lexeme );
    void appendBase( const QString& lexeme );
    void appendHotspot();

    bool isEmpty() const { return s.isEmpty(); }
    QString toString() const;
    QStringList toPath() const;
    const QString& base() const { return b; }
    QString left() const { return s.left(hotspot == -1 ? s.length() : hotspot); }
    QString right() const { return s.mid(hotspot == -1 ? s.length() : hotspot); }

private:
    QString s;
    QString b;
    int bstart;
    int blen;
    int hotspot;
};

inline bool operator==( const CodeChunk& c, const CodeChunk& d ) {
    return c.toString() == d.toString();
}

inline bool operator!=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c == d );
}

inline bool operator<( const CodeChunk& c, const CodeChunk& d ) {
    return c.toString() < d.toString();
}

inline bool operator>( const CodeChunk& c, const CodeChunk& d ) {
    return d < c;
}

inline bool operator<=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c > d );
}

inline bool operator>=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c < d );
}

#endif
