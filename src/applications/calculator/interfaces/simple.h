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

#ifndef SIMPLE_H
#define SIMPLE_H

#include <qtopiaglobal.h>
#include <qtopianamespace.h>
#include <QPushButton>

#include "../engine.h"
#include "../doubledata.h"
#include "stdinputwidgets.h"

class FormSimple:public DecimalInputWidget {
Q_OBJECT
public:
    FormSimple(QWidget * parent = 0);
    virtual ~FormSimple(){};

    QString interfaceName() { return QString(tr("Simple")); };

public slots:
    void showEvent ( QShowEvent * );
    void init( int fromRow, int fromCol);

private slots:
    void MPlusClicked();
    void MCClicked();
    void MRClicked();
    void CEClicked();
};

#endif
