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

#include <qtopia/qpeglobal.h>
#include <qtopia/qmath.h>
#include <qpushbutton.h>

#include <qtopia/calc/engine.h>
#include <qtopia/calc/doubledata.h>
#include <qtopia/calc/stdinputwidgets.h>

class FormSimple:public QWidget {
Q_OBJECT
public:
    FormSimple(QWidget *,const char*,WFlags);
    ~FormSimple(){};

public slots:
    void showEvent ( QShowEvent * );
    
private slots:
    void MPlusClicked();
    void MCClicked();
    void MRClicked();
    void CEClicked();
};

#endif
