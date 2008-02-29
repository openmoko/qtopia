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
#ifndef __DATETIMEEDIT_H__
#define __DATETIMEEDIT_H__

#include "datepicker.h"
#include <qtopia/qpeglobal.h>
#include <qspinbox.h> 		// for the time.
#include <qhbox.h>
#include <qdatetime.h>

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
