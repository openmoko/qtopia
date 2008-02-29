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

#include <qcalllist.h>
#include <QtopiaServiceRequest>
#include "e1_callhistory.h"

// E1CallHistory definition
E1CallHistory::E1CallHistory( QWidget* parent )
    : E1Dialog( parent, E1Dialog::Return )
{
   E1CallHistorySelector* selector = new E1CallHistorySelector( this );
   setContentsWidget( selector );
   connect( selector, SIGNAL(selected(const E1CallHistory::Type&)), this, SLOT(typeSelected(const E1CallHistory::Type&)) );
}

void E1CallHistory::typeSelected(const E1CallHistory::Type& t)
{
    E1Dialog* dialog = new E1Dialog( parentWidget(), E1Dialog::Return ); // peer not chlid
    E1CallHistoryList* list = new E1CallHistoryList( dialog, t );
    dialog->setContentsWidget( list );
    QObject::connect(list, SIGNAL(closeMe()), dialog, SLOT(reject()));
    QObject::connect(list, SIGNAL(closeMe()), this, SLOT(reject()));
    dialog->exec();
}

// E1CallHistorySelector Definition

E1CallHistorySelector::E1CallHistorySelector( QWidget* parent )
    : QListWidget( parent  )
{
    // Get the amount of call history items for each type of item

    QCallList historyListAccess;
    QList<QCallListItem> historyList = historyListAccess.allCalls();
    int numDialed = 0, numAnswered = 0, numMissed = 0;
    for( QList<QCallListItem>::ConstIterator it = historyList.begin() ; it != historyList.end() ; ++it ) {
        switch( (*it).type() ) {
            case QCallListItem::Dialed:
                ++numDialed;
                break;
            case QCallListItem::Received:
                ++numAnswered;
                break;
            case QCallListItem::Missed:
                ++numMissed;
                break;
        }
    }
    insertItem( (int)E1CallHistory::Dialed, QString("Dialed Calls(") + QString::number(numDialed) + ")" );
    insertItem( (int)E1CallHistory::Answered, QString("Answered Calls(") + QString::number(numAnswered) + ")" );
    insertItem( (int)E1CallHistory::Missed, QString("Missed Calls(") + QString::number(numMissed) + ")" );

    connect( this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)) );
    connect( this, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)) );
}

void E1CallHistorySelector::itemSelected( QListWidgetItem* i )
{
    int cr = currentRow();
    Q_ASSERT(item(cr) == i);
    Q_ASSERT(cr == E1CallHistory::Dialed || cr == E1CallHistory::Answered || cr == E1CallHistory::Missed);
    emit selected( (E1CallHistory::Type)currentRow() );
}

// E1CallHistoryList Definition
E1CallHistoryList::E1CallHistoryList( QWidget* parent, const E1CallHistory::Type& t )
    : QListWidget( parent )
{

    // header item
    switch( t ) {
        case E1CallHistory::Dialed:
            insertItem( 0, "Dialed Calls" );
            break;
        case E1CallHistory::Answered:
            insertItem( 0, "Answered Calls" );
            break;
        case E1CallHistory::Missed:
            insertItem( 0, "Missed Calls" );
            break;
    }

    // populate
    QCallList historyListAccess;
    QList<QCallListItem> historyList = historyListAccess.allCalls();
    for( QList<QCallListItem>::ConstIterator it = historyList.begin() ; it != historyList.end() ; ++it ) {
        if( (E1CallHistory::Type)(*it).type() == t ) {
            // put into list
            QTime startTime = (*it).start().time();
            QString label = QString("%1:%2  ").arg(startTime.hour(), 2, 10, QLatin1Char('0')).arg(startTime.minute(), 2, 10, QLatin1Char('0'));

            if( (*it).number().isEmpty() ) {
                label += "Unknown";
                numbers.append(QString());
            } else {
                // TODO : Get contact name if it exists
                label += (*it).number();
                numbers.append(it->number());
            }

            addItem( label );
        }
    }

    setCurrentRow( 0 );
    QObject::connect(this, SIGNAL(itemActivated(QListWidgetItem*)),
                     this, SLOT(clicked()));
    QObject::connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
                     this, SLOT(clicked()));
}

void E1CallHistoryList::clicked()
{
    if(currentRow() && !numbers.at(currentRow()).isEmpty()) {
        QtopiaServiceRequest req("Dialer", "showDialer(QString)");
        req << numbers.at(currentRow());
        req.send();
        emit closeMe();
    }
}
