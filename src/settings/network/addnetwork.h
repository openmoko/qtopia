/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef ADD_NETWORK_UI_H
#define ADD_NETWORK_UI_H

#include <QDialog>
class QListWidget;
class QLabel;

class AddNetworkUI : public QDialog
{
    Q_OBJECT
public:
    AddNetworkUI(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~AddNetworkUI();

    QString selectedConfig() const;

private:
    void init();

private slots:
    void updateHint();
    void itemSelected();

private:
    QListWidget* list;
    QLabel *hint;
};


#endif
