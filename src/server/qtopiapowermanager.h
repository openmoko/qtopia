/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QObject>
#include <QList>
#include <qtopiaapplication.h>
#include <custom.h>
#include <qvaluespace.h>
#include <QPowerStatus>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

class QSettings;

class QtopiaPowerManager : public QObject
#ifdef Q_WS_QWS
    , public QWSScreenSaver
#endif
{
Q_OBJECT
public:
    QtopiaPowerManager();

    virtual void restore();

    void setDefaultIntervals();

    virtual void setIntervals(int* a, int size) = 0;
    virtual bool save(int level) = 0;

    void setBacklight(int bright);
    int backlight();
    static void setActive(bool on);

    void setConstraint(QtopiaApplication::PowerConstraint m);

protected:
    int interval(int interval, QSettings& cfg, const QString &enable,
            const QString& value, int def);

    QtopiaApplication::PowerConstraint m_powerConstraint;
    bool m_dimLightEnabled;
    bool m_lightOffEnabled;
    QMap<int,int> m_levelToAction;
    static QValueSpaceObject *m_vso;
    virtual void forceSuspend();

private slots:
    virtual void powerStatusChanged();
    void _forceSuspend();

private:
    QPowerStatus powerstatus;
};

class QtopiaPowerConstraintManager : public QObject
{
    Q_OBJECT
public:
    QtopiaPowerConstraintManager(QObject *parent = 0);

    void setConstraint( QtopiaApplication::PowerConstraint ,const QString &app);

    static QtopiaPowerConstraintManager *instance();

public slots:
    void applicationTerminated(const QString &app);

signals:
    void forceSuspend();

protected:
    void timerEvent(QTimerEvent *);

private:
    bool removeOld(const QString &);
    void updateAll();
    int timerValue();

private:
    QList<QString> sStatus[3];
    int currentMode;
    int timerId;
};

#endif //SCREENSAVER_H

