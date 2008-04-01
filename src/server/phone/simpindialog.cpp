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

#include "simpindialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSoftMenuBar>
#include <QTimer>

#ifdef QT_ILLUME_LAUNCHER

SimPinDialog::SimPinDialog(QWidget* parent)
    : QDialog(parent)
    , m_mode(Enter_None)
    , m_pinLabel(0)
    , m_pukLabel(0)
    , m_pinEntry(0)
    , m_pukEntry(0)
    , m_rejected(false)
{
    hide();
    QtopiaApplication::prepareMainWidget(this);
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), tr("Enter"));

    m_cellModem = qtopiaTask<CellModemManager>();
    Q_ASSERT(m_cellModem);

    m_introduction = new QLabel(this);
    m_introduction->setWordWrap(true);
    m_introduction->setTextFormat(Qt::RichText);
    m_introduction->setText(tr("To use the phone please enter the needed PIN."));

    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_status->setTextFormat(Qt::RichText);

    connect(m_cellModem, SIGNAL(stateChanged(CellModemManager::State, CellModemManager::State)),
            SLOT(stateChanged(CellModemManager::State, CellModemManager::State)));

    // Be safe and call stateChanged at least once
    checkStatus();
}

void SimPinDialog::checkStatus()
{
    stateChanged(m_cellModem->state(), CellModemManager::NoCellModem);
}

void SimPinDialog::initializePinEntry()
{
    if (!m_pinLabel)
        m_pinLabel = new QLabel(this);

    if (!m_pinEntry) {
        m_pinEntry = new QLineEdit(this);
        m_pinEntry->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    }
}

void SimPinDialog::initializePukEntry()
{
    if (!m_pukLabel)
        m_pukLabel = new QLabel(this);

    if (!m_pukEntry) {
        m_pukEntry = new QLineEdit(this);
        m_pukEntry->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    }
}

/*
 * Setup the UI and ask for a SIM Pin
 */
void SimPinDialog::askForSimPin()
{
    m_mode = Enter_SIMPin;
    setEnabled(true);


    // Initialize the GUI Elements
    initializePinEntry();

    setWindowTitle(tr("SIM Pin required"));
    m_pinLabel->setText(tr("SIM Pin:"));
    m_pinEntry->setText(QString());

    // Arrange the GUI Elements. We just delete the old layout
    delete layout();
    QFormLayout* layout = new QFormLayout;
    layout->addRow(m_introduction);
    layout->addRow(m_pinLabel, m_pinEntry);
    layout->addRow(m_status);

    setLayout(layout);

    QtopiaApplication::showDialog(this);
}

/*
 * Setup the UI and ask for a SIM Puk
 */
void SimPinDialog::askForSimPuk()
{
    m_mode = Enter_SIMPuk;
    setEnabled(true);

    initializePinEntry();
    initializePukEntry();

    // Initialize
    setWindowTitle(tr("SIM Puk required"));
    m_pukLabel->setText(tr("SIM Puk:"));
    m_pukEntry->setText(QString());
    m_pinLabel->setText(tr("New SIM Pin:"));
    m_pinEntry->setText(QString());

    // Arrange the GUI Elements. We just delete the old layout
    delete layout();
    QFormLayout* layout = new QFormLayout;
    layout->addRow(m_introduction);
    layout->addRow(m_pukLabel, m_pukEntry);
    layout->addRow(m_pinLabel, m_pinEntry);
    layout->addRow(m_status);
    setLayout(layout);

    QtopiaApplication::showDialog(this);
}

/*
 * Keep the dialog around until we go from the PIN verification to another state.
 */
void SimPinDialog::accept()
{
    if (m_mode == Enter_SIMPin && m_pinEntry) {
        m_cellModem->setSimPin(m_pinEntry->text());
        setEnabled(false);
    } else if (m_mode == Enter_SIMPuk && m_pukEntry && m_pinEntry) {
        m_cellModem->setSimPuk(m_pukEntry->text(), m_pinEntry->text());
        setEnabled(false);
    } else {
        QDialog::accept();
    }
}

void SimPinDialog::reject()
{
    m_rejected = true;
    QDialog::reject();
}

void SimPinDialog::hideDialog()
{
    m_mode = Enter_None;
    if (m_pinEntry)
        m_pinEntry->setText(QString());
    if (m_pukEntry)
        m_pukEntry->setText(QString());

    QTimer::singleShot(1000, this, SLOT(hide()));
}

void SimPinDialog::stateChanged(CellModemManager::State newState, CellModemManager::State oldState)
{
    switch (newState) {
    case CellModemManager::NoCellModem:
        m_status->setText(tr("No Cell Modem"));
        hideDialog();
        break;

    case CellModemManager::Initializing:
        m_status->setText(tr("Initializing"));
        hideDialog();
        break;

    case CellModemManager::Initializing2:
        m_status->setText(tr("Initializing"));
        hideDialog();
        break;

    case CellModemManager::Ready:
        m_status->setText(tr("Ready"));
        hideDialog();
        break;

    case CellModemManager::SIMDead:
        m_status->setText(tr("SIM is dead"));
        hideDialog();
        break;

    case CellModemManager::SIMMissing:
        m_status->setText(tr("SIM is missing"));
        hideDialog();
        break;

    case CellModemManager::AerialOff:
        m_status->setText(tr("Airplane mode"));
        hideDialog();
        break;

    case CellModemManager::FailureReset:
        m_status->setText(tr("Reset failure"));
        hideDialog();
        break;

    case CellModemManager::UnrecoverableFailure:
        m_status->setText(tr("Unrecoverable failure"));
        hideDialog();
        break;

    case CellModemManager::WaitingSIMPin:
        m_status->setText(QString());
        if (oldState == CellModemManager::VerifyingSIMPin)
            m_status->setText(tr("<qt>The SIM Pin was incorrect please try it again.</qt>"));
        askForSimPin();
        break;

    case CellModemManager::WaitingSIMPuk:
        m_status->setText(QString());
        if (oldState == CellModemManager::VerifyingSIMPin)
            m_status->setText(tr("<qt>The SIM Pin was incorrect. The SIM is locked and you need to enter the SIM Puk now.</qt>"));
        else if (oldState == CellModemManager::VerifyingSIMPuk)
            m_status->setText(tr("<qt>The SIM Puk or the new SIMwas wrong or the new SIM Pin.</qt>"));
        askForSimPuk();
        break;

    case CellModemManager::VerifyingSIMPin:
        m_status->setText(tr("Verifying the SIM Pin"));
        break;
    case CellModemManager::VerifyingSIMPuk:
        m_status->setText(tr("Verifying the SIM Puk"));
        break;
    }
}

#endif

#include "simpindialog.moc"