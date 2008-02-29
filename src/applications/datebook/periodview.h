/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef PERIODVIEW
#define PERIODVIEW

#include <qtopia/pim/event.h>

#include <qwidget.h>

class DateBookTable;

class PeriodView : public QWidget
{
    Q_OBJECT
public:
    PeriodView( DateBookTable *datedb, bool stm, 
	    QWidget *parent = 0, const char *name = 0 );

    QDate currentDate() const { return cDate; }
    bool startsOnMonday() const { return bOnMonday; }
    int dayStarts() const { return sHour; }

    virtual bool hasSelection() const;
    virtual PimEvent currentEvent() const;

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
    DateBookTable *db;
    QDate cDate;
    bool bOnMonday;
    int sHour;
};

#endif
