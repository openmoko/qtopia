/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef SIMPLEIMPL_H
#define SIMPLEIMPL_H
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

#endif //SIMPLEIMPL_H
