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



#include "searchview.h"
#include "accountlist.h"
#include "common.h"


#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <QMenu>
#include <QDesktopWidget>



using namespace QtMail;

SearchView::SearchView(bool query,  QWidget* parent, Qt::WFlags fl )
    : QDialog(parent, fl)
{
    setupUi( this );

    delete layout();
    QGridLayout *g = new QGridLayout(this);
    sv = new QScrollArea(this);
    sv->setFocusPolicy(Qt::NoFocus);
    sv->setWidgetResizable( true );
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidget(searchFrame);
    sv->setFocusPolicy(Qt::NoFocus);
    g->addWidget(sv, 0, 0);

#ifdef QTOPIA_PHONE
    int dw = QApplication::desktop()->availableGeometry().width();
    searchFrame->setMaximumWidth(dw - qApp->style()->pixelMetric(QStyle::PM_SliderLength) + 4 );
    sv->setMaximumWidth(dw);
#endif

    queryType = query;

    if ( queryType ) {
        setWindowTitle( tr("Query") );
        nameLabel->show();
        nameLine->show();
    } else {
        setWindowTitle( tr("Search") );
        nameLabel->hide();
        nameLine->hide();
    }

    init();
}

SearchView::~SearchView()
{
}

void SearchView::init()
{
    /*  Set up dateBookMonth popups */



    dateAfter = QDate::currentDate();
    dateAfterButton->setDate( dateAfter );
    connect( dateAfterButton, SIGNAL( dateChanged(const QDate&) ),
            this, SLOT( dateAfterChanged(const QDate&) ) );




    dateBefore = QDate::currentDate();
    dateBeforeButton->setDate( dateBefore );
    connect( dateBeforeButton, SIGNAL( dateChanged(const QDate&) ),
            this, SLOT( dateBeforeChanged(const QDate&) ) );

    /*  Fix tab order   */
    setTabOrder(mailbox, status);
    setTabOrder(status, fromLine);
    setTabOrder(fromLine, toLine);
    setTabOrder(toLine,subjectLine);
    setTabOrder(subjectLine, bodyLine);
    setTabOrder(bodyLine, dateAfterBox);
    setTabOrder(dateAfterBox, dateAfterButton);
    setTabOrder(dateAfterButton, dateBeforeBox);
    setTabOrder(dateBeforeBox, dateBeforeButton);



}

void SearchView::setQueryBox(QString box)
{
    box = mailboxTrName(box);
    for (int i = 0; i < mailbox->count(); i++) {
        if (mailbox->itemText(i).toLower() == box.toLower() ) {
            mailbox->setCurrentIndex(i);
            break;
        }
    }

    mailbox->setEnabled(false);
}

// this function is assumed call directly after the constructor
void SearchView::setSearch(Search *in)
{
    QString str = in->mailbox();
    if ( str == InboxString )
        mailbox->setCurrentIndex(0);
    else if ( str == OutboxString )
        mailbox->setCurrentIndex(1);
    else if ( str == DraftsString )
        mailbox->setCurrentIndex(2);
    else if ( str == SentString )
        mailbox->setCurrentIndex(3);
    else if ( str == TrashString )
        mailbox->setCurrentIndex(4);

    switch( in->status() ) {
        case Search::Read: {
        status->setCurrentIndex( 1 );
            break;
        }
        case Search::Unread: {
        status->setCurrentIndex( 2 );
            break;
        }
        case Search::Replied: {
        status->setCurrentIndex( 3 );
            break;
        }
    }

    fromLine->setText( in->getFrom() );
    toLine->setText( in->getTo() );
    subjectLine->setText( in->getSubject() );
    bodyLine->setText( in->getBody() );

    dateBefore = in->getBeforeDate();
    dateAfter = in->getAfterDate();

    if ( !dateAfter.isNull() ) {
        dateAfterBox->setChecked(true);
        dateAfterButton->setDate( dateAfter );

    } else {
        dateAfter = QDate::currentDate();
    }

    if ( !dateBefore.isNull() ) {
        dateBeforeBox->setChecked(true);
        dateBeforeButton->setDate( dateBefore );

    } else {
        dateBefore = QDate::currentDate();
    }

    nameLine->setText( in->name() );
}

// creates a new Search object, and returns.  Caller assumes
// ownership of search object
Search* SearchView::getSearch()
{
    Search *search = new Search();

    int i = mailbox->currentIndex();
    switch(i) {
        case 0: search->setMailbox(InboxString); break;
        case 1: search->setMailbox(OutboxString); break;
        case 2: search->setMailbox(DraftsString); break;
        case 3: search->setMailbox(SentString); break;
        case 4: search->setMailbox(TrashString); break;
    }

    search->setMailFrom( fromLine->text() );
    search->setMailTo( toLine->text() );
    search->setMailSubject( subjectLine->text() );
    search->setMailBody( bodyLine->text() );

    int statusNum = status->currentIndex();
    switch( statusNum ) {
        case 1: {
            search->setStatus( Search::Read );
            break;
        }
        case 2: {
            search->setStatus( Search::Unread );
            break;
        }
        case 3: {
            search->setStatus( Search::Replied );
            break;
        }
        default: search->setStatus( Search::Any );
    }

    if ( dateAfterBox->isChecked() ) {
        search->setAfterDate(dateAfter);
    }
    if ( dateBeforeBox->isChecked() ) {
        search->setBeforeDate(dateBefore);
    }

    if ( !nameLine->text().isEmpty() ) {
        search->setName( nameLine->text() );
    }

    return search;
}





void SearchView::dateAfterChanged(const QDate &ymd)
{
    dateAfter = ymd;
}

void SearchView::dateBeforeChanged(const QDate &ymd)
{
    dateBefore = ymd;
}
