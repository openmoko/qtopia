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

#ifndef NOEDITSPINBOX_H
#define NOEDITSPINBOX_H

#include <QtGui>

class NoEditSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit  NoEditSpinBox(QWidget *parent = 0);
    ~NoEditSpinBox(){}

    void focusInEvent(QFocusEvent*);
};   

#endif
