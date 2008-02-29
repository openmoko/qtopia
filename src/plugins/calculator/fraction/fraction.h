/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef STANDARDIMPL_H
#define STANDARDIMPL_H

#include <qpe/qmath.h>
#include <qpushbutton.h>

#include <qtopia/calc/engine.h>
#include <qtopia/calc/doubledata.h>
#include <qtopia/calc/stdinputwidgets.h>

class FormFraction:public QWidget {
Q_OBJECT
public:
    FormFraction(QWidget *,const char*,WFlags);
    ~FormFraction(){};

public slots:
    void showEvent ( QShowEvent * );
    
private:
    Engine sys;
    Type1FractionInputWidget *siw;
    QPushButton *PBCE,*PBMR,*PBMC,*PBMPlus;
    QGridLayout* FractionLayout;

private slots:
    void MPlusClicked();
    void MCClicked();
    void MRClicked();
    void CEClicked();
    void CClicked();
};

#endif
