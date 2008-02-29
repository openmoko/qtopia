/****************************************************************************
**
** Copyright (C) 2006-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef STARTUPFLAGS_H
#define STARTUPFLAGS_H

#include <QMap>
#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;

class StartupFlags : public QDialog
{
    Q_OBJECT

public:
    StartupFlags(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~StartupFlags();

protected slots:
    void accept();
    void showWhatsThis();
    void flagChanged(QTreeWidgetItem *item, int column);

private:
    QMap<QString,QTreeWidgetItem*> contexts;
    QMap<QString,QTreeWidgetItem*> item;
    QTreeWidget* list;

    void loadSettings();
    QVariant currentValue(QString group);
};

#endif // STARTUPFLAGS_H

