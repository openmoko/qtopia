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
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include "ui_alarmdialogbase.h"

#include <qtopia/pim/qappointment.h>


class AlarmDialog : public QDialog, public Ui::AlarmDialogBase
{
    Q_OBJECT
public:
    AlarmDialog( QWidget *parent, Qt::WFlags f = 0 );

    void init();

    enum Button { Cancel = 0, Snooze, Details };
    Button exec(const QOccurrence &e);
    Button result();

    int snoozeLength();
    bool getSkipDialogs();

protected slots:
    void snoozeClicked();
    void detailsClicked();

private:
    Button mButton;
};


#endif
