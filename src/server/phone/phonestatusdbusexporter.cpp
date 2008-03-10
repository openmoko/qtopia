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

#include "phonestatusdbusexporter.h"

#include "simpindialog.h"
#include "infrastructure/signalstrength/defaultsignal.h"

#ifdef QT_ILLUME_LAUNCHER


// No direct mapping due Initializing2 which we don't want to expose. Where is the difference between 1 and 2?
// Maybe make this an array..
QString PhoneStatusDBusExporter::cellStatusToModemState(CellModemManager::State state)
{
    switch (state) {
    case CellModemManager::NoCellModem:
        return QLatin1String("NoModem");
    case CellModemManager::Initializing:
        return QLatin1String("Initializing");
    case CellModemManager::Initializing2:
        return QLatin1String("Initializing");
    case CellModemManager::Ready:
        return QLatin1String("Ready");
    case CellModemManager::WaitingSIMPin:
        return QLatin1String("WaitingForSIMPin");
    case CellModemManager::VerifyingSIMPin:
        return QLatin1String("VerifyingSIMPin");
    case CellModemManager::WaitingSIMPuk:
        return QLatin1String("WaitingForSIMPuk");
    case CellModemManager::VerifyingSIMPuk:
        return QLatin1String("VerifyingSIMPuk");
    case CellModemManager::SIMDead:
        return QLatin1String("SIMIsDead");
    case CellModemManager::SIMMissing:
        return QLatin1String("SIMIsMissing");
    case CellModemManager::AerialOff:
        return QLatin1String("AerialOff");
    case CellModemManager::FailureReset:
        return QLatin1String("FailureReset");
    case CellModemManager::UnrecoverableFailure:
        return QLatin1String("UnrecoverableFailure");
    }

    return QLatin1String("NoStatus");
}

PhoneStatusDBusExporter::PhoneStatusDBusExporter(SimPinDialog* dialog, QObject* parent)
    : QObject(parent)
    , m_pinDialog(dialog)
{
    m_cellModem = qtopiaTask<CellModemManager>();
    Q_ASSERT(m_cellModem);

    m_defaultSignal = new DefaultSignal(this);

    connect(m_cellModem, SIGNAL(stateChanged(CellModemManager::State, CellModemManager::State)),
            SLOT(_q_stateChanged(CellModemManager::State, CellModemManager::State)));
    connect(m_cellModem, SIGNAL(planeModeEnabledChanged(bool)),
            SLOT(_q_planeModeEnabledChanged(bool)));

    connect(m_defaultSignal, SIGNAL(signalStrengthChanged(int)),
            SIGNAL(signalStrengthChanged(int)));
}

QString PhoneStatusDBusExporter::phoneState() const
{
    return cellStatusToModemState(m_cellModem->state());
}

bool PhoneStatusDBusExporter::planeModeEnabled() const
{
    return m_cellModem->planeModeEnabled();
}

int PhoneStatusDBusExporter::signalStrength() const
{
    return m_defaultSignal->signalStrength();
}

void PhoneStatusDBusExporter::setPlaneModeEnabled(bool enable)
{
    m_cellModem->setPlaneModeEnabled(enable);
}

void PhoneStatusDBusExporter::triggerPinDialog()
{
    m_pinDialog->checkStatus();
}

void PhoneStatusDBusExporter::_q_stateChanged(CellModemManager::State state, CellModemManager::State)
{
    emit stateChanged(cellStatusToModemState(state));
}

void PhoneStatusDBusExporter::_q_planeModeEnabledChanged(bool enabled)
{
    emit planeModeChanged(enabled);
}


#endif


