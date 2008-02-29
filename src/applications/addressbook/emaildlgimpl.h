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

#ifndef EMAILDIALOG_H
#define EMAILDIALOG_H

#include "ui_emaildlg.h"
#include <qstringlist.h>
#include <QDialog>

class EmailDialog : public QDialog, Ui::EmailDialogBase
{
    Q_OBJECT

public:
    EmailDialog( QWidget *parent );
    ~EmailDialog();

    void setEmails(const QString &def, const QStringList &em);

    QString defaultEmail() const;
    QStringList emails() const;

protected slots:
    void editCurrent( const QString & );
    void addEmail();
    void addEmail( const QString &address );
    void removeCurrent();
    void moveCurrentUp();
    void moveCurrentDown();

private:
    void updateButtons(void);
};

#endif // EMAILDIALOG_H
