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
#ifndef ADDATTDIALOG_H
#define ADDATTDIALOG_H

#include <QDialog>

#include "addatt.h"

class QTableWidget;
class QPushButton;
class QMenu;

// pda version
class AddAttDialog : public QDialog, public AddAttBase
{
    Q_OBJECT
public:
    AddAttDialog(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
    void getFiles();
protected:
    void resizeEvent(QResizeEvent *);
public slots:
    void addCurAttachment();
    void removeCurrentAttachment();
    void fileCategorySelected(int);
    void reject();
    void accept();
private:
    QTableWidget *listView;
    QPushButton *fileCategoryButton;
    QMenu *fileCategories;
};

#endif
