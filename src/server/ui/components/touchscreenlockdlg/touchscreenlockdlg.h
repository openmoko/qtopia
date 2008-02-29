/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef TOUCHSCREENLOCKDLG_H
#define TOUCHSCREENLOCKDLG_H

#include <QDialog>

class QGraphicsScene;

class TouchScreenLockDialog : public QDialog
{

public:
    TouchScreenLockDialog(QWidget *parent = 0, Qt::WFlags fl = 0);

private:
    QPixmap generatePixmap(const QString &filename, int width, int height) const;

private:
    QGraphicsScene *scene;
};

#endif
