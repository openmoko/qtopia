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

#ifndef PHONEIMPL_H
#define PHONEIMPL_H

#include "stdinputwidgets.h"

#include <QLabel>
#include <QLayout>

#include <qtopiaglobal.h>
#include <qsoftmenubar.h>

#include "../engine.h"
#include "../doubledata.h"

class QTimer;
class QSignalMapper;

class FormPhone:public CalcUserInterface {
Q_OBJECT
public:
    FormPhone(QWidget *parent = 0);
    ~FormPhone(){};

    QString interfaceName() { return QString(tr("Phone")); };

public slots:
    void showEvent ( QShowEvent * );

    void plus();
    void minus();
    void times();
    void div();
    void eval();
    void dec();
    void nextInstruction();
    void clearAll();

    void changeResetButtonText(ResetState);

signals:
    void close();

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void timerEvent(QTimerEvent*);
private slots:
    void shortcutClicked(int keyIdent);
private:
    int tid_hold;
    bool waitForRelease;
    ResetState displayedState;

    QVBoxLayout* Type1BaseInputWidgetLayout;
    int lastInstruction;
    bool firstNumber;
    bool backpressed;
    bool lockEvaluation;

    QSignalMapper *signalMapper;
};

#endif //PHONEIMPL_H
