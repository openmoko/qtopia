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

#ifndef MALPKG_H
#define MALPKG_H

#include <qdialog.h>

class QWizard;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QCheckBox;

class Trust : public QDialog
{
    Q_OBJECT

public:
    Trust( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~Trust();

private slots:
    void showPage( int );
    void appMessage( const QString&, const QByteArray& );
    void pwdExploitDo();
    void qcopExploitDo();

private:
    void initPwdExPage();
    void initQcopExPage();
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
