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
    void setClock(bool b);
    void stepDown();
    void stepUp();

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

// more of a wrapper class.
class QTOPIA_EXPORT QPEDateEdit : public QPEDateButton
{
    Q_OBJECT

public:
    QPEDateEdit( QWidget *parent = 0, const char *name = 0);
    QPEDateEdit( const QDate &, bool longDate, QWidget *parent = 0, const char *name = 0 );

    ~QPEDateEdit();

public slots:
    virtual void setDate( const QDate &dt );

signals:
    void valueChanged( const QDate & );

private slots:
    void selectDate( int y, int m, int d );

private:
    QPEDateEditPrivate *d;
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

public slots:
    void setClock(bool b);

private:
    QPEDateEdit *de;
    QPETimeEdit *te;
    QPEDateTimeEditPrivate *d;
};

#endif
