/****************************************************************************
**
** Copyright (C) 2006-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef LOGGINGEDIT_H
#define LOGGINGEDIT_H

#include <qtopialog.h>
#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;

class LoggingEdit : public QDialog
{
    Q_OBJECT

public:
    LoggingEdit(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~LoggingEdit();

protected slots:
    void accept();
    void showWhatsThis();

private:
    QMap<QString,QTreeWidgetItem*> item;
    QTreeWidget* list;
};

#endif // LOGGINGEDIT_H

