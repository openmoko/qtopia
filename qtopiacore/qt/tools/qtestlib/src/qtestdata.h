/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
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
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTESTDATA_H
#define QTESTDATA_H

#include <QtTest/qtest_global.h>

#include <QtCore/qmetatype.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

class QTestTable;
class QTestDataPrivate;

class Q_TESTLIB_EXPORT QTestData
{
public:
    ~QTestData();

    void append(int type, const void *data);
    void *data(int index) const;
    const char *dataTag() const;
    QTestTable *parent() const;
    int dataCount() const;

private:
    friend class QTestTable;
    QTestData(const char *tag = 0, QTestTable *parent = 0);

    Q_DISABLE_COPY(QTestData)

    QTestDataPrivate *d;
};

template<typename T>
QTestData &operator<<(QTestData &data, const T &value)
{
    data.append(qMetaTypeId<T>(), &value);
    return data;
}

inline QTestData &operator<<(QTestData &data, const char * value)
{
    QString str = QString::fromAscii(value);
    data.append(QMetaType::QString, &str);
    return data;
}

QT_END_HEADER

#endif
