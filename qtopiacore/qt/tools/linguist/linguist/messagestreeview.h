/****************************************************************************
**
** Copyright (C) 2006-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MESSAGESTREEVIEW_H
#define MESSAGESTREEVIEW_H

#include <QtGui/QTreeView>

#define TREEVIEW_ODD_COLOR QColor(235,245,255)

class MessagesTreeView : public QTreeView
{
    Q_OBJECT
public:
    MessagesTreeView(QWidget *parent = 0);
    virtual void setModel(QAbstractItemModel * model);
};


#endif // MESSAGESTREEVIEW_H

