/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef CONTACTSOURCE_H
#define CONTACTSOURCE_H

#include <QDialog>

class QContactModel;
class QPimSourceModel;
class QListView;
class ContactSourceDialog : public QDialog
{
    Q_OBJECT
public:
    ContactSourceDialog(QWidget *parent = 0);
    ~ContactSourceDialog();

    void setContactModel(QContactModel *);

private slots:
    void importActiveSim();
    void exportActiveSim();

private:
    void accept();

    QContactModel *contactModel;
    QListView *view;
    QPimSourceModel *model;
};
#endif//CONTACTSOURCE_H
