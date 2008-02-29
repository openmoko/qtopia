/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "itemdialog.h"
#include "item.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>

QList<ItemDialog *> ItemDialog::openDialogs;

ItemDialog::ItemDialog(QWidget *parent, const Item *item)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
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

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btnBox, SIGNAL(rejected()), this, SLOT(close()));

    layout->addWidget(lblPix, 0, 0, 2, 2);
    layout->addWidget(lblName, 0, 2, 1, 1);
    layout->addWidget(lblDesc, 1, 2, 1, 1);
    layout->addWidget(btnBox, 2, 3, 1, 1);
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
