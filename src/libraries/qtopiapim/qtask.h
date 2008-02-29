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
#ifndef QTOPIAPIM_TASK_H__
#define QTOPIAPIM_TASK_H__

#include <qtopianamespace.h>
#include <qtopia/pim/qpimrecord.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

#include <QList>
#include <QDateTime>
#include <QSharedData>
#include <QSharedDataPointer>

class QTaskData;
class QTOPIAPIM_EXPORT QTask : public QPimRecord
{
public:
    enum Status {
        NotStarted = 0,
        InProgress,
        Completed,
        Waiting,
        Deferred
    };

    enum Priority {
        VeryHigh=1,
        High,
        Normal,
        Low,
        VeryLow
    };

    QTask();
    QTask(const QTask &);

    QTask &operator=(const QTask &other);

    bool operator==(const QTask &other) const;
    bool operator!=(const QTask &other) const;

    virtual ~QTask();

    static void writeVCalendar( const QString &filename, const QList<QTask> &tasks);
    static void writeVCalendar( const QString &filename, const QTask &task);
    static QList<QTask> readVCalendar( const QString &filename );

    void setPriority( Priority priority );
    void setPriority( int priority );
    Priority priority() const;

    void setDescription( const QString& description );
    QString description() const;

    void setDueDate( const QDate& date);
    void clearDueDate();

    QDate dueDate() const;
    bool hasDueDate() const;

    QDate startedDate() const;
    void setStartedDate(const QDate &date);
    bool hasStartedDate() const;

    QDate completedDate() const;
    void setCompletedDate(const QDate &date);

    Status status() const;
    void setStatus(Status s);
    void setStatus(int s);

    bool isCompleted() const;
    void setCompleted( bool b );

    uint percentCompleted() const;
    void setPercentCompleted( uint u );

    QString notes() const;
    void setNotes(const QString &s);

    bool match( const QRegExp &r ) const;

    QString toRichText() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

//protected:
    //virtual int endFieldMarker() const {return TaskFieldCount; }
    static QString statusToText(Status s);

protected:
    QUniqueId &uidRef();
    const QUniqueId &uidRef() const;

    QList<QString> &categoriesRef();
    const QList<QString> &categoriesRef() const;

    QMap<QString, QString> &customFieldsRef();
    const QMap<QString, QString> &customFieldsRef() const;

private:
    QSharedDataPointer<QTaskData> d;
};

Q_DECLARE_USER_METATYPE(QTask)

#endif
