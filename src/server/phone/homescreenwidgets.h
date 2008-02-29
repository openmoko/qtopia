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

#ifndef HOMESCREENWIDGETS_H
#define HOMESCREENWIDGETS_H

// Qt includes
#include <QWidget>
#include <QMap>
#include <QString>
#include <QList>

// Qtopia includes
#include <QContent>
#include <qtopiapim/qappointmentmodel.h>
#include <QValueSpaceItem>
#include <QValueSpaceObject>
#include <QWorldmap>
#include <QAnalogClock>

// ============================================================================
//
// HomeScreenWidgets
//
// ============================================================================

class HomeScreenWidgets
{
public:
    static QWidget* create( const QString& name, QWidget* parent = 0 );

    typedef QWidget* (*CreateHSWidgetFunc)(void *);
    static void add( const char* name, CreateHSWidgetFunc function );


private:
    static QMap<QString, CreateHSWidgetFunc> mWidgets;
};

#define HSWIDGET(name, function) \
    static QWidget *_hswidget_install_create_ ## name(void *) { \
        return new function; \
    } \
    struct _hswidget_install_ ## name { \
        _hswidget_install_ ## name() { \
            HomeScreenWidgets::add( \
                # name, \
                _hswidget_install_create_ ## name ); \
        } \
    }; \
    static _hswidget_install_ ## name _hswidget_install_instance_ ## name;

// ============================================================================
//
// LauncherIcon
//
// ============================================================================

class LauncherIcon : public QWidget
{
    Q_OBJECT

public:
    LauncherIcon( const QContent& app, QWidget* parent = 0 );

protected:
    void paintEvent( QPaintEvent* /*event*/ );
    void keyPressEvent( QKeyEvent* event );

public slots:
    void launch();

private:
    const QContent mApp;
};

// ============================================================================
//
// LauncherHSWidget
//
// ============================================================================

class LauncherHSWidget : public QWidget
{
    Q_OBJECT

public:
    LauncherHSWidget( QWidget* parent = 0 );

public slots:
    void launch();

private:
    QList<QContent> mApps;
    QList<LauncherIcon*> mIcons;
};

// ============================================================================
//
// AppointmentsHSWidget
//
// ============================================================================

class AppointmentsHSWidget : public QWidget
{
    Q_OBJECT

public:
    AppointmentsHSWidget( QWidget* parent = 0 );

public slots:
    void showNextAppointment();

private slots:
    void update();

private:
    bool updateModel();
    QString appProgress( const int minutesPast );
    QString appScheduled( const QOccurrence& occurence );

    QValueSpaceObject* mVsObject;
    QValueSpaceItem* mVsItem;
    QUniqueId mUid;
    QDate     mDate;
    QOccurrenceModel* mModel;
};

// ============================================================================
//
// WorldmapHSWidget
//
// ============================================================================

class WorldmapHSWidget : public QWorldmap
{
    Q_OBJECT

public:
    WorldmapHSWidget( QWidget* parent = 0 );

public slots:
    void showCity();

protected:
    void paintEvent( QPaintEvent *event );

private slots:
    void showTZ();

private:
    int mCheck;
};

// ============================================================================
//
// AnalogClockHSWidget
//
// ============================================================================

class AnalogClockHSWidget : public QAnalogClock
{
    Q_OBJECT

public:
    AnalogClockHSWidget( QWidget* parent = 0 );

private slots:
    void update();
};

#endif //HOMESCREENWIDGETS_H
