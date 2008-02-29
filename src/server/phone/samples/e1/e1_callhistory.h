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

#include "e1_dialog.h"

#include <QListWidget>

class E1CallHistory : public E1Dialog // creates an E1Dialog and sets a selector in it
{
    Q_OBJECT
public:
    enum Type
    {
        Dialed,
        Answered,
        Missed
    };
    E1CallHistory( QWidget* parent );
private slots:
    void typeSelected(const E1CallHistory::Type& t);
};

class E1CallHistorySelector : public QListWidget // the selector
{
    Q_OBJECT
public:
    E1CallHistorySelector( QWidget* parent );

private slots:
    void itemSelected(QListWidgetItem*);

signals:
    void selected(const E1CallHistory::Type&);
};

class E1CallHistoryList : public QListWidget
{
    Q_OBJECT
public:
    E1CallHistoryList( QWidget* parent, const E1CallHistory::Type& t );

signals:
    void closeMe();

private slots:
    void clicked();

private:
    QStringList numbers;
};
