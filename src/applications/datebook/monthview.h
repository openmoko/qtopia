/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>

#include <QDateTime>
#include <QCalendarWidget>

class QTimer;

class MonthView : public QCalendarWidget
{
    Q_OBJECT

public:
    MonthView(QWidget *parent = 0, const QCategoryFilter& c = QCategoryFilter(), QSet<QPimSource> set = QSet<QPimSource>());
    ~MonthView();

    bool eventFilter(QObject *o, QEvent *e);

signals:
    void closeView();

public slots:
    void updateModelRange(int year, int month);
    void categorySelected( const QCategoryFilter &c );
    void setVisibleSources( QSet<QPimSource> set);

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void resetFormatsNow();
    void resetFormatsSoon();
    void resetFormats() const;

private:
    QList<QOccurrence> daysAppointments;

    QOccurrenceModel *model;

    mutable QTimer *dirtyTimer;
    mutable bool dirtyModel;
};

#endif
