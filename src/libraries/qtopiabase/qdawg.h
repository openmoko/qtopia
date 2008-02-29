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
#ifndef QDAWG_H
#define QDAWG_H

#include <qtopiaglobal.h>
#include <qstringlist.h>

class QIODevice;
class QDawgPrivate;

class QTOPIABASE_EXPORT QDawg {
public:
    QDawg();
    ~QDawg();

    bool readFile(const QString&); // may mmap
    bool read(QIODevice* dev);
    bool write(QIODevice* dev) const;
    bool createFromWords(QIODevice* dev);
    void createFromWords(const QStringList&);
    QStringList allWords() const;

    bool contains(const QString&) const;
    int countWords() const;

    class Node {
        friend class QDawgPrivate;
        uint let:16;
        uint a : 14 ; //not used
        uint isword:1;
        uint islast:1;
        int offset:18;
        int b : 14 ; //not used
        Node() { a = b = 0; /*set zero for better compression*/ }
    public:
        QChar letter() const { return QChar((ushort)let); }
        bool isWord() const { return isword; }
        bool isLast() const { return islast; }
        const Node* next() const { return islast ? 0 : this+1; }
        const Node* jump() const { return offset ? this+offset : 0; }
    };

    const Node* root() const;

    void dump() const; // debug

private:
    friend class QDawgPrivate;
    QDawgPrivate* d;
};


#endif
