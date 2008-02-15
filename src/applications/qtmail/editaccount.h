/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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



#ifndef EDITACCOUNT_H
#define EDITACCOUNT_H

#include "ui_editaccountbasephone.h"

#include <qlistwidget.h>
#include <qtextedit.h>
#include <qtimer.h>

class QMailAccount;
class QTabWidget;

class EditAccount : public QDialog, public Ui::EditAccountBase
{
    Q_OBJECT

public:
    EditAccount( QWidget* parent = 0, const char* name = 0,Qt::WFlags fl = 0 );
    virtual ~EditAccount(){};
    void setAccount(QMailAccount *in, bool newOne = true);
    bool eventFilter( QObject *, QEvent * );
//  bool event(QEvent* e);

protected slots:
    void accept();
    void deleteAccount();
    void emailModified();
    void typeChanged(int);
    void sigPressed();
//  void configureFolders();
    void authChanged(int index);
    void createTabbedView();
    void currentTabChanged(int index);

private:
    QMailAccount *account;
    bool emailTyped;
    QString sig;
    QTabWidget* tabWidget;
    QLineEdit* accountNameInput;
};

class SigEntry : public QDialog
{
    Q_OBJECT
public:
    SigEntry(QWidget *parent, const char* name, Qt::WFlags fl = 0 );
    void setEntry(QString sig) { input->insertPlainText(sig); };
    QString entry() { return input->toPlainText(); };

private:
    QTextEdit *input;
};
/*
class MailboxView : public QListWidget
{
    Q_OBJECT
  public:
    MailboxView(QWidget *parent, const char *name);

  public slots:
    void showMessageOptions();

  protected:
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );
  protected slots:
    void cancelMenuTimer();

    void changeMessageSettings(QAction* e);

  private:
    QTimer menuTimer;
    QMenu *pop;
};

class MailboxSelector : public QDialog
{
    Q_OBJECT
  public:
    MailboxSelector(QWidget *parent, const char *name, QMailAccount *account );

  protected slots:
    void accept();

    void selectAll();
    void clearAll();
    void itemSelected();

  private:
    QMailAccount *_account;
    MailboxView *view;
};*/

#endif
