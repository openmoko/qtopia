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

#ifndef CALC_H
#define CALC_H

#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qwidgetstack.h>

#include <qtopia/calc/plugininterface.h>

#if QT_VERSION >= 300
#include <private/qcomlibrary_p.h>
#define QLibrary QComLibrary
#else
#include <qpe/qlibrary.h>
#endif

struct Plugin {
    CalculatorInterface *interface;
    QLibrary *library;
    QWidget *widget;
};

class Calculator:public QWidget {
Q_OBJECT
public:
  Calculator (
#ifdef QTEST
	  int,char**,
#endif
	  QWidget * p = 0, const char *n = 0);
   ~Calculator ();

private:
   QVBoxLayout *calculatorLayout;
   QValueList<Plugin> *pluginList;
   QWidgetStack *pluginWidgetStack;
   QComboBox *modeBox;
   QLineEdit *LCD;
};

#endif
