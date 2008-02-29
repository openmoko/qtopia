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

#ifndef _QABSTRACTSECONDARYDISPLAY_H_
#define _QABSTRACTSECONDARYDISPLAY_H_

#include <QWidget>

class QAbstractSecondaryDisplay : public QWidget
{
Q_OBJECT
public:
    QAbstractSecondaryDisplay(QWidget *parent = 0, Qt::WFlags f = 0)
    : QWidget(parent, f) {}

    virtual void applyBackgroundImage() = 0;

    // Persist the new background, after applying it.
    virtual void updateBackground() = 0;
};

#endif // _QABSTRACTSECONDARYDISPLAY_H_

