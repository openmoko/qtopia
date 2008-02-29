/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_H
#define QSQL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

namespace QSql
{
    enum Location {
        BeforeFirstRow = -1,
        AfterLastRow = -2
#ifdef QT3_SUPPORT
        , BeforeFirst = BeforeFirstRow,
        AfterLast = AfterLastRow
#endif
    };

    enum ParamTypeFlag {
        In = 0x00000001,
        Out = 0x00000002,
        InOut = In | Out,
        Binary = 0x00000004
    };
    Q_DECLARE_FLAGS(ParamType, ParamTypeFlag)

    enum TableType {
        Tables = 0x01,
        SystemTables = 0x02,
        Views = 0x04,
        AllTables = 0xff
    };

#ifdef QT3_SUPPORT
    enum Op {
        None = -1,
        Insert = 0,
        Update = 1,
        Delete = 2
    };

    enum Confirm {
        Cancel = -1,
        No = 0,
        Yes = 1
    };
#endif
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QSql::ParamType)

QT_END_HEADER

#endif // QSQL_H
