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

#ifndef Q3SQLMANAGER_P_H
#define Q3SQLMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qvariant.h"
#include "QtCore/qglobal.h"
#include "QtCore/qstring.h"
#include "QtCore/qstringlist.h"
#include "QtSql/qsql.h"
#include "QtSql/qsqlerror.h"
#include "QtSql/qsqlindex.h"
#include "Qt3Support/q3sqlcursor.h"

#ifndef QT_NO_SQL

class Q3SqlCursor;
class Q3SqlForm;
class Q3SqlCursorManagerPrivate;

class Q_COMPAT_EXPORT Q3SqlCursorManager
{
public:
    Q3SqlCursorManager();
    virtual ~Q3SqlCursorManager();

    virtual void setSort(const QSqlIndex& sort);
    virtual void setSort(const QStringList& sort);
    QStringList  sort() const;
    virtual void setFilter(const QString& filter);
    QString filter() const;
    virtual void setCursor(Q3SqlCursor* cursor, bool autoDelete = false);
    Q3SqlCursor* cursor() const;

    virtual void setAutoDelete(bool enable);
    bool autoDelete() const;

    virtual bool refresh();
    virtual bool findBuffer(const QSqlIndex& idx, int atHint = 0);

private:
    Q3SqlCursorManagerPrivate* d;
};

#ifndef QT_NO_SQL_FORM

class Q3SqlFormManagerPrivate;

class Q_COMPAT_EXPORT Q3SqlFormManager
{
public:
    Q3SqlFormManager();
    virtual ~Q3SqlFormManager();

    virtual void setForm(Q3SqlForm* form);
    Q3SqlForm* form();
    virtual void setRecord(QSqlRecord* record);
    QSqlRecord* record();

    virtual void clearValues();
    virtual void readFields();
    virtual void writeFields();

private:
    Q3SqlFormManagerPrivate* d;
};

#endif

class QWidget;
class Q3DataManagerPrivate;

class Q_COMPAT_EXPORT Q3DataManager
{
public:
    Q3DataManager();
    virtual ~Q3DataManager();

    virtual void setMode(QSql::Op m);
    QSql::Op mode() const;
    virtual void setAutoEdit(bool autoEdit);
    bool autoEdit() const;

    virtual void handleError(QWidget* parent, const QSqlError& error);
    virtual QSql::Confirm confirmEdit(QWidget* parent, QSql::Op m);
    virtual QSql::Confirm confirmCancel(QWidget* parent, QSql::Op m);

    virtual void setConfirmEdits(bool confirm);
    virtual void setConfirmInsert(bool confirm);
    virtual void setConfirmUpdate(bool confirm);
    virtual void setConfirmDelete(bool confirm);
    virtual void setConfirmCancels(bool confirm);

    bool confirmEdits() const;
    bool confirmInsert() const;
    bool confirmUpdate() const;
    bool confirmDelete() const;
    bool confirmCancels() const;

private:
    Q3DataManagerPrivate* d;
};

#endif // QT_NO_SQL

#endif // Q3SQLMANAGER_P_H
