/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef EMAILDIALOG_H
#define EMAILDIALOG_H

#include "emaildlg.h"
#include <qstringlist.h>

class EmailDialog : public EmailDialogBase
{
    Q_OBJECT

public:
    EmailDialog( QWidget *parent, const char *name = 0, bool modal = FALSE, WFlags fl = 0);
    ~EmailDialog();

    void setEmails(const QString &def, const QStringList &em);

    QString defaultEmail() const;
    QStringList emails() const;

protected slots:
    void editCurrent( const QString &);
    void addEmail();
    void removeCurrent();
    void moveCurrentUp();
    void moveCurrentDown();
};

#endif // EMAILDIALOG_H
