/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef PHONENETWORKS_H
#define PHONENETWORKS_H

#include <QDialog>
#include <QListWidget>

class PhoneNetworks : public QDialog
{
    Q_OBJECT

public:
    PhoneNetworks( QWidget* parent = 0, Qt::WFlags fl = 0 );

private slots:
    void serviceSelected( QListWidgetItem * );

private:
    void init();

private:
    QListWidget *m_list;
};


#endif // PHONENETWORKS_H

