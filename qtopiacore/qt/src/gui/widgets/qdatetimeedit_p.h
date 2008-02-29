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

#ifndef QDATETIMEEDIT_P_H
#define QDATETIMEEDIT_P_H

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

#include "QtGui/qcombobox.h"
#include "QtGui/qcalendarwidget.h"
#include "QtGui/qspinbox.h"
#include "QtGui/qtoolbutton.h"
#include "QtGui/qmenu.h"
#include "QtGui/qlabel.h"
#include "qdebug.h"

#ifndef QT_NO_DATETIMEEDIT

class QCalendarPopup : public QWidget
{
    Q_OBJECT
public:
    QCalendarPopup(const QDate &date, QWidget *parent = 0);
    QDate selectedDate() { return calendar->selectedDate(); }
    void setDate(const QDate &date);
    void setDateRange(const QDate &min, const QDate &max);

Q_SIGNALS:
    void activated(const QDate &date);
    void newDateSelected(const QDate &newDate);
    void hidingCalendar(const QDate &oldDate);
    void resetButton();

private Q_SLOTS:
    void dateSelected(const QDate &date);
    void dateSelectionChanged();

protected:
    void hideEvent(QHideEvent *);
    void mousePressEvent(QMouseEvent *e); 
    void mouseReleaseEvent(QMouseEvent *);
    bool event(QEvent *e);

private:
    QCalendarWidget *calendar;
    QDate oldDate;
    bool dateChanged;

};

#endif // QT_NO_DATETIMEEDIT

#endif // QDATETIMEEDIT_P_H
