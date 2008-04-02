/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef APMSUSPENDOBSERVER_H
#define APMSUSPENDOBSERVER_H

#ifdef QT_ILLUME_LAUNCHER

#include <QObject>

#include <linux/apm_bios.h>


class QSocketNotifier;

/**
 * Observe the apm_bios and on suspend/resume ask the Modem and
 * other services to suspend/resume.
 */
class APMSuspendObserver : public QObject {
    Q_OBJECT
public:
    APMSuspendObserver();

private Q_SLOTS:
    void readApmEvents();
    void modemSuspendDone();

private:
    void handleSuspendEvent();
    void handleResumeEvent();

    void ackSuspend();

private:
    QSocketNotifier* m_apmBiosNotifier;
    apm_event_t m_apmEvents[16];
    int m_apmBiosFd;
};

#endif

#endif
