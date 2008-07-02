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

#ifndef WPICKER_H
#define WPICKER_H

#include "picker.h"
#include <qstring.h>
#include <qstringlist.h>

class QTOPIA_INPUTMATCH_EXPORT WordPicker : public Picker
{
    Q_OBJECT
public:
    WordPicker(QWidget *parent=0);
    ~WordPicker();

    void setChoices(const QStringList &);

signals:
    void wordChosen(const QString &);

protected:
    void drawCell(QPainter *p, int, int, bool);

private slots:
    void sendWordChoice(int, int);

private:
    QStringList choices;
};

#endif

