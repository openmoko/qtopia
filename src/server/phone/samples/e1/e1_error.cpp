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

#include "e1_error.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "e1_bar.h"
#include <QPixmap>
#include <QLabel>
#include <QSizePolicy>

void E1Error::error(const QString &str)
{
    E1Error * e = new E1Error(str);
    e->exec();
    delete e;
}

E1Error::E1Error(const QString &error)
: E1Dialog(0, Generic)
{
    QWidget *myWid = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(myWid);
    myWid->setLayout(layout);
    QLabel *pix = new QLabel(myWid);
    pix->setPixmap(QPixmap(":image/alert_warning"));
    layout->addWidget(pix);
    QLabel *desc = new QLabel(myWid);
    desc->setText(error);
    layout->addWidget(desc);

    setContentsWidget(myWid);

    E1Button *button = new E1Button;
    button->setFlag(E1Button::Expanding);
    button->setText("OK");
    bar()->addItem(button);
    QObject::connect(button, SIGNAL(clicked()),
                     this, SLOT(ok()));

}

void E1Error::ok()
{
    accept();
}
