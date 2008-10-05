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
#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <qdatetime.h>
#include <QDialog>
#include <QBasicTimer>
#include <qtopiaabstractservice.h>

class QToolButton;
class QLabel;
class QTimeZoneSelector;
class QComboBox;
class QDateEdit;
class QTimeEdit;
class QValueSpaceItem;

class SetDateTime : public QDialog
{
    Q_OBJECT
public:
    SetDateTime( QWidget *parent=0, Qt::WFlags f=0 );

    void setTimezoneEditable(bool tze);

protected slots:
    void tzChange( const QString &tz );
    void dateChange(const QDate &);
    void timeChange(const QTime &);
    void updateDateFormat();
    void updateTimeFormat(int);

public slots:
    void editTime();
    void editDate();
    void setAutomatic(int on);
    void sysTimeZoneChanged();

protected:
    virtual void timerEvent( QTimerEvent* );
    virtual void accept();
    virtual void reject();

private:
    QTimeEdit *time;
    QDateEdit *date;
    QComboBox *atz;
    QTimeZoneSelector *tz;
    QComboBox *weekStartCombo;
#ifndef QT_ILLUME_LAUNCHER
    QComboBox *ampmCombo;
#endif
    QComboBox *dateFormatCombo;
    QLabel *tz_label, *time_label, *date_label;

    QStringList date_formats;
    bool dateChanged;
    bool timeChanged;
    bool tzChanged;
    bool tzEditable;
    QLabel *tzLabel;

    QBasicTimer clocktimer;

    QString selectedDateFormat() const;
    void storeSettings();
};

class TimeService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SetDateTime;
private:
    TimeService( SetDateTime *parent );

public:
    ~TimeService();

public slots:
    void editTime();

private:
    SetDateTime *parent;
};

class DateService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SetDateTime;
private:
    DateService( SetDateTime *parent );

public:
    ~DateService();

public slots:
    void editDate();

private:
    SetDateTime *parent;
};

#endif //SYSTEM_TIME_H
