/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef ACCOUNTEDITOR_H
#define ACCOUNTEDITOR_H

#include <QWidget>

class QListWidget;
class QProgressBar;
class QListWidget;
class QAction;
class QTimer;
class QAppointmentModel;
class QListWidgetItem;

class AccountEditor : public QWidget
{
    Q_OBJECT
public:
    AccountEditor(QWidget *parent = 0);
    ~AccountEditor();

    void setModel(QAppointmentModel *model);

    static bool editableAccounts(const QAppointmentModel *);

private slots:
    void addAccount();
    void removeCurrentAccount();
    void editCurrentAccount();

    void syncAllAccounts();
    void syncCurrentAccount();

    void updateActions();

    void updateProgress();

    void populate();

    void hideProgressBar();

    void currentAccountChanged(QListWidgetItem *);

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    QAppointmentModel *mModel;

    QListWidget *mChoices;
    QProgressBar *mProgress; // if a sync is activated, activate this.  more than one?  or is a busy indicator more appropriate?

    QAction *actionAdd;
    QAction *actionEdit;
    QAction *actionRemove;
    QAction *actionSync;
    QAction *actionSyncAll;

    QTimer *progressHideTimer;
};

#endif // ACCOUNTEDITOR_H
