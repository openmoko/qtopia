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
