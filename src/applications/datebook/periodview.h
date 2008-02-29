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
#ifndef PERIODVIEW
#define PERIODVIEW

#include <qtopia/pim/qappointment.h>

#include <qwidget.h>

class QOccurrenceModel;

class PeriodView : public QWidget
{
    Q_OBJECT
public:
    PeriodView( QOccurrenceModel *datedb, bool stm,
            QWidget *parent = 0 );

    QDate currentDate() const { return cDate; }
    bool startsOnMonday() const { return bOnMonday; }
    int dayStarts() const { return sHour; }

    virtual bool hasSelection() const;
    virtual QAppointment currentAppointment() const;

public slots:
    virtual void selectDate(const QDate &);
    virtual void setStartOnMonday( bool );
    virtual void setDayStarts( int h );

private slots:
    virtual void databaseUpdated();

signals:
    void dateActivated( const QDate &);
    void selectionChanged( );

    // prot, not priv as this is app, and hence not as dangerous.
protected:
    QOccurrenceModel *db;
    QDate cDate;
    bool bOnMonday;
    int sHour;
};

#endif
