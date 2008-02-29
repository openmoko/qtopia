/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "flagbox_p.h"

#include <QtGui/QAbstractItemView>
#include <QtGui/QItemDelegate>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

FlagBox::FlagBox(QWidget *parent)
    : QComboBox(parent)
{
    m_model = new FlagBoxModel(this);
    setModel(m_model);

    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.editable = isEditable();
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this))
        setItemDelegate(new QItemDelegate(this));

    connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
}

FlagBox::~FlagBox()
{
}

void FlagBox::slotActivated(int index)
{
    QVariant value = itemData(index, Qt::CheckStateRole);
    Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
    setItemData(index, (state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
}
