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
QString PhoneStatusDBusExporter::cellModemStateToString(CellModemManager::State state)
{
    QString str = CellModemManager::stateToString(state);
    if (str == QLatin1String("Initializing2"))
        return QLatin1String("Initializing");

    return str;
}

QString PhoneStatusDBusExporter::registrationStateToString(QTelephony::RegistrationState state)
{
    switch(state) {
    case QTelephony::RegistrationNone:
        return QLatin1String("RegistrationNone");
    case QTelephony::RegistrationHome:
        return QLatin1String("RegistrationHome");
    case QTelephony::RegistrationSearching:
        return QLatin1String("RegistrationSearching");
    case QTelephony::RegistrationDenied:
        return QLatin1String("RegistrationDenied");
    case QTelephony::RegistrationUnknown:
        return QLatin1String("RegistrationUnknown");
    case QTelephony::RegistrationRoaming:
        return QLatin1String("RegistrationRoaming");
    }

    return QString();
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
    connect(m_cellModem, SIGNAL(registrationStateChanged(QTelephony::RegistrationState)),
            SLOT(_q_registrationStateChanged(QTelephony::RegistrationState)));
    connect(m_cellModem, SIGNAL(networkOperatorChanged(const QString&)),
            SIGNAL(networkOperatorChanged(QString)));

    connect(m_defaultSignal, SIGNAL(signalStrengthChanged(int)),
            SIGNAL(signalStrengthChanged(int)));
}

QString PhoneStatusDBusExporter::phoneState() const
{
    return cellModemStateToString(m_cellModem->state());
}

bool PhoneStatusDBusExporter::planeModeEnabled() const
{
    return m_cellModem->planeModeEnabled();
}

int PhoneStatusDBusExporter::signalStrength() const
{
    return m_defaultSignal->signalStrength();
}

QString PhoneStatusDBusExporter::networkOperator() const
{
    return m_cellModem->networkOperator();
}

QString PhoneStatusDBusExporter::registrationState() const
{
    return registrationStateToString(m_cellModem->registrationState());
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
    emit stateChanged(cellModemStateToString(state));
}

void PhoneStatusDBusExporter::_q_planeModeEnabledChanged(bool enabled)
{
    emit planeModeChanged(enabled);
}

void PhoneStatusDBusExporter::_q_registrationStateChanged(QTelephony::RegistrationState state)
{
    emit registrationStateChanged(registrationStateToString(state));
}


#endif


