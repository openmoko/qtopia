/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef MPICKER_H
#define MPICKER_H

#include <qtopia/inputmatch/picker.h>
#include <qstringlist.h>

class InputMatcherSet;

class ModePicker : public Picker
{
    Q_OBJECT
public:
    ModePicker(InputMatcherSet *, QWidget *parent=0);
    ~ModePicker();

signals:
    void modeSelected(const QString &, bool);

protected slots:
    void setModeFor(int, int);

protected:
    void showEvent(QShowEvent *ev);
    void drawCell(QPainter *p, int, int, bool);

private:
    void updateModeList();

    QStringList list;
    InputMatcherSet *set;
};

#endif

