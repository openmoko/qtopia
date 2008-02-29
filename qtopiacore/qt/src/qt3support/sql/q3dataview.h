/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3DATAVIEW_H
#define Q3DATAVIEW_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_VIEW_WIDGETS

class Q3SqlForm;
class QSqlRecord;
class Q3DataViewPrivate;

class Q_COMPAT_EXPORT Q3DataView : public QWidget
{
    Q_OBJECT

public:
    Q3DataView(QWidget* parent=0, const char* name=0, Qt::WindowFlags fl = 0);
    ~Q3DataView();

    virtual void setForm(Q3SqlForm* form);
    Q3SqlForm* form();
    virtual void setRecord(QSqlRecord* record);
    QSqlRecord* record();

public Q_SLOTS:
    virtual void refresh(QSqlRecord* buf);
    virtual void readFields();
    virtual void writeFields();
    virtual void clearValues();

private:
    Q_DISABLE_COPY(Q3DataView)

    Q3DataViewPrivate* d;
};

#endif // QT_NO_SQL_VIEW_WIDGETS

QT_END_HEADER

#endif // Q3DATAVIEW_H
