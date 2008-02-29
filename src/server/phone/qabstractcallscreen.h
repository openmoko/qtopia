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

#ifndef _QABSTRACTCALLSCREEN_H_
#define _QABSTRACTCALLSCREEN_H_

#include <QWidget>

class QAbstractCallScreen : public QWidget
{
Q_OBJECT
public:
    QAbstractCallScreen(QWidget *parent = 0, Qt::WFlags f = 0)
    : QWidget(parent, f) {}
};

#endif // _QABSTRACTCALLSCREEN_H_

