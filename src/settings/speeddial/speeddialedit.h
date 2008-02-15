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
#ifndef SPEEDDIALEDIT_H
#define SPEEDDIALEDIT_H

#include <qsettings.h>

#include <qspeeddial.h>
#include <QDialog>


class QSpeedDialEdit : public QDialog
{
    Q_OBJECT

public:
    QSpeedDialEdit(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~QSpeedDialEdit();

protected slots:
    void reject();

private:
    QSpeedDialList* list;
    QList<QString>  origEmpty;
    QMap<QString, QtopiaServiceDescription> origSet;
};

#endif // SPEEDDIALEDIT_H

