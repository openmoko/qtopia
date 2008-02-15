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

#ifndef MALPKG_H
#define MALPKG_H

#include <qdialog.h>
#include <QWizard>
class QWizardPage;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QCheckBox;

class MalPkg : public QWizard
{
    Q_OBJECT

public:
    MalPkg( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~MalPkg();

private slots:
    void showPage( int );
    void appMessage( const QString&, const QByteArray& );
    void pwdExploitDo();
    void qcopExploitDo();

private:
    QWizardPage *initPwdExPage();
    QWizardPage *initQcopExPage();
    QString statusMsg( int );

    QWizard *wiz;
    QTextEdit *pwdExMsgs;
    QLineEdit *pwdExLine;
    QPushButton *pwdExButton;
    QCheckBox *pwdExCheck;
    QTextEdit *qcopExMsgs;
    QLineEdit *qcopExLine;
    QLineEdit *qcopExLineNum;
    QLineEdit *qcopExLineMsg;
    QPushButton *qcopExButton;
    QCheckBox *qcopExCheck;
};

#endif
