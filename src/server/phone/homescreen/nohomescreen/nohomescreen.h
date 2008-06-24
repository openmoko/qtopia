/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef NOHOMESCREEN_H
#define NOHOMESCREEN_H

#include "qabstracthomescreen.h"

class NoHomeScreen : public QAbstractHomeScreen {
    Q_OBJECT
public:
    NoHomeScreen(QWidget *parent = 0, Qt::WFlags fl = 0);
};

#endif
