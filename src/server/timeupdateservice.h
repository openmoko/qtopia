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

#ifndef _TIMEUPDATESERVICE_H_
#define _TIMEUPDATESERVICE_H_

#include <QtopiaAbstractService>
#include <QTimeZone>
#include <QBasicTimer>

class QMessageBox;

class QValueSpaceItem;

class TimeUpdater : public QObject {
    Q_OBJECT
public:
    TimeUpdater(int timeoffset, const QTimeZone& tz, bool chtime, bool chzone);
    void ask();

    void commitUpdate();

protected:
    void timerEvent(QTimerEvent*);

signals:
    void changeSystemTime(uint time, QString newtz);

private slots:
    void userCommit(int);

private:
    QMessageBox *prompt;
    QBasicTimer timer;
    int offset;
    QTimeZone zone;
    bool updatetime;
    bool updatezone;
};

class TimeUpdateService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    TimeUpdateService();

public slots:
    void updateFromExternalSources();
    void updateFromExternalSources(bool autotz, bool autotm, bool ask_tz, bool ask_time);
    void storeExternalSource(QString, uint time, int tzoffset, int dstoffset);
    void changeSystemTime(uint time, QString newtz);

private:
    int lasttz;
    QString externalTimeZone;
    uint externalTimeTimeStamp;
    uint externalTime;
    int externalDstOffset;
    TimeUpdater *updater;
};

#endif // _TIMEUPDATESERVICE_H_

