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
#ifndef LONGSTRING_H
#define LONGSTRING_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <qtopiaglobal.h>

class LongStringPrivate;

class QTOPIAMAIL_EXPORT LongString
{
public:
    LongString();
    LongString(const LongString &s);
    LongString(const QByteArray &ba);
    LongString(const QString &fileName, bool autoRemove = true);
    virtual ~LongString();
    LongString &operator=(const LongString &);
    bool isEmpty() const;
    int length() const;
    int indexOf(const QByteArray &ba, int from = 0) const;
    const LongString mid(int i, int len = -1) const;
    const LongString left(int len) const;
    const LongString right(int len) const;
    QString toQString() const;

    // WARNING the QByteArray returned may become invalid when this LongString
    // is destroyed
    const QByteArray toQByteArray() const;
    QString fileName() const;

protected:
    LongString(const QByteArray &ba, const QByteArray &pBa, LongStringPrivate *);

private:
    QByteArray mBa;
    QByteArray mPrimaryBa;
    LongStringPrivate *d;
    bool mAutoRemove;
};
#endif
