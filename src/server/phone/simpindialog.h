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

#ifndef SIMPINDIALOG_H
#define SIMPINDIALOG_H

#ifdef QT_ILLUME_LAUNCHER

#include <QDialog>

#include "cellmodemmanager.h"


class QLabel;
class QLineEdit;


/**
 * A SimPinDialog for Qtopia
 */
class SimPinDialog : public QDialog {
    Q_OBJECT
public:
    enum EnterMode {
        Enter_None,
        Enter_SIMPin,
        Enter_SIMPuk
    };

    SimPinDialog(QWidget*);

    void checkStatus();

protected:
    void accept();
    void reject();

private Q_SLOTS:
    void stateChanged(CellModemManager::State, CellModemManager::State);

private:
    void askForSimPin();
    void askForSimPuk();
    void hideDialog();

private:
    EnterMode m_mode;
    CellModemManager* m_cellModem;
    QLabel* m_pinLabel;
    QLabel* m_pukLabel;
    QLineEdit* m_pinEntry;
    QLineEdit* m_pukEntry;
    QLabel* m_introduction;
    QLabel* m_status;
    bool m_rejected;
};
#endif

#endif
