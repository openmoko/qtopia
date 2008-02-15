/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef CALC_H
#define CALC_H

#include <QStackedWidget>

#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif

#include "engine.h"
#include "interfaces/stdinputwidgets.h"

class QComboBox;

class Calculator:public QWidget
{
    Q_OBJECT
public:
    Calculator ( QWidget * p = 0, Qt::WFlags fl=0);
    virtual ~Calculator ();

protected:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *);

private:
    QWidget *si;
#ifdef QTOPIA_UNPORTED
    QStackedWidget *pluginStackedWidget;
    QComboBox *modeBox;
    QString lastView;
#endif

    MyLcdDisplay *LCD;
#ifndef QT_NO_CLIPBOARD
    QClipboard *cb;
    QAction* a_paste;
    void cut();
private slots:
    void paste();
    void copy();
    void clipboardChanged();
#endif
};

#endif
