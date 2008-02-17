/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtSql module of the Qt Toolkit.
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

#ifndef QSQLERROR_H
#define QSQLERROR_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class Q_SQL_EXPORT QSqlError
{
public:
    enum ErrorType {
        NoError,
        ConnectionError,
        StatementError,
        TransactionError,
        UnknownError
#ifdef QT3_SUPPORT
        , None = NoError,
        Connection = ConnectionError,
        Statement = StatementError,
        Transaction = TransactionError,
        Unknown = UnknownError
#endif
    };
    QSqlError( const QString& driverText = QString(),
                const QString& databaseText = QString(),
                ErrorType type = NoError,
                int number = -1);
    QSqlError(const QSqlError& other);
    QSqlError& operator=(const QSqlError& other);
    ~QSqlError();

    QString driverText() const;
    void setDriverText(const QString& driverText);
    QString databaseText() const;
    void setDatabaseText(const QString& databaseText);
    ErrorType type() const;
    void setType(ErrorType type);
    int number() const;
    void setNumber(int number);
    QString text() const;
    bool isValid() const;

private:
    QString driverError;
    QString databaseError;
    ErrorType errorType;
    int errorNumber;
};

#ifndef QT_NO_DEBUG_STREAM
Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlError &);
#endif

QT_END_HEADER

#endif // QSQLERROR_H
