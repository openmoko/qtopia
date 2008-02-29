/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "colorselector.h"
#include "scribblepad.h"
#include "scribbler.h"

#include <qcombobox.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qscrollview.h>
#include <qwidget.h>

//
//
//
Scribbler::Scribbler(QWidget *parent, const char *name, bool modal):
    QDialog(parent, name, modal)
{
    QVBoxLayout	*vbox = new QVBoxLayout(this, 4, 4, "vbox");
    QHBoxLayout	*hbox = new QHBoxLayout(0, 4, 4, "hbox");
    QComboBox	*pensize = new QComboBox(this);

    QHBoxLayout	*deschbox = new QHBoxLayout;
    deschbox->addWidget(new QLabel(tr("Description"), this));
    description = new QLineEdit(this);
    deschbox->addWidget(description);

    sv = new QScrollView(this);
    pad = new ScribblePad(this, "ScribblerPad");
    sv->addChild(pad);

    clearPad = new QPushButton(tr("Clear"), this);
    connect(clearPad, SIGNAL(clicked()), this, SLOT(clearScribblePad()));

    colorPicker = new ColorButton(this, "colorPicker");
    colorPicker->setDefaultColor(Qt::black);
    connect(colorPicker, SIGNAL(selected(const QColor&)),
	pad, SLOT(changeColor(const QColor&)));

    pensize->insertItem("1");
    pensize->insertItem("3");
    pensize->insertItem("5");
    pensize->insertItem("10");
    pensize->setCurrentItem(1);
    connect(pensize, SIGNAL(activated(const QString&)),
	pad, SLOT(changePenSize(const QString&)));

    hbox->addWidget(clearPad);
    hbox->addWidget(colorPicker);
    hbox->addWidget(pensize);

    vbox->addLayout(deschbox);
    vbox->addWidget(sv);
    vbox->addLayout(hbox);
}

//
//
//
Scribbler::~Scribbler(void)
{
    delete colorPicker;
    delete clearPad;
    delete pad;
    delete sv;
    delete description;
}

//
//
//
void
Scribbler::clearScribblePad(void)
{
    pad->clear();
    sv->setContentsPos(0, 0);
}

//
//
//
QString
Scribbler::desc(void) const
{
    return description->text();
}

//
//
//
void
Scribbler::setDesc(const QString &s)
{
    description->setText(s);
}

//
//
//
QPixmap
Scribbler::picture(void) const
{
    return pad->picture();
}

//
//
//
void
Scribbler::setPicture(const QPixmap &p)
{
    pad->setPicture(p);
}
