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

#ifndef Q3SQLFORM_H
#define Q3SQLFORM_H

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_FORM

class QSqlField;
class QSqlRecord;
class Q3SqlEditorFactory;
class Q3SqlPropertyMap;
class QWidget;
class Q3SqlFormPrivate;

class Q_COMPAT_EXPORT Q3SqlForm : public QObject
{
    Q_OBJECT
public:
    Q3SqlForm(QObject * parent = 0);
    ~Q3SqlForm();

    virtual void insert(QWidget * widget, const QString& field);
    virtual void remove(const QString& field);
    int         count() const;

    QWidget *   widget(int i) const;
    QSqlField * widgetToField(QWidget * widget) const;
    QWidget *   fieldToWidget(QSqlField * field) const;

    void        installPropertyMap(Q3SqlPropertyMap * map);

    virtual void setRecord(QSqlRecord* buf);

public Q_SLOTS:
    virtual void readField(QWidget * widget);
    virtual void writeField(QWidget * widget);
    virtual void readFields();
    virtual void writeFields();

    virtual void clear();
    virtual void clearValues();

protected:
    virtual void insert(QWidget * widget, QSqlField * field);
    virtual void remove(QWidget * widget);
    void clearMap();

private:
    Q_DISABLE_COPY(Q3SqlForm)

    virtual void sync();
    Q3SqlFormPrivate* d;
};

#endif // QT_NO_SQL_FORM

QT_END_HEADER

#endif // Q3SQLFORM_H
