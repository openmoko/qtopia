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

#ifndef ITEMDIALOG_H
#define ITEMDIALOG_H

#include <QtGui/QDialog>

template <typename T> class QList;
class Item;

class ItemDialog : public QDialog
{
    Q_OBJECT
public:
    ItemDialog(QWidget *parent, const Item *item);

protected:
    void closeEvent(QCloseEvent *);

private:
    static QList<ItemDialog *> openDialogs;
};

#endif
