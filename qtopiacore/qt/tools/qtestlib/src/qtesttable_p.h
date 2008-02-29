/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTESTTABLE_P_H
#define QTESTTABLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtTest/qtest_global.h>

class QTestData;
class QTestTablePrivate;

class QTestTable
{
public:
    QTestTable();
    ~QTestTable();

    void addColumn(int elementType, const char *elementName);
    QTestData *newData(const char *tag);

    int elementCount() const;
    int dataCount() const;

    int elementTypeId(int index) const;
    const char *dataTag(int index) const;
    int indexOf(const char *elementName) const;
    bool isEmpty() const;
    QTestData *testData(int index) const;

    static QTestTable *globalTestTable();
    static QTestTable *currentTestTable();
    static void clearGlobalTestTable();

private:
    Q_DISABLE_COPY(QTestTable)

    QTestTablePrivate *d;
};

#endif
