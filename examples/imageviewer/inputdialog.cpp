/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "inputdialog.h"

#include <QVBoxLayout>
#include <QKeyEvent>


InputDialog::InputDialog(QWidget *parent) 
: QDialog(parent) 
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    _edit = new QLineEdit(this);
    layout->addWidget(_edit);
    setLayout(layout);
    setFocusProxy(_edit);
}


void InputDialog::setText(const QString &text) 
{
    _edit->setText(text);
}

QString InputDialog::text() 
{
    return _edit->text();
}

void InputDialog::keyPressEvent(QKeyEvent *event) 
{
    if (event->key() == Qt::Key_Select) {
        accept();
    } else {
        QDialog::keyPressEvent(event);
    }
}
