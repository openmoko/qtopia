/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QBYTEARRAYMATCHER_H
#define QBYTEARRAYMATCHER_H

#include <QtCore/qbytearray.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QByteArrayMatcherPrivate;

class Q_CORE_EXPORT QByteArrayMatcher
{
public:
    QByteArrayMatcher();
    explicit QByteArrayMatcher(const QByteArray &pattern);
    QByteArrayMatcher(const QByteArrayMatcher &other);
    ~QByteArrayMatcher();

    QByteArrayMatcher &operator=(const QByteArrayMatcher &other);

    void setPattern(const QByteArray &pattern);

    int indexIn(const QByteArray &ba, int from = 0) const;
    inline QByteArray pattern() const { return q_pattern; }

private:
    QByteArrayMatcherPrivate *d;
    QByteArray q_pattern;
    uint q_skiptable[256];
};

QT_END_HEADER

#endif // QBYTEARRAYMATCHER_H
