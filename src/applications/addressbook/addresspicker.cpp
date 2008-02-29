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

#include "addresspicker.h"

#include <qlayout.h>

/*!
  \a tab is reparented for use in the picker. Take it back out if you want
  to regain ownership.
*/
AddressPicker::AddressPicker(AbTable* tab, QWidget* parent, const char* name, bool modal) :
    QDialog(parent,name,modal)
{
    QVBoxLayout* vb = new QVBoxLayout(this);
    tab->reparent(this,QPoint(0,0));
    table = tab;
    vb->addWidget(table);
}

void AddressPicker::setChoiceNames(const QStringList& list)
{
    table->setChoiceNames(list);
}

void AddressPicker::setSelection(int index, const QStringList& list)
{
    table->setChoiceSelection(index,list);
}

QStringList AddressPicker::selection(int index) const
{
    return table->choiceSelection(index);
}
