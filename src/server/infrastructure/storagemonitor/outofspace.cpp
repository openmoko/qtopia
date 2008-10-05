/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "outofspace.h"

#include <QMessageBox>
#include <QtopiaService>

OutOfSpace::OutOfSpace(const QString &msg, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setupUi(this);

    message->setText(msg);
    icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Critical));

    if (QtopiaService::apps("CleanupWizard").isEmpty()) {
        // no cleanup wizard
        cleanup->setEnabled(false);
        remind->setChecked(true);
    }
}

void OutOfSpace::accept()
{
    if (cleanup->isChecked()) {
        done(CleanupNow);
    } else {
        switch (delay->currentIndex()) {
        case 0:
            done(HourDelay);
            break;
        case 1:
            done(DayDelay);
            break;
        case 2:
            done(WeekDelay);
            break;
        case 3:
            done(Never);
            break;
        }
    }
}

