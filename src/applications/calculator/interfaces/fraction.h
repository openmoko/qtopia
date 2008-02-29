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
#ifdef ENABLE_FRACTION

#ifndef FRACTIONIMPL_H
#define FRACTIONIMPL_H

#include <qtopianamespace.h>
#include <QPushButton>

#include "../engine.h"
#include "../doubledata.h"
#include "stdinputwidgets.h"

class FormFraction:public FractionInputWidget {
Q_OBJECT
public:
    FormFraction(QWidget * parent = 0, Qt::WFlags = 0);
    ~FormFraction(){};

    QString interfaceName() { return QString(tr("Fraction")); };

public slots:
    void showEvent ( QShowEvent * );

protected:
    void init(int fromRow, int fromCol);

private slots:
    void MPlusClicked();
    void MCClicked();
    void MRClicked();
    void CEClicked();
};

#endif
#endif //ENABLE_FRACTION
