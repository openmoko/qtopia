/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QBYTEARRAYMATCHER_H
#define QBYTEARRAYMATCHER_H

#include <QtCore/qbytearray.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

QT_END_HEADER

#endif // QBYTEARRAYMATCHER_H
