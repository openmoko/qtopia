/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __DATETIMEEDIT_H__
#define __DATETIMEEDIT_H__

#include <qtopia/datepicker.h>
#include <qtopia/qpeglobal.h>
#include <qspinbox.h> 		// for the time.
#include <qhbox.h>
#include <qdatetime.h>
#include <qvbox.h>

class QPETimeEditPrivate;
class TimeValidator;
class QTOPIA_EXPORT QPETimeEdit : public QSpinBox
{
    Q_OBJECT
public:
    QPETimeEdit( QWidget *parent = 0, const char *name = 0);
    QPETimeEdit( const QTime &, QWidget *parent = 0, const char *name = 0 );

    ~QPETimeEdit();

    QTime time() const;

signals:
    void valueChanged( const QTime & );

public slots:
    virtual void setTime( const QTime & );
    void stepDown();
    void stepUp();
    void clockChanged();

protected:
    QString mapValueToText(int);
    int mapTextToValue(bool *ok);
    QTime mapValueToTime(int) const;

protected slots:
    void changeTimeUsingValue(int);

private:
    QPETimeEditPrivate *d;
    TimeValidator *tv;
    bool ampm;
};

class QPEDateEditPrivate;
class QTOPIA_EXPORT QPEDateEdit : public QPushButton
{
    Q_OBJECT

public:
    QPEDateEdit(  QWidget *parent, const char * name = 0,
		    bool longFormat = FALSE,
		    bool allowNullDate = FALSE );
    QPEDateEdit(  const QDate &, QWidget *parent, const char * name = 0,
		    bool longFormat = FALSE,
		    bool allowNullDate = FALSE );

    QDate date() const;

    void setDateFormat( DateFormat );
    void setLongFormat( bool l );
    bool longFormat() const { return longFmt; }
    bool allowNullDate() const;
    // void setAllowNullDate();

public slots:
    //void setDate( int y, int m, int d );
    virtual void setDate( const QDate & );

    // these will later be removed.
    void setWeekStartsMonday( bool );

signals:
    void valueChanged( const QDate &);

private slots:
    void setNull();
    void updateButtonText();
    void clockChanged();
    void hidingMonthView(void);

private:
    void init();

    bool longFmt;
    bool weekStartsMonday;
    bool mAllowNullButton;
    DateFormat df;
    QDate currDate;
    QPEDatePicker *monthView;
    QPushButton *noneButton;

    class QPEDateEditPrivate *d;
};


class QPEDateTimeEditPrivate;

class QTOPIA_EXPORT QPEDateTimeEdit : public QHBox
{
    Q_OBJECT

public:
    QPEDateTimeEdit( QWidget *parent = 0, const char *name = 0);
    QPEDateTimeEdit( const QDateTime &, QWidget *parent = 0, const char *name = 0 );

    ~QPEDateTimeEdit();

    void setTimeEnabled(bool);
    void setDateEnabled(bool);
    bool timeIsEnabled() const;
    bool dateIsEnabled() const;

    void setOrientation(Orientation); // qtopia2
    Orientation orientation() const; // qtopia2

    QDate date() const;
    QTime time() const;
    QDateTime dateTime() const;

public slots:
    virtual void setDateTime( const QDateTime & );
    virtual void setDate( const QDate & );
    virtual void setTime( const QTime & );

signals:
    void valueChanged( const QDateTime & );

private:
    QPEDateEdit *de;
    QPETimeEdit *te;
    QPEDateTimeEditPrivate *d;
};

#endif
