/****************************************************************************
**
** Copyright (C) 2005-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtCore/QList>

#include "itemdialog.h"
#include "item.h"

QList<ItemDialog *> ItemDialog::openDialogs;

ItemDialog::ItemDialog(QWidget *parent, const Item *item)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *lblPix = new QLabel();
    const BusinessCard *bcard = static_cast<const BusinessCard *>(item);
    if (bcard) {
        QPixmap pm(bcard->bigPicture());
        if (pm.isNull())
            pm = QPixmap(QLatin1String(":/qthack/images/qt.png"));
        lblPix->setPixmap(pm);
    } else {
        lblPix->setPixmap(item->pixmapName());
    }

    QLabel *lblName = new QLabel(item->name());
    QLabel *lblDesc = new QLabel(item->description());
    lblDesc->setWordWrap(true);
    QFont descFont = lblDesc->font();
    descFont.setItalic(true);
    descFont.setPointSize(descFont.pointSize() - 2);
    lblDesc->setFont(descFont);
    QPushButton *btn = new QPushButton(tr("Close"));
    btn->setDefault(true);
    connect(btn, SIGNAL(clicked()), this, SLOT(close()));

    layout->addWidget(lblPix, 0, 0, 2, 2);
    layout->addWidget(lblName, 0, 2, 1, 1);
    layout->addWidget(lblDesc, 1, 2, 1, 1);
    layout->addWidget(btn, 2, 3, 1, 1);
    openDialogs.append(this);
}

void ItemDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    openDialogs.removeAll(this);
    if (openDialogs.isEmpty()) {
        parentWidget()->activateWindow();
    } else {
        ItemDialog *itemDialog = openDialogs.last();
        itemDialog->show();
        itemDialog->raise();
        itemDialog->activateWindow();
    }
}
