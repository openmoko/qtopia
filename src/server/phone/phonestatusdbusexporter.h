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

#ifndef PHONESTATUSDBUSEXPORTER_H
#define PHONESTATUSDBUSEXPORTER_H

#ifdef QT_ILLUME_LAUNCHER

#include <QDBusConnection>

#include "cellmodemmanager.h"

class SimPinDialog;
class DefaultSignal;

class PhoneStatusDBusExporter : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openmoko.qtopia.Phonestatus")
    Q_PROPERTY(QString phoneState READ phoneState)
    Q_PROPERTY(bool planeModeEnabled READ planeModeEnabled WRITE setPlaneModeEnabled)
    Q_PROPERTY(int signalStrength READ signalStrength)
    Q_PROPERTY(QString networkOperator READ networkOperator)

public:
    PhoneStatusDBusExporter(SimPinDialog* dialog, QObject* parent);

Q_SIGNALS:
    Q_SCRIPTABLE void stateChanged(QString);
    Q_SCRIPTABLE void planeModeChanged(bool);
    Q_SCRIPTABLE void signalStrengthChanged(int percent);
    Q_SCRIPTABLE void networkOperatorChanged(QString);

public Q_SLOTS:
    Q_SCRIPTABLE QString phoneState() const;
    Q_SCRIPTABLE bool planeModeEnabled() const;
    Q_SCRIPTABLE int signalStrength() const;
    Q_SCRIPTABLE QString networkOperator() const;

    Q_SCRIPTABLE Q_NOREPLY void setPlaneModeEnabled(bool);
    Q_SCRIPTABLE Q_NOREPLY void triggerPinDialog();

private Q_SLOTS:
    void _q_stateChanged(CellModemManager::State, CellModemManager::State);
    void _q_planeModeEnabledChanged(bool);

private:
    static QString cellModemStateToString(CellModemManager::State);

private:
    SimPinDialog *m_pinDialog;
    CellModemManager *m_cellModem;
    DefaultSignal *m_defaultSignal;
};

#endif

#endif
