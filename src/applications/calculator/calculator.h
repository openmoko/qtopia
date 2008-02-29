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

#ifndef CALC_H
#define CALC_H

#include <QLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>

#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif

#include "engine.h"
#include "interfaces/stdinputwidgets.h"

class Calculator:public QWidget {
Q_OBJECT
public:
    Calculator ( QWidget * p = 0, Qt::WFlags fl=0);
    virtual ~Calculator ();

protected:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *);

private:
#ifdef QTOPIA_PHONE
   QWidget *si;
#else
   QStackedWidget *pluginStackedWidget;
   QComboBox *modeBox;
   QString lastView;
#endif

   MyLcdDisplay *LCD;
#ifndef QT_NO_CLIPBOARD
   QClipboard *cb;
   void paste();
   void cut();
private slots:
   void copy();
#endif
};

#endif
