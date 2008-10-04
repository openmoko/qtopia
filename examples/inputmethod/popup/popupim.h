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

#ifndef POPUPIM_H
#define POPUPIM_H

#include <qwidget.h>

class PopupIM : public QWidget
{
    Q_OBJECT
public:
    PopupIM(QWidget *parent, const char *name = 0, int flags = 0);
    ~PopupIM();

signals:
    void keyPress( ushort unicode, ushort keycode,
	    ushort modifiers, bool press, bool repeat );
};

#endif
