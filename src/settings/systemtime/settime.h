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
#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H


#include <qdatetime.h>
#include <QMainWindow>
#include <QSpinBox>

#include <qtimestring.h>
#include <qtopiaabstractservice.h>

#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

class QToolButton;
class QLabel;
class QHBoxLayout;
class QTimeZoneWidget;
class QComboBox;
class MinuteSpinBox;

class SetTime : public QWidget
{
    Q_OBJECT
public:
    SetTime( QWidget *parent=0 );

    QTime time() const;

    bool changed() const { return userChanged; }

public slots:
    void slotTzChange( const QString& tz );
    void show12hourTime( int );

protected slots:
    void hourChanged( int value );
    void minuteChanged( int value );

    void checkedPM( int );

protected:
    void focusInEvent( QFocusEvent *e );

    int hour;
    int minute;
    bool use12hourTime;
    QComboBox *ampm;
    QSpinBox *sbHour;
    MinuteSpinBox *sbMin;
    bool userChanged;
};

class QDateEdit;

class SetDateTime : public QMainWindow
{
    Q_OBJECT
public:
    SetDateTime( QWidget *parent=0, Qt::WFlags f=0 );

    void setTimezoneEditable(bool tze);

protected slots:
    void tzChange( const QString &tz );
    void dateChange(const QDate &);
    void setDateFormat();
    void updateDateFormat();

public slots:
    void editTime();
    void editDate();

protected:
    virtual void closeEvent( QCloseEvent* e );

    SetTime *time;
    QDateEdit *date;
    QTimeZoneWidget *tz;
    QComboBox *weekStartCombo;
    QComboBox *ampmCombo;
    QComboBox *dateFormatCombo;

    QStringList date_formats;
    bool dateChanged;
    bool tzChanged;
    bool tzEditable;
    QLabel *tzLabel;
    QHBoxLayout *tzLayout;
#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif

private:
    void storeSettings();
};

class MinuteSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    MinuteSpinBox( QWidget * parent = 0 );

protected:
    virtual QString textFromValue( int value ) const;
};

#ifndef QT_NO_COP

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

#endif

#endif //SYSTEM_TIME_H
